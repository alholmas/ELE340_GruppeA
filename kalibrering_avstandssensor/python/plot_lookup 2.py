#!/usr/bin/env python3
"""Plot lookup CSV (mV,distance_mm) and save as PNG.

Usage:
  python3 plot_lookup.py --csv lookup.csv --out lookup_plot.png

Requires: matplotlib, numpy, pandas (pandas optional; fallback to csv module)
"""

import argparse
import csv
import os
import sys

try:
    import matplotlib.pyplot as plt
    import numpy as np
except Exception:
    print('This script requires matplotlib and numpy. Install with: pip install matplotlib numpy', file=sys.stderr)
    sys.exit(1)


def read_csv(path):
    mvs = []
    dists = []
    with open(path, 'r', newline='') as f:
        reader = csv.reader(f)
        header = next(reader, None)
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


def main():
    parser = argparse.ArgumentParser(description='Plot lookup CSV (mV -> distance_mm)')
    parser.add_argument('--csv', default='lookup4.csv', help='Input CSV file path')
    parser.add_argument('--out', default='lookup_plot.png', help='Output PNG path')
    parser.add_argument('--dpi', type=int, default=150)
    args = parser.parse_args()

    if not os.path.exists(args.csv):
        print(f'CSV file not found: {args.csv}', file=sys.stderr)
        sys.exit(1)

    mvs, dists = read_csv(args.csv)
    if mvs.size == 0:
        print('No data found in CSV', file=sys.stderr)
        sys.exit(1)

    plt.figure(figsize=(10, 5))

    # Plot only points where both distance and mV are non-zero.
    # Plot contiguous non-zero segments separately so matplotlib does not connect across gaps.
    mask = (dists != 0) & (mvs != 0)
    if not np.any(mask):
        print('No non-zero data points to plot', file=sys.stderr)
        sys.exit(1)

    indices = np.where(mask)[0]
    # split into contiguous runs where diff == 1
    runs = np.split(indices, np.where(np.diff(indices) > 1)[0] + 1)
    first_segment = True
    for run in runs:
        if run.size > 0:
            if first_segment:
                plt.plot(dists[run], mvs[run], linestyle='-', color='black', linewidth=1, label='Linær approksimasjon')
                first_segment = False
            else:
                plt.plot(dists[run], mvs[run], linestyle='-', color='black', linewidth=1)

    # Highlight physical measured points (200,300,...,1000 mm)
    measured_dists = np.array(list(range(200, 1001, 100)))

    # For interpolation we need monotonic distance array. If it's decreasing, reverse.
    try:
        if np.all(np.diff(dists) <= 0):
            # decreasing -> reverse for interp
            dists_for_interp = dists[::-1]
            mvs_for_interp = mvs[::-1]
        elif np.all(np.diff(dists) >= 0):
            dists_for_interp = dists
            mvs_for_interp = mvs
        else:
            # not strictly monotonic: sort by distance for a usable mapping
            idx = np.argsort(dists)
            dists_for_interp = dists[idx]
            mvs_for_interp = mvs[idx]

        # find mV values corresponding to the measured distances using linear interpolation
        mvs_at_measured = np.interp(measured_dists, dists_for_interp, mvs_for_interp)

        # Plot measured points as red circles (distance on x, mV on y) and label them
        plt.scatter(measured_dists, mvs_at_measured, s=60, color='red', zorder=5, label='Fysiske målte punkter')
        for x, y in zip(measured_dists, mvs_at_measured):
            # annotate with distance in mm near the marker
            plt.annotate(f'{int(x)} mm', (x, y), textcoords='offset points', xytext=(6, 4), fontsize=8)
    except Exception:
        # Fallback: if anything goes wrong, skip measured overlay gracefully
        pass

    plt.xlabel('Avstand (mm)')
    plt.ylabel('mV')
    plt.title('Oppslagstabell')
    plt.grid(True, linestyle='--', alpha=0.4)
    plt.legend()

    # set x-axis ticks every 100 mm and start plot at first valid (non-zero) distance
    try:
        # consider only non-zero distances for tick range and axis limits
        nonzero_dists = dists[mask]
        max_dist = int(np.nanmax(nonzero_dists))
        min_dist_nonzero = int(np.nanmin(nonzero_dists))

        # start ticks at the nearest lower 100 to the smallest non-zero distance
        start_tick = (min_dist_nonzero // 100) * 100
        xticks = np.arange(start_tick, ((max_dist + 99) // 100) * 100 + 1, 100)
        plt.xticks(xticks)

        # set x-axis limits from smallest non-zero distance to the last tick (or max_dist)
        left = min_dist_nonzero
        right = int(xticks[-1]) if xticks.size > 0 else max_dist

        # If the physical 1000 mm point is part of the measured distances, ensure
        # there's a small margin after 1000 mm so the marker and its label are fully visible.
        if 1000 in measured_dists:
            right = max(right, 1000 + 50)

        # Add a small visual margin to the right (at least 10 mm or 3% of range)
        margin = max(10, int(0.03 * max(1, right - left)))
        right = right + margin

        # Recompute xticks so they include the expanded right limit
        xticks = np.arange(start_tick, ((right + 99) // 100) * 100 + 1, 100)
        plt.xticks(xticks)
        plt.xlim(left, right)
    except Exception:
        pass

    plt.tight_layout()
    plt.savefig(args.out, dpi=args.dpi)
    print(f'Saved plot to {args.out}')


if __name__ == '__main__':
    main()
