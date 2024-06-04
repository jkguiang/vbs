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
import matplotlib.patches as patches
from matplotlib import gridspec
import yahist
import mplhep as hep
plt.rcParams.update({"figure.facecolor":  (1,1,1,0)})
# Other
import itertools
import pickle
# Custom
from utils.plotter import PandasPlotter

def plot_abcd_cartoon(x_arm, y_arm, presel_cuts=None, y_lim=[0, 1], y_label=""):
    
    fig, axes = plt.subplots()

    axes.set_ylim(y_lim)

    # Move the left and bottom spines to x = 0 and y = 0, respectively.
    axes.spines[["left", "bottom"]].set_position(("data", 0))
    # Hide the top and right spines.
    axes.spines[["top", "right"]].set_visible(False)
    axes.spines[["left", "bottom"]].set_linewidth(8)

    axes.plot(1, 0, ">k", markersize=30, clip_on=False)
    axes.plot(*y_lim, "^k", markersize=30, clip_on=False)

    x_wp = float(x_arm.split(" > ")[-1])
    y_wp = float(y_arm.split(" > ")[-1])
    
    axes.axhline(y_wp, color="k", linewidth=8)
    axes.axvline(x_wp, color="k", linewidth=8)

    axes.set_xlabel("ABCDNet", weight="bold", size=48)
    axes.set_ylabel(y_label, weight="bold", size=48)

    centered = {"horizontalalignment": "center", "verticalalignment": "center", "size": 48, "weight": "bold"}

    axes.text((1 + x_wp)/2, (y_lim[-1] + y_wp)/2, "A", **centered)
    axes.text((1 + x_wp)/2, (y_wp + 0)/2, "B", **centered)
    axes.text((0 + x_wp)/2, (y_lim[-1] + y_wp)/2, "C", **centered)
    axes.text((x_wp + 0)/2, (y_wp + 0)/2, "D", **centered)

    axes.set_xticks([x_wp])
    axes.set_yticks([y_wp])
    
    # Remove all margins
    axes.margins(0)
    # Remove CMS-style ticks on top and right sides of plot
    axes.tick_params(top=False, right=False, labelsize=48, pad=12)
    axes.tick_params(top=False, right=False, which="minor")
    
    axes.patch.set_alpha(0)
    
    # Add title with preselection
    if not presel_cuts is None:
        title = " and ".join(presel_cuts)
        # Hard-coded replacements
        title = title.replace("ld_vqqfatjet_xwqq", r"XWqq(ld $V\rightarrow qq$)")
        title = title.replace("tr_vqqfatjet_xwqq", r"XWqq(tr $V\rightarrow qq$)")
        title = title.replace("hbbfatjet_xbb", r"Xbb($H\rightarrow bb$)")
        # Hard-coded length limit (make newline)
        if len(title) > 100:
            title = title.split(" and ")
            title[-1] = "\n" + title[-1]
            title = " and ".join(title)
        
        axes.set_title(title, size=28, pad=28)
    

def plot_abcd(x_arm, y_arm, x_mid=None, y_mid=None, y_lim=[0, 1], y_label="", presel_cuts=None, plots_dir=None):
    
    plot_x_mid = (not x_mid is None)
    plot_xy_mid = (plot_x_mid and not y_mid is None)
    
    fig, axes = plt.subplots()

    axes.set_ylim(y_lim)

    # Move the left and bottom spines to x = 0 and y = 0, respectively.
    axes.spines[["left", "bottom"]].set_position(("data", 0))
    # Hide the top and right spines.
    axes.spines[["top", "right"]].set_visible(False)
    axes.spines[["left", "bottom"]].set_linewidth(4)

    axes.plot(1, 0, ">k", markersize=15, clip_on=False)
    axes.plot(*y_lim, "^k", markersize=15, clip_on=False)

    x_wp = float(x_arm.split(" > ")[-1])
    y_wp = float(y_arm.split(" > ")[-1])
    if plot_x_mid:
        x_md = float(x_mid.split(" > ")[-1])
    if plot_xy_mid:
        y_md = float(y_mid.split(" > ")[-1])
    
    axes.axhline(y_wp, color="k", linewidth=4)
    axes.axvline(x_wp, color="k", linewidth=4)
    if plot_x_mid:
        axes.axvline(x_md, color="grey", linewidth=4)
    if plot_xy_mid:
        axes.axhline(y_md, color="grey", linewidth=4)

    axes.set_xlabel("ABCDNet score")
    axes.set_ylabel(y_label)

    text_kwargs = {"horizontalalignment": "center", "verticalalignment": "center", "size": 48}

    if plot_xy_mid:
        axes.text((1 + x_wp)/2, (y_lim[-1] + y_wp)/2, "A", **text_kwargs)
        axes.text((x_wp + x_md)/2, (y_lim[-1] + y_wp)/2, "C$_1$", **text_kwargs)
        axes.text((x_md + 0)/2, (y_lim[-1] + y_wp)/2, "C$_2$", **text_kwargs)

        axes.text((1 + x_wp)/2, (y_wp + y_md)/2, "B$_1$", **text_kwargs)
        axes.text((x_wp + x_md)/2, (y_wp + y_md)/2, "D$_1$", **text_kwargs)
        axes.text((x_md + 0)/2, (y_wp + y_md)/2, "D$_3$", **text_kwargs)

        axes.text((1 + x_wp)/2, (y_md + 0)/2, "B$_2$", **text_kwargs)
        axes.text((x_wp + x_md)/2, (y_md + 0)/2, "D$_2$", **text_kwargs)
        axes.text((x_md + 0)/2, (y_md + 0)/2, "D$_4$", **text_kwargs)
    elif plot_x_mid:
        axes.text((1 + x_wp)/2, (y_lim[-1] + y_wp)/2, "A", **text_kwargs)
        axes.text((x_wp + x_md)/2, (y_lim[-1] + y_wp)/2, "C$_1$", **text_kwargs)
        axes.text((x_md + 0)/2, (y_lim[-1] + y_wp)/2, "C$_2$", **text_kwargs)

        axes.text((1 + x_wp)/2, (y_wp + 0)/2, "B", **text_kwargs)
        axes.text((x_wp + x_md)/2, (y_wp + 0)/2, "D$_1$", **text_kwargs)
        axes.text((x_md + 0)/2, (y_wp + 0)/2, "D$_2$", **text_kwargs)
    else:
        axes.text((1 + x_wp)/2, (y_lim[-1] + y_wp)/2, "A", **text_kwargs)
        axes.text((1 + x_wp)/2, (y_wp + 0)/2, "B", **text_kwargs)
        axes.text((0 + x_wp)/2, (y_lim[-1] + y_wp)/2, "C", **text_kwargs)
        axes.text((x_wp + 0)/2, (y_wp + 0)/2, "D", **text_kwargs)

    if plot_xy_mid:
        axes.set_xticks([x_md, x_wp])
        axes.set_yticks([y_md, y_wp])
    elif plot_x_mid:
        axes.set_xticks([x_md, x_wp])
        axes.set_yticks([y_wp])
    else:
        axes.set_xticks([x_wp])
        axes.set_yticks([y_wp])
    
    # Remove all margins
    axes.margins(0)
    # Remove CMS-style ticks on top and right sides of plot
    axes.tick_params(top=False, right=False, labelsize=24, width=4)
    axes.tick_params(top=False, bottom=False, right=False, left=False, which="minor")
    
    # Create a Rectangle patch for each region
    A_rect = patches.Rectangle(
        (x_wp, y_wp), (1 - x_wp), (y_lim[-1] - y_wp), 
        linewidth=0, facecolor="#61D836", alpha=0.25
    )
    B_rect = patches.Rectangle(
        (x_wp, 0), (1 - x_wp), (y_wp), 
        linewidth=0, facecolor="#56C1FF", alpha=0.25
    )
    C_rect = patches.Rectangle(
        (0, y_wp), (x_wp), (y_lim[-1] - y_wp), 
        linewidth=0, facecolor="#B51700", alpha=0.25
    )
    D_rect = patches.Rectangle(
        (0, 0), (x_wp), (y_wp), 
        linewidth=0, facecolor="#FFD932", alpha=0.25
    )

    # Add the patches to the Axes
    axes.add_patch(A_rect)
    axes.add_patch(B_rect)
    axes.add_patch(C_rect)
    axes.add_patch(D_rect)
    
    # Add title with preselection
    if not presel_cuts is None:
        title = " and ".join(presel_cuts)
        # Hard-coded replacements
        title = title.replace("ld_vqqfatjet_xwqq", r"XWqq(ld $V\rightarrow qq$)")
        title = title.replace("tr_vqqfatjet_xwqq", r"XWqq(tr $V\rightarrow qq$)")
        title = title.replace("hbbfatjet_xbb", r"Xbb($H\rightarrow bb$)")
        # Hard-coded length limit (make newline)
        if len(title) > 100:
            title = title.split(" and ")
            title[-1] = "\n" + title[-1]
            title = " and ".join(title)
        
        axes.set_title(title, size=24, pad=24)
    
    axes.patch.set_alpha(0)
    
    if plots_dir:
        x_arm_str = PandasPlotter.get_selection_str(x_arm)
        y_arm_str = PandasPlotter.get_selection_str(y_arm)
        if plot_xy_mid:
            plot_file = f"{plots_dir}/AB1B2C1C2D1D2_{x_arm_str}_vs_{y_arm_str}.png"
        elif plot_x_mid:
            plot_file = f"{plots_dir}/ABC1C2D1D2_{x_arm_str}_vs_{y_arm_str}.png"
        else:
            plot_file = f"{plots_dir}/ABCD_{x_arm_str}_vs_{y_arm_str}.png"
            
        print(f"Wrote to {plot_file}")
        plt.savefig(plot_file, bbox_inches="tight")
        plt.savefig(plot_file.replace(".png", ".pdf"), bbox_inches="tight")

def get_abcd(plotter, regions=None, names=None, plots_dir=None):
    A, B, C, D = regions
    A_name, B_name, C_name, D_name = names

    mc_pred_A_count = 0
    mc_pred_A_error = 0
    data_pred_A_count = 0
    data_pred_A_error = 0
    show_data = False
    tex = [
        f"{'region':>10} & {'bkg':>10} & {'bkg err':>10} & {'sig':>10} & {'sig err':>10} & {'data':>10} & {'data err':>10} \\\\",
        "\hline"
    ]
    txt = [f"{'bkg':>10},{'bkg err':>10},{'sig':>10},{'sig err':>10},{'data':>10},{'data err':>10}"]
    regions = ["A", "B", "C", "D"]
    for region_i, region in enumerate([A, B, C, D]):
        # MC counts
        sig_count = plotter.sig_count(selection=region)
        sig_error = plotter.sig_error(selection=region)
        bkg_count = plotter.bkg_count(selection=region)
        bkg_error = plotter.bkg_error(selection=region)
        csv_line = f"{bkg_count:>10.4f},{bkg_error:>10.4f},{sig_count:>10.4f},{sig_error:>10.4f}"
        tex_line = f"{bkg_count:>10.1f} & {bkg_error:>10.1f} & {sig_count:>10.1f} & {sig_error:>10.1f}"
        # Data counts
        data_count = plotter.data_count(selection=region)
        data_error = plotter.data_error(selection=region)
        if sig_count < bkg_error and sig_count < bkg_count*0.1:
            csv_line += f",{int(data_count):>10},{data_error:>10.4f}"
            tex_line += f" & {int(data_count):>10} & {data_error:>10.1f}"
            show_data = True
        else:
            csv_line += f",{' — ':>10},{' — ':>10}"
            tex_line += f" &  —         &  —        "
        txt.append(csv_line)
        tex.append(f"{regions[region_i]:>10} & {tex_line} \\\\")
        
        # Do extrapolation
        if region_i == 1:
            mc_pred_A_count = bkg_count
            data_pred_A_count = data_count
        elif region_i == 2:
            mc_pred_A_count *= bkg_count
            data_pred_A_count *= data_count
        elif region_i == 3 and bkg_count > 0:
            mc_pred_A_count /= bkg_count
            data_pred_A_count /= data_count
        # Calculate error
        if region_i > 0 and bkg_count > 0:
            mc_pred_A_error += (bkg_error/bkg_count)**2
            data_pred_A_error += 1/data_count
    
    mc_pred_A_error = np.sqrt(mc_pred_A_error)*mc_pred_A_count
    data_pred_A_error = np.sqrt(data_pred_A_error)*data_pred_A_count
    txt.append("")
    txt.append(f"MC pred_A_count = {mc_pred_A_count:.2f}")
    txt.append(f"MC pred_A_error = {mc_pred_A_error:.2f}")
    txt.append(f"Data pred_A_count = {data_pred_A_count:.2f}")
    txt.append(f"Data pred_A_error = {data_pred_A_error:.2f}")
    txt.append("")
    tex.append("           & ")
    
    if plots_dir:
        with open("{A_name}_{B_name}_{C_name}_{D_name}.txt", "w") as txt_file:
            for txt_line in txt:
                txt_file.write(txt_line+"\n")
        with open("{A_name}_{B_name}_{C_name}_{D_name}.tex", "w") as tex_file:
            for tex_line in tex:
                tex_file.write(tex_line+"\n")


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
        legend_loc="upper right", legend_ncol=2, autoblind=True, logy=True, hist_ylim=[1e-2, 1e10]
    )

    plotter.plot_data_vs_mc(
        "abcdnet_score", np.linspace(0, 1, 51), selection="presel", 
        x_label="ABCDNet score", logy=True, legend_loc="upper right", legend_ncol=2,
        blinded_cut="abcdnet_score <= 0.89", sig_scale=1
    )

def extra_plots(plotter, plots_dir):
    print(f"Writing extra plots to {plots_dir}")
    os.makedirs(plots_dir, exist_ok=True)

    # --- ABCD plots ---
    # Save original weights
    orig_event_weight = plotter.df.event_weight.values.copy()
    # Apply ParticleNet scale factors
    plotter.df.event_weight *= plotter.df.xbb_sf*plotter.df.xwqq_ld_vqq_sf*plotter.df.xwqq_tr_vqq_sf

    cuts = "abcdnet_score > 0.89 and abs_deta_jj > 5.0 and ld_vqqfatjet_xwqq > 0.8 and tr_vqqfatjet_xwqq > 0.7 and hbbfatjet_xbb > 0.8".split(" and ")
    x_arm = cuts[0]
    y_arm = cuts[1]
    presel = cuts[2:]

    # How to sub-divide the control regions for closure tests
    x_mid = "abcdnet_score > 0.4"
    y_mid = "abs_deta_jj > 2.5"

    A = " and ".join([x_arm, y_arm, *presel])
    B = " and ".join([x_arm, f"(not {y_arm})", *presel])
    C = " and ".join([f"(not {x_arm})", y_arm, *presel])
    D = " and ".join([f"(not {x_arm})", f"(not {y_arm})", *presel])

    B1 = " and ".join([x_arm, f"({y_mid}) and (not {y_arm})", *presel])
    B2 = " and ".join([x_arm, f"(not {y_mid}) and (not {y_arm})", *presel])
    C1 = " and ".join([f"({x_mid}) and (not {x_arm})", y_arm, *presel])
    C2 = " and ".join([f"(not {x_mid}) and (not {x_arm})", y_arm, *presel])
    D1 = " and ".join([f"({x_mid}) and (not {x_arm})", f"({y_mid}) and (not {y_arm})", *presel])
    D2 = " and ".join([f"({x_mid}) and (not {x_arm})", f"(not {y_mid}) and (not {y_arm})", *presel])
    D3 = " and ".join([f"(not {x_mid}) and (not {x_arm})", f"({y_mid}) and (not {y_arm})", *presel])
    D4 = " and ".join([f"(not {x_mid}) and (not {x_arm})", f"(not {y_mid}) and (not {y_arm})", *presel])

    plot_abcd(x_arm, y_arm, y_lim=[0, 10], y_label=r"$|\Delta\eta_{jj}|$", presel_cuts=presel, plots_dir=plots_dir)
    plot_abcd(x_arm, y_arm, x_mid=x_mid, y_lim=[0, 10], y_label=r"$|\Delta\eta_{jj}|$", presel_cuts=presel, plots_dir=plots_dir)
    plot_abcd(x_arm, y_arm, x_mid=x_mid, y_mid=y_mid, y_lim=[0, 10], y_label=r"$|\Delta\eta_{jj}|$", presel_cuts=presel, plots_dir=plots_dir)

    # A = B*C/D
    get_abcd(plotter, regions=[A, B, C, D], names=["A", "B", "C", "D"], plots_dir=plots_dir)
    # B1 = B2*D1/D2
    get_abcd(plotter, regions=[B1, B2, D1, D2], names=["B1", "B2", "D1", "D2"], plots_dir=plots_dir)
    # D1 = D2*D3/D4
    get_abcd(plotter, regions=[D1, D2, D3, D4], names=["D1", "D2", "D3", "D4"], plots_dir=plots_dir)
    # C1 = D1*C2/D3
    get_abcd(plotter, regions=[C1, D1, C2, D3], names=["C1", "D1", "C2", "D3"], plots_dir=plots_dir)
    # A = B*C1/D1
    get_abcd(plotter, regions=[A, B, C1, f"({D1}) or ({D2})"], names=["A", "B", "C1", "D12"], plots_dir=plots_dir)
    # C1 = D1*C2/D2
    get_abcd(plotter, regions=[C1, f"({D1}) or ({D2})", C2, f"({D3}) or ({D4})"], names=["C1", "D12", "C2", "D34"], plots_dir=plots_dir)

    # Reset event weights
    plotter.df.event_weight = orig_event_weight
    # ------------------

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
    if not sig_babies or not bkg_babies or not data_babies:
        print(f"Missing some (or all) babies! Looking for {baby_dir}/Run2/*.root")
        exit()

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
    # if plot_all or args.opt:
    #     opt_plots(plotter, f"{plot_dir}/opt")
    # if plot_all or args.val:
    #     val_plots(plotter, f"{plot_dir}/val")
    if plot_all or args.extra:
        extra_plots(plotter, f"{plot_dir}/extra")
