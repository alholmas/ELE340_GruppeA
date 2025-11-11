import tkinter as tk
from tkinter import messagebox
import serial
import struct
from pid_gui import PIDGUI

def main():
    rot = tk.Tk()
    app = PIDGUI(rot)
    app.pack(fill=tk.BOTH, expand=True)

    # --- Tilkoblingshåndterere ---
    def min_koble_til(portstreng: str):
        sp_gammel = getattr(app, "serieport", None)
        if sp_gammel and sp_gammel.is_open:
            try:
                sp_gammel.close()
            except Exception:
                pass

        try:
            sp_ny = serial.Serial(port=portstreng, baudrate=115200, timeout=1)
        except Exception as e:
            raise RuntimeError("Kunne ikke åpne {}: {}".format(portstreng, e)) from e

        if not sp_ny.is_open:
            raise RuntimeError("Port {} ble ikke åpnet.".format(portstreng))

        app.serieport = sp_ny
        print("Status: tilkoblet {}".format(portstreng))
        return True

    # PID-sender
    def min_pid_handler(settpunkt_mm, kp, ki, kd, intbegr, start):
        HEADER = 0xAA
        TAIL   = 0x55

        sp = getattr(app, "serieport", None)
        if sp is None or not sp.is_open:
            print("Status: ikke tilkoblet – sender ikke PID")
            return

        try:
            # Pakkeformat: HEADER(1) + start(1) + Kp(2) + Ti(2) + Td(2) + IntLimit(4) + SP(2) + TAIL(1) = 15 bytes
            pkt = struct.pack("<BBHHHLHB", HEADER, start, kp, ki, kd, intbegr, settpunkt_mm, TAIL)
            sp.write(pkt)
            print(
                f"Sendt pakke (len={len(pkt)}): "
                f"H=0x{HEADER:02X}, Start={start}, "
                f"Kp={kp}, Ti={ki}, Td={kd}, IntB={intbegr}, SP={settpunkt_mm}, "
                f"Tail=0x{TAIL:02X}"
            )
        except Exception as e:
            messagebox.showerror("Sendefeil", f"Kunne ikke sende PID-verdier: {e}")

    # Registrer i GUI
    app.registrer_tilkoblingshandler(min_koble_til)
    app.registrer_pid_callback(min_pid_handler)

    # Lukking
    def on_close():
        app.lukk()
        rot.destroy()

    rot.protocol("WM_DELETE_WINDOW", on_close)
    rot.mainloop()


if __name__ == "__main__":
    main()
