import tkinter as tk
from tkinter import ttk, messagebox
from collections import deque
import struct
import threading
import queue
import time
import serial
import serial.tools.list_ports
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure


class PIDGUI(ttk.Frame):
    def __init__(self, master: tk.Tk):
        super().__init__(master)
        self.master.title("PID styring av avstandssensor")
        self.master.geometry("900x560")
        self.master.minsize(820, 520)

        # Tema
        self._sett_tema()

        # Tilstandsvariabler (HELTALL)
        self.settpunkt_var = tk.IntVar(value=200)
        self.kp_var = tk.IntVar(value=1)
        self.ki_var = tk.IntVar(value=0)
        self.kd_var = tk.IntVar(value=0)
        self.port_var = tk.StringVar(value="")
        self._tilkoblet: bool = False
        self.serieport = None  # type: serial.Serial | None

        # Databuffer for plott (heltall for PV, tid kan være flyt for akse)
        self.max_punkt = 2000
        self.t_data = deque(maxlen=self.max_punkt)   # tid [s] relativ
        self.pv_data = deque(maxlen=self.max_punkt)  # avstand (heltall)

        # Callbacks injisert utenfra
        self._pid_callback = None
        self._koble_til_fn = None
        self._koble_fra_fn = None

        # Tråd/logging
        self._lesetraads_kø: queue.Queue[tuple[float, int]] = queue.Queue()
        self._lesetraads_stop = threading.Event()
        self._lesetraad: threading.Thread | None = None
        self._t0 = None  # starttid for x-akse

        # Bygg UI
        self._bygg_layout()

        # Fyll porter nå, ved dropdown, og periodisk
        self._oppdater_porter()
        self.port_cb.bind("<<ComboboxDropdown>>", lambda e: self._oppdater_porter())
        self.after(5000, self._periodisk_portscan)

        # Periodisk synk av GUI
        self.after(1000, self._periodisk_statussynk)
        self.after(50, self._tøm_kø_og_oppdater)

    # ---------- Tema ----------

    def _sett_tema(self) -> None:
        style = ttk.Style()
        try:
            style.theme_use("clam")
        except tk.TclError:
            pass

        bakgrunn = "#1f2227"
        panel = "#262a31"
        tekst = "#e8e8e8"

        self.master.configure(bg=bakgrunn)
        style.configure(".", background=panel, foreground=tekst, fieldbackground="#2f3540")
        style.configure("TFrame", background=panel)
        style.configure("TLabel", background=panel, foreground=tekst)
        style.configure("TButton", background="#2f3540", foreground=tekst, padding=6)
        style.map("TButton", background=[("active", "#3a4352")])

    def _ramme(self, parent, tittel: str | None = None):
        return ttk.LabelFrame(parent, text=tittel) if tittel else ttk.Frame(parent)

    # ---------- Layout ----------

    def _bygg_layout(self) -> None:
        # Venstre kontrollpanel
        venstre = self._ramme(self.master)
        venstre.pack(side=tk.LEFT, fill=tk.Y, padx=(12, 8), pady=12)

        # Høyre plott
        hoyre = self._ramme(self.master)
        hoyre.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 12), pady=12)

        # Tittel
        ttk.Label(venstre, text="Avstandssensor", font=("Segoe UI", 14, "bold")).pack(
            side=tk.TOP, anchor="w", padx=6, pady=(0, 8)
        )

        # Tilkoblingsboks
        tilk_boks = self._ramme(venstre)
        tilk_boks.pack(fill=tk.X, padx=0, pady=(0, 10))

        rad_port = ttk.Frame(tilk_boks)
        rad_port.pack(fill=tk.X, padx=8, pady=4)
        ttk.Label(rad_port, text="Port").pack(side=tk.LEFT)
        self.port_cb = ttk.Combobox(rad_port, textvariable=self.port_var, width=18)
        self.port_cb.pack(side=tk.LEFT, padx=(8, 8))
        ttk.Button(rad_port, text="Oppdater porter", command=self._oppdater_porter).pack(side=tk.LEFT)

        rad_knapp = ttk.Frame(tilk_boks)
        rad_knapp.pack(fill=tk.X, padx=8, pady=(6, 8))
        self.koble_knapp = ttk.Button(rad_knapp, text="Koble til", command=self._toggle_tilkobling)
        self.koble_knapp.pack(side=tk.LEFT)

        self.status_lbl = ttk.Label(venstre, text="Status: frakoblet")
        self.status_lbl.pack(fill=tk.X, padx=4, pady=(6, 10))

        # PID-innstillinger (kun heltall)
        ttk.Label(venstre, text="PID-innstillinger", font=("Segoe UI", 14, "bold")).pack(
            side=tk.TOP, anchor="w", padx=6, pady=(0, 8)
        )
        pidboks = self._ramme(venstre)
        pidboks.pack(fill=tk.X, padx=0, pady=(0, 10))
        self._rad_med_entry(pidboks, "Settpunkt", self.settpunkt_var)
        self._rad_med_entry(pidboks, "Kp", self.kp_var)
        self._rad_med_entry(pidboks, "Ki", self.ki_var)
        self._rad_med_entry(pidboks, "Kd", self.kd_var)

        radkn = ttk.Frame(pidboks)
        radkn.pack(fill=tk.X, padx=8, pady=(6, 8))
        ttk.Button(radkn, text="Bruk verdier", command=self._bruk_pid).pack(side=tk.LEFT)
        ttk.Button(radkn, text="Nullstill graf", command=self._nullstill_graf).pack(side=tk.LEFT, padx=8)

        # Info-linje
        self.info_lbl = ttk.Label(venstre, text="PV: —")
        self.info_lbl.pack(side=tk.TOP, fill=tk.X, padx=4, pady=(6, 0))

        # Matplotlib-figur
        figur = Figure(figsize=(5, 4), dpi=100)
        self.akse = figur.add_subplot(111)
        self.akse.set_title("Avstandsmåling")
        self.akse.set_xlabel("Tid [s]")
        self.akse.set_ylabel("PV (heltall)")
        self.akse.grid(True, alpha=0.25)
        self.linje_pv, = self.akse.plot([], [], label="PV")
        self.akse.legend()

        self.canvas = FigureCanvasTkAgg(figur, master=hoyre)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def _rad_med_entry(self, parent, etikett: str, var: tk.Variable) -> None:
        rad = ttk.Frame(parent)
        rad.pack(fill=tk.X, padx=8, pady=4)
        ttk.Label(rad, text=etikett).pack(side=tk.LEFT)
        ent = ttk.Entry(rad, textvariable=var, width=12)
        ent.pack(side=tk.RIGHT)

    # ---------- Offentlige metoder ----------

    def oppdater_data(self, tid_s: float, pv: int) -> None:
        """Legg til nye data (heltall PV) og oppdater plott."""
        self.t_data.append(float(tid_s))
        self.pv_data.append(int(pv))
        self.info_lbl.config(text=f"PV: {int(pv)}")
        self._oppdater_plott()

    def registrer_pid_callback(self, funksjon) -> None:
        """Registrer callback for PID-verdier."""
        self._pid_callback = funksjon

    def registrer_tilkoblingshandler(self, koble_til_fn, koble_fra_fn) -> None:
        """Registrer callbacks for tilkobling/frakobling."""
        self._koble_til_fn = koble_til_fn
        self._koble_fra_fn = koble_fra_fn

    def sett_pid(self, kp: int, ki: int, kd: int, settpunkt: int | None = None) -> None:
        """Oppdater PID-verdier i GUI (heltall)."""
        if settpunkt is not None:
            self.settpunkt_var.set(int(settpunkt))
        self.kp_var.set(int(kp))
        self.ki_var.set(int(ki))
        self.kd_var.set(int(kd))

    def set_port_list(self, porter: list[str]) -> None:
        """Fyll combobox med gitt liste over porter."""
        self.port_cb["values"] = list(porter) if porter else []

    # ---------- Hjelpe-metoder ----------

    def _bruk_pid(self) -> None:
        """Hent heltallsverdier og kall registrert PID-callback."""
        try:
            settpunkt = int(self.settpunkt_var.get())
            kp = int(self.kp_var.get())
            ki = int(self.ki_var.get())
            kd = int(self.kd_var.get())
        except Exception:
            messagebox.showwarning("Ugyldig verdi", "Settpunkt/Kp/Ki/Kd må være heltall.")
            return

        if self._pid_callback is None:
            messagebox.showinfo("Ingen handler", "Ingen PID-handler registrert.")
            return

        try:
            self._pid_callback(settpunkt, kp, ki, kd)
        except Exception as e:
            messagebox.showerror("Callback-feil", str(e))

    def _nullstill_graf(self) -> None:
        """Tøm data og oppdater plott."""
        self.t_data.clear()
        self.pv_data.clear()
        self.info_lbl.config(text="PV: —")
        self._oppdater_plott()

    def _oppdater_plott(self) -> None:
        """Oppdater plott med nåværende data (PV som heltall)."""
        self.linje_pv.set_data(self.t_data, self.pv_data)
        if len(self.t_data) >= 2:
            xmin, xmax = self.t_data[0], self.t_data[-1]
            self.akse.set_xlim(xmin, xmax)
            if self.pv_data:
                y_min = min(self.pv_data)
                y_max = max(self.pv_data)
            else:
                y_min, y_max = -1, 1
            margin = max(1, int(0.1 * (y_max - y_min + 1)))
            self.akse.set_ylim(y_min - margin, y_max + margin)
        else:
            self.akse.set_xlim(0, 1)
            self.akse.set_ylim(-1, 1)
        self.canvas.draw_idle()

    def _oppdater_porter(self) -> None:
        """Oppdater liste over tilgjengelige serieporter."""
        try:
            porter = [p.device for p in serial.tools.list_ports.comports()]
        except Exception as e:
            messagebox.showerror("Portoppdatering feilet", str(e))
            porter = []
        self.port_cb["values"] = porter
        # Behold valgt port om mulig
        nåv = self.port_var.get().strip()
        if nåv and nåv in porter:
            pass
        elif porter:
            self.port_var.set(porter[0])
        else:
            self.port_var.set("")

    def _periodisk_portscan(self) -> None:
        """Periodisk oppdatering av portliste."""
        self._oppdater_porter()
        self.after(5000, self._periodisk_portscan)

    def _toggle_tilkobling(self) -> None:
        """Koble til eller fra serieport."""
        if not self._tilkoblet:
            port = self.port_var.get().strip()
            if not port:
                messagebox.showwarning("Port mangler", "Skriv/velg en port før tilkobling.")
                return
            if self._koble_til_fn is None:
                messagebox.showwarning("Mangler handler", "Ingen tilkoblingshandler registrert.")
                return
            try:
                resultat = self._koble_til_fn(port)
                if resultat is True and getattr(self, "serieport", None) and self.serieport.is_open:
                    self._tilkoblet = True
                    self._start_logging()
                else:
                    raise RuntimeError(f"Klarte ikke åpne {port}.")
            except Exception as e:
                messagebox.showerror("Tilkoblingsfeil", str(e))
                self._tilkoblet = False
        else:
            self._stopp_logging()
            if self._koble_fra_fn is None:
                messagebox.showwarning("Mangler handler", "Ingen frakoblingshandler registrert.")
            else:
                try:
                    self._koble_fra_fn()
                except Exception as e:
                    messagebox.showerror("Frakoblingsfeil", str(e))
            self._tilkoblet = False

        self._oppdater_tilkoblings_ui()

    def _oppdater_tilkoblings_ui(self) -> None:
        """Oppdater UI-elementer basert på tilkoblingsstatus."""
        sp = getattr(self, "serieport", None)
        faktisk_apen = bool(sp and sp.is_open)
        self._tilkoblet = faktisk_apen

        if faktisk_apen:
            self.koble_knapp.config(text="Koble fra")
            portnavn = self.port_var.get().strip() or "<ukjent>"
            self.status_lbl.config(text=f"Status: tilkoblet ({portnavn})")
        else:
            self.koble_knapp.config(text="Koble til")
            self.status_lbl.config(text="Status: frakoblet")

    def _periodisk_statussynk(self) -> None:
        """Periodisk synk av tilkoblingsstatus."""
        self._oppdater_tilkoblings_ui()
        self.after(1000, self._periodisk_statussynk)

    # --------- Serieport lesetråd ----------

    def _start_logging(self) -> None:
        """Start kommunikasjon i bakgrunnstråd."""
        if self._lesetraad and self._lesetraad.is_alive():
            return
        self._lesetraads_stop.clear()
        self._t0 = time.monotonic()
        self._lesetraad = threading.Thread(target=self._les_loop, name="les_serie", daemon=True)
        self._lesetraad.start()

    def _stopp_logging(self) -> None:
        """Stopp bakgrunnstråd."""
        self._lesetraads_stop.set()
        if self._lesetraad and self._lesetraad.is_alive():
            self._lesetraad.join(timeout=1.0)
        self._lesetraad = None

    def _les_loop(self) -> None:
        """Les data fra serieport i egen tråd og legg i kø for GUI."""
        sp = getattr(self, "serieport", None)
        if sp is None or not sp.is_open:
            return
        while not self._lesetraads_stop.is_set() and sp.is_open:
            try:
                data = sp.read(3)
                if len(data) != 3:
                    continue
                tid8, verdi = struct.unpack("<Bh", data)  # tid:uint8, verdi:int16 (LE)
                tid_s = time.monotonic() - (self._t0 or time.monotonic())
                self._lesetraads_kø.put((tid_s, int(verdi)))
            except Exception as e:
                self.status_lbl.config(text=f"Lesefeil: {e}")
                break

    def _tøm_kø_og_oppdater(self) -> None:
        """Tøm køen for nye data og oppdater plott."""
        try:
            while True:
                tid_s, pv = self._lesetraads_kø.get_nowait()
                self.oppdater_data(tid_s, pv)
        except queue.Empty:
            pass
        self.after(50, self._tøm_kø_og_oppdater)

    def lukk(self) -> None:
        """Steng serieport og tråder ved lukking."""
        self._stopp_logging()
        sp = getattr(self, "serieport", None)
        if sp is not None:
            try:
                if sp.is_open:
                    sp.close()
            except Exception:
                pass


def main() -> None:
    rot = tk.Tk()
    app = PIDGUI(rot)
    app.pack(fill=tk.BOTH, expand=True)

    # --------- Eksterne handler-funksjoner (I/O) ---------

    def min_koble_til(portstreng: str) -> bool:
        """Åpne serieport og lagre referanse i app."""
        sp_gammel = getattr(app, "serieport", None)
        if sp_gammel is not None and sp_gammel.is_open:
            try:
                sp_gammel.close()
            except Exception:
                pass

        try:
            sp_ny = serial.Serial(port=portstreng, baudrate=115200, timeout=1)
        except Exception as e:
            raise RuntimeError(f"Kunne ikke åpne {portstreng}: {e}") from e

        if not sp_ny.is_open:
            raise RuntimeError(f"Port {portstreng} ble ikke åpnet.")

        app.serieport = sp_ny
        print(f"Status: tilkoblet: {portstreng}")
        return True

    def min_koble_fra() -> None:
        """Lukk serieport."""
        sp = getattr(app, "serieport", None)
        if sp is not None:
            try:
                if sp.is_open:
                    sp.close()
            except Exception:
                pass
        app.serieport = None
        print("Status: koblet fra")

    def min_pid_handler(settpunkt: int, kp: int, ki: int, kd: int) -> None:
        """Send PID-verdier som 4×int32 little-endian (SP, Kp, Ki, Kd)."""
        if not app._tilkoblet:
            print("Status: ikke tilkoblet – sender ikke PID")
            return
        sp = getattr(app, "serieport", None)
        if sp is None or not sp.is_open:
            print("Status: serieport ikke tilgjengelig/åpen – sender ikke PID")
            return

        try:
            # Legg header (69) i MSB av settpunkt
            header = 69
            sp_encoded = ((header & 0xFF) << 24) | (int(settpunkt) & 0xFFFFFF)
            pkt = struct.pack("<iiii", sp_encoded, int(kp), int(ki), int(kd))
            sp.write(pkt)
            sp.flush()
            print(f"Status: sendt 16B pakke: header=69, SP={settpunkt}, Kp={kp}, Ki={ki}, Kd={kd}")
        except Exception as e:
            messagebox.showerror("Sendefeil", f"Kunne ikke sende PID-verdier: {e}")


    # Registrer handlers
    app.registrer_tilkoblingshandler(min_koble_til, min_koble_fra)
    app.registrer_pid_callback(min_pid_handler)

    # Vindu-lukk
    def on_close():
        app.lukk()
        rot.destroy()

    rot.protocol("WM_DELETE_WINDOW", on_close)
    rot.mainloop()


if __name__ == "__main__":
    main()
