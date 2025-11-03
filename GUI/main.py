import tkinter as tk
from tkinter import messagebox
import serial
import struct
from pid_gui import PIDGUI


def main():
    rot = tk.Tk()
    app = PIDGUI(rot)
    app.pack(fill=tk.BOTH, expand=True)

    # Serieport-tilkoblingshandler

    def min_koble_til(portstreng):
        # Lukk gammel port hvis åpen
        sp_gammel = getattr(app, "serieport", None)
        if sp_gammel is not None and sp_gammel.is_open:
            try:
                sp_gammel.close()
            except Exception:
                pass

        # Åpne ny
        try:
            sp_ny = serial.Serial(port=portstreng, baudrate=115200, timeout=1)
        except Exception as e:
            raise RuntimeError(f"Kunne ikke åpne {portstreng}: {e}") from e

        if not sp_ny.is_open:
            raise RuntimeError(f"Port {portstreng} ble ikke åpnet.")

        app.serieport = sp_ny
        print(f"Status: tilkoblet: {portstreng}")
        return True

    def min_koble_fra():
        sp = getattr(app, "serieport", None)
        if sp is not None:
            try:
                if sp.is_open:
                    sp.close()
            except Exception:
                pass
        app.serieport = None
        print("Status: koblet fra")

    def min_pid_handler(settpunkt, kp, ki, kd):
        # Sjekk tilkoblingsstatus før sending
        if not app._tilkoblet:
            print("Status: ikke tilkoblet – sender ikke PID")
            return
        sp = getattr(app, "serieport", None)
        if sp is None or not sp.is_open:
            print("Status: serieport ikke tilgjengelig/åpen – sender ikke PID")
            return
        try:
            # Header i MSB (69) + 24-bit payload for settpunkt
            header = 0xFF
            sp_encoded = ((header & 0xFF) << 24) | (int(settpunkt) & 0xFFFFFF)
            pkt = struct.pack("<iiii", sp_encoded, int(kp), int(ki), int(kd))
            sp.write(pkt)
            sp.flush()
            print(f"Status: sendt 16B pakke: header=69, SP={settpunkt}, Kp={kp}, Ki={ki}, Kd={kd}")
        except Exception as e:
            messagebox.showerror("Sendefeil", f"Kunne ikke sende PID-verdier: {e}")

    # Registrer handlers i GUI
    app.registrer_tilkoblingshandler(min_koble_til, min_koble_fra)
    app.registrer_pid_callback(min_pid_handler)

    # Ryddig lukking
    def on_close():
        app.lukk()
        rot.destroy()

    rot.protocol("WM_DELETE_WINDOW", on_close)
    rot.mainloop()


if __name__ == "__main__":
    main()