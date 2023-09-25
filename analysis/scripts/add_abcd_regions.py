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

cli = argparse.ArgumentParser(description="Add ABCD regions to cutflow")
cli.add_argument(
    "study", type=str,
    help="name of study"
)
cli.add_argument(
    "--tag", type=str, default="",
    help="tag for input/output"
)
cli.add_argument(
    "--SR_x", type=str, default="",
    help="signal regoin cut along 'x-axis'"
)
cli.add_argument(
    "--SR_y", type=str, default="",
    help="signal region cut along 'y-axis'"
)
cli.add_argument(
    "--SR_global", type=str, default="",
    help="global signal region cuts"
)
cli.add_argument(
    "--basedir", type=str, default="studies",
    help="Base directory for output"
)
args = cli.parse_args()

# Get babies
babies = sorted(glob.glob(f"{args.basedir}/{args.study}/output_{args.tag}/Run2/inferences/*.root"))
sig_babies = [baby for baby in babies if "VBS" in baby]
bkg_babies = [baby for baby in babies if "VBS" not in baby and "data.root" not in baby]
data_babies = [baby for baby in babies if "data.root" in baby]
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

# Define regions
vbs.df["regionA"] = f"({args.SR_x}) and ({args.SR_y}) and {args.SR_global}"
vbs.df["regionB"] = f"({args.SR_x}) and (not {args.SR_y}) and {args.SR_global}"
vbs.df["regionC"] = f"(not {args.SR_x}) and ({args.SR_y}) and {args.SR_global}"
vbs.df["regionD"] = f"(not {args.SR_x}) and (not {args.SR_y}) and {args.SR_global}"

# Re-normalize QCD
bkg_count = vbs.bkg_count()
qcd_count = vbs.sample_count("QCD")
data_count = vbs.data_count()
vbs.df.loc[vbs.df.name == "QCD", "event_weight"] *= (data_count - (bkg_count - qcd_count))/(qcd_count)

for name in vbs.df.name.unique():
    if name == "data":
        continue

    df = vbs.sample_df(name)

    cflow_file = f"{args.basedir}/{args.study}/output_{args.tag}/Run2/{name}_cutflow.cflow"
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
    presel_cut = Cut(
        "AllMerged_PreselectionDiverted", 
        n_pass=n_pass_raw,
        n_pass_weighted=n_pass_wgt,
        n_fail=(base_cut.n_fail - n_pass_raw),
        n_fail_weighted=(base_cut.n_fail_weighted - n_pass_wgt),
    )
    cutflow.insert(base_cut.name, presel_cut, direction="left")

    # Make global SR cuts
    df = df.query(args.SR_global)
    n_pass_raw = len(df)
    n_pass_wgt = df.event_weight.sum()
    global_cut = Cut(
        "AllMerged_SignalRegionGlobalCuts", 
        n_pass=n_pass_raw,
        n_pass_weighted=n_pass_wgt,
        n_fail=(presel_cut.n_pass - n_pass_raw),
        n_fail_weighted=(presel_cut.n_pass_weighted - n_pass_wgt),
    )
    cutflow.insert(presel_cut.name, global_cut, direction="right")

    # Make SR cut along "x-axis"
    x_df = df.query(args.SR_x)
    n_pass_raw = len(x_df)
    n_pass_wgt = x_df.event_weight.sum()
    x_cut = Cut(
        "AllMerged_SignalRegionXCut", 
        n_pass=n_pass_raw,
        n_pass_weighted=n_pass_wgt,
        n_fail=(global_cut.n_pass - n_pass_raw),
        n_fail_weighted=(global_cut.n_pass_weighted - n_pass_wgt),
    )
    cutflow.insert(global_cut.name, x_cut, direction="right")

    ynotx_df = df.query(f"({args.SR_y}) and not ({args.SR_x})")
    n_pass_raw = len(ynotx_df)
    n_pass_wgt = ynotx_df.event_weight.sum()
    ynotx_cut = Cut(
        "AllMerged_SignalRegionYNotXCut", 
        n_pass=n_pass_raw,
        n_pass_weighted=n_pass_wgt,
        n_fail=(x_cut.n_fail - n_pass_raw),
        n_fail_weighted=(x_cut.n_fail_weighted - n_pass_wgt),
    )
    cutflow.insert(x_cut.name, ynotx_cut, direction="left")

    C_dummy = Cut(
        "AllMerged_RegionC", 
        n_pass=n_pass_raw,
        n_pass_weighted=n_pass_wgt,
        n_fail=0,
        n_fail_weighted=0,
    )
    cutflow.insert(ynotx_cut.name, C_dummy, direction="right")

    D_dummy = Cut(
        "AllMerged_RegionD", 
        n_pass=(x_cut.n_fail - n_pass_raw),
        n_pass_weighted=(x_cut.n_fail_weighted - n_pass_wgt),
        n_fail=0,
        n_fail_weighted=0,
    )
    cutflow.insert(ynotx_cut.name, D_dummy, direction="left")

    # Make SR cut along "y-axis"
    yandx_df = x_df.query(args.SR_y)
    n_pass_raw = len(yandx_df)
    n_pass_wgt = yandx_df.event_weight.sum()
    yandx_cut = Cut(
        "AllMerged_SignalRegionYCut", 
        n_pass=n_pass_raw,
        n_pass_weighted=n_pass_wgt,
        n_fail=(x_cut.n_pass - n_pass_raw),
        n_fail_weighted=(x_cut.n_pass_weighted - n_pass_wgt),
    )
    cutflow.insert(x_cut.name, yandx_cut, direction="right")

    A_dummy = Cut(
        "AllMerged_RegionA", 
        n_pass=n_pass_raw,
        n_pass_weighted=n_pass_wgt,
        n_fail=0,
        n_fail_weighted=0,
    )
    cutflow.insert(yandx_cut.name, A_dummy, direction="right")

    B_dummy = Cut(
        "AllMerged_RegionB", 
        n_pass=(x_cut.n_pass - n_pass_raw),
        n_pass_weighted=(x_cut.n_pass_weighted - n_pass_wgt),
        n_fail=0,
        n_fail_weighted=0,
    )
    cutflow.insert(yandx_cut.name, B_dummy, direction="left")

    new_cflow_file = cflow_file.replace(".cflow", "_ABCD.cflow")
    cutflow.write_cflow(new_cflow_file)
    print(f"Wrote {new_cflow_file}")
