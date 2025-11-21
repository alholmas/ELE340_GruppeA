#!/usr/bin/env python3
"""Generell plot-skript.

Bruk: python3 generell_plot.py <csvfil> <pdffil> <label>

Eksempel: python3 generell_plot.py step10cm_inn1.csv resultat.pdf "Måling 1"

CSV-filen leter i underkatalogen `plots/` ved siden av dette skriptet.
Første kolonne plottes på x-aksen (tid) og andre kolonne på y-aksen (avstand i mm).
"""

import os
import sys
import argparse
import numpy as np

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def read_csv_two_columns(path):
	"""Les første to kolonner fra CSV-fil.

	Forsøker å bruke pandas hvis tilgjengelig, ellers bruker numpy.genfromtxt.
	Returnerer (x, y) som 1D numpy-arrays.
	"""
	try:
		import pandas as pd
	except Exception:
		pd = None

	if pd is not None:
		df = pd.read_csv(path, header=None)
		data = df.values
	else:
		data = np.genfromtxt(path, delimiter=",", dtype=float)

	if data is None:
		raise ValueError("Kunne ikke lese data fra CSV")

	# Sørg for 2D-array
	data = np.atleast_2d(data)

	if data.shape[1] < 2:
		raise ValueError("CSV må ha minst to kolonner (tid, avstand)")

	x = data[:, 0]
	y = data[:, 1]
	return x, y


def main():
	parser = argparse.ArgumentParser(
		description="Plotter første og andre kolonne fra CSV i ./plots og lagrer som PDF"
	)
	parser.add_argument("csvfile", help="Navn på CSV-fil i `plots/` (f.eks. step10cm_inn1.csv)")
	parser.add_argument("pdffile", help="Navn på PDF-utdata (f.eks. resultat.pdf)")
	parser.add_argument("label", help="Label for dataserien (vises i legend)")
	parser.add_argument("--xlabel", default="Tid", help="Label for x-aksen (default: Tid)")
	parser.add_argument("--ylabel", default="Avstand (mm)", help="Label for y-aksen (default: Avstand (mm))")
	parser.add_argument("--show", action="store_true", help="Vis plot i vindu (brukes kun om du kjører interaktivt)")
	args = parser.parse_args()

	script_dir = os.path.dirname(os.path.abspath(__file__))
	csv_path = os.path.join(script_dir, "plots", args.csvfile)
	if not os.path.isfile(csv_path):
		print(f"Feil: Fil ikke funnet: {csv_path}", file=sys.stderr)
		sys.exit(2)

	try:
		x, y = read_csv_two_columns(csv_path)
	except Exception as e:
		print(f"Feil ved lesing av CSV: {e}", file=sys.stderr)
		sys.exit(3)

	plt.figure()
	plt.plot(x, y, label=args.label)
	plt.xlabel(args.xlabel)
	plt.ylabel(args.ylabel)
	plt.legend()
	plt.grid(True)
	# Start x-aksen fra 0
	try:
		plt.xlim(left=0)
	except Exception:
		pass

	out_name = args.pdffile
	if not out_name.lower().endswith(".pdf"):
		out_name += ".pdf"

	# Lagre alltid i `fig/`-mappen ved siden av skriptet (opprett ved behov)
	fig_dir = os.path.join(script_dir, "fig")
	os.makedirs(fig_dir, exist_ok=True)
	# Bruk kun filnavnet brukeren oppgir og legg i fig-mappen
	out_basename = os.path.basename(out_name)
	out_path = os.path.join(fig_dir, out_basename)

	try:
		plt.savefig(out_path, format="pdf", bbox_inches="tight")
		print(f"Lagrer plot som: {out_path}")
	except Exception as e:
		print(f"Feil ved lagring av PDF: {e}", file=sys.stderr)
		sys.exit(4)

	if args.show:
		# Merk: bruker Agg backend som standard; --show vil fungere hvis du kjører interaktivt
		plt.show()


if __name__ == "__main__":
	main()

