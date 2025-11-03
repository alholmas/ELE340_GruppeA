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
        self.master.geometry("1400x800")
        self.master.minsize(820, 520)

        # Tema
        self._sett_tema()

        # Tilstandsvariabler (streng for robust validering)
        self.settpunkt_var = tk.StringVar(value="200")
        self.kp_var = tk.StringVar(value="200")
        self.ki_var = tk.StringVar(value="0")     # Ti
        self.intbegr_var = tk.StringVar(value="10")
        self.kd_var = tk.StringVar(value="2000")  # Td
        self.port_var = tk.StringVar(value="")
        self.filnavn_var = tk.StringVar(value="logg.txt")
        self.serieport = None  # serial.Serial eller None

        # Data-buffere for plott (PV/SP)
        self.max_punkt = 1000
        self.t_data  = deque(maxlen=self.max_punkt)   # tid [s]
        self.pv_data = deque(maxlen=self.max_punkt)   # prosessverdi (avstand mm)
        self.sp_data = deque(maxlen=self.max_punkt)   # settpunkt tidsserie

        # Låst SP: endres kun ved Start (1) og Oppdater PID (2)
        self.sp_gjeldende = self._parse_int(self.settpunkt_var.get()) or 0

        # Dataserier fra styrenode (direkte mottatt)
        self.err_data = deque(maxlen=self.max_punkt)  # e
        self.u_data   = deque(maxlen=self.max_punkt)  # u
        self.up_data  = deque(maxlen=self.max_punkt)  # up
        self.ui_data  = deque(maxlen=self.max_punkt)  # ui
        self.ud_data  = deque(maxlen=self.max_punkt)  # ud

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

        # MCU-tidsrekonstruksjon
        self._mcu_tid_start = None  # første tid-tikk (uint32) fra styrenode

        # Bygg UI
        self._bygg_layout()

        # Fyll porter nå og ved dropdown
        self._oppdater_porter()
        self.port_cb.bind("<<ComboboxDropdown>>", lambda e: self._oppdater_porter())

        # Start periodisk tømming av kø
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
        # VIKTIG: pakk venstre panel FØRST, så ender panelet lengst til venstre
        venstre_panel = self._ramme(self.master)
        venstre_panel.pack(side=tk.LEFT, fill=tk.Y, padx=(12, 8), pady=12)

        # Hovedområde for figurer (stort plott + fem små til høyre)
        hoved = self._ramme(self.master)
        hoved.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 12), pady=12)

        # --- Venstre kontrollpanel ---
        ttk.Label(venstre_panel, text="Avstandssensor", font=("Segoe UI", 14, "bold")).pack(
            side=tk.TOP, anchor="w", padx=6, pady=(0, 8)
        )

        tilk_boks = self._ramme(venstre_panel)
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
        self.status_lbl = ttk.Label(venstre_panel, text="Status: frakoblet")
        self.status_lbl.pack(fill=tk.X, padx=4, pady=(6, 10))

        ttk.Label(venstre_panel, text="PID-innstillinger", font=("Segoe UI", 14, "bold")).pack(
            side=tk.TOP, anchor="w", padx=6, pady=(0, 8)
        )
        pidboks = self._ramme(venstre_panel)
        pidboks.pack(fill=tk.X, padx=0, pady=(0, 10))
        self._rad_med_entry(pidboks, "Settpunkt", self.settpunkt_var)
        self._rad_med_entry(pidboks, "Kp", self.kp_var)
        self._rad_med_entry(pidboks, "Ti", self.ki_var)
        self._rad_med_entry(pidboks, "Integrator begrensning", self.intbegr_var)
        self._rad_med_entry(pidboks, "Td", self.kd_var)

        radkn = ttk.Frame(pidboks)
        radkn.pack(fill=tk.X, padx=8, pady=(6, 8))
        ttk.Button(radkn, text="Start",        command=lambda: self._bruk_pid(start=1)).pack(side=tk.LEFT, padx=(0, 8))
        ttk.Button(radkn, text="Oppdater PID", command=lambda: self._bruk_pid(start=2)).pack(side=tk.LEFT, padx=(0, 8))
        ttk.Button(radkn, text="Stop",         command=lambda: (self._bruk_pid(start=0), self._nullstill_graf())).pack(side=tk.LEFT)

        self.info_lbl = ttk.Label(venstre_panel, text="Avstand: —")
        self.info_lbl.pack(side=tk.TOP, fill=tk.X, padx=4, pady=(6, 0))

        # --- Stort plott (PV/SP) ---
        venstre_fig_frame = ttk.Frame(hoved)
        venstre_fig_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 8))

        figur = Figure(figsize=(6.4, 6), dpi=100)  # litt bredere for å være "stort"
        self.akse = figur.add_subplot(111)
        self.akse.set_title("Avstandsmåling")
        self.akse.set_xlabel("Tid [s]")
        self.akse.set_ylabel("Avstand [mm]")
        self.akse.grid(True, alpha=0.25)

        # Tykkere linjer for tydelig hovedplott
        self.linje_pv, = self.akse.plot([], [], label="Avstand (PV)", linewidth=1.6)
        self.linje_sp, = self.akse.plot([], [], linestyle="--", label="Settpunkt (SP)", linewidth=1.4)
        self.akse.legend(loc="upper left", framealpha=0.2, borderaxespad=0.5, fontsize=9)
        self.akse.ticklabel_format(axis="x", style="plain", useOffset=False)

        self.canvas = FigureCanvasTkAgg(figur, master=venstre_fig_frame)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        # --- Fem plott i høyre kolonne ---
        høyre_fig_frame = ttk.Frame(hoved)
        høyre_fig_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(8, 0))
        høyre_fig_frame.update_idletasks()
        # Lås en fornuftig bredde for å sikre smal høyrekolonne
        høyre_fig_frame.configure(width=480)
        høyre_fig_frame.pack_propagate(False)

        figur5 = Figure(figsize=(6.2, 9.2), dpi=100)
        gs = figur5.add_gridspec(5, 1, hspace=0.55)
        figur5.subplots_adjust(right=0.84)

        self.ax_e  = figur5.add_subplot(gs[0, 0])
        self.ax_u  = figur5.add_subplot(gs[1, 0], sharex=self.ax_e)
        self.ax_up = figur5.add_subplot(gs[2, 0], sharex=self.ax_e)
        self.ax_ui = figur5.add_subplot(gs[3, 0], sharex=self.ax_e)
        self.ax_ud = figur5.add_subplot(gs[4, 0], sharex=self.ax_e)

        self.ax_e.set_title("Avvik e [mm]")
        self.ax_u.set_title("Pådrag u")
        self.ax_up.set_title("P-del")
        self.ax_ui.set_title("I-del")
        self.ax_ud.set_title("D-del")
        self.ax_ud.set_xlabel("Tid [s]")

        # Grid + ryddige akser
        for i, ax in enumerate((self.ax_e, self.ax_u, self.ax_up, self.ax_ui, self.ax_ud)):
            ax.grid(True, alpha=0.25)
            ax.ticklabel_format(axis="x", style="plain", useOffset=False)
            if i < 4:
                # Skjul x-etiketter for de øverste 4 for å spare plass
                ax.label_outer()

        self.linje_e,  = self.ax_e.plot([], [], linewidth=1.2)
        self.linje_u,  = self.ax_u.plot([], [], linewidth=1.2)
        self.linje_up, = self.ax_up.plot([], [], linewidth=1.2)
        self.linje_ui, = self.ax_ui.plot([], [], linewidth=1.2)
        self.linje_ud, = self.ax_ud.plot([], [], linewidth=1.2)

        # Tekstfelt for verdier til høyre for hvert subplot
        bbox_stil = dict(boxstyle="round,pad=0.2", fc=(1, 1, 1, 0.08), ec=(1, 1, 1, 0.25))
        self.valtxt_e  = self.ax_e .text(1.02, 0.5, "—", transform=self.ax_e .transAxes, va="center", ha="left", fontsize=9, bbox=bbox_stil)
        self.valtxt_u  = self.ax_u .text(1.02, 0.5, "—", transform=self.ax_u .transAxes, va="center", ha="left", fontsize=9, bbox=bbox_stil)
        self.valtxt_up = self.ax_up.text(1.02, 0.5, "—", transform=self.ax_up.transAxes, va="center", ha="left", fontsize=9, bbox=bbox_stil)
        self.valtxt_ui = self.ax_ui.text(1.02, 0.5, "—", transform=self.ax_ui.transAxes, va="center", ha="left", fontsize=9, bbox=bbox_stil)
        self.valtxt_ud = self.ax_ud.text(1.02, 0.5, "—", transform=self.ax_ud.transAxes, va="center", ha="left", fontsize=9, bbox=bbox_stil)
                                        
        self.canvas5 = FigureCanvasTkAgg(figur5, master=høyre_fig_frame)
        self.canvas5.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def _rad_med_entry(self, parent, etikett, var):
        rad = ttk.Frame(parent)
        rad.pack(fill=tk.X, padx=8, pady=4)
        ttk.Label(rad, text=etikett).pack(side=tk.LEFT)
        ent = ttk.Entry(rad, textvariable=var, width=12)
        ent.pack(side=tk.RIGHT)

    # ---------- Hjelpefunksjoner ----------

    @staticmethod
    def _parse_int(s):
        s = (s or "").strip()
        if s in ("", "+", "-"):
            return None
        try:
            return int(s)
        except ValueError:
            return None

    def _trygg_lukk_port(self):
        sp = self.serieport
        if sp and sp.is_open:
            try:
                sp.close()
            except Exception:
                pass
        self.serieport = None

    def _oppdater_status_from_port(self):
        apen = bool(self.serieport and self.serieport.is_open)
        self.koble_knapp.config(text="Koble fra" if apen else "Koble til")
        self.status_lbl.config(
            text=f"Status: tilkoblet ({self.port_var.get().strip() or '<ukjent>'})" if apen else "Status: frakoblet"
        )

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

    def _bruk_pid(self, start):
        sp  = self._parse_int(self.settpunkt_var.get())
        kp  = self._parse_int(self.kp_var.get())
        ti  = self._parse_int(self.ki_var.get())
        td  = self._parse_int(self.kd_var.get())
        ib  = self._parse_int(self.intbegr_var.get())
        if None in (sp, kp, ti, td, ib):
            messagebox.showwarning("Ugyldig verdi", "SP/Kp/Ti/Td/IntBegr må være heltall.")
            return

        # Lås SP ved Start (1) og Oppdater PID (2)
        if int(start) in (1, 2) and sp is not None:
            self.sp_gjeldende = int(sp)

        try:
            if callable(self._pid_callback):
                self._pid_callback(sp, kp, ti, td, ib, int(start))
            else:
                messagebox.showinfo("Ingen handler", "Ingen PID-handler registrert.")
        except Exception as e:
            messagebox.showerror("Callback-feil", str(e))

    def _nullstill_graf(self):
        self.t_data.clear()
        self.pv_data.clear()
        self.sp_data.clear()
        self.err_data.clear()
        self.u_data.clear()
        self.up_data.clear()
        self.ui_data.clear()
        self.ud_data.clear()
        self._oppdater_plott()

    def oppdater_data(self, tid_s, pv, e, u, up, ui, ud):
        self.t_data.append(float(tid_s))
        self.pv_data.append(int(pv))
        self.sp_data.append(int(self.sp_gjeldende))

        # Direkte mottatte deler
        self.err_data.append(int(e))
        self.u_data.append(int(u))
        self.up_data.append(int(up))
        self.ui_data.append(int(ui))
        self.ud_data.append(int(ud))

        self.info_lbl.config(text=f"Avstand: {int(pv)}")
        self._oppdater_plott()

    def _oppdater_plott(self):
        # Venstre (stort plott)
        self.linje_pv.set_data(self.t_data, self.pv_data)
        self.linje_sp.set_data(self.t_data, self.sp_data)
        self.akse.relim()
        self.akse.autoscale_view()
        self.canvas.draw_idle()

        # Høyre (5 plott)
        self.linje_e.set_data(self.t_data, self.err_data)
        self.linje_u.set_data(self.t_data, self.u_data)
        self.linje_up.set_data(self.t_data, self.up_data)
        self.linje_ui.set_data(self.t_data, self.ui_data)
        self.linje_ud.set_data(self.t_data, self.ud_data)
        self.valtxt_e.set_text(str(self.err_data[-1]) if self.err_data else "—")
        self.valtxt_u.set_text(str(self.u_data[-1]) if self.u_data else "—")
        self.valtxt_up.set_text(str(self.up_data[-1]) if self.up_data else "—")
        self.valtxt_ui.set_text(str(self.ui_data[-1]) if self.ui_data else "—")
        self.valtxt_ud.set_text(str(self.ud_data[-1]) if self.ud_data else "—")
        for ax in (self.ax_e, self.ax_u, self.ax_up, self.ax_ui, self.ax_ud):
            ax.relim()
            ax.autoscale_view()
        self.canvas5.draw_idle()

    # ---------- Tilkobling / port / tråd ----------

    def _oppdater_porter(self):
        try:
            porter = [p.device for p in serial.tools.list_ports.comports()]
        except Exception as e:
            messagebox.showerror("Portoppdatering feilet", str(e))
            porter = []
        self.port_cb["values"] = porter

        nåv = (self.port_var.get() or "").strip()
        if not (nåv and nåv in porter):
            self.port_var.set(porter[0] if porter else "")

    def _toggle_tilkobling(self):
        if not bool(self.serieport and self.serieport.is_open):
            try:
                if not callable(self._koble_til_fn):
                    raise RuntimeError("Tilkoblingshandler ikke satt.")
                resultat = self._koble_til_fn(self.port_var.get())
                if resultat is True and self.serieport and self.serieport.is_open:
                    self._nullstill_graf()
                    self._start_logging()
                else:
                    raise RuntimeError("Klarte ikke koble til.")
            except Exception as e:
                messagebox.showerror("Tilkoblingsfeil", str(e))
                self._trygg_lukk_port()
        else:
            self._stopp_logging()
            if callable(self._koble_fra_fn):
                try:
                    self._koble_fra_fn()
                except Exception as e:
                    messagebox.showerror("Frakoblingsfeil", str(e))
            self._trygg_lukk_port()
        self._oppdater_status_from_port()

    # ---------- Logging og lesetråd ----------

    def _start_logging(self):
        if self._lesetraad and self._lesetraad.is_alive():
            return
        try:
            filnavn = (self.filnavn_var.get() or "").strip() or "logg.txt"
            self._logg_fil = open(filnavn, "a", buffering=1, encoding="utf-8")
            self._logg_fil.write("tid_s,pv,sp,e,u,up,ui,ud\n")
        except Exception as e:
            messagebox.showerror("Loggfil-feil", f"Kunne ikke åpne loggfil: {e}")
            self._logg_fil = None

        self._lesetraads_stop.clear()
        self._mcu_tid_start = None
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

        self._oppdater_status_from_port()

    def _les_loop(self):
        sp = self.serieport
        if sp is None or not sp.is_open:
            return

        # Pakkeformat fra styrenode:
        # <B I H h h h h h B>  (little-endian)
        # header(0xAA), tid(uint32), avstand_mm(uint16),
        # e(int16), u(int16), up(int16), ui(int16), ud(int16), tail(0x55)
        PAKKE = struct.Struct("<BIHhhhhhB")

        while not self._lesetraads_stop.is_set() and sp.is_open:
            try:
                # Finn header 0xAA
                b = sp.read(1)
                if not b or b[0] != 0xAA:
                    continue

                # Les resten av pakken
                rest = sp.read(PAKKE.size - 1)
                if len(rest) != PAKKE.size - 1:
                    continue

                hdr, tid_raw, avstand, e, u, up, ui, ud, tlr = PAKKE.unpack(b + rest)
                if tlr != 0x55:
                    continue

                # Relativ tid i sekunder (antatt tid_raw i 10 ms ticks -> /100)
                if self._mcu_tid_start is None:
                    self._mcu_tid_start = tid_raw
                delta = (tid_raw - self._mcu_tid_start) & 0xFFFFFFFF
                tid_s = delta / 100.0  # 10 ms per tick

                # Logg til fil: tid, PV, SP, e, u, up, ui, ud
                if self._logg_fil is not None:
                    self._logg_fil.write(
                        f"{tid_s},{int(avstand)},{int(self.sp_gjeldende)},{int(e)},{int(u)},{int(up)},{int(ui)},{int(ud)}\n"
                    )

                # Legg på GUI-kø
                self._lesetraads_kø.put((tid_s, avstand, e, u, up, ui, ud))

            except Exception as e_exc:
                print(f"Lesefeil: {e_exc}")
                try:
                    if sp and sp.is_open:
                        sp.close()
                except Exception:
                    pass
                self.serieport = None
                self.after(50, self._oppdater_status_from_port)
                break

    def _tøm_kø_og_oppdater(self):
        try:
            while True:
                tid_s, pv, e, u, up, ui, ud = self._lesetraads_kø.get_nowait()
                self.oppdater_data(tid_s, pv, e, u, up, ui, ud)
        except queue.Empty:
            pass
        self._oppdater_status_from_port()
        self.after(50, self._tøm_kø_og_oppdater)

    def lukk(self):
        self._stopp_logging()
        self._trygg_lukk_port()