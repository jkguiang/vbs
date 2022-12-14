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
import mplhep as hep
plt.style.use(hep.style.CMS)

BKG_SAMPLE_MAP = {
    "EWKWLep": ["EWKW*WToLNu*"],
}

CEPH_DIR = "/ceph/cms/store/user/jguiang/VBSVHSkim/ewkcheck_5f"

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
            group_root_files += glob.glob(f"{CEPH_DIR}/{globber}/merged.root")

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

    # Correctly label/color/merge each hist
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

def plot_M_qq_overlay(genclass_hist, lheclass_hist, n_rebin=None, title="", x_label="", logy=False, norm=False, outfile=None):
    # fig, axes = plt.subplots(figsize=(12, 9))
    fig, axes = plt.subplots()

    if n_rebin:
        genclass_hist = genclass_hist.rebin(n_rebin)
        lheclass_hist = lheclass_hist.rebin(n_rebin)
    if norm:
        genclass_hist_norm = genclass_hist.normalize()
        lheclass_hist_norm = lheclass_hist.normalize()
        genclass_hist_norm.plot(ax=axes, log=logy, color="#fdb366", label=f"gen-classified [{np.sum(genclass_hist.counts):0.1f} events]")
        lheclass_hist_norm.plot(ax=axes, log=logy, color="#dd3c2d", label=f"lhe-classified [{np.sum(lheclass_hist.counts):0.1f} events]")
    else:
        genclass_hist.plot(ax=axes, log=logy, color="#fdb366", label=f"gen-classified [{np.sum(genclass_hist.counts):0.1f} events]")
        lheclass_hist.plot(ax=axes, log=logy, color="#dd3c2d", label=f"lhe-classified [{np.sum(lheclass_hist.counts):0.1f} events]")
    
    if not logy:
        axes.yaxis.set_minor_locator(AutoMinorLocator())

    axes.xaxis.set_minor_locator(AutoMinorLocator())
    axes.set_title(title, size=18)
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

def plot_pt_overlay(inb_hist, nob_hist, n_rebin=None, title="", x_label="", logy=False, norm=False, outfile=None):
    # fig, axes = plt.subplots(figsize=(12, 9))
    fig, axes = plt.subplots()

    if n_rebin:
        inb_hist = inb_hist.rebin(n_rebin)
        nob_hist = nob_hist.rebin(n_rebin)
    if norm:
        inb_hist_norm = yahist.Hist1D.from_bincounts(
            inb_hist.counts/np.sum(inb_hist.counts),
            bins=inb_hist.edges,
            color="#fdb366"
        )
        # inb_hist_norm = inb_hist.normalize()
        nob_hist_norm = yahist.Hist1D.from_bincounts(
            nob_hist.counts/np.sum(nob_hist.counts),
            bins=nob_hist.edges,
            color="#dd3c2d"
        )
        # nob_hist_norm = nob_hist.normalize()
        inb_hist_norm.plot(ax=axes, log=logy, color="#fdb366", label=f"has incoming b [{np.sum(inb_hist.counts):0.1f} events]")
        nob_hist_norm.plot(ax=axes, log=logy, color="#dd3c2d", label=f"no incoming b [{np.sum(nob_hist.counts):0.1f} events]")
    else:
        inb_hist.plot(ax=axes, log=logy, color="#fdb366", label=f"has incoming b [{np.sum(inb_hist.counts):0.1f} events]")
        nob_hist.plot(ax=axes, log=logy, color="#dd3c2d", label=f"no incoming b [{np.sum(nob_hist.counts):0.1f} events]")
    
    if not logy:
        axes.yaxis.set_minor_locator(AutoMinorLocator())

    axes.xaxis.set_minor_locator(AutoMinorLocator())
    axes.set_title(title, size=18)
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

def plot_parton_pair_pdgIDs(hist, n_rebin=None, title="", x_label="", logy=False, norm=False, outfile=None):
    # fig, axes = plt.subplots(figsize=(15, 9))
    fig, axes = plt.subplots()

    if n_rebin:
        hist = hist.rebin(n_rebin)
    if norm:
        hist = hist.normalize()

    inb_counts = np.zeros(1)
    inb_counts[0] = np.sum(hist.counts[:5])
    inb_hist = yahist.Hist1D.from_bincounts(
        inb_counts,
        bins=[0, 5],
        color="#fdb366",
        label=f"has incoming b"
    )
    inb_hist.plot(
        ax=axes, 
        log=logy, histtype="stepfilled", 
        counts=True, 
        # counts_fontsize=14, 
        counts_fontsize=10, 
        counts_formatter="{:0.2%}".format
    )

    nob_counts = np.zeros(1)
    nob_counts[0] = np.sum(hist.counts[5:])
    nob_hist = yahist.Hist1D.from_bincounts(
        nob_counts,
        bins=[5, 15],
        color="#dd3c2d",
        label=f"no incoming b"
    )
    nob_hist.plot(
        ax=axes, 
        log=logy, histtype="stepfilled", 
        counts=True, 
        # counts_fontsize=14, 
        counts_fontsize=10, 
        counts_formatter="{:0.2%}".format
    )

    hist.plot(
        ax=axes, color="k", alpha=0.25,
        log=logy, histtype="stepfilled", label="", 
        counts=True, 
        # counts_fontsize=14, 
        counts_fontsize=10, 
        counts_formatter="{:0.2%}".format
    )
    
    if not logy:
        axes.yaxis.set_minor_locator(AutoMinorLocator())
    # axes.set_title(title, size=18)
    axes.set_xticks(np.linspace(0.5, 14.5, 15))
    axes.set_xticklabels(
        ["bb", "bc", "bs", "bu", "bd", "cc", "cs", "cu", "cd", "ss", "su", "sd", "uu", "ud", "dd"],
        # fontsize=14
    )
    # axes.set_xlabel(x_label, size=18)
    axes.set_xlabel(x_label)
    axes.set_ylim(bottom=0)
    if norm:
        # axes.set_ylabel("a.u.", size=18)
        axes.set_ylabel("a.u.")
    else:
        # axes.set_ylabel("Events", size=18)
        axes.set_ylabel("Events")

    # axes.legend(fontsize=12, loc="upper center")
    axes.legend()
            
    hep.cms.label(
        "Preliminary",
        data=False,
        lumi=138,
        loc=0,
        ax=axes,
    )

    if outfile:
        plt.savefig(outfile)

    plt.close()

    return axes

def plot_parton_pdgIDs(hist, n_rebin=None, title="", x_label="", logy=False, norm=False, outfile=None, abs=False):
    # fig, axes = plt.subplots(figsize=(12, 9))
    fig, axes = plt.subplots()

    if n_rebin:
        hist = hist.rebin(n_rebin)
    if norm:
        hist = hist.normalize()

    hist.plot(
        ax=axes, 
        log=logy, histtype="stepfilled", 
        counts=True, 
        # counts_fontsize=14, 
        counts_fontsize=10, 
        counts_formatter="{:0.2%}".format
    )
    
    if not logy:
        axes.yaxis.set_minor_locator(AutoMinorLocator())
    # axes.set_title(title, size=18)
    if abs:
        axes.set_xticks(np.linspace(1.5, 6.5, 6))
        # axes.set_xticklabels(["d", "u", "s", "c", "b", "t"], fontsize=14)
        axes.set_xticklabels(["d", "u", "s", "c", "b", "t"])
    else:
        axes.set_xticks(np.linspace(-6.5, 6.5, 14))
        # axes.set_xticklabels([r"$\overline{t}$", r"$\overline{b}$", r"$\overline{c}$", 
        #                       r"$\overline{s}$", r"$\overline{u}$", r"$\overline{d}$", 
        #                       "", "", r"$d$", r"$u$", r"$s$", r"$c$", r"$b$", r"$t$"], fontsize=14)
        axes.set_xticklabels([r"$\overline{t}$", r"$\overline{b}$", r"$\overline{c}$", 
                              r"$\overline{s}$", r"$\overline{u}$", r"$\overline{d}$", 
                              "", "", r"$d$", r"$u$", r"$s$", r"$c$", r"$b$", r"$t$"])
    # axes.set_xlabel(x_label, size=18)
    axes.set_xlabel(x_label)
    axes.set_ylim(bottom=0)
    # axes.legend(fontsize=12)
    axes.legend()
            
    hep.cms.label(
        "Preliminary",
        data=False,
        lumi=138,
        loc=0,
        ax=axes,
    )

    if norm:
        # axes.set_ylabel("a.u.", size=18)
        axes.set_ylabel("a.u.")
    else:
        # axes.set_ylabel("Events", size=18)
        axes.set_ylabel("Events")

    if outfile:
        plt.savefig(outfile)

    plt.close()

    return axes

if __name__ == "__main__":
    hists = get_hists(BKG_SAMPLE_MAP, colors=["#fdb366"])
    for hist_name in tqdm(hists.keys()):
        outfile = f"/home/users/jguiang/public_html/ewkcheck_5f_plots/{hist_name}.pdf"
        if "pdgID" in hist_name:
            if "partons" in hist_name:
                plot_parton_pair_pdgIDs(
                    hists[hist_name][0], 
                    title=hist_name, logy=False, norm=True,
                    outfile=outfile
                )
            else:
                plot_parton_pdgIDs(
                    hists[hist_name][0], 
                    title=hist_name, logy=False, norm=True,
                    outfile=outfile,
                    abs=("abs_pdgID" in hist_name),
                )
        elif "pt_inb" in hist_name or "eta_inb" in hist_name:
            outfile = outfile.replace("_inb", "")
            if "FindGen" in hist_name or hist_name[:4] == "SKIM":
                n_rebin = 10
            elif "POSTSKIM" in hist_name:
                n_rebin = 10
            else:
                n_rebin = 50
            plot_pt_overlay(
                hists[hist_name][0],
                hists[hist_name.replace("_inb", "_nob")][0],
                title=hist_name.replace("_inb", ""), 
                n_rebin=n_rebin, logy=False, norm=True,
                outfile=outfile
            )
        elif "genclass" in hist_name:
            outfile = outfile.replace("_genclass", "")
            if "FindGen" in hist_name or hist_name[:4] == "SKIM":
                n_rebin = 1
            elif "POSTSKIM" in hist_name:
                n_rebin = 3
            else:
                n_rebin = 5
            plot_M_qq_overlay(
                hists[hist_name][0],
                hists[hist_name.replace("_genclass", "_lheclass")][0],
                title=hist_name.replace("_genclass", ""), 
                n_rebin=n_rebin, logy=False, norm=False,
                outfile=outfile
            )
