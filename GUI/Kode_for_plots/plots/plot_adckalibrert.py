#!/usr/bin/env python3
"""
En enkel plotting-script for `adckalibrert.csv`.
Bruk:
  python plot_adckalibrert.py            # åpner interaktiv plot
  python plot_adckalibrert.py -f path/to/file.csv -s out.png -w 5

Filplassering antas å være i samme mappe som scriptet hvis ikke angitt med full vei.
"""

import argparse
import os
import sys

try:
    import pandas as pd
except Exception:
    pd = None
import numpy as np
import matplotlib.pyplot as plt


def read_csv(path):
    # Prøv pandas først for robust lesing, fallback til numpy
    if pd is not None:
        df = pd.read_csv(path, header=None, names=["t", "adc"] )
        return df
    else:
        data = np.loadtxt(path, delimiter=",")
        return np.column_stack((data[:, 0], data[:, 1]))


def main():
    p = argparse.ArgumentParser(description="Plot adckalibrert.csv")
    p.add_argument("-f", "--file", default="adckalibrert.csv", help="CSV-fil (standard: adckalibrert.csv)")
    p.add_argument("-s", "--save", help="Lagre figur til fil (f.eks. out.png)")
    p.add_argument("-w", "--window", type=int, default=1, help="Glattingsvindu (samples) for moving average, 1 = ingen")
    args = p.parse_args()

    path = args.file
    if not os.path.isabs(path):
        # søk relativt til denne script-mappen
        path = os.path.join(os.path.dirname(__file__), path)

    if not os.path.exists(path):
        print(f"Filen finnes ikke: {path}")
        sys.exit(1)

    data = read_csv(path)
    if pd is not None and isinstance(data, pd.DataFrame):
        t = data['t'].astype(float).to_numpy()
        adc = data['adc'].astype(float).to_numpy()
    else:
        arr = np.asarray(data)
        t = arr[:, 0].astype(float)
        adc = arr[:, 1].astype(float)

    # Smoothe hvis ønsket
    if args.window and args.window > 1:
        win = args.window
        kernel = np.ones(win) / win
        adc_smooth = np.convolve(adc, kernel, mode='same')
    else:
        adc_smooth = adc

    fig, ax = plt.subplots(figsize=(10, 4))
    ax.plot(t, adc, label='ADC (raw)', alpha=0.4)
    if args.window and args.window > 1:
        ax.plot(t, adc_smooth, label=f'Glattet (win={args.window})', color='C1')
    else:
        ax.plot(t, adc_smooth, label='ADC', color='C1')

    ax.set_xlabel('Tid [s]')
    ax.set_ylabel('Avstand [mm]')
    ax.grid(alpha=0.3)
    ax.legend()
    plt.tight_layout()

    if args.save:
        fig.savefig(args.save, dpi=200)
        print(f"Lagret figur: {args.save}")

    plt.show()


if __name__ == '__main__':
    main()
