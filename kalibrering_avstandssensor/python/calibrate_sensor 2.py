#!/usr/bin/env python3
"""
calibrate_sensor.py

Verktøy for å kalibrere en avstandssensor ved å samle målepunkter fra en
seriell enhet (eller tidligere innspilt data) og bygge en oppslagstabell som
mapper millivolt (mV) til avstand (mm).

Kort om oppførsel:
- Leser 4-bytes pakker der de første 2 byte er avstand (uint16) og de neste
    2 byte er målt spenning i millivolt (uint16).
- En pakke med fire nuller (0x00 0x00 0x00 0x00) avslutter ett kalibreringsløp.
- Skriptet kan samle flere løp, ta gjennomsnitt per avstand og deretter
    interpolere en oppslagstabell for mV i området 0..3000.

Eksempler:
    python3 calibrate_sensor.py --runs 1 --out lookup.csv --c-header lookup.h

Merk: Hvis enheten bruker big-endian eller et annet pakkeformat, juster
parsingen i funksjonen `parse_packet`.
"""

import argparse
import csv
import sys
import time
from collections import defaultdict
from typing import Dict, List, Tuple

try:
    import serial
    import serial.tools.list_ports as list_ports
    from serial.serialutil import SerialException
except Exception:
    serial = None
    list_ports = None
    SerialException = Exception


DISTANCES_EXPECTED = [150] + list(range(200, 1001, 100))  # 150,200,300,...,1000
BAUDRATE = 115200
EXTRAPOLATE_BUFFER_MV = 150  # allow extrapolation for this many mV above the maximum measured mv


def parse_packet(packet: bytes) -> Tuple[int, int]:
    """Parse a 4-byte packet -> (distance_mm, mv).

    Assumes little-endian uint16 for each field. Change 'little' to
    'big' if your device sends big-endian.
    """
    if len(packet) != 4:
        raise ValueError("packet must be 4 bytes")
    distance = int.from_bytes(packet[0:2], byteorder="little", signed=False)
    mv = int.from_bytes(packet[2:4], byteorder="little", signed=False)
    return distance, mv


def collect_runs_from_serial(port: str, baud: int, runs: int, timeout: float = 1.0) -> List[Dict[int, int]]:
    if serial is None:
        raise RuntimeError("pyserial not available; install with 'pip install pyserial'")

    try:
        ser = serial.Serial(port, baudrate=baud, timeout=timeout)
    except Exception as e:
        # Normalize serial open errors as SerialException
        raise SerialException(f"could not open port {port}: {e}") from e

    print(f"Opened serial {port}@{baud}")
    runs_collected: List[Dict[int, int]] = []

    try:
        current_run: Dict[int, int] = {}
        while len(runs_collected) < runs:
            chunk = ser.read(4)
            if not chunk:
                # timeout with no data; continue waiting
                continue

            if chunk == b"\x00\x00\x00\x00":
                # end of run
                if current_run:
                    print(f"Run {len(runs_collected)+1} complete with {len(current_run)} points")
                    runs_collected.append(current_run)
                    current_run = {}
                else:
                    # received terminator but no data yet; ignore
                    continue
                continue

            try:
                distance, mv = parse_packet(chunk)
            except Exception as e:
                print(f"Failed to parse packet {chunk!r}: {e}")
                continue

            # store (overwrite if duplicate)
            current_run[distance] = mv
            # print each registered measurement
            try:
                print(f"Mottatt: {mv} mV, {distance} mm")
            except Exception:
                # ensure printing never crashes the collection loop
                pass

    finally:
        ser.close()

    return runs_collected


def collect_runs_from_bytes_stream(data: bytes, runs: int) -> List[Dict[int, int]]:
    """Parse a bytes stream that contains sequences of 4-byte packets and
    4-zero terminators. Useful for simulated captures.
    """
    runs_collected: List[Dict[int, int]] = []
    current_run: Dict[int, int] = {}
    i = 0
    while i + 4 <= len(data) and len(runs_collected) < runs:
        packet = data[i : i + 4]
        i += 4
        if packet == b"\x00\x00\x00\x00":
            if current_run:
                runs_collected.append(current_run)
                current_run = {}
            continue
        d, mv = parse_packet(packet)
        current_run[d] = mv

    # if stream ended without final terminator, accept last run
    if current_run and len(runs_collected) < runs:
        runs_collected.append(current_run)

    return runs_collected


# Simulation removed: script now expects a real device or a capture file.


def list_serial_ports_fallback() -> List[str]:
    """Return a list of available serial port device strings.

    Uses pyserial's list_ports if available, otherwise falls back to
    globbing /dev/tty.* and /dev/cu.* on POSIX systems.
    """
    ports = []
    if list_ports is not None:
        try:
            ports = [p.device for p in list_ports.comports()]
        except Exception:
            ports = []
    if not ports:
        # fallback to OS listing
        import glob

        ports = glob.glob('/dev/tty.*') + glob.glob('/dev/cu.*')
    return sorted(ports)


def prompt_user_port_choice(ports: List[str]) -> str:
    """Present a numbered menu to the user to choose a serial port."""
    if not ports:
        return ""
    # show menu
    print("Select serial port to use:")
    for i, p in enumerate(ports, start=1):
        print(f"  {i}) {p}")
    while True:
        try:
            choice = input("Enter number (or 'q' to quit): ").strip()
        except EOFError:
            return ""
        if choice.lower() == 'q':
            return ""
        if not choice:
            continue
        if not choice.isdigit():
            print("Please enter a number from the list or 'q' to quit.")
            continue
        idx = int(choice) - 1
        if 0 <= idx < len(ports):
            return ports[idx]
        print("Selection out of range; try again.")


def average_runs(runs_list: List[Dict[int, int]]) -> Dict[int, int]:
    """Average multiple runs and return integer (rounded) mV per distance.

    Returns distance_mm -> avg_mv as integers (uint16-compatible).
    """
    sums = defaultdict(float)
    counts = defaultdict(int)
    for run in runs_list:
        for d, mv in run.items():
            sums[d] += mv
            counts[d] += 1
    # Round the averaged mV to the nearest integer so we have only integers
    averaged = {d: int(round(sums[d] / counts[d])) for d in sums}
    return averaged


def build_lookup(avg_map: Dict[int, int], mv_min: int = 0, mv_max: int = 3000) -> List[int]:
    """Build a lookup list of distances for every integer mV between mv_min and mv_max.

    avg_map: distance_mm -> avg_mv. We invert this to mv -> distance and then
    do piecewise linear interpolation across sorted mv points.
    Returns a list L where L[m] is the interpolated distance for m mV.
    """
    # invert avg_map -> list of (mv, distance)
    points = [(mv, d) for d, mv in avg_map.items()]
    if not points:
        raise ValueError("No calibration points available to build lookup")

    # sort by mv ascending
    points.sort(key=lambda x: x[0])
    mvs = [p[0] for p in points]
    dists = [p[1] for p in points]

    def interp(m: float) -> float:
        # No extrapolation below the minimum measured mV — return 0
        if m < mvs[0]:
            return 0.0

        # Allow small extrapolation above the maximum measured mV up to
        # EXTRAPOLATE_BUFFER_MV. Values above max + buffer return 0.
        if m > mvs[-1]:
            if m > (mvs[-1] + EXTRAPOLATE_BUFFER_MV):
                return 0.0
            # extrapolate from last segment
            if len(mvs) >= 2:
                x0, y0 = mvs[-2], dists[-2]
                x1, y1 = mvs[-1], dists[-1]
                if x1 == x0:
                    return float(y1)
                return y1 + (m - x1) * (y1 - y0) / (x1 - x0)
            else:
                # only one point available: use its distance for the buffer
                return float(dists[0])

        # If there's only one calibration point and m is within range, return that distance
        if len(mvs) == 1:
            return dists[0] if m == mvs[0] else 0.0

        # find containing segment and interpolate linearly
        for i in range(len(mvs) - 1):
            x0, x1 = mvs[i], mvs[i + 1]
            if x0 <= m <= x1:
                y0, y1 = dists[i], dists[i + 1]
                if x1 == x0:
                    return (y0 + y1) / 2.0
                return y0 + (m - x0) * (y1 - y0) / (x1 - x0)

        # fallback (shouldn't happen)
        return 0.0

    # Build lookup, clamp negative values to 0 and convert to uint16 (integers)
    raw = [interp(m) for m in range(mv_min, mv_max + 1)]
    lookup: List[int] = []
    for v in raw:
        v_clamped = max(0.0, v)
        v_int = int(round(v_clamped))
        # clamp to uint16 range just in case
        if v_int < 0:
            v_int = 0
        if v_int > 0xFFFF:
            v_int = 0xFFFF
        lookup.append(v_int)
    return lookup


def save_csv(lookup: List[int], out_path: str):
    with open(out_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["mV", "distance_mm"])
        for m, d in enumerate(lookup):
            writer.writerow([m, d])


def save_c_header(lookup: List[int], out_path: str, array_name: str = "lookup"):
    """Save the lookup as a C header with a uint16_t const array.

    The header will contain an include guard, <stdint.h>, a length macro
    and the static const uint16_t array initialized with the lookup values.
    """
    import os
    base = os.path.basename(out_path)
    # create a safe guard name from filename
    guard = os.path.splitext(base)[0].upper()
    guard = ''.join([c if c.isalnum() else '_' for c in guard]) + '_H'

    with open(out_path, 'w', newline='') as f:
        f.write(f"#ifndef {guard}\n")
        f.write(f"#define {guard}\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define {array_name.upper()}_LEN {len(lookup)}\n\n")
        f.write(f"static const uint16_t {array_name}[{array_name.upper()}_LEN] = {{\n")

        # write values, 16 per line
        per_line = 16
        for i in range(0, len(lookup), per_line):
            chunk = lookup[i:i+per_line]
            line = ', '.join(str(v) for v in chunk)
            if i + per_line < len(lookup):
                f.write(f"    {line},\n")
            else:
                f.write(f"    {line}\n")

        f.write("};\n\n")
        f.write(f"#endif /* {guard} */\n")


def main():
    parser = argparse.ArgumentParser(description="Calibrate distance sensor and build mV->distance lookup")
    parser.add_argument("--port", help="Serial port (e.g. /dev/cu.usbserial-XXXX). If omitted you'll get a menu to select one.")
    parser.add_argument("--runs", type=int, default=3, help="Number of calibration runs to collect")
    parser.add_argument("--out", default="lookup.csv", help="Output CSV file path (CSV)")
    parser.add_argument("--c-header", dest="c_header", help="Optional C header output path (e.g. lookup.h)")
    args = parser.parse_args()

    # interactive port selection if no --port provided
    if not args.port:
        ports = list_serial_ports_fallback()
        # only keep ports containing 'usbserial' in their name
        ports = [p for p in ports if 'usbserial' in p]
        if not ports:
            print("No serial ports found on the system. Plug in the device and try again.", file=sys.stderr)
            sys.exit(1)
        selected = prompt_user_port_choice(ports)
        if not selected:
            print("No port selected; exiting.", file=sys.stderr)
            sys.exit(1)
        args.port = selected

    try:
        runs = collect_runs_from_serial(args.port, BAUDRATE, args.runs)
    except SerialException as e:
        print(f"Failed to open serial port '{args.port}': {e}", file=sys.stderr)
        ports = list_serial_ports_fallback()
        ports = [p for p in ports if 'usbserial' in p]
        if ports:
            print("Available serial ports (filtered for 'usbserial'):")
            for p in ports:
                print(f"  {p}")
        else:
            print("No serial ports found on the system.")
        sys.exit(1)

    if not runs:
        print("No runs collected. Exiting.")
        sys.exit(1)

    print(f"Collected {len(runs)} runs")
    avg_map = average_runs(runs)
    print("Averaged calibration points (distance_mm -> avg_mV):")
    for d in sorted(avg_map.keys()):
        print(f"  {d} mm -> {avg_map[d]} mV")

    lookup = build_lookup(avg_map, mv_min=0, mv_max=3000)
    save_csv(lookup, args.out)
    print(f"Saved lookup CSV with {len(lookup)} rows to {args.out}")
    if getattr(args, 'c_header', None):
        try:
            save_c_header(lookup, args.c_header)
            print(f"Saved C header to {args.c_header}")
        except Exception as e:
            print(f"Failed to write C header {args.c_header}: {e}", file=sys.stderr)


if __name__ == "__main__":
    main()
