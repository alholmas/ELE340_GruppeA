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
    def min_pid_handler(settpunkt, kp, ki, kd):
        HEADER_VERDI = 0xAA
        sp = getattr(app, "serieport", None)
        if sp is None or not sp.is_open:
            print("Status: ikke tilkoblet – sender ikke PID")
            return

        try:
            # 32-bit: [8-bit header | 24-bit settpunkt] (MSB = header)
            sp_encoded = ((HEADER_VERDI & 0xFF) << 24) | (int(settpunkt) & 0xFFFFFF)

            # Pakk som USIGNERT 32-bit + tre SIGNERTE 32-bit
            pkt = struct.pack("<Iiii", sp_encoded, int(kp), int(ki), int(kd))
            sp.write(pkt)
            print("Sendt 16B pakke: header={}, SP={}, Kp={}, Ki={}, Kd={}".format(
                HEADER_VERDI, settpunkt, kp, ki, kd
            ))
        except Exception as e:
            messagebox.showerror("Sendefeil", "Kunne ikke sende PID-verdier: {}".format(e))

    # Registrer i GUI
    app.registrer_tilkoblingshandler(min_koble_til, min_koble_fra)
    app.registrer_pid_callback(min_pid_handler)

    # Lukking
    def on_close():
        app.lukk()   # lukk GUI, stopp tråder, lukk port
        rot.destroy()

    rot.protocol("WM_DELETE_WINDOW", on_close)
    rot.mainloop()


if __name__ == "__main__":
    main()
