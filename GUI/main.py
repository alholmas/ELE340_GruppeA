import tkinter as tk
from tkinter import messagebox
import serial
import struct
from pid_gui import PIDGUI

def main():
    rot = tk.Tk()
    app = PIDGUI(rot)
    app.pack(fill=tk.BOTH, expand=True)
    app.serieport = None  # initér eksplisitt

    # --- Tilkoblingshåndterere ---
    def min_koble_til(portstreng):
        # Lukk gammel port (viktig for å unngå lås)
        sp_gammel = getattr(app, "serieport", None)
        if sp_gammel and sp_gammel.is_open:
            try:
                sp_gammel.close()
            except Exception:
                pass

        try:
            sp_ny = serial.Serial(port=portstreng, baudrate=115200, timeout=1)
        except Exception as e:
            raise RuntimeError("Kunne ikke åpne {}: {}".format(portstreng, e))

        if not sp_ny.is_open:
            raise RuntimeError("Port {} ble ikke åpnet.".format(portstreng))

        app.serieport = sp_ny
        print("Status: tilkoblet {}".format(portstreng))
        return True

    def min_koble_fra():
        sp = getattr(app, "serieport", None)
        if sp and sp.is_open:
            try:
                sp.close()
            except Exception:
                pass
        app.serieport = None
        print("Status: koblet fra")

    # PID-sender
    def min_pid_handler(settpunkt_mm, kp, ti, td, intbegr, start):
        HEADER = 0xAA
        TAIL   = 0x55

        sp = getattr(app, "serieport", None)
        if sp is None or not sp.is_open:
            print("Status: ikke tilkoblet – sender ikke PID")
            return

        try:
            pkt = struct.pack(
                "<BBHHHHHB",
                HEADER,
                int(start),
                int(kp),
                int(ti)   & 0xFFFF,
                int(td),
                int(intbegr) & 0xFFFF,
                int(settpunkt_mm) & 0xFFFF, 
                TAIL
            )
            sp.write(pkt)
            print(
                f"Sendt pakke (len={len(pkt)}): "
                f"H=0x{HEADER:02X}, Start={int(start)}, "
                f"Kp={int(kp)}, Ti={int(ti)}, Td={int(td)}, IntB={int(intbegr)}, SP={int(settpunkt_mm)}, "
                f"Tail=0x{TAIL:02X}"
            )
        except Exception as e:
            messagebox.showerror("Sendefeil", f"Kunne ikke sende PID-verdier: {e}")

    # Registrer i GUI
    app.registrer_tilkoblingshandler(min_koble_til, min_koble_fra)
    app.registrer_pid_callback(min_pid_handler)

    # Lukking
    def on_close():
        app.lukk()
        rot.destroy()

    rot.protocol("WM_DELETE_WINDOW", on_close)
    rot.mainloop()


if __name__ == "__main__":
    main()
