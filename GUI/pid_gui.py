import tkinter as tk
from tkinter import ttk, messagebox
from collections import deque
import queue
import threading
import struct
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import serial.tools.list_ports


class PIDGUI(ttk.Frame):
    def __init__(self, master):
        super().__init__(master)
        self.master.title("PID styring av avstandssensor")
        self.master.geometry("1400x600")
        self.master.minsize(820, 520)
        self.master.protocol("WM_DELETE_WINDOW", self.lukk)

        # Tema
        self._sett_tema()

        # Tilstandsvariabler
        self.settpunkt_var = tk.StringVar(value="200")
        self.kp_var = tk.StringVar(value="200")
        self.ki_var = tk.StringVar(value="0")
        self.kd_var = tk.StringVar(value="2000")
        self.port_var = tk.StringVar(value="")
        self.filnavn_var = tk.StringVar(value="logg.txt")
        self._tilkoblet = False
        self.serieport = None  # serieport-objekt settes av tilkoblingshandler

        # Data-buffere for plott
        self.max_punkt = 2000
        self.t_data = deque(maxlen=self.max_punkt)   # tid [s]
        self.pv_data = deque(maxlen=self.max_punkt)  # prosessverdi

        # Eksterne callbacks (injiseres fra main)
        self._pid_callback = None
        self._koble_til_fn = None
        self._koble_fra_fn = None

        # Tråd og kø for innkommende data
        self._lesetraads_kø = queue.Queue()
        self._lesetraads_stop = threading.Event()
        self._lesetraad = None

        # Loggfilhåndtak
        self._logg_fil = None

        # MCU-tidsrekonstruksjon (uint8 teller @ 100 Hz)
        self._mcu_tid_forrige = None
        self._mcu_tick_sum = 0
        self._sample_periode_s = 0.01  # 10 ms/pakke

        # Bygg UI
        self._bygg_layout()

        # Fyll porter nå og ved dropdown
        self._oppdater_porter()
        self.port_cb.bind("<<ComboboxDropdown>>", lambda e: self._oppdater_porter())

        # Start periodisk tømming av kø (≈ 30 Hz)
        self.after(50, self._tøm_kø_og_oppdater)

    # ---------- Tema / layout ----------

    def _sett_tema(self):
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

    def _ramme(self, parent, tittel=None):
        return ttk.LabelFrame(parent, text=tittel) if tittel else ttk.Frame(parent)

    def _bygg_layout(self):
        venstre = self._ramme(self.master)
        venstre.pack(side=tk.LEFT, fill=tk.Y, padx=(12, 8), pady=12)

        hoyre = self._ramme(self.master)
        hoyre.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 12), pady=12)

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
        self._rad_med_entry(tilk_boks, "Filnavn på logg:", self.filnavn_var)
        self.status_lbl = ttk.Label(venstre, text="Status: frakoblet")
        self.status_lbl.pack(fill=tk.X, padx=4, pady=(6, 10))

        # PID-innstillinger
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
        self.info_lbl = ttk.Label(venstre, text="Avstand: —")
        self.info_lbl.pack(side=tk.TOP, fill=tk.X, padx=4, pady=(6, 0))

        # Matplotlib-figur
        figur = Figure(figsize=(5, 4), dpi=100)
        self.akse = figur.add_subplot(111)
        self.akse.set_title("Avstandsmåling")
        self.akse.set_xlabel("Tid [s]")
        self.akse.set_ylabel("Avstand")
        self.akse.grid(True, alpha=0.25)
        self.linje_pv, = self.akse.plot([], [], label="Avstand")
        self.linje_sp, = self.akse.plot([], [], linestyle="--", label="Settpunkt")
        self.akse.legend(loc="upper left", framealpha=0.2, borderaxespad=0.5, fontsize=9)

        self.canvas = FigureCanvasTkAgg(figur, master=hoyre)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def _rad_med_entry(self, parent, etikett, var):
        rad = ttk.Frame(parent)
        rad.pack(fill=tk.X, padx=8, pady=4)
        ttk.Label(rad, text=etikett).pack(side=tk.LEFT)
        ent = ttk.Entry(rad, textvariable=var, width=12)
        ent.pack(side=tk.RIGHT)

    # ---------- Hjelpefunksjoner ----------

    @staticmethod
    def _parse_int(s):
        # Parsing av heltall fra streng
        s = (s or "").strip()
        if s in ("", "+", "-"):
            return None
        try:
            return int(s)
        except ValueError:
            return None

    # ---------- Offentlig API ----------

    def registrer_pid_callback(self, funksjon):
        self._pid_callback = funksjon

    def registrer_tilkoblingshandler(self, koble_til_fn, koble_fra_fn):
        self._koble_til_fn = koble_til_fn
        self._koble_fra_fn = koble_fra_fn

    def sett_pid(self, kp, ki, kd, settpunkt=None):
        if settpunkt is not None:
            self.settpunkt_var.set(str(int(settpunkt)))
        self.kp_var.set(str(int(kp)))
        self.ki_var.set(str(int(ki)))
        self.kd_var.set(str(int(kd)))

    # ---------- Hovedhandlinger ----------

    def _bruk_pid(self):
        # Hent PID-verdier og kall callback (robust parsing)
        sp = self._parse_int(self.settpunkt_var.get())
        kp = self._parse_int(self.kp_var.get())
        ki = self._parse_int(self.ki_var.get())
        kd = self._parse_int(self.kd_var.get())

        if None in (sp, kp, ki, kd):
            messagebox.showwarning("Ugyldig verdi", "Settpunkt/Kp/Ki/Kd må være heltall.")
            return

        if not callable(self._pid_callback):
            messagebox.showinfo("Ingen handler", "Ingen PID-handler registrert.")
            return

        try:
            self._pid_callback(sp, kp, ki, kd)
        except Exception as e:
            messagebox.showerror("Callback-feil", str(e))

    def _nullstill_graf(self):
        self.t_data.clear()
        self.pv_data.clear()
        self._oppdater_plott()

    def oppdater_data(self, tid_s, pv):
        # Legg til nye data og oppdater plott
        self.t_data.append(float(tid_s))
        self.pv_data.append(int(pv))
        self.info_lbl.config(text=f"Avstand: {int(pv)}")
        self._oppdater_plott()

    def _oppdater_plott(self):
        # Oppdater PV-kurve
        self.linje_pv.set_data(self.t_data, self.pv_data)

        # Auto-akse X (unngå identisk xmin/xmax)
        if len(self.t_data) >= 1:
            xmin, xmax = self.t_data[0], self.t_data[-1]
            if xmax <= xmin:
                xmin, xmax = xmin - 1.0, xmin + 1.0
            self.akse.set_xlim(xmin, xmax)
        else:
            self.akse.set_xlim(0, 1)

        # Auto-akse Y
        if self.pv_data:
            y_min = min(self.pv_data)
            y_max = max(self.pv_data)
            margin = max(1, int(0.1 * (y_max - y_min + 1)))
            self.akse.set_ylim(y_min - margin, y_max + margin)
        else:
            self.akse.set_ylim(-1, 1)

        # Settpunktlinje – kun hvis gyldig tall
        sp = self._parse_int(self.settpunkt_var.get())
        if sp is None or not self.t_data:
            self.linje_sp.set_data([], [])
        else:
            self.linje_sp.set_data([self.t_data[0], self.t_data[-1]], [sp, sp])

        self.canvas.draw_idle()

    # ---------- Tilkobling / port / tråd ----------

    def _oppdater_porter(self):
        try:
            porter = [p.device for p in serial.tools.list_ports.comports()]
        except Exception as e:
            messagebox.showerror("Portoppdatering feilet", str(e))
            porter = []
        self.port_cb["values"] = porter
        nåv = (self.port_var.get() or "").strip()
        if nåv and nåv in porter:
            pass
        elif porter:
            self.port_var.set(porter[0])
        else:
            self.port_var.set("")

    def _toggle_tilkobling(self):
        if not self._tilkoblet:
            # --- Koble til ---
            try:
                if not callable(self._koble_til_fn):
                    raise RuntimeError("Tilkoblingshandler ikke satt.")
                resultat = self._koble_til_fn(self.port_var.get())
                if resultat is True and getattr(self, "serieport", None) and self.serieport.is_open:
                    self._tilkoblet = True
                    self._start_logging()
                else:
                    raise RuntimeError("Klarte ikke koble til.")
            except Exception as e:
                messagebox.showerror("Tilkoblingsfeil", str(e))
                self._tilkoblet = False
        else:
            # --- Koble fra ---
            self._stopp_logging()
            if callable(self._koble_fra_fn):
                try:
                    self._koble_fra_fn()
                except Exception as e:
                    messagebox.showerror("Frakoblingsfeil", str(e))
            self._tilkoblet = False

        # Oppdater statuslinje/knapp
        sp = getattr(self, "serieport", None)
        faktisk_apen = bool(sp and sp.is_open)
        self._tilkoblet = faktisk_apen
        if faktisk_apen:
            self.koble_knapp.config(text="Koble fra")
            self.status_lbl.config(text=f"Status: tilkoblet ({self.port_var.get().strip() or '<ukjent>'})")
        else:
            self.koble_knapp.config(text="Koble til")
            self.status_lbl.config(text="Status: frakoblet")

    # ---------- Logging og lesetråd ----------

    def _start_logging(self):
        if self._lesetraad and self._lesetraad.is_alive():
            return
        try:
            filnavn = (self.filnavn_var.get() or "").strip() or "logg.txt"
            self._logg_fil = open(filnavn, "a", buffering=1, encoding="utf-8")
        except Exception as e:
            messagebox.showerror("Loggfil-feil", f"Kunne ikke åpne loggfil: {e}")
            self._logg_fil = None

        self._lesetraads_stop.clear()
        self._mcu_tid_forrige = None
        self._mcu_tick_sum = 0
        self._lesetraad = threading.Thread(target=self._les_loop, name="les_serie", daemon=True)
        self._lesetraad.start()

    def _stopp_logging(self):
        self._lesetraads_stop.set()
        if self._lesetraad and self._lesetraad.is_alive():
            self._lesetraad.join(timeout=1.0)
        self._lesetraad = None

        if self._logg_fil is not None:
            try:
                self._logg_fil.close()
            except Exception:
                pass
            self._logg_fil = None

        self._tilkoblet = False
        self.koble_knapp.config(text="Koble til")
        self.status_lbl.config(text="Status: frakoblet")

    def _les_loop(self):
        import struct

        sp = getattr(self, "serieport", None)
        if sp is None or not sp.is_open:
            return

        FMT = "<BIHB"   # 1B header (0xAA), 4B tid_ms (uint32), 2B verdi (uint16), 1B tail (0x55)

        while not self._lesetraads_stop.is_set() and sp.is_open:
            try:
                data = sp.read(8)
                if len(data) != 8:
                    continue

                hdr, tid_ms, verdi, tlr = struct.unpack(FMT, data)
                if hdr != 0xAA or tlr != 0x55:
                    continue

                # MCU-relativ tid med 32-bit wrap (tid_ms er 4 byte)
                if self._mcu_tid_forrige is None:
                    self._mcu_tid_forrige = tid_ms
                    self._mcu_tick_sum = 0
                    mcu_tid_s = 0.0
                else:
                    delta = (tid_ms - self._mcu_tid_forrige) & 0xFFFFFFFF
                    self._mcu_tick_sum += int(delta)
                    self._mcu_tid_forrige = tid_ms
                    mcu_tid_s = self._mcu_tick_sum * self._sample_periode_s

                if self._logg_fil is not None:
                    try:
                        self._logg_fil.write(f"{mcu_tid_s:.3f},{int(verdi)}\n")
                    except Exception as le:
                        self.status_lbl.config(text=f"Loggfeil: {le}")

                self._lesetraads_kø.put((mcu_tid_s, int(verdi)))

            except Exception as e:
                self.status_lbl.config(text=f"Lesefeil: {e}")
                try:
                    if sp and sp.is_open:
                        sp.close()
                except Exception:
                    pass
                self.serieport = None
                self._tilkoblet = False
                self.koble_knapp.config(text="Koble til")
                break

    def _tøm_kø_og_oppdater(self):
        try:
            while True:
                tid_s, pv = self._lesetraads_kø.get_nowait()
                self.oppdater_data(tid_s, pv)
        except queue.Empty:
            pass
        self.after(33, self._tøm_kø_og_oppdater)

    def lukk(self):
        self._stopp_logging()
        sp = getattr(self, "serieport", None)
        if sp is not None:
            try:
                if sp.is_open:
                    sp.close()
            except Exception:
                pass