import os
import uproot
import numpy as np
import pandas as pd
from matplotlib import pyplot as plt
from matplotlib import gridspec
from matplotlib.ticker import AutoMinorLocator
from tqdm import tqdm
import yahist
import mplhep as hep
plt.style.use(hep.style.CMS)
plt.rcParams.update({"figure.facecolor":  (1,1,1,0)})
from mpl_toolkits.axes_grid1 import make_axes_locatable
import matplotlib.colors as clr

from utils.cutflow import Cut, Cutflow, CutflowCollection

def clip(np_array, bins):
    clip_low = 0.5*(bins[0] + bins[1])
    clip_high = 0.5*(bins[-2] + bins[-1])
    return np.clip(np_array, clip_low, clip_high)

class PandasAnalysis:
    def __init__(self, sig_root_files=None, bkg_root_files=None, data_root_files=None, 
                 ttree_name="Events", weight_columns=None, reweight_column=None, 
                 plots_dir=None, sample_labels={}, drop_columns=[], lumi=138, 
                 cms_label="Preliminary", cms_size=None, stack_order=None, stack_order_pos=-1):

        if reweight_column:
            drop_columns.append(reweight_column)

        self.lumi = lumi
        self.cms_label = cms_label
        self.cms_size = cms_size
        self.stack_order = stack_order or []
        self.stack_order_pos = stack_order_pos
        dfs = []
        self.sig_reweights = None
        # Load signal
        if sig_root_files:
            for root_file in tqdm(sig_root_files, desc="Loading sig babies"):
                name = root_file.split("/")[-1].replace(".root", "")
                with uproot.open(root_file) as f:
                    df = f[ttree_name].arrays([k for k in f[ttree_name].keys() if k not in drop_columns], library="pd")
                    df["name"] = name
                    df["is_signal"] = True
                    df["is_data"] = False
                    dfs.append(df)
                    if reweight_column:
                        self.sig_reweights = np.stack(f[ttree_name].arrays(reweight_column, library="np")[reweight_column])
        # Load background
        bkg_names = []
        if bkg_root_files:
            for root_file in tqdm(bkg_root_files, desc="Loading bkg babies"):
                name = root_file.split("/")[-1].replace(".root", "")
                bkg_names.append(name)
                with uproot.open(root_file) as f:
                    df = f[ttree_name].arrays([k for k in f[ttree_name].keys() if k not in drop_columns], library="pd")
                    df["name"] = name
                    df["is_signal"] = False
                    df["is_data"] = False
                    dfs.append(df)
        # Load data
        if data_root_files:
            for root_file in tqdm(data_root_files, desc="Loading data babies"):
                name = root_file.split("/")[-1].replace(".root", "")
                with uproot.open(root_file) as f:
                    df = f[ttree_name].arrays([k for k in f[ttree_name].keys() if k not in drop_columns], library="pd")
                    df["name"] = name
                    df["is_signal"] = False
                    df["is_data"] = True
                    dfs.append(df)

        self.df = pd.concat(dfs)
        self.df["name"] = self.df.name.astype("category")

        if weight_columns:
            self.df["event_weight"] = self.df[weight_columns[0]]
            for weight_col in weight_columns[1:]:
                self.df["event_weight"] *= self.df[weight_col]
        else:
            self.df["event_weight"] = np.ones(self.df.shape[0])

        self.plots_dir = plots_dir
        if self.plots_dir:
            os.makedirs(self.plots_dir, exist_ok=True)

        self.sample_labels = sample_labels

        # self.cutflows = CutflowCollection(
        #     cutflows={name: Cutflow() for name in self.df.name.unique()}
        # )
        # self.__update_cutflows("base")

        if len(bkg_names) <= 10:
            self.colors = [
                "#364b9a", "#e8eff6", "#6ea5cd", "#97cae1", "#c2e4ee", 
                "#eaeccc", "#feda8a", "#fdb366", "#f67e4b", "#dd3c2d"
            ]
            self.bkg_colors = {name: self.colors[i] for i, name in enumerate(bkg_names)}
        else:
            self.bkg_colors = {}

    @staticmethod
    def get_selection_str(selection):
        selection = selection.replace(" ", "_")
        repl_map = {
            ">=": "geq", "<=": "leq", ">": "gt", "<": "lt", "==": "eq", "!=": "neq",
            ".": "p", "-": "m"
        }
        for char, repl in repl_map.items():
            selection = selection.replace(char, repl)

        return selection

    def __update_cutflows(self, selection):
        selection = PandasAnalysis.get_selection_str(selection)
        for name, cutflow in self.cutflows.items():
            # Get previous cut
            if len(cutflow) > 0:
                prev_cut = cutflow[cutflow.terminal_cut_names[-1]]
            else:
                prev_cut = Cut("Dummy")
            # Create new cut
            _df = self.df[self.df.name == name]
            n_pass_raw = len(_df)
            n_pass_wgt = _df.event_weight.sum()
            new_cut = Cut(
                selection, 
                n_pass=n_pass_raw,
                n_pass_weighted=n_pass_wgt,
                n_fail=(prev_cut.n_pass - n_pass_raw),
                n_fail_weighted=(prev_cut.n_pass_weighted - n_pass_wgt),
            )
            # Insert new cut
            if len(cutflow) > 0:
                cutflow.insert(prev_cut.name, new_cut)
            else:
                cutflow.set_root_cut(new_cut)

        if "TotalBkg" in self.cutflows.names:
            self.cutflows.pop("TotalBkg")

        self.cutflows["TotalBkg"] = self.cutflows.sum()

        for name in self.df[self.df.is_data | self.df.is_signal].name.unique():
            self.cutflows["TotalBkg"] -= self.cutflows[name]

        if self.plots_dir:
            self.cutflows.write_csv(
                f"{self.plots_dir}/cutflow.csv",
                cutflow.terminal_cut_names[-1]
            )

    def print_cutflow(self):
        print("\n".join(self.cutflows.get_csv(self.cutflows.terminal_cut_names[-1])))

    def test_selections(self, selections, coupled=True):
        orig_cutflows = self.cutflows.copy()
        orig_df = self.df.copy()
        if not coupled:
            # Print headers
            self.print_cutflow()
        # Make selections
        for selection in selections:
            self.make_selection(selection)
            if not coupled:
                # Print only the row for this cut
                print(self.cutflows.get_csv(self.cutflows.terminal_cut_names[-1])[-1])
                # Reset
                self.cutflows = orig_cutflows
                self.df = orig_df
        if coupled:
            # Show final cutflow
            self.print_cutflow()
            # Reset
            self.cutflows = orig_cutflows
            self.df = orig_df

    def make_selection(self, selection):
        if selection and type(selection) == str:
            self.df = self.df[self.df.eval(selection)].copy()
            # self.__update_cutflows(selection)

    def sample_df(self, name, selection=None):
        if not selection:
            return self.df[self.df.name == name]
        else:
            if selection and type(selection) == str:
                selection = self.df.eval(selection)
            return self.df[(self.df.name == name) & selection].copy()

    def sig_df(self, selection=None):
        if not selection:
            return self.df[~self.df.is_data & self.df.is_signal]
        else:
            if selection and type(selection) == str:
                selection = self.df.eval(selection)
            return self.df[~self.df.is_data & self.df.is_signal & selection].copy()

    def bkg_df(self, selection=None):
        if not selection:
            return self.df[~self.df.is_data & ~self.df.is_signal]
        else:
            if selection and type(selection) == str:
                selection = self.df.eval(selection)
            return self.df[~self.df.is_data & ~self.df.is_signal & selection].copy()

    def data_df(self, selection=None):
        if not selection:
            return self.df[self.df.is_data]
        else:
            if selection and type(selection) == str:
                selection = self.df.eval(selection)
            return self.df[self.df.is_data & selection].copy()

    def sig_count(self, selection=None, raw=False):
        df = self.sig_df(selection=selection)
        return len(df) if raw else df.event_weight.sum()

    def sig_error(self, selection=None, raw=False):
        df = self.sig_df(selection=selection)
        weights = np.ones(len(df)) if raw else df.event_weight
        return np.sqrt(np.sum(weights**2))

    def bkg_count(self, selection=None, raw=False):
        df = self.bkg_df(selection=selection)
        return len(df) if raw else df.event_weight.sum()

    def bkg_error(self, selection=None, raw=False):
        df = self.bkg_df(selection=selection)
        weights = np.ones(len(df)) if raw else df.event_weight
        return np.sqrt(np.sum(weights**2))

    def data_count(self, selection=None):
        df = self.data_df(selection=selection)
        return len(df)

    def data_error(self, selection=None):
        df = self.data_df(selection=selection)
        return np.sqrt(len(df))

    def sample_count(self, name, selection=None, raw=False):
        df = self.sample_df(name, selection=selection)
        return len(df) if raw else df.event_weight.sum()

    def sample_error(self, name, selection=None, raw=False):
        df = self.sample_df(name, selection=selection)
        weights = np.ones(len(df)) if raw else df.event_weight
        return np.sqrt(np.sum(weights**2))

    def dump_plots(self, plots_yaml):
        return

    def __plot(self, typ, column, bins, weights=None, selection="", transf=lambda x: x, 
               raw=False, x_label="", logy=False, axes=None, norm=False, stacked=False, 
               legend_loc="best", color="r"):

        if not axes:
            fig, axes = plt.subplots()
        
        if typ == "bkg":
            df = self.bkg_df(selection=selection)
        elif typ == "sig":
            df = self.sig_df(selection=selection)
        else:
            return
        
        if weights:
            for weight in weights:
                df.event_weight *= df[weight]

        if raw:
            weights = np.ones(len(df))
        else:
            weights = df.event_weight

        # Get stacked backgrounds
        hists = []
        if typ == "bkg":
            if stacked:
                for name in df.name.unique():
                    weights = weights[df.name == name]
                    if norm:
                        weights /= sum(weights)
                    sample_label = self.sample_labels[name] if name in self.sample_labels else name
                    hist = yahist.Hist1D(
                        transf(df[df.name == name][column]),
                        bins=bins, 
                        weights=weights,
                        label=f"{sample_label} [{sum(weights):.1f} events]",
                        color=self.bkg_colors[name] if self.bkg_colors else None
                    )
                    hists.append(hist)

        # Get total background
        hist = yahist.Hist1D(
            transf(df[column]),
            bins=bins, 
            weights=weights,
            label=f"Total [{sum(weights):0.1f} events]",
            color=("k" if typ == "bkg" else color if typ == "sig" else "r")
        )

        if norm:
            hist = hist.normalize()

        # Plot everything
        if hists:
            hists.sort(key=lambda h: sum(h.counts))
            yahist.utils.plot_stack(hists, ax=axes, histtype="stepfilled", log=logy)
        if typ == "bkg":
            hist.plot(ax=axes, alpha=0.5, log=logy)
        elif typ == "sig":
            hist.plot(ax=axes, linewidth=2, log=logy)

        hep.cms.label(self.cms_label, data=(self.cms_label != "Preliminary"), lumi=self.lumi, loc=0, ax=axes, fontsize=self.cms_size)
        
        axes.set_xlabel(x_label)
        if not logy:
            axes.set_ylim(bottom=0)
        else:
            axes.set_ylim(bottom=0.01)
        if stacked:
            axes.legend(fontsize=14, loc=legend_loc)
        else:
            axes.legend(loc=legend_loc)
        if norm:
            axes.set_ylabel("a.u.")
        else:
            axes.set_ylabel("Events")

        if self.plots_dir:
            plot_file = f"{self.plots_dir}/{column}_{typ}.pdf"
            if stacked:
                plot_file = plot_file.replace(".pdf", "_stacked.pdf")
            if norm:
                plot_file = plot_file.replace(".pdf", "_norm.pdf")
            if logy:
                plot_file = plot_file.replace(".pdf", "_logy.pdf")
            if selection:
                plot_file = plot_file.replace(".pdf", f"_{PandasAnalysis.get_selection_str(selection)}.pdf")
                with open(plot_file.replace("pdf", "txt"), "w") as plot_txt:
                    plot_txt.write(f"SELECTION: {selection}")

            print(f"Wrote plot to {plot_file}")
            plt.savefig(plot_file, bbox_inches="tight")
            plt.savefig(plot_file.replace(".pdf", ".png"), bbox_inches="tight")

        return axes

    def plot_sig(self, *args, **kwargs):
        return self.__plot("sig", *args, **kwargs)

    def plot_bkg(self, *args, **kwargs):
        return self.__plot("bkg", *args, **kwargs)

class Optimization(PandasAnalysis):

    def get_event_counts(self, selection=None, raw=False):
        df = self.df[~self.df.is_data]
        if selection:
            if selection and type(selection) == str:
                selection = df.eval(selection)
            df = df[selection]
        if raw:
            return len(df[df.is_signal]), len(df[~df.is_signal])
        else:
            return df[df.is_signal].event_weight.sum(), df[~df.is_signal].event_weight.sum()

    def get_event_errors(self, selection=None, raw=False):
        sig_error = self.sig_error(selection=selection, raw=raw)
        bkg_error = self.bkg_error(selection=selection, raw=raw)
        return sig_error, bkg_error

    def set_split_column(self, ratio=0.5, name="split"):
        self.df[name] = (np.random.rand(len(self.df)) < ratio)
        self.df[f"{name}_weight"] = self.df.event_weight
        for sample_name in self.df.name.unique():
            split_left = ((self.df.name == sample_name) & ~self.df[name])
            split_right = ((self.df.name == sample_name) & self.df[name])
            orig_integral = self.df[split_left | split_right].event_weight.sum()
            split_left_integral = self.df[split_left].event_weight.sum()
            split_right_integral = self.df[split_right].event_weight.sum()
            if orig_integral*split_left_integral*split_right_integral == 0.:
                self.df.loc[(split_left | split_right), f"{name}_weight"] *= 0.
            else:
                self.df.loc[split_left, f"{name}_weight"] *= orig_integral/split_left_integral
                self.df.loc[split_right, f"{name}_weight"] *= orig_integral/split_right_integral

    def plot_correlation(self, column, bins, weights=None, base_selection=None, selections=None, x_label="", 
                         logy=False, norm=True, transf=lambda x: x):

        markers = ["s", "o", "^", "v", "<", ">", "+", "*"]
        if len(selections) + 1 > len(markers):
            raise Exception(f"no more than {len(markers)} selections are supported")

        fig = plt.figure()
        gs = gridspec.GridSpec(ncols=1, nrows=2, figure=fig, height_ratios=[2, 0.65], hspace=0.08)
        hist_axes = fig.add_subplot(gs[0])
        ratio_axes = fig.add_subplot(gs[1])

        if not base_selection:
            base_selection = "name != ''"
        if not selections:
            return hist_axes, ratio_axes

        denom_df = self.bkg_df(selection=base_selection)
        if weights:
            for weight in weights:
                denom_df.event_weight *= denom_df[weight]
        denom = yahist.Hist1D(
            transf(denom_df[column]),
            bins=bins, 
            weights=denom_df.event_weight
        );
        if norm:
            denom = denom.normalize()

        denom.plot(ax=hist_axes, color="k")
        denom.plot(ax=hist_axes, label="before", color="k", alpha=0.15, histtype="stepfilled")

        for selection_i, selection in enumerate(selections):
            numer_df = self.bkg_df(selection=f"({base_selection}) and ({selection})")
            if weights:
                for weight in weights:
                    numer_df.event_weight *= numer_df[weight]
            numer = yahist.Hist1D(
                transf(numer_df[column]),
                bins=bins, 
                weights=numer_df.event_weight,
                color=self.colors[selection_i//2 - selection_i*(selection_i%2)]
            );
            if norm:
                numer = numer.normalize()

            numer.plot(ax=hist_axes)
            numer.plot(ax=hist_axes, errors=True, label=f"after {selection}", fmt=markers[selection_i])
            (numer/denom).plot(ax=ratio_axes, errors=True)
            
        hep.cms.label(self.cms_label, data=(self.cms_label != "Preliminary"), lumi=self.lumi, loc=0, ax=hist_axes, fontsize=self.cms_size)

        hist_axes.legend(fontsize=14)
        if norm:
            hist_axes.set_ylabel("a.u.")
        else:
            hist_axes.set_ylabel("Events")

        hist_axes.set_xticklabels([])
        ratio_axes.axhline(y=1, color="k", linestyle="--", alpha=0.75, linewidth=0.75)
        ratio_axes.set_xlabel(x_label)
        ratio_axes.set_ylabel("after/before")
        ratio_axes.set_ylim([0, 2.0])

        if self.plots_dir:
            plot_file = f"{self.plots_dir}/{column}_correlations.pdf"
            with open(plot_file.replace("pdf", "txt"), "w") as plot_txt:
                if base_selection:
                    plot_file = plot_file.replace(".pdf", f"_{PandasAnalysis.get_selection_str(base_selection)}.pdf")
                    plot_txt.write(f"BASE_SELECTION: {base_selection}\n")
                plot_txt.write("SELECTIONS:\n")
                plot_txt.write("\n".join(selections))

            print(f"Wrote plot to {plot_file}")
            plt.savefig(plot_file, bbox_inches="tight")
            plt.savefig(plot_file.replace(".pdf", ".png"), bbox_inches="tight")

        return hist_axes, ratio_axes

    def plot_correlation2D(self, x_col, y_col, x_bins, y_bins, 
                           sel=None, sample=None, flip=False, year=None, rebin=None, show_counts=False,
                           x_label=None, y_label=None, x_lim=None, y_lim=None, 
                           x_transf=lambda x: x, y_transf=lambda y: y):

        x_label = x_label or x_col
        y_label = y_label or y_col

        if sample is None:
            df = self.bkg_df(selection=sel)
        else:
            df = self.sample_df(sample, selection=sel)
            
        if not year is None:
            df = df[df.year.abs() == year]
            if year == 2016:
                lumi = 36.33
            elif year == 2017:
                lumi = 41.48
            elif year == 2018:
                lumi = 59.83
        else:
            lumi = 138

        # Plot 2D hist
        hist2d = yahist.Hist2D(
            (x_transf(df[x_col]), y_transf(df[y_col])),
            bins=[x_bins, y_bins],
            weights=df.event_weight
        )
        if flip:
            hist2d = hist2d.transpose()
        if not rebin is None:
            hist2d = hist2d.rebin(rebin)

        # UCSD colors
        cmap = clr.LinearSegmentedColormap.from_list("trident", ["#182B49","#00629B", "#C69214", "#FFCD00"], N=256)
        cmap.set_under("#182B49")

        # Plot 2D hist and 1D profile
        fig, axes = plt.subplots(figsize=(11, 11))
        mpl_objs = hist2d.plot(
            ax=axes, colorbar=False, counts=show_counts, 
            counts_formatter="{:.2f}".format, counts_fontsize=6, 
            cmap=cmap, zrange=(0, None), hide_empty=False
        );
        if flip:
            axes.set_ylabel(x_label);
            axes.set_xlabel(y_label);
            hist2d.profile("x").plot(errors=True, color="white", fmt=".", label="Profile")
            axes.set_ylim([x_bins[0], x_bins[-1]])
            axes.set_xlim([y_bins[0], y_bins[-1]])
        else:
            axes.set_xlabel(x_label);
            axes.set_ylabel(y_label);
            # hist2d.profile("x").plot(errors=True, color="white", fmt=".", label=y_label+" profile")
            hist2d.profile("x").plot(errors=True, color="white", fmt=".", label="Profile")
            axes.set_xlim([x_bins[0], x_bins[-1]])
            axes.set_ylim([y_bins[0], y_bins[-1]])

        # Plot colorbar
        mappable = mpl_objs[0]
        fig = mappable.axes.figure
        divider = make_axes_locatable(mappable.axes)
        cax = divider.append_axes("right", size="5%", pad=0.1)
        cbar = fig.colorbar(mappable, cax=cax)
        cbar.set_label(r"Count");

        # Plot legend
        legend = axes.legend(title="All background" if sample is None else self.sample_labels.get(sample, sample))
        legend.get_title().set_color("white")
        for label in legend.get_texts():
            label.set_color("white")

        # Add CMS label
        hep.cms.label(
            "Preliminary",
            data=False,
            lumi=lumi,
            loc=0,
            ax=axes,
        );
        
        # Save plot
        outfile = f"{self.plots_dir}/correlation2D_{x_col}_{y_col}_1Dprofile.pdf"
        if not sample is None:
            outfile = outfile.replace(".pdf", f"_{sample}.pdf")
        if flip:
            outfile = outfile.replace(".pdf", "_flipped.pdf")
        if not year is None:
            outfile = outfile.replace(".pdf", f"_{year}.pdf")
            
        plt.savefig(outfile, bbox_inches="tight")
        plt.savefig(outfile.replace(".pdf", ".png"), bbox_inches="tight")
        print(f"Wrote plot to {outfile}")

    def plot_sig_vs_bkg(self, column, bins, weights=None, selection="", transf=lambda x: x, 
                        raw=False, x_label="", logy=False, axes=None, norm=False, stacked=False, 
                        legend_loc="best", sig_scale=1):
        if not axes:
            fig, axes = plt.subplots()

        bkg_df = self.bkg_df(selection=selection)
        sig_df = self.sig_df(selection=selection)
        
        if weights:
            for weight in weights:
                bkg_df.event_weight *= bkg_df[weight]
                sig_df.event_weight *= sig_df[weight]

        if raw:
            bkg_weights, sig_weights = np.ones(len(bkg_df)), np.ones(len(sig_df))
        else:
            bkg_weights, sig_weights = bkg_df.event_weight.copy(), sig_df.event_weight.copy()

        if sig_scale != 1:
            sig_weights *= sig_scale

        # Get stacked backgrounds
        bkg_hists = []
        if stacked:
            for name in bkg_df.name.unique():
                weights = bkg_weights[bkg_df.name == name]
                if norm:
                    weights /= sum(bkg_weights)
                sample_label = self.sample_labels[name] if name in self.sample_labels else name
                hist = yahist.Hist1D(
                    transf(bkg_df[bkg_df.name == name][column]),
                    bins=bins, 
                    weights=weights,
                    label=f"{sample_label} [{sum(weights):.1f} events]",
                    color=self.bkg_colors[name] if self.bkg_colors else None,
                    metadata={"sample_name": name}
                )
                bkg_hists.append(hist)

        # Get total background
        bkg_hist = yahist.Hist1D(
            transf(bkg_df[column]),
            bins=bins, 
            weights=bkg_weights,
            label=f"Total background [{sum(bkg_weights):0.1f} events]",
            color="k"
        )

        # Get signal
        sig_label = f"Total signal [{sum(sig_weights):0.1f} events]"
        if sig_scale != 1:
            sig_label = f"Total signal (x{sig_scale}) [{sum(sig_weights):0.1f} events]"

        sig_hist = yahist.Hist1D(
            transf(sig_df[column]),
            bins=bins, 
            weights=sig_weights,
            label=sig_label,
            color="r"
        )
        if norm:
            bkg_hist = bkg_hist.normalize()
            sig_hist = sig_hist.normalize()

        # Plot everything
        if bkg_hists:
            bkg_hists.sort(key=lambda h: sum(h.counts))

            hists_to_reorder = []
            for sample_name in self.stack_order:
                for hist in bkg_hists:
                    if hist.metadata.get("sample_name") == sample_name:
                        hists_to_reorder.append(hist)

            for hist in hists_to_reorder:
                bkg_hists.remove(hist)
                if self.stack_order_pos == -1:
                    bkg_hists.append(hist)
                elif self.stack_order_pos < 0:
                    bkg_hists.insert(self.stack_order_pos+1, hist)
                else:
                    bkg_hists.insert(self.stack_order_pos, hist)
                    
            yahist.utils.plot_stack(bkg_hists, ax=axes, histtype="stepfilled", log=logy)

        bkg_hist.plot(ax=axes, alpha=0.5, log=logy)
        sig_hist.plot(ax=axes, linewidth=2, log=logy)

        # Plot bkg MC error on histogram
        bkg_counts = bkg_hist.counts
        bkg_counts[bkg_counts == 0] = 1e-12
        err_points = np.repeat(bkg_hist.edges, 2)[1:-1]
        err_high = np.repeat(bkg_counts + bkg_hist.errors, 2)
        err_low = np.repeat(bkg_counts - bkg_hist.errors, 2)
        axes.fill_between(
            err_points, err_high, err_low,
            step="mid",
            hatch="///////",
            facecolor="none",
            edgecolor=(0.85, 0.85, 0.85),
            linewidth=0.0,
            linestyle="-",
            zorder=1.1,
            label=u"Background unc. [stat]"
        )

        hep.cms.label(self.cms_label, data=(self.cms_label != "Preliminary"), lumi=self.lumi, loc=0, ax=axes, fontsize=self.cms_size)
        
        axes.set_xlabel(x_label)
        if not logy:
            axes.set_ylim(bottom=0)
        else:
            axes.set_ylim(bottom=0.01)
        if stacked:
            axes.legend(fontsize=14, loc=legend_loc)
        else:
            axes.legend(loc=legend_loc)
        if norm:
            axes.set_ylabel("a.u.")
        else:
            axes.set_ylabel("Events")

        if self.plots_dir:
            plot_file = f"{self.plots_dir}/{column}_sig_vs_bkg.pdf"
            if stacked:
                plot_file = plot_file.replace(".pdf", "_stacked.pdf")
            if norm:
                plot_file = plot_file.replace(".pdf", "_norm.pdf")
            if logy:
                plot_file = plot_file.replace(".pdf", "_logy.pdf")
            if selection:
                plot_file = plot_file.replace(".pdf", f"_{PandasAnalysis.get_selection_str(selection)}.pdf")
                with open(plot_file.replace("pdf", "txt"), "w") as plot_txt:
                    plot_txt.write(f"SELECTION: {selection}")

            print(f"Wrote plot to {plot_file}")
            plt.savefig(plot_file, bbox_inches="tight")
            plt.savefig(plot_file.replace(".pdf", ".png"), bbox_inches="tight")

        return axes

    def plot_2D(self, x_col, y_col, x_bins, y_bins, 
                sel=None, sample="bkg", flip=False, year=None, rebin=None, show_counts=False,
                x_label=None, y_label=None, x_lim=None, y_lim=None, 
                x_transf=lambda x: x, y_transf=lambda y: y):

        x_label = x_label or x_col
        y_label = y_label or y_col

        if sample == "bkg":
            df = self.bkg_df(selection=sel)
        elif sample == "sig":
            df = self.sig_df(selection=sel)
        else:
            df = self.sample_df(sample, selection=sel)
            
        if not year is None:
            df = df[df.year.abs() == year]
            if year == 2016:
                lumi = 36.33
            elif year == 2017:
                lumi = 41.48
            elif year == 2018:
                lumi = 59.83
        else:
            lumi = 138

        # Plot 2D hist
        hist2d = yahist.Hist2D(
            (x_transf(df[x_col]), y_transf(df[y_col])),
            bins=[x_bins, y_bins],
            weights=df.event_weight
        )
        if flip:
            hist2d = hist2d.transpose()
        if not rebin is None:
            hist2d = hist2d.rebin(rebin)

        # UCSD colors
        cmap = clr.LinearSegmentedColormap.from_list("trident", ["#182B49","#00629B", "#C69214", "#FFCD00"], N=256)
        cmap.set_under("#182B49")

        # Plot 2D hist and 1D profile
        fig, axes = plt.subplots(figsize=(11, 11))
        mpl_objs = hist2d.plot(
            ax=axes, colorbar=False, counts=show_counts, 
            counts_formatter="{:.2f}".format, counts_fontsize=6, 
            cmap=cmap, zrange=(0, None), hide_empty=False
        );
        if flip:
            axes.set_ylabel(x_label);
            axes.set_xlabel(y_label);
        else:
            axes.set_xlabel(x_label);
            axes.set_ylabel(y_label);

        # Plot colorbar
        mappable = mpl_objs[0]
        fig = mappable.axes.figure
        divider = make_axes_locatable(mappable.axes)
        cax = divider.append_axes("right", size="5%", pad=0.1)
        cbar = fig.colorbar(mappable, cax=cax)
        cbar.set_label(r"Count");

        # Add CMS label
        hep.cms.label(
            "Preliminary",
            data=False,
            lumi=lumi,
            loc=0,
            ax=axes,
        );
        
        # Save plot
        outfile = f"{self.plots_dir}/hist2D_{sample}_{x_col}_{y_col}_1Dprofile.pdf"
        if not sample is None:
            outfile = outfile.replace(".pdf", f"_{sample}.pdf")
        if flip:
            outfile = outfile.replace(".pdf", "_flipped.pdf")
        if not year is None:
            outfile = outfile.replace(".pdf", f"_{year}.pdf")
            
        plt.savefig(outfile, bbox_inches="tight")
        plt.savefig(outfile.replace(".pdf", ".png"), bbox_inches="tight")
        print(f"Wrote plot to {outfile}")

    def fom_scan(self, variable, working_points, abs=False, show_raw=False, operator=">", 
                 base_selection="", fom=lambda s, b: s/np.sqrt(b)):

        working_points = list(working_points)
        if abs:
            variable = f"abs({variable})"

        if not show_raw:
            print("sel,sig,bkg,fom")
        else:
            print("sel,sig,,bkg,,fom")

        if base_selection:
            working_points.insert(0, "BASE")
        for wp in working_points:
            if wp == "BASE":
                sel = base_selection
            else:
                if base_selection:
                    sel = f"{base_selection} and {variable} {operator} {wp}"
                else:
                    sel = f"{variable} {operator} {wp}"
                
            sig_wgt, bkg_wgt = self.get_event_counts(selection=sel)
            if not show_raw:
                print(f"{sel},{sig_wgt},{bkg_wgt},{fom(sig_wgt, bkg_wgt)}")
            else:
                sig_raw, bkg_raw = self.get_event_counts(selection=sel, raw=True)
                print(f"{sel},{sig_wgt},{sig_raw},{bkg_wgt},{bkg_raw},{fom(sig_wgt, bkg_wgt)}")

    def bf_scan(self, scans, base_scan=None, 
                fom=lambda S, B: S/np.sqrt(B) if B > 0 else S/np.sqrt(0.0001)):
        scans = {
            "bdt": np.linspace(0.85, 0.95, 11),
            "hbbfatjet_score": np.linspace(0.88, 0.98, 11),
            "ld_vqqfatjet_score": np.linspace(0.88, 0.98, 11),
            "tr_vqqfatjet_score": np.linspace(0.88, 0.98, 11)
        }

        cuts = {}
        for var, wps in scans.items():
            cuts[var] = []
            for wp in wps:
                cuts[var].append(f"{var} > {wp}")

        selections = [" and ".join(combo) for combo in itertools.product(*cuts.values())]
        if base_selection:
            selections = [f"{sel} and {base_selection}" for sel in selections]
                    
        results = []
        for sel in tqdm(selections):
            sig, bkg = self.get_event_counts(selection=sel)
            results.append((sel, sig, bkg, fom(sig, bkg)))

        return results

class Validation(PandasAnalysis):

    def get_event_counts(self, selection=None, raw=False):
        data_count = self.data_count(selection=selection)
        mc_count = self.bkg_count(selection=selection, raw=raw)
        return data_count, mc_count

    def get_event_errors(self, selection=None, raw=False):
        data_error = self.data_error(selection=selection, raw=raw)
        mc_error = self.bkg_error(selection=selection, raw=raw)
        return data_error, mc_error

    def plot_blind_box(self, hist_axes, blind_low, blind_high, ratio_axes=None, box_hatch="//////", do_label=True):
        if do_label:
            hist_axes.hist(
                [],
                facecolor="whitesmoke",
                edgecolor="lightgrey",
                hatch=box_hatch,
                label="Blinded"
            )
        hist_axes.axvspan(
            blind_low, blind_high,
            facecolor="whitesmoke",
            edgecolor="lightgrey",
            hatch=box_hatch,
            zorder=1.9
        )
        if ratio_axes:
            ratio_axes.axvspan(
                blind_low, blind_high,
                facecolor="whitesmoke",
                edgecolor="lightgrey",
                hatch=box_hatch,
                zorder=1.9
            )

    def plot_data_vs_mc(self, column, bins, weights=None, blinded_range=None, blinded_box=True, autoblind=True,
                        sig_scale=0, selection="", x_label="", logy=False, transf=lambda x: x, norm=False, 
                        stacked=False, axes=None, return_hists=False, legend_loc="best", ratio_ylim=[0, 2],
                        group_hists={}, legend_counts=True):
        if not axes:
            fig = plt.figure()
            gs = gridspec.GridSpec(ncols=1, nrows=2, figure=fig, height_ratios=[2, 0.65], hspace=0.08)
            hist_axes = fig.add_subplot(gs[0])
            ratio_axes = fig.add_subplot(gs[1])
        else:
            hist_axes, ratio_axes = axes

        if logy:
            hist_axes.set_yscale("log", nonpositive="clip")

        # Get data (numerator)
        data_df = self.data_df(selection=selection)
        if blinded_range and not blinded_box:
            blind_low, blind_high = blinded_range
            data_df = data_df.query(f"not ({column} >= {blind_low} and {column} < {blind_high})")

        # Get bkg MC (denominator)
        bkg_df = self.bkg_df(selection=selection)
        sig_df = self.sig_df(selection=selection)
        
        data_weights = data_df.event_weight.copy()
        bkg_weights = bkg_df.event_weight.copy()
        sig_weights = sig_df.event_weight.copy()
        
        if weights:
            for weight in weights:
                data_weights *= data_df[weight]
                bkg_weights *= bkg_df[weight]
                sig_weights *= sig_df[weight]

        data_hist = yahist.Hist1D(
            transf(data_df[column]),
            bins=bins,
            weights=data_weights,
            label=f"Data [{len(data_df)} events]" if legend_counts else "Data",
            color="k"
        )
        bkg_hist = yahist.Hist1D(
            transf(bkg_df[column]),
            bins=bins,
            weights=bkg_weights,
            label=f"Total background [{bkg_df.event_weight.sum():0.1f} events]" if legend_counts else "Total background",
            color="k"
        )

        sig_hist = yahist.Hist1D(
            transf(sig_df[column]),
            bins=bins,
            weights=sig_weights,
            label=f"Total signal [{sig_df.event_weight.sum():0.1f} events]" if legend_counts else "Total signal",
            color="r"
        )

        # Get bkg MC bin counts for automatic blinding
        bkg_counts = bkg_hist.counts.copy()
        bkg_counts[bkg_counts == 0] = 1e-12
            
        # Find bins to automatically blind
        autoblind_bins = (bkg_counts + sig_hist.counts > bkg_counts + 0.2*bkg_hist.errors)

        if norm:
            data_hist = data_hist.normalize()
            bkg_hist = bkg_hist.normalize()

        # Do automatic blinding
        if autoblind:
            data_hist.counts[autoblind_bins] = 0

        # Get stacked backgrounds
        grouped_hists = {}
        bkg_hists = []
        if stacked:
            for name in bkg_df.name.unique():
                weights = bkg_df.event_weight[bkg_df.name == name]
                if norm:
                    weights /= bkg_df.event_weight.sum()
                sample_label = self.sample_labels[name] if name in self.sample_labels else name
                if legend_counts:
                    label = f"{sample_label} [{bkg_df.event_weight[bkg_df.name == name].sum():.1f} events]" 
                else:
                    label = sample_label

                hist = yahist.Hist1D(
                    transf(bkg_df[bkg_df.name == name][column]),
                    bins=bins,
                    weights=weights,
                    label=label,
                    color=self.bkg_colors[name] if self.bkg_colors else None,
                    metadata={"sample_name": name}
                )

                in_group = False
                for group_name, sample_names in group_hists.items():
                    if name in sample_names:
                        hist.metadata["label"] = group_name
                        in_group = True
                        if group_name in grouped_hists:
                            grouped_hists[group_name] += hist
                        else:
                            grouped_hists[group_name] = hist

                if not in_group:
                    bkg_hists.append(hist)

        for group_name, hist in grouped_hists.items():
            hist.metadata["sample_name"] = group_name

        bkg_hists += grouped_hists.values()

        # Get ratio
        ratio_hist = data_hist/bkg_hist
        # Set ratio errors to data relative stat error times the ratio
        data_counts = data_hist.counts.copy()
        data_counts[data_counts == 0] = 1e-12
        ratio_hist._errors = (data_hist.errors/data_counts)*ratio_hist.counts

        # Plot stacked bkg hists
        if bkg_hists:
            bkg_hists.sort(key=lambda h: sum(h.counts))

            hists_to_reorder = []
            for sample_name in self.stack_order:
                for hist in bkg_hists:
                    if hist.metadata.get("sample_name") == sample_name:
                        hists_to_reorder.append(hist)

            for hist in hists_to_reorder:
                bkg_hists.remove(hist)
                if self.stack_order_pos == -1:
                    bkg_hists.append(hist)
                elif self.stack_order_pos < 0:
                    bkg_hists.insert(self.stack_order_pos+1, hist)
                else:
                    bkg_hists.insert(self.stack_order_pos, hist)

            yahist.utils.plot_stack(bkg_hists, ax=hist_axes, histtype="stepfilled")

        # Plot hists and ratio
        bkg_hist.plot(ax=hist_axes, alpha=0.5, zorder=1)
        data_hist.plot(ax=hist_axes, errors=True, zorder=1.2)
        ratio_hist.plot(ax=ratio_axes, errors=True, color="k", zorder=1.2)

        if sig_scale > 0:
            if sig_scale != 1:
                sig_hist *= sig_scale
                if legend_counts:
                    sig_hist.metadata["label"] = f"Total signal (x{sig_scale}) [{np.sum(sig_hist.counts):0.1f} events]"
                else:
                    sig_hist.metadata["label"] = f"Total signal (x{sig_scale})"
            sig_hist.plot(ax=hist_axes, linewidth=2)

        # Get bkg MC bin counts for error calculation (again in case of norm)
        bkg_counts = bkg_hist.counts.copy()
        bkg_counts[bkg_counts == 0] = 1e-12

        # Plot MC relative stat error on unity; this makes no sense, but is LHC common practice
        err_points = np.repeat(bkg_hist.edges, 2)[1:-1]
        err_high = np.repeat(1 + bkg_hist.errors/bkg_counts, 2)
        err_low = np.repeat(1 - bkg_hist.errors/bkg_counts, 2)
        ratio_axes.fill_between(
            err_points, err_high, err_low,
            step="mid",
            hatch="///////",
            facecolor="none",
            edgecolor=(0.85, 0.85, 0.85),
            linewidth=0.0,
            linestyle="-",
            zorder=1.1
        )

        # Plot MC error on histogram
        err_high = np.repeat(bkg_counts + bkg_hist.errors, 2)
        err_low = np.repeat(bkg_counts - bkg_hist.errors, 2)
        hist_axes.fill_between(
            err_points, err_high, err_low,
            step="mid",
            hatch="///////",
            facecolor="none",
            edgecolor=(0.85, 0.85, 0.85),
            linewidth=0.0,
            linestyle="-",
            zorder=1.1,
            # label=u"MC unc. [stat \u2295 syst]" # uncomment this if you (a) have syst errors and (b) can add them here
            label=u"MC unc. [stat]"
        )

        if blinded_range and blinded_box:
            blind_low, blind_high = blinded_range
            self.plot_blind_box(hist_axes, blind_low, blind_high, ratio_axes=ratio_axes)

        hist_axes.set_xticklabels([])
        ratio_axes.axhline(y=1, color="k", linestyle="--", alpha=0.75, linewidth=0.75)
        ratio_axes.legend().remove()
        ratio_axes.set_xlabel(x_label)
        ratio_axes.set_ylabel("data/MC")
        ratio_axes.set_ylim(ratio_ylim)

        hep.cms.label(self.cms_label, data=True, lumi=self.lumi, loc=0, ax=hist_axes, fontsize=self.cms_size)

        if stacked:
            hist_axes.legend(fontsize=14, loc=legend_loc).set_zorder(101)
        else:
            hist_axes.legend(loc=legend_loc).set_zorder(101)
            
        if not logy:
            hist_axes.set_ylim(bottom=0)
        else:
            hist_axes.set_ylim(bottom=0.001)
            if norm:
                hist_axes.set_ylim(top=1)
            
        if norm:
            hist_axes.set_ylabel("a.u.")
        else:
            hist_axes.set_ylabel("Events")

        if self.plots_dir:
            plot_file = f"{self.plots_dir}/{column}_data_vs_mc.pdf"
            if norm:
                plot_file = plot_file.replace(".pdf", "_norm.pdf")
            if logy:
                plot_file = plot_file.replace(".pdf", "_log.pdf")
            if selection:
                plot_file = plot_file.replace(".pdf", f"_{PandasAnalysis.get_selection_str(selection)}.pdf")
                with open(plot_file.replace("pdf", "txt"), "w") as plot_txt:
                    plot_txt.write(f"SELECTION: {selection}")

            print(f"Wrote plot to {plot_file}")
            plt.savefig(plot_file, bbox_inches="tight")
            plt.savefig(plot_file.replace(".pdf", ".png"), bbox_inches="tight")

        if return_hists:
            return hist_axes, ratio_axes, (data_hist, bkg_hist, ratio_hist)
        else:
            return hist_axes, ratio_axes

class Extrapolation(PandasAnalysis):

    def get_event_counts(self, selection=None, raw=False):
        sig_count = self.sig_count(selection=selection, raw=raw)
        bkg_count = self.bkg_count(selection=selection, raw=raw)
        return sig_count, bkg_count

    def get_event_errors(self, selection=None, raw=False):
        sig_error = self.sig_error(selection=selection, raw=raw)
        bkg_error = self.bkg_error(selection=selection, raw=raw)
        return sig_error, bkg_error

    def ABCD(self, h_right, v_up, base_selection, h_dir="right", v_dir="up", label="",
             show_raw=False, show_data=False):
        """
        Compute yields for an ABCD extrapolation in the following configuration:

               D   |   C
            -------|-------
               A   |   B

        where Region D is the signal region, and Regions A, B, and C are control regions 
        such that the yield in Region D can be estimated from data as follows: 

            Predicted Region D = (Region C)*(Region A)/(Region B)

        Returns the predicted yield in Region D with the statistical and systematic 
        percent errors when possible. 
        """
        # Assemble regions
        h_left = f"(not ({h_right}))"
        v_down = f"(not ({v_up}))"
        if h_dir.lower() == "right" and v_dir.lower() == "up":
            A_sel = f"{base_selection} and {h_left} and {v_down}"
            B_sel = f"{base_selection} and {h_right} and {v_down}"
            C_sel = f"{base_selection} and {h_right} and {v_up}"
            D_sel = f"{base_selection} and {h_left} and {v_up}"
        if h_dir.lower() == "left" and v_dir.lower() == "up":
            A_sel = f"{base_selection} and {h_right} and {v_down}"
            B_sel = f"{base_selection} and {h_left} and {v_down}"
            C_sel = f"{base_selection} and {h_left} and {v_up}"
            D_sel = f"{base_selection} and {h_right} and {v_up}"
        if h_dir.lower() == "right" and v_dir.lower() == "down":
            A_sel = f"{base_selection} and {h_left} and {v_up}"
            B_sel = f"{base_selection} and {h_right} and {v_up}"
            C_sel = f"{base_selection} and {h_right} and {v_down}"
            D_sel = f"{base_selection} and {h_left} and {v_down}"
        if h_dir.lower() == "left" and v_dir.lower() == "down":
            A_sel = f"{base_selection} and {h_left} and {v_up}"
            B_sel = f"{base_selection} and {h_right} and {v_up}"
            C_sel = f"{base_selection} and {h_right} and {v_down}"
            D_sel = f"{base_selection} and {h_left} and {v_down}"

        # Get yields and errors for each region
        A_sig_wgt, A_bkg_wgt = self.get_event_counts(selection=A_sel)
        A_sig_err, A_bkg_err = self.get_event_errors(selection=A_sel)
        B_sig_wgt, B_bkg_wgt = self.get_event_counts(selection=B_sel)
        B_sig_err, B_bkg_err = self.get_event_errors(selection=B_sel)
        C_sig_wgt, C_bkg_wgt = self.get_event_counts(selection=C_sel)
        C_sig_err, C_bkg_err = self.get_event_errors(selection=C_sel)
        D_sig_wgt, D_bkg_wgt = self.get_event_counts(selection=D_sel)
        D_sig_err, D_bkg_err = self.get_event_errors(selection=D_sel)
        if show_raw:
            A_sig_raw, A_bkg_raw = self.get_event_counts(selection=A_sel, raw=True)
            B_sig_raw, B_bkg_raw = self.get_event_counts(selection=B_sel, raw=True)
            C_sig_raw, C_bkg_raw = self.get_event_counts(selection=C_sel, raw=True)
            D_sig_raw, D_bkg_raw = self.get_event_counts(selection=D_sel, raw=True)
        if show_data:
            if A_sig_wgt/A_bkg_wgt <= 0.2:
                A_data = self.data_count(selection=A_sel)
                A_data_err = np.sqrt(A_data)
            else:
                print("WARNING: Region A is signal polluted, blinding this region")
                A_data = "BLINDED"
                A_data_err = "BLINDED"
            if B_sig_wgt/B_bkg_wgt <= 0.2:
                B_data = self.data_count(selection=B_sel)
                B_data_err = np.sqrt(B_data)
            else:
                print("WARNING: Region B is signal polluted, blinding this region")
                B_data = "BLINDED"
                B_data_err = "BLINDED"
            if C_sig_wgt/C_bkg_wgt <= 0.2:
                C_data = self.data_count(selection=C_sel)
                C_data_err = np.sqrt(C_data)
            else:
                print("WARNING: Region C is signal polluted, blinding this region")
                C_data = "BLINDED"
                C_data_err = "BLINDED"

        # Construct tables
        if show_raw:
            header = "cut,region,bkg_raw,bkg_wgt,bkg_err,sig_raw,sig_wgt,sig_err"
            A_row = f"{A_sel},A,{A_bkg_raw},{A_bkg_wgt},{A_bkg_err},{A_sig_raw},{A_sig_wgt},{A_sig_err}"
            B_row = f"{B_sel},B,{B_bkg_raw},{B_bkg_wgt},{B_bkg_err},{B_sig_raw},{B_sig_wgt},{B_sig_err}"
            C_row = f"{C_sel},C,{C_bkg_raw},{C_bkg_wgt},{C_bkg_err},{C_sig_raw},{C_sig_wgt},{C_sig_err}"
            D_row = f"{D_sel},D,{D_bkg_raw},{D_bkg_wgt},{D_bkg_err},{D_sig_raw},{D_sig_wgt},{D_sig_err}"
        else:
            header = "cut,region,bkg_wgt,bkg_err,sig_wgt,sig_err"
            A_row = f"{A_sel},A,{A_bkg_wgt},{A_bkg_err},{A_sig_wgt},{A_sig_err}"
            B_row = f"{B_sel},B,{B_bkg_wgt},{B_bkg_err},{B_sig_wgt},{B_sig_err}"
            C_row = f"{C_sel},C,{C_bkg_wgt},{C_bkg_err},{C_sig_wgt},{C_sig_err}"
            D_row = f"{D_sel},D,{D_bkg_wgt},{D_bkg_err},{D_sig_wgt},{D_sig_err}"
        if show_data:
            header += ",data,data_err"
            A_row += f",{A_data},{A_data_err}"
            B_row += f",{B_data},{B_data_err}"
            C_row += f",{C_data},{C_data_err}"
            D_row += f",BLINDED,BLINDED"

        # Calculate extrapolation factors and relative errors
        BtoA_MC_extp = A_bkg_wgt/B_bkg_wgt
        BtoA_MC_err = np.sqrt((B_bkg_err/B_bkg_wgt)**2 + (A_bkg_err/A_bkg_wgt)**2)
        if A_data == "BLINDED" or B_data == "BLINDED" or C_data == "BLINDED":
            BtoA_data_extp = None
            BtoA_data_err = None
        else:
            BtoA_data_extp = A_data/B_data
            BtoA_data_err = np.sqrt(1/A_data + 1/B_data)

        # Print tables
        print(header)
        print(A_row)
        print(B_row)
        print(C_row)
        print(D_row)
        print("")
        print("name,extp,rel_err")
        print(f"BtoA_MC,{BtoA_MC_extp},{BtoA_MC_err}")
        print(f"BtoA_data,{BtoA_data_extp},{BtoA_data_err}")

        if A_data == "BLINDED" or B_data == "BLINDED" or C_data == "BLINDED":
            return 0, 0, 0

        # Compute systematic and statistical percent errors
        D_pred_MC = A_bkg_wgt/B_bkg_wgt*C_bkg_wgt
        syst_err = abs(1 - D_pred_MC/D_bkg_wgt)
        stat_err = np.sqrt(1/A_data + 1/B_data + 1/C_data)

        # Compute predicted yield in data
        D_pred_data = A_data/B_data*C_data

        return D_pred_data, stat_err, syst_err
