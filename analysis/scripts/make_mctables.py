#!/usr/bin/env python3
# -*- coding: utf-8 -*
import json
import re
import argparse
from data.samples.vbswh_mc import nanoaodv9_bkg

BKG_SAMPLE_MAP = {
    "SingleTop": ["ST.*"],
    "TTbar1L":  ["TTToSemiLep.*"],
    "TTbar2L":  ["TTTo2L.*"],
    "TTX":  ["ttH.*", "TTW.*", "TTZ.*", "TTbb.*", "TTToHadronic.*"],
    "WJets":  ["WJets.*"],
    "Bosons":  ["DY.*", "WW.*", "WZ.*", "ZZ.*", "EWK.*?(WToQQ|ZTo).*"],
    "EWKWLep":  ["EWKW.*?(WToLNu).*"],
    "VH":  ["^.+?HToBB.*", "VHToNonbb_M125.*"]
}

TEMPLATE="""\\begin{table}[H]
\\begin{center}
\\scriptsize
\\begin{tabular}{|c|l|c|}
\\hline
Process  & Sample Name & $\sigma$ [pb] \\\\
\\hline
\\hline
ROWS_TO_REPLACE
\\end{tabular}
\\end{center}
\\scriptsize{
FOOTNOTES_TO_REPLACE
}
\\caption{
  CAPTION_TO_REPLACE
}
\\label{LABEL_TO_REPLACE}
\\end{table}
"""

with open("data/xsecs.json", "r") as f_in:
    XSECS = json.load(f_in)

with open("data/wjets_ht-binned_xsecs.json", "r") as f_in:
    WJETS_XSECS = json.load(f_in)

def get_xsec(file_name):
    if not XSECS:
        return 1
    matched_keys = []
    matched_chars = []
    for key, xsec in XSECS.items():
        if key in file_name:
            matched_keys.append(key)
            matched_chars.append(len(key))
    if len(matched_keys) > 0:
        best_match = matched_keys[matched_chars.index(max(matched_chars))]
        xsec = XSECS[best_match]
        if xsec == 0:
            return "\\textcolor{red}{MISSING}"
        else:
            return xsec
    else:
        print(f"ERROR: no xsec for {file_name}")
        return 1

def get_year(input_file):
    if "HIPM_UL2016" in input_file or "UL16NanoAODAPVv9" in input_file or "RunIISummer20UL16APV" in input_file:
        return "2016preVFP"
    elif "UL2016" in input_file or "UL16NanoAODv9" in input_file or "RunIISummer20UL16" in input_file:
        return "2016postVFP"
    elif "UL2017" in input_file or "UL17NanoAODv9" in input_file or "RunIISummer20UL17" in input_file:
        return "2017"
    elif "UL2018" in input_file or "UL18NanoAODv9" in input_file or "RunIISummer20UL18" in input_file:
        return "2018"
    else:
        raise Exception(f"No year found in {input_file}")

def make_mctable(table_year):

    suffixes = []
    all_rows = {k: [] for k in BKG_SAMPLE_MAP.keys()}
    for sample in nanoaodv9_bkg:
        name = sample.get_datasetname()
        year = get_year(name)

        if year != table_year:
            continue

        _, prefix, suffix, _ = name.split("/")

        if "WJetsToLNu_HT-" in prefix:
            xsec = WJETS_XSECS[year][prefix.split("_Tune")[0]]
        else:
            xsec = get_xsec(name)

        if suffix not in suffixes:
            suffixes.append(suffix)

        nickname = ""
        for short_name, regexes in BKG_SAMPLE_MAP.items():
            for regex in regexes:
            # for regex in regexes:
                is_match = bool(re.match(regex, prefix))
                if is_match:
                    nickname = short_name
                    break
            if nickname:
                break
        if not nickname:
            raise Exception(f"no nickname found for {name}")

        row = f"& /{prefix}$^{{{suffixes.index(suffix)}}}$ & {xsec} \\\\"
        row = row.replace("_", "\\_")
        all_rows[nickname].append(row)

    table = str(TEMPLATE)
    rows_text = []
    for nickname, rows in all_rows.items():
        rows_text.append(f"{nickname} {rows[0]}")
        for row in rows[1:]:
            rows_text.append(f"{row}")
        rows_text.append("\hline")
    table = table.replace("ROWS_TO_REPLACE", "\n".join(rows_text))
    
    suffix_text = []
    for suffix_i, suffix in enumerate(suffixes):
        suffix = suffix.replace("_", "\\_")
        suffix_text.append(f"  $^{{{suffix_i}}}$ /{suffix}/NANOAODSIM \\\\")
    table = table.replace("FOOTNOTES_TO_REPLACE", "\n".join(suffix_text))
    table = table.replace(
        "CAPTION_TO_REPLACE",
        f"  Background MC samples used in this analysis, corresponding to {table_year} detector conditions (UL), with their respective cross sections in picobarns. \nThe cross sections for the \HT-binned W+jets samples are scaled by a ``stitching'' factor such that the samples together fill a continuous \HT distribution."
    )
    table = table.replace("LABEL_TO_REPLACE", f"tab:samples{table_year}")
    return table

if __name__ == "__main__":
    cli = argparse.ArgumentParser(description="Make a table of MC samples")
    cli.add_argument(
        "--stdout", action="store_true",
        help="Dump latex to stdout (writes to samples.tex otherwise)"
    )
    args = cli.parse_args()

    years = ["2016preVFP", "2016postVFP", "2017", "2018"]
    latex = "\n".join([make_mctable(year) for year in years])

    if args.stdout:
        print(latex)
    else:
        with open("samples.tex", "w") as tex_file:
            tex_file.write(latex)
