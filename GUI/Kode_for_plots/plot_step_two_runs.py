#!/usr/bin/env python3
"""Plotter to kjøringer fra en CSV og lagrer som PDF i `fig/`.

Bruk: python3 plot_step_two_runs.py <csvfile> [pdffile]

CSV ligger i `plots/` under skriptmappen. Første kolonne: tid, andre kolonne: avstand (mm).
Skal finne to kjøringer (split når tid < forrige tid), merke den økende som "kjøring inn"
og den minkende som "kjøring ut". x-aksen starter i 0. PDF lagres i `fig/`.
"""

import os
import sys
import argparse
import numpy as np

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def read_csv(path):
    data = np.genfromtxt(path, delimiter=",", dtype=float)
    if data is None:
        raise ValueError("Kunne ikke lese CSV")
    data = np.atleast_2d(data)
    if data.shape[1] < 2:
        raise ValueError("CSV må ha minst to kolonner: tid, avstand")
    return data[:, 0], data[:, 1]


def split_runs_by_time_reset(x, y):
    # Finn indekser der tid går tilbakke (når en ny kjøring starter)
    resets = np.where(np.diff(x) < 0)[0]
    if len(resets) == 0:
        # Ingen opplagt reset; prøv å se om vi har eksakt to blokker ved gjentatte tid-verdier
        # Hvis det finnes duplikate tid-verdier som forekommer to ganger, prøv å reshapedata
        unique, counts = np.unique(x, return_counts=True)
        if np.all(counts == 2):
            # data er sannsynlig interleaved: ta hver andre rad for en kjøring
            idx1 = np.arange(0, len(x), 2)
            idx2 = np.arange(1, len(x), 2)
            return (x[idx1], y[idx1]), (x[idx2], y[idx2])
        else:
            # Fallback: anta hele data er én kjøring
            return (x, y), None

    # Splitt ved første reset
    splits = np.split(np.arange(len(x)), resets + 1)
    runs = []
    for idx in splits:
        if len(idx) > 0:
            runs.append((x[idx], y[idx]))
    return tuple(runs)


def trend_label(x, y):
    # Enkel trend: lineær trend mellom start og slutt
    if len(x) < 2:
        return 0.0
    try:
        coeff = np.polyfit(x, y, 1)
        return coeff[0]
    except Exception:
        return y[-1] - y[0]


def main():
    parser = argparse.ArgumentParser(description="Plot two runs from CSV and save PDF to fig/")
    parser.add_argument("csvfile", help="CSV-filnavn i `plots/`")
    parser.add_argument("pdffile", nargs="?", default="step_inn_ut.pdf", help="Utdata PDF-navn (lagres i `fig/`)")
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    csv_path = os.path.join(script_dir, "plots", args.csvfile)
    if not os.path.isfile(csv_path):
        print(f"Finner ikke fil: {csv_path}", file=sys.stderr)
        sys.exit(2)

    x, y = read_csv(csv_path)

    runs = split_runs_by_time_reset(x, y)
    if runs is None:
        print("Fant ikke to kjøringer i filen.", file=sys.stderr)
        sys.exit(3)

    # Hvis bare én run funnet, abort
    if len(runs) < 2:
        print("Var kun én kjøring funnet. Forventer to kjøringer.", file=sys.stderr)
        sys.exit(4)

    (x1, y1), (x2, y2) = runs[0], runs[1]

    # Bestem hvilken som er 'inn' (øker) og 'ut' (minker)
    slope1 = trend_label(x1, y1)
    slope2 = trend_label(x2, y2)

    if slope1 > slope2:
        label1 = "kjøring inn" if slope1 < 0 else "kjøring ut"
        label2 = "kjøring ut" if label1 == "kjøring inn" else "kjøring inn"
    else:
        label2 = "kjøring inn" if slope2 < 0 else "kjøring ut"
        label1 = "kjøring ut" if label2 == "kjøring inn" else "kjøring inn"

    # Plot
    plt.figure()
    plt.plot(x1, y1, label=label1)
    plt.plot(x2, y2, label=label2)
    plt.xlabel("Tid")
    plt.ylabel("Avstand (mm)")
    plt.legend()
    plt.grid(True)
    plt.xlim(left=0)

    fig_dir = os.path.join(script_dir, "fig")
    os.makedirs(fig_dir, exist_ok=True)

    out_name = args.pdffile
    if not out_name.lower().endswith(".pdf"):
        out_name += ".pdf"
    out_path = os.path.join(fig_dir, out_name)

    try:
        plt.savefig(out_path, format="pdf", bbox_inches="tight")
        print(f"Lagrer plot som: {out_path}")
    except Exception as e:
        print(f"Feil ved lagring: {e}", file=sys.stderr)
        sys.exit(5)


if __name__ == "__main__":
    main()
