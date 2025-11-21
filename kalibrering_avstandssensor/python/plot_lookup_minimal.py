#!/usr/bin/env python3
"""Create a minimal PDF with white background and no axes.

This script plots the lookup curve but only where distance != 0,
drawing contiguous non-zero segments as a single black line.

Usage:
  python3 plot_lookup_minimal.py --csv lookup4.csv --out lookup4_minimal.pdf

Requires: matplotlib, numpy
"""

import argparse
import csv
import os
import sys

try:
    import numpy as np
    import matplotlib.pyplot as plt
except Exception:
    print('Requires numpy and matplotlib: pip install numpy matplotlib', file=sys.stderr)
    sys.exit(1)


def read_csv(path):
    mvs = []
    dists = []
    with open(path, 'r', newline='') as f:
        reader = csv.reader(f)
        _ = next(reader, None)
        for row in reader:
            if not row:
                continue
            try:
                mv = int(row[0])
                dist = int(row[1])
            except Exception:
                continue
            mvs.append(mv)
            dists.append(dist)
    return np.array(mvs), np.array(dists)


def contiguous_nonzero_segments(mask):
    """Return list of (start, end) index pairs for contiguous True regions in mask."""
    if not np.any(mask):
        return []
    idxs = np.where(mask)[0]
    segments = []
    start = idxs[0]
    prev = idxs[0]
    for i in idxs[1:]:
        if i == prev + 1:
            prev = i
            continue
        # break
        segments.append((start, prev))
        start = i
        prev = i
    segments.append((start, prev))
    return segments


def main():
    parser = argparse.ArgumentParser(description='Minimal PDF plot: white bg, no axes, black line for non-zero')
    parser.add_argument('--csv', default='lookup4.csv', help='Input CSV file')
    parser.add_argument('--out', default='lookup4_minimal.pdf', help='Output PDF file')
    parser.add_argument('--dpi', type=int, default=300)
    args = parser.parse_args()

    if not os.path.exists(args.csv):
        print(f'CSV not found: {args.csv}', file=sys.stderr)
        sys.exit(1)

    mvs, dists = read_csv(args.csv)
    if mvs.size == 0:
        print('No data in CSV', file=sys.stderr)
        sys.exit(1)

    mask = dists != 0
    segments = contiguous_nonzero_segments(mask)

    # Create white background figure and no axes
    fig = plt.figure(figsize=(10, 4), facecolor='white')
    ax = fig.add_subplot(111)
    ax.set_facecolor('white')

    # Plot each contiguous segment as a black line
    # Swap axes: distance on x-axis, mV on y-axis
    for (s, e) in segments:
        xs = dists[s:e+1]
        ys = mvs[s:e+1]
        ax.plot(xs, ys, color='black', linewidth=1)

    # Remove all axes, ticks, margins
    ax.set_axis_off()

    # Save as PDF with white background and no extra padding
    fig.savefig(args.out, dpi=args.dpi, facecolor='white', bbox_inches='tight', pad_inches=0)
    print(f'Saved minimal plot to {args.out}')


if __name__ == '__main__':
    main()
