import os
import glob
import tkinter as tk
from tkinter import ttk, messagebox
from tkinter.filedialog import askopenfilename

from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib.pyplot as plt


class LoggPlotter(ttk.Frame):
    def __init__(self, master):
        super().__init__(master)
        self.master.title("Logg-plotter for PIDGUI")
        self.master.geometry("1200x700")
        self.master.minsize(900, 520)

        # Tema (mørk-ish, matcher ca. den andre appen)
        self._sett_tema()

        # Tilstandsvariabler
        self.valgt_fil_var = tk.StringVar(value="")
        self.status_var = tk.StringVar(value="Klar")

        # Bygg GUI
        self._bygg_layout()

        # Fyll combobox med *.txt ved oppstart
        self._oppdater_filer()

    # -------------------- Tema og layout --------------------

    def _sett_tema(self):
        stil = ttk.Style()
        try:
            stil.theme_use("clam")
        except tk.TclError:
            pass
        bakgrunn = "#1f2227"
        panel = "#262a31"
        tekst = "#e8e8e8"
        self.master.configure(bg=bakgrunn)
        stil.configure(".", background=panel, foreground=tekst, fieldbackground="#2f3540")
        stil.configure("TFrame", background=panel)
        stil.configure("TLabel", background=panel, foreground=tekst)
        stil.configure("TButton", background="#2f3540", foreground=tekst, padding=6)
        stil.map("TButton", background=[("active", "#3a4352")])
        stil.configure("TLabelframe", background=panel, foreground=tekst)
        stil.configure("TLabelframe.Label", background=panel, foreground=tekst)

    def _ramme(self, parent, tittel=None):
        return ttk.LabelFrame(parent, text=tittel) if tittel else ttk.Frame(parent)

    def _bygg_layout(self):
        # Venstre kontrollpanel
        venstre = self._ramme(self.master)
        venstre.pack(side=tk.LEFT, fill=tk.Y, padx=(12, 8), pady=12)

        ttk.Label(venstre, text="Logg-plotter", font=("Segoe UI", 14, "bold")).pack(
            side=tk.TOP, anchor="w", padx=6, pady=(0, 8)
        )

        filboks = self._ramme(venstre, "Filvalg")
        filboks.pack(fill=tk.X, padx=0, pady=(0, 12))

        rad1 = ttk.Frame(filboks); rad1.pack(fill=tk.X, padx=8, pady=(8, 6))
        ttk.Label(rad1, text="Velg loggfil (.txt)").pack(side=tk.LEFT)
        self.fil_cb = ttk.Combobox(rad1, textvariable=self.valgt_fil_var, width=36, state="readonly")
        self.fil_cb.pack(side=tk.LEFT, padx=(8, 8))

        rad2 = ttk.Frame(filboks); rad2.pack(fill=tk.X, padx=8, pady=(0, 8))
        ttk.Button(rad2, text="Oppdater liste", command=self._oppdater_filer).pack(side=tk.LEFT)
        ttk.Button(rad2, text="Åpne valgt", command=self._last_valgt_fil).pack(side=tk.LEFT, padx=(8, 0))
        ttk.Button(rad2, text="Bla til fil…", command=self._bla_til_fil).pack(side=tk.LEFT, padx=(8, 0))

        self.status_lbl = ttk.Label(venstre, textvariable=self.status_var)
        self.status_lbl.pack(fill=tk.X, padx=6, pady=(8, 0))

        # Høyre plottområde
        høyre = self._ramme(self.master)
        høyre.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 12), pady=12)

        # Figur med to akser (øverst PV/SP, nederst e/u/up/ui/ud)
        self.fig = Figure(figsize=(8, 6), dpi=100)
        # Gi plass til legend utenpå til høyre
        self.fig.subplots_adjust(right=0.76, hspace=0.3)

        self.ax1 = self.fig.add_subplot(211)
        self.ax2 = self.fig.add_subplot(212, sharex=self.ax1)

        self.ax1.set_title("Avstand (PV) og settpunkt (SP)")
        self.ax1.set_ylabel("Avstand [mm]")
        self.ax1.grid(True, alpha=0.25)

        self.ax2.set_title("Feil og pådrag")
        self.ax2.set_xlabel("Tid [s]")
        self.ax2.set_ylabel("e / u / oppdelinger")
        self.ax2.grid(True, alpha=0.25)

        # Init linjer (uten data ennå)
        (self.l_pv,)  = self.ax1.plot([], [], linewidth=1.6, label="PV (mm)")
        (self.l_sp,)  = self.ax1.plot([], [], linestyle="--", linewidth=1.4, label="SP (mm)")

        (self.l_e,)   = self.ax2.plot([], [], linewidth=1.0, label="e (mm)")
        (self.l_u,)   = self.ax2.plot([], [], linewidth=1.0, label="u")
        (self.l_up,)  = self.ax2.plot([], [], linewidth=1.0, label="up")
        (self.l_ui,)  = self.ax2.plot([], [], linewidth=1.0, label="ui")
        (self.l_ud,)  = self.ax2.plot([], [], linewidth=1.0, label="ud")

        # Legg én legend "ved siden av" (til høyre) som samler alle linjene
        alle_linjer = [self.l_pv, self.l_sp, self.l_e, self.l_u, self.l_up, self.l_ui, self.l_ud]
        etiketter = [l.get_label() for l in alle_linjer]
        self.legend = self.fig.legend(
            alle_linjer,
            etiketter,
            loc="center left",
            bbox_to_anchor=(0.79, 0.5),
            framealpha=0.2,
            fontsize=9,
        )

        self.canvas = FigureCanvasTkAgg(self.fig, master=høyre)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        # Legg til navigasjonsverktøylinje
        from matplotlib.backends.backend_tkagg import NavigationToolbar2Tk
        self.toolbar = NavigationToolbar2Tk(self.canvas, høyre)
        self.toolbar.update()
        self.toolbar.pack(fill=tk.X)

    # -------------------- Filhåndtering --------------------

    def _oppdater_filer(self):
        """Fyll combobox med alle *.txt i plots-mappen."""
        try:
            # Opprett plots-mappen hvis den ikke eksisterer
            plots_mappe = os.path.join(os.getcwd(), "plots")
            if not os.path.exists(plots_mappe):
                os.makedirs(plots_mappe)

            filer = sorted(glob.glob(os.path.join(plots_mappe, "*.txt")))
            # Vis bare filnavn, men lagre full sti i verdien
            visningsliste = [os.path.basename(f) for f in filer]
            self._filmap = {os.path.basename(f): f for f in filer}
            self.fil_cb["values"] = visningsliste
            if visningsliste and self.valgt_fil_var.get() == "":
                self.valgt_fil_var.set(visningsliste[0])
            self.status_var.set(f"Fant {len(visningsliste)} .txt-fil(er) i plots-mappen")
        except Exception as e:
            messagebox.showerror("Feil ved oppdatering", str(e))

    def _få_valgt_filsti(self):
        navn = self.valgt_fil_var.get().strip()
        if not navn:
            return None
        # Dersom filnavn ikke finnes i map, prøv som direkte sti
        if hasattr(self, "_filmap") and navn in self._filmap:
            return self._filmap[navn]
        return navn if os.path.exists(navn) else None

    def _last_valgt_fil(self):
        sti = self._få_valgt_filsti()
        if not sti:
            messagebox.showinfo("Ingen fil", "Velg en loggfil i comboboxen først.")
            return
        self._last_inn_og_plot(sti)

    def _bla_til_fil(self):
        fil = askopenfilename(
            title="Velg loggfil",
            initialdir=os.getcwd(),
            filetypes=[("Tekstfiler", "*.txt"), ("Alle filer", "*.*")],
        )
        if not fil:
            return
        # Sett combobox til valgt filnavn (om i samme mappe), ellers hele stien
        bn = os.path.basename(fil)
        if os.path.dirname(fil) == os.getcwd():
            self.valgt_fil_var.set(bn)
        else:
            self.valgt_fil_var.set(fil)
        self._last_inn_og_plot(fil)

    # -------------------- Lesing og plotting --------------------

    def _last_inn_og_plot(self, filsti):
        """Les fil (robust mot blanke linjer/rare linjer) og oppdater plott."""
        try:
            t, pv, sp, e, u, up, ui, ud = [], [], [], [], [], [], [], []
            ant_ok = 0
            ant_skip = 0
            with open(filsti, "r", encoding="utf-8", errors="ignore") as f:
                for linje in f:
                    s = linje.strip()
                    if not s:
                        continue
                    # Hopp over header-linjer eller alt som ikke ser ut som csv med tall
                    if s.startswith("#") or "tid" in s.lower():
                        ant_skip += 1
                        continue
                    deler = s.split(",")
                    if len(deler) < 8:
                        ant_skip += 1
                        continue
                    try:
                        # Konverter
                        tt  = float(deler[0])
                        ppv = float(deler[1])
                        ssp = float(deler[2])
                        ee  = float(deler[3])
                        uu  = float(deler[4])
                        uup = float(deler[5])
                        uui = float(deler[6])
                        uud = float(deler[7])
                    except ValueError:
                        ant_skip += 1
                        continue
                    # Lagre
                    t.append(tt); pv.append(ppv); sp.append(ssp)
                    e.append(ee); u.append(uu); up.append(uup); ui.append(uui); ud.append(uud)
                    ant_ok += 1

            if ant_ok == 0:
                messagebox.showwarning("Ingen data", "Fant ingen gyldige data i fila.")
                return

            # Oppdater linjer
            self.l_pv.set_data(t, pv)
            self.l_sp.set_data(t, sp)
            self.l_e.set_data(t, e)
            self.l_u.set_data(t, u)
            self.l_up.set_data(t, up)
            self.l_ui.set_data(t, ui)
            self.l_ud.set_data(t, ud)

            # Autoskalér
            for ax in (self.ax1, self.ax2):
                ax.relim()
                ax.autoscale_view()

            self.canvas.draw_idle()
            self.status_var.set(f"Lastet '{os.path.basename(filsti)}' – {ant_ok} linjer (hoppet over {ant_skip})")

        except Exception as e:
            messagebox.showerror("Feil ved lesing", f"Kunne ikke lese '{filsti}': {e}")


def main():
    rot = tk.Tk()
    app = LoggPlotter(rot)
    app.pack(fill=tk.BOTH, expand=True)
    rot.mainloop()


if __name__ == "__main__":
    main()
