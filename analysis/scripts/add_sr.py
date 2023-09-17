import argparse
import uproot
import os
import glob
import json
import pandas as pd
from warnings import simplefilter
simplefilter(action="ignore", category=pd.errors.PerformanceWarning)
import numpy as np
import itertools
from tqdm import tqdm
from utils.analysis import PandasAnalysis
from utils.cutflow import Cutflow, Cut

cli = argparse.ArgumentParser(description="Add a signal region (SR) to cutflow")
cli.add_argument(
    "study", type=str,
    help="name of study"
)
cli.add_argument(
    "--tag", type=str, default="",
    help="tag for input/output"
)
cli.add_argument(
    "--SR", type=str, default="",
    help="signal region cuts"
)
args = cli.parse_args()

# Get babies
babies = sorted(glob.glob(f"studies/{args.study}/output_{args.tag}/Run2/inferences/*.root"))
sig_babies = [baby for baby in babies if "VBS" in baby]
bkg_babies = [baby for baby in babies if "VBS" not in baby and "data" not in baby]
data_babies = [baby for baby in babies if "data" in baby]
print("Signal:")
print("\n".join(sig_babies))
print("Background:")
print("\n".join(bkg_babies))
print("Data:")
print("\n".join(data_babies))

# Load VBS VVH analyis object
vbs = PandasAnalysis(
    sig_root_files=sig_babies,
    bkg_root_files=bkg_babies,
    data_root_files=data_babies,
    ttree_name="tree",
    weight_columns=[
        "xsec_sf", 
        "pu_sf",
        "prefire_sf"
    ]
)

# Add presel
vbs.df["objsel"] = True
vbs.df["presel"] = vbs.df.eval(
    "objsel and hbbfatjet_xbb > 0.5 and ld_vqqfatjet_xwqq > 0.3 and tr_vqqfatjet_xwqq > 0.3"
)
vbs.make_selection("presel")

# Re-normalize QCD
bkg_count = vbs.bkg_count()
qcd_count = vbs.sample_count("QCD")
data_count = vbs.data_count()
vbs.df.loc[vbs.df.name == "QCD", "event_weight"] *= (data_count - (bkg_count - qcd_count))/(qcd_count)

for name in vbs.df.name.unique():
    if name == "data":
        continue

    df = vbs.sample_df(name)

    cflow_file = f"studies/{args.study}/output_{args.tag}/Run2/{name}_cutflow.cflow"
    cutflow = Cutflow.from_file(cflow_file)

    # Divert cutflow to a new path of cuts
    base_cut = Cut(
        "AllMerged_DivertCutflow", 
        n_pass=0,
        n_pass_weighted=0,
        n_fail=cutflow["AllMerged_SavePDFWeights"].n_pass,
        n_fail_weighted=cutflow["AllMerged_SavePDFWeights"].n_pass,
    )
    cutflow.insert("AllMerged_SavePDFWeights", base_cut, direction="right")
    next_cut = cutflow[base_cut.name].right
    while not next_cut is None:
        zero_cut = Cut(
            next_cut.name,
            n_pass=0,
            n_pass_weighted=0,
            n_fail=0,
            n_fail_weighted=0,
        )
        cutflow.replace(next_cut.name, zero_cut)
        next_cut = cutflow[zero_cut.name].right

    # Add preselection
    n_pass_raw = len(df)
    n_pass_wgt = df.event_weight.sum()
    next_cut = Cut(
        "AllMerged_Preselection", 
        n_pass=n_pass_raw,
        n_pass_weighted=n_pass_wgt,
        n_fail=(base_cut.n_fail - n_pass_raw),
        n_fail_weighted=(base_cut.n_fail_weighted - n_pass_wgt),
    )
    cutflow.insert(base_cut.name, next_cut, direction="left")
    prev_cut = next_cut

    # Add SR
    df = df.query(args.SR)
    n_pass_raw = len(df)
    n_pass_wgt = df.event_weight.sum()
    next_cut = Cut(
        "AllMerged_SignalRegion", 
        n_pass=n_pass_raw,
        n_pass_weighted=n_pass_wgt,
        n_fail=(prev_cut.n_pass - n_pass_raw),
        n_fail_weighted=(prev_cut.n_pass_weighted - n_pass_wgt),
    )
    cutflow.insert(prev_cut.name, next_cut, direction="right")
    prev_cut = next_cut

    new_cflow_file = cflow_file.replace(".cflow", "_SR.cflow")
    cutflow.write_cflow(new_cflow_file)
    print(f"Wrote {new_cflow_file}")
