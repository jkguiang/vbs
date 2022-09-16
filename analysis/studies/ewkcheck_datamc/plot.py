print("Importing packages...")
import os
import glob
import json
import uproot
import yahist
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from matplotlib import gridspec
from matplotlib.ticker import AutoMinorLocator
from utils.file_info import parse
from tqdm import tqdm

BKG_SAMPLE_MAP = {
    "SingleTop": ["ST*"],
    "TTbar1L": ["TTToSemiLep*"],
    "TTbar2L": ["TTTo2L*"],
    "TTX": ["ttH*", "TTW*", "TTZ*", "TTbb*", "TTToHadronic*"],
    "WJets": ["WJets*"],
    "Bosons": ["WW*", "WZ*", "ZZ*"],
    "EWKWLep": ["EWKW*WToLNu*"],
    "EWKVOther": ["EWKW*WToQQ*", "EWKZ*ZToNuNu*", "EWKZ*ZToLL*", "EWKZ*ZToQQ*"],
    "DYJets": ["DY*"],
    "VH": ["*HToBB*", "VHToNonbb_M125*"]
}

SIG_SAMPLE_MAP = {
    "VBSWH_mkW": ["VBSWH_mkW_Inclusive*"]
}

DATA_SAMPLE_MAP = {
    "data": ["*Run201*"]
}

CEPH_DIR = "/ceph/cms/store/user/jguiang/VBSVHSkim/ewkcheck_datamc"

def get_hists(sample_map, colors=None):
    print("Gathering histograms...")
    if not colors:
        colors = [
            "#364b9a", "#e8eff6", "#6ea5cd", "#97cae1", "#c2e4ee", 
            "#eaeccc", "#feda8a", "#fdb366", "#f67e4b", "#dd3c2d",
            "#9b2226"
        ]

    color_map = {}
    for group_name in sorted(sample_map.keys(), reverse=True):
        color_map[group_name] = colors.pop(0)

    # Sum together histograms for each group
    n_root_files = 0
    all_hists = {}
    for group_name, globbers in sample_map.items():
        group_root_files = []
        for globber in globbers:
            group_root_files += glob.glob(f"{CEPH_DIR}/{globber}/*.root")

        n_root_files += len(group_root_files)

        for root_file in tqdm(group_root_files):
            if "Run201" not in root_file:
                info = parse(root_file, xsecs_json="data/xsecs.json")
            with uproot.open(root_file) as tfile:
                for hist_name in filter(lambda n: "__" in n, tfile.keys()):
                    hist_name = hist_name.split(";")[0]
                    counts, edges = tfile[hist_name].to_numpy()
                    if "Run201" not in root_file:
                        counts *= info["xsec"]*1000*info["lumi"]/info["n_events"]
                    if "WJetsToLNu_HT-" in root_file:
                        name = root_file.split("_Tune")[0].split('/')[-1]
                        with open("data/wjets_ht-binned_xsecs.json", "r") as f_in:
                            wjets_xsecs = json.load(f_in)
                        counts *= wjets_xsecs[info["year"]][name]
                    if "WJetsToLNu_TuneCP5" in root_file and "TT" not in root_file:
                        counts *= 55960

                    hist = yahist.Hist1D.from_bincounts(counts, bins=edges)

                    if hist_name not in all_hists:
                        all_hists[hist_name] = {}
                    if group_name not in all_hists[hist_name]:
                        all_hists[hist_name][group_name] = []

                    all_hists[hist_name][group_name].append(hist)

    # Correctly label/color each hist
    for hist_name, groups in all_hists.items():
        hists = []
        for group_name, group_hists in groups.items():
            hist_counts = np.sum([h.counts for h in group_hists], axis=0)
            hist = yahist.Hist1D.from_bincounts(
                hist_counts,
                bins=group_hists[0].edges,
                color=color_map[group_name],
                label=f"{group_name} [{np.sum(hist_counts):0.1f} events]"
            )
            hists.append(hist)

        all_hists[hist_name] = hists

    print(f"Parsed {n_root_files} ROOT files")
    return all_hists

def plot_sig_vs_bkg(bkg_hists, sig_hist, n_rebin=None, x_label="", logy=False, norm=False, outfile=None):

    fig, axes = plt.subplots(figsize=(12, 9))

    bkg_counts = np.sum([hist.counts for hist in bkg_hists], axis=0)
    bkg_hist = yahist.Hist1D.from_bincounts(
        bkg_counts,
        bins=bkg_hists[0].edges,
        color="k",
        label=f"total background [{np.sum(bkg_counts):0.1f} events]"
    )

    if n_rebin:
        bkg_hist = bkg_hist.rebin(n_rebin)
        sig_hist = sig_hist.rebin(n_rebin)
        for hist_i, hist in enumerate(bkg_hists):
            bkg_hists[hist_i] = hist.rebin(n_rebin)

    if norm:
        bkg_hist = bkg_hist.normalize()
        sig_hist = sig_hist.normalize()

    # Plot stacked bkg hists
    bkg_hists.sort(key=lambda h: sum(h.counts))
    yahist.utils.plot_stack(bkg_hists, ax=axes, histtype="stepfilled", log=logy)

    bkg_hist.plot(ax=axes, alpha=0.5, log=logy)
    sig_hist.plot(ax=axes, linewidth=2, log=logy)
    
    axes.xaxis.set_minor_locator(AutoMinorLocator())
    if not logy:
        axes.yaxis.set_minor_locator(AutoMinorLocator())
    axes.set_xlabel(x_label, size=18)
    axes.set_ylim(bottom=0)
    axes.legend(fontsize=10)
    if norm:
        axes.set_ylabel("a.u.", size=18)
    else:
        axes.set_ylabel("Events", size=18)

    if outfile:
        plt.savefig(outfile)

    plt.close()

    return axes

def plot_data_vs_mc(mc_hists, data_hist, n_rebin=None, x_label="", title="", logy=False, norm=False, outfile=None):

    fig = plt.figure(figsize=(6.4*1.5, 4.8*1.25*1.5))
    gs = gridspec.GridSpec(ncols=1, nrows=2, figure=fig, height_ratios=[2, 0.65], hspace=0.1)
    hist_axes = fig.add_subplot(gs[0])
    ratio_axes = fig.add_subplot(gs[1])

    if logy:
        hist_axes.set_yscale("log", nonpositive="clip")

    mc_counts = np.sum([hist.counts for hist in mc_hists], axis=0)
    mc_hist = yahist.Hist1D.from_bincounts(
        mc_counts,
        bins=mc_hists[0].edges,
        color="k",
        label=f"total background [{np.sum(mc_counts):0.1f} events]"
    )

    old_mc_hists = mc_hists
    mc_hists = []
    if n_rebin:
        mc_hist = mc_hist.rebin(n_rebin)
        data_hist = data_hist.rebin(n_rebin)
        for hist_i, hist in enumerate(old_mc_hists):
            mc_hists.append(hist.rebin(n_rebin))

    if norm:
        data_hist = data_hist.normalize()
        mc_hist = mc_hist.normalize()

    # Get ratio
    ratio = data_hist/mc_hist
    # Get ratio errors
    data_counts = data_hist.counts
    data_counts[data_counts == 0] = 1e-12
    mc_counts = mc_hist.counts
    mc_counts[mc_counts == 0] = 1e-12
    rel_errors = np.sqrt(
        (data_hist.errors/mc_counts)**2 
        + (mc_hist.errors*data_hist.counts/mc_hist.counts**2)**2
    )

    # Plot stacked bkg hists
    mc_hists.sort(key=lambda h: np.sum(h.counts))
    yahist.utils.plot_stack(mc_hists, ax=hist_axes, histtype="stepfilled")

    # Plot hists and ratio
    mc_hist.plot(ax=hist_axes, alpha=0.5)
    data_hist.plot(ax=hist_axes, errors=True)
    ratio.plot(ax=ratio_axes, errors=True, color="k")
    # Plot relative errors
    err_points = np.repeat(mc_hist.edges, 2)[1:-1]
    err_high = np.repeat(1 + rel_errors, 2)
    err_low = np.repeat(1 - rel_errors, 2)
    ratio_axes.fill_between(
        err_points, err_high, err_low, 
        step="mid", 
        hatch="///////", 
        facecolor="none",
        edgecolor=(0.75, 0.75, 0.75), 
        linewidth=0.0, 
        linestyle="-",
        zorder=2
    )
    ratio_axes.axhline(y=1, color="k", linestyle="--", alpha=0.75, linewidth=0.75)

    hist_axes.legend(fontsize=10)

    if not logy:
        hist_axes.xaxis.set_minor_locator(AutoMinorLocator())
        hist_axes.yaxis.set_minor_locator(AutoMinorLocator())
        hist_axes.set_ylim(bottom=0)
        ratio_axes.xaxis.set_minor_locator(AutoMinorLocator())
        ratio_axes.yaxis.set_minor_locator(AutoMinorLocator())
    else:
        hist_axes.set_ylim(bottom=0.1)

    if norm:
        hist_axes.set_ylabel("a.u.", size=18)
    else:
        hist_axes.set_ylabel("Events", size=18)

    hist_axes.set_xmargin(0)
    hist_axes.set_title(title, size=18)
    ratio_axes.set_xlabel(x_label, size=18)
    ratio_axes.set_ylabel("data/MC", size=18)
    ratio_axes.set_ylim([0.5, 2.0])

    if outfile:
        plt.savefig(outfile)

    plt.close()

    return hist_axes, ratio_axes

if __name__ == "__main__":
    NO_EWKW = True
    if NO_EWKW:
        BKG_SAMPLE_MAP.pop("EWKWLep")
    mc_hists = get_hists(BKG_SAMPLE_MAP)
    sig_hists = get_hists(SIG_SAMPLE_MAP, colors=["r"])
    data_hists = get_hists(DATA_SAMPLE_MAP, colors=["k"])
    for hist_name in tqdm(mc_hists.keys()):
        if "n_jets_pt30" in hist_name:
            n_rebin=None
        else:
            n_rebin=100
        outfile = f"/home/users/jguiang/public_html/ewkcheck_datamc_plots/{hist_name}.png"
        if NO_EWKW:
            outfile = outfile.replace(".png", "__noEWKW.png")
        plot_data_vs_mc(
            mc_hists[hist_name], data_hists[hist_name][0], 
            n_rebin=n_rebin, title=hist_name, logy=True, norm=False,
            outfile=outfile
        )
        outfile = outfile.replace(".png", "_linear.png")
        plot_data_vs_mc(
            mc_hists[hist_name], data_hists[hist_name][0], 
            n_rebin=n_rebin, title=hist_name, logy=False, norm=False,
            outfile=outfile
        )
