import os
import glob
import argparse
# Columnar I/O
import uproot
import pandas as pd
import warnings
warnings.simplefilter(action="ignore", category=pd.errors.PerformanceWarning)
import numpy as np
# Plotting
import matplotlib.pyplot as plt
from matplotlib import gridspec
import yahist
import mplhep as hep
plt.rcParams.update({"figure.facecolor":  (1,1,1,0)})
# Other
import itertools
import pickle
# Custom
from utils.plotter import PandasPlotter

def opt_plots(plotter, plots_dir):
    print(f"Writing optimization plots to {plots_dir}")
    plotter.plots_dir = plots_dir

    # --- Plots after Preselection ---
    nbins = 50
    col_bins_label_tuples = [
        ("HT_fat", np.linspace(0, 5000, nbins+1), r"AK8 $H_{T}$ [GeV]"), 
        ("M_jj", np.linspace(0, 5000, nbins+1), r"$M_{jj}$ [GeV]"), 
        ("abs_deta_jj", np.linspace(0, 10, nbins+1), r"$|\Delta\eta_{jj}|$"), 
        ("hbbfatjet_pt", np.linspace(0, 2000, nbins+1), r"$p_T(H\rightarrow bb)$ [GeV]"), 
        ("hbbfatjet_mass", np.linspace(0, 300, nbins+1), r"$M_{PNet}(H\rightarrow bb)$ [GeV]"), 
        ("ld_vqqfatjet_pt", np.linspace(0, 2000, nbins+1), r"$p_T(lead V\rightarrow qq)$ [GeV]"), 
        ("ld_vqqfatjet_mass", np.linspace(0, 300, nbins+1), r"$M_{PNet}(lead V\rightarrow qq)$ [GeV]"), 
        ("tr_vqqfatjet_pt", np.linspace(0, 2000, nbins+1), r"$p_T(trail V\rightarrow qq)$ [GeV]"), 
        ("tr_vqqfatjet_mass", np.linspace(0, 300, nbins+1), r"$M_{PNet}(trail V\rightarrow qq)$ [GeV]"), 
        ("abcdnet_score", np.linspace(0, 1, nbins+1), r"ABCDNet score"), 
    ]
    plotter.plot_many_sig_vs_bkg(
        col_bins_label_tuples, selection="presel", legend_ncol=2
    )
    # --------------------------------

    # --- Plots after ParticleNet cuts ---
    # Save original weights
    orig_event_weight = plotter.df.event_weight.values.copy()
    # Apply ParticleNet scale factors
    plotter.df.event_weight *= plotter.df.xbb_sf*plotter.df.xwqq_ld_vqq_sf*plotter.df.xwqq_tr_vqq_sf

    # Plot correlation plots in ABCD regions
    plotter.plot_correlation2D(
        "abcdnet_score", "abs_deta_jj", 
        np.linspace(0, 1, 11), np.linspace(0, 8, 11),
        selection="presel and hbbfatjet_xbb > 0.8 and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7",
        flip=False,
        x_label=r"ABCDNet score",
        y_label=r"$|\Delta\eta_{jj}|$"
    )
    plotter.plot_correlation2D(
        "abcdnet_score", "abs_deta_jj", 
        np.linspace(0, 1, 11), np.linspace(0, 8, 11),
        selection="presel and hbbfatjet_xbb > 0.8 and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7",
        flip=True,
        x_label=r"ABCDNet score",
        y_label=r"$|\Delta\eta_{jj}|$"
    )

    # Plot ROC curve
    plotter.plot_roc_curve(
        "abcdnet_score", 
        selection="presel and hbbfatjet_xbb > 0.8 and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7 and abs_deta_jj > 5",
        name="ABCDNet", signal_tests=[3, 2, 1]
    )

    # Reset event weights
    plotter.df.event_weight = orig_event_weight
    # ------------------------------------

def val_plots(plotter, plots_dir):
    print(f"Writing validation plots to {plots_dir}")
    plotter.plots_dir = plots_dir

    nbins = 50
    col_bins_label_tuples = [
        # Fat jet ParticleNet score
        ("hbbfatjet_xbb", np.linspace(0, 1, nbins+1), r"PNet $X\rightarrow bb(H\rightarrow bb)$"),
        ("ld_vqqfatjet_xwqq", np.linspace(0, 1, nbins+1), r"PNet $XW\rightarrow qq($ld $V\rightarrow qq)$"),
        ("tr_vqqfatjet_xwqq", np.linspace(0, 1, nbins+1), r"PNet $XW\rightarrow qq($tr $V\rightarrow qq)$"),
        # Event-level variables
        ("ST", np.linspace(0, 5000, nbins+1), r"$S_T$ [GeV]"),
        ("HT", np.linspace(0, 1500, nbins+1), r"$H_{T, AK4}$ [GeV]"),
        ("HT_fat", np.linspace(0, 5000, nbins+1), r"$H_{T, AK8}$ [GeV]"),
        ("MET", np.linspace(0, 500, nbins+1), "MET [GeV]"),
        ("n_jets", np.linspace(0, 10, 11), r"$N_{AK4}$"),
        ("n_fatjets", np.linspace(0, 10, 11), r"$N_{AK8}$"),
        # VBS variables
        ("M_jj", np.linspace(0, 2000, nbins+1), r"$M_{jj}$ [GeV]"),
        ("abs_deta_jj", np.linspace(0, 10, nbins+1), r"$|\Delta\eta_{jj}|$"),
        ("ld_vbsjet_pt", np.linspace(0, 500, nbins+1), r"$p_T($ld VBS$)$ [GeV]"),
        ("tr_vbsjet_pt", np.linspace(0, 500, nbins+1), r"$p_T($tr VBS$)$ [GeV]"),
        ("ld_vbsjet_eta", np.linspace(-6, 6, nbins+1), r"$\eta($ld VBS$)$"),
        ("tr_vbsjet_eta", np.linspace(-6, 6, nbins+1), r"$\eta($tr VBS$)$"),
        # VVH variables
        ("M_VVH", np.linspace(1000, 5000, nbins+1), r"$M_{VVH}$ [GeV]"),
        # Fat jet MSD
        ("hbbfatjet_msoftdrop", np.linspace(0, 500, nbins+1), r"$M_{SD}(H\rightarrow bb)$ [GeV]"),
        ("ld_vqqfatjet_msoftdrop", np.linspace(0, 500, nbins+1), r"$M_{SD}($ld $V\rightarrow qq)$ [GeV]"),
        ("tr_vqqfatjet_msoftdrop", np.linspace(0, 500, nbins+1), r"$M_{SD}($tr $V\rightarrow qq)$ [GeV]"),
        # Fat jet ParticleNet mass
        ("hbbfatjet_mass", np.linspace(0, 500, nbins+1), r"$M_{PNet}(H\rightarrow bb)$ [GeV]"),
        ("ld_vqqfatjet_mass", np.linspace(0, 500, nbins+1), r"$M_{PNet}($ld $V\rightarrow qq)$ [GeV]"),
        ("tr_vqqfatjet_mass", np.linspace(0, 500, nbins+1), r"$M_{PNet}($tr $V\rightarrow qq)$ [GeV]"),
        # Fat jet pt
        ("hbbfatjet_pt", np.linspace(200, 1700, nbins+1), r"$p_T(H\rightarrow bb)$ [GeV]"),
        ("ld_vqqfatjet_pt", np.linspace(200, 1700, nbins+1), r"$p_T($ld $V\rightarrow qq)$ [GeV]"),
        ("tr_vqqfatjet_pt", np.linspace(200, 1700, nbins+1), r"$p_T($tr $V\rightarrow qq)$ [GeV]"),
        # Fat jet eta
        ("hbbfatjet_eta", np.linspace(-2.5, 2.5, nbins+1), r"$\eta(H\rightarrow bb)$"),
        ("ld_vqqfatjet_eta", np.linspace(-2.5, 2.5, nbins+1), r"$\eta($ld $V\rightarrow qq)$"),
        ("tr_vqqfatjet_eta", np.linspace(-2.5, 2.5, nbins+1), r"$\eta($tr $V\rightarrow qq)$"),
    ]
    plotter.plot_many_data_vs_mc(
        col_bins_label_tuples, selection="objsel", 
        legend_loc="upper right", legend_ncol=2, autoblind=True
    )

    plotter.plot_data_vs_mc(
        "abcdnet_score", np.linspace(0, 1, 51), selection="presel", 
        x_label="ABCDNet score", logy=True, legend_loc="upper right", legend_ncol=2,
        blinded_cut="abcdnet_score <= 0.89", sig_scale=1
    )

def extra_plots(plotter, plots_dir):
    print(f"Writing extra plots to {plots_dir}")
    # TODO: Add stuff here
    return

if __name__ == "__main__":
    cli = argparse.ArgumentParser(description="Run a given study in parallel")
    cli.add_argument("tag", type=str, help="Tag of the study output (studies/vbsvvhjets/output_{TAG})")
    cli.add_argument("--opt", action="store_true", help="Make optimization plots")
    cli.add_argument("--val", action="store_true", help="Make validation plots")
    cli.add_argument("--extra", action="store_true", help="Make extra plots")
    args = cli.parse_args()

    baby_dir = f"/data/userdata/{os.getenv('USER')}/vbs_studies/vbsvvhjets/output_{args.tag}"
    plot_dir = f"/home/users/{os.getenv('USER')}/public_html/vbsvvhjets_plots/{args.tag}"

    # Collect babies
    babies = sorted(glob.glob(f"{baby_dir}/Run2/*.root"))
    sig_babies = []
    bkg_babies = []
    data_babies = []
    for baby_path in babies:
        baby_name = baby_path.split("/")[-1]
        if "VBSVVH" in baby_name:
            sig_babies.append(baby_path)
        elif "VBS" not in baby_name and "Private" not in baby_name:
            if "data" in baby_name:
                data_babies.append(baby_path)
            else:
                bkg_babies.append(baby_path)
    print("Signal:")
    print("\n".join(sig_babies))
    print("Background:")
    print("\n".join(bkg_babies))
    print("Data:")
    print("\n".join(data_babies))
    if not babies:
        raise Exception(f"No babies found in {baby_dir}/Run2")

    # Initialize plotter (loads babies into pandas dfs + methods for plotting)
    plotter = PandasPlotter(
        sig_root_files=sig_babies,
        bkg_root_files=bkg_babies,
        data_root_files=data_babies,
        ttree_name="tree",
        weight_columns=[
            "xsec_sf", 
            "pu_sf", 
            "prefire_sf", 
        ],
        sample_labels = {
            "VBSVVH": r"VBS VVH $(C_{2V} = 2)$", 
            "TTHad": r"$t\bar{t}+$jets",
            "TT1L": r"$t\bar{t}+1\ell$",
            "QCD": "QCD",
            "SingleTop": r"Single $t$",
            "TTH": r"$t\bar{t}+H$",
            "TTW": r"$t\bar{t}+W$",
        },
    )

    # Define preselection
    plotter.df["objsel"] = True
    plotter.df["presel"] = plotter.df.eval(
        "objsel and hbbfatjet_xbb > 0.5 and ld_vqqfatjet_xwqq > 0.3 and tr_vqqfatjet_xwqq > 0.3"
    )

    # Renormalize QCD to (data - nonQCD) because of resampling
    bkg_count = plotter.bkg_count()
    qcd_count = plotter.sample_count("QCD")
    data_count = plotter.data_count()
    plotter.df.loc[plotter.df.name == "QCD", "event_weight"] *= (data_count - (bkg_count - qcd_count))/(qcd_count)

    # Get reweights
    with uproot.open(f"{baby_dir}/Run2/VBSVVH.root") as f:
        reweights = np.stack(f["rwgt_tree"].arrays(library="np")["reweights"])
        reweights = np.insert(reweights, 28, 1, axis=1) # insert reweight = 1 for central value (C2V = 2)
        n_events, n_reweights = reweights.shape
    # Get reweight names
    with open("data/VBSVVH_reweights.txt", "r") as f_in:
        reweight_names = f_in.read().splitlines()
    # Add reweights to plotter df for certain C2V values
    for C2V in [1.3, 1.5, 2.0]:
        C2V_str = str(C2V).replace(".", "p").replace("-", "m")
        plotter.df[f"reweights_{C2V_str}"] = 1
        print(f"Adding 'reweights_{C2V_str}' to plotter dataframe for signal")
        reweight_name = f"scan_CV_1p0_C2V_{C2V_str}_C3_1p0"
        if reweight_name in reweight_names:
            reweight_i = reweight_names.index(reweight_name)
            plotter.df.loc[plotter.df.is_signal & plotter.df.presel, f"reweights_{C2V_str}"] = reweights.T[reweight_i]
        else:
            print(f"WARNING: reweight {reweight_name} not found, set to 1")

    # Make plots
    plot_all = not (args.opt or args.val or args.extra)
    if plot_all or args.opt:
        opt_plots(plotter, f"{plot_dir}/opt")
    if plot_all or args.val:
        val_plots(plotter, f"{plot_dir}/val")
    if plot_all or args.extra:
        extra_plots(plotter, f"{plot_dir}/extra")
