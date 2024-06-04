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
from sklearn.metrics import roc_curve

def clip(np_array, bins):
    clip_low = 0.5*(bins[0] + bins[1])
    clip_high = 0.5*(bins[-2] + bins[-1])
    return np.clip(np_array, clip_low, clip_high)

class PandasPlotter:
    def __init__(self, sig_root_files=None, bkg_root_files=None, data_root_files=None, 
                 ttree_name="Events", weight_columns=None, reweight_column=None, 
                 plots_dir=None, sample_labels={}, drop_columns=[], lumi=138, legend_counts=True, 
                 cms_label="Preliminary", cms_size=None, stack_order=None, stack_order_pos=-1):

        if reweight_column:
            drop_columns.append(reweight_column)

        self.lumi = lumi
        self.cms_label = cms_label
        self.cms_size = cms_size
        self.stack_order = stack_order or []
        self.stack_order_pos = stack_order_pos
        self.legend_counts = legend_counts

        # Load dataframes
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

    def make_selection(self, selection):
        if selection and type(selection) == str:
            self.df = self.df[self.df.eval(selection)].copy()

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

    def data_hist(self, column, bins, selection="", transf=lambda x: x, norm=False):
        # Get dataframe
        df = self.data_df(selection=selection)

        # Get weights
        weights = np.ones(len(df))
        # Normalize
        if norm:
            weights /= sum(weights)

        return yahist.Hist1D(
            transf(df[column]),
            bins=bins, 
            label=f"Data [{len(df)} events]",
            color="k"
        )

    def sig_hist(self, column, bins, selection="", transf=lambda x: x, 
                 extra_weights=None, raw=False, norm=False, color="r", scale=1):

        # Get dataframe
        df = self.sig_df(selection=selection)

        # Get weights
        if raw:
            weights = np.ones(len(df))
        else:
            weights = df.event_weight.values
        # Apply extra weights
        if not (extra_weights is None):
            for weight in extra_weights:
                weights *= df[weight]
        # Normalize
        if norm:
            weights /= sum(weights)

        # Get label
        label = "Total signal"
        if scale != 1:
            label += f" (x{scale})"
        if self.legend_counts:
            label += f" [{sum(weights):0.1f} events]"

        # Get histogram
        hist = yahist.Hist1D(transf(df[column]), bins=bins, weights=weights, label=label, color=color)

        return hist*scale

    def bkg_hist(self, column, bins, selection="", transf=lambda x: x, 
                 extra_weights=None, raw=False, norm=False, groups={}):

        df = self.bkg_df(selection=selection)

        # Get weights
        if raw:
            weights = np.ones(len(df))
        else:
            weights = df.event_weight.values
        # Apply extra weights
        if not (extra_weights is None):
            for weight in extra_weights:
                weights *= df[weight]

        # Get individual histograms
        grouped_hists ={}
        hists = []
        for name in df.name.unique():
            # Get weights
            wgts = weights[df.name == name]
            if norm:
                wgts /= sum(weights)
            # Get label
            sample_label = self.sample_labels[name] if name in self.sample_labels else name
            if self.legend_counts:
                sample_label += f" [{sum(wgts):0.1f} events]"
            # Get histogram
            hist = yahist.Hist1D(
                transf(df[df.name == name][column]),
                bins=bins, 
                weights=wgts,
                label=sample_label,
                color=self.bkg_colors[name] if self.bkg_colors else None
            )

            # Check if histogram belongs to a group
            in_group = False
            for group_name, sample_names in groups.items():
                if name in sample_names:
                    hist.metadata["label"] = group_name
                    in_group = True
                    # Add the histogram to the group
                    if group_name in grouped_hists:
                        grouped_hists[group_name] += hist
                    else:
                        grouped_hists[group_name] = hist

            if not in_group:
                hists.append(hist)

        # Change metadata for grouped hists
        for group_name, hist in grouped_hists.items():
            hist.metadata["sample_name"] = group_name

        # Add grouped hists back to stack
        hists += grouped_hists.values()

        # Sort individual histograms by integral by default
        hists.sort(key=lambda h: sum(h.counts))
        # Check if any of them have been arbitrarily sorted
        hists_to_reorder = []
        for sample_name in self.stack_order:
            for hist in hists:
                if hist.metadata.get("sample_name") == sample_name:
                    hists_to_reorder.append(hist)
        # Reorder to the arbitrary sorting
        for hist in hists_to_reorder:
            hists.remove(hist)
            if self.stack_order_pos == -1:
                hists.append(hist)
            elif self.stack_order_pos < 0:
                hists.insert(self.stack_order_pos+1, hist)
            else:
                hists.insert(self.stack_order_pos, hist)

        # Normalize total
        if norm:
            weights /= sum(weights)

        # Get label
        label = "Total bkg"
        if self.legend_counts:
            label += f" [{sum(weights):0.1f} events]"

        # Get total hist
        hist = yahist.Hist1D(transf(df[column]), bins=bins, weights=weights, label=label, color="k")

        return hist, hists

    def plot_hatched_error(self, hist, axes, ratio=False, hatch="///////", color=(0.85, 0.85, 0.85), label=""):
        counts = hist.counts
        counts[counts == 0] = 1e-12
        err_points = np.repeat(hist.edges, 2)[1:-1]
        if ratio:
            err_high = np.repeat(1 + hist.errors/counts, 2)
            err_low = np.repeat(1 - hist.errors/counts, 2)
        else:
            err_high = np.repeat(counts + hist.errors, 2)
            err_low = np.repeat(counts - hist.errors, 2)

        axes.fill_between(
            err_points, err_high, err_low, step="mid",
            hatch=hatch, facecolor="none", edgecolor=color,
            linewidth=0.0, linestyle="-", label=label
        )

    def plot_sig_vs_bkg(self, column, bins, axes=None, extra_weights=None, selection="", transf=lambda x: x, 
                        grouped_bkgs={}, raw=False, logy=False, norm=False, stacked=True, x_label="",
                        xlim=[None, None], ylim=[None, None],
                        legend_loc="best", legend_ncol=1, sig_scale=1, return_hists=False, extra_hists=[]):
        if not axes:
            fig, axes = plt.subplots()

        sig_hist = self.sig_hist(
            column, bins, selection=selection, transf=transf,
            extra_weights=extra_weights, raw=raw, norm=norm, scale=sig_scale
        )

        bkg_hist, bkg_stacked = self.bkg_hist(
            column, bins, selection=selection, transf=transf,
            extra_weights=extra_weights, raw=raw, norm=norm, groups=grouped_bkgs
        )

        # Plot stacked background
        if stacked:
            yahist.utils.plot_stack(bkg_stacked, ax=axes, histtype="stepfilled", log=logy)

        # Plot total signal and background
        bkg_hist.plot(ax=axes, alpha=0.5, log=logy)
        sig_hist.plot(ax=axes, linewidth=2, log=logy)
        for hist in extra_hists:
            hist.plot(ax=axes, linewidth=2, log=logy)

        # Plot bkg MC error on histogram
        self.plot_hatched_error(bkg_hist, axes, label=u"Background unc. [stat]")
        
        # Plot CMS label
        hep.cms.label(
            self.cms_label, data=(self.cms_label != "Preliminary"), 
            lumi=self.lumi, loc=0, ax=axes, fontsize=self.cms_size
        )
        
        # Format axes
        axes.set_xlabel(x_label)

        ylow, yhigh = ylim
        if ylow is None:
            if not logy:
                axes.set_ylim(bottom=0)
            else:
                hist_axes.set_ylim(bottom=0.01)
                if norm:
                    hist_axes.set_ylim(top=1)
        else:
            axes.set_ylim(bottom=ylow, top=yhigh)
            
        xlow, xhigh = xlim
        if not (xlow is None) or not (xhigh is None):
            axes.set_xlim(left=xlow, right=xhigh)

        if stacked:
            axes.legend(fontsize=14, loc=legend_loc, ncol=legend_ncol)
        else:
            axes.legend(loc=legend_loc, ncol=legend_ncol)
        if norm:
            axes.set_ylabel("Normalized to unity")
        else:
            axes.set_ylabel("Events")

        # Save plot
        if self.plots_dir:
            os.makedirs(self.plots_dir, exist_ok=True)
            # Generate plot file name
            plot_file = f"{self.plots_dir}/{column}_sig_vs_bkg.pdf"
            if stacked:
                plot_file = plot_file.replace(".pdf", "_stacked.pdf")
            if norm:
                plot_file = plot_file.replace(".pdf", "_norm.pdf")
            if logy:
                plot_file = plot_file.replace(".pdf", "_logy.pdf")
            if selection:
                plot_file = plot_file.replace(".pdf", f"_{PandasPlotter.get_selection_str(selection)}.pdf")
                with open(plot_file.replace("pdf", "txt"), "w") as plot_txt:
                    plot_txt.write(f"SELECTION: {selection}")
            # Write plot to .png and .pdf
            print(f"Wrote plot to {plot_file}")
            plt.savefig(plot_file, bbox_inches="tight")
            plt.savefig(plot_file.replace(".pdf", ".png"), bbox_inches="tight")

        if return_hists:
            return axes, (sig_hist, bkg_stacked)
        else:
            return axes, 

    def plot_many_sig_vs_bkg(self, col_bins_label_tuples, **kwargs):
        for col, bins, x_label in col_bins_label_tuples:
            self.plot_sig_vs_bkg(col, bins=bins, x_label=x_label, **kwargs)
            plt.close()

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

    def plot_data_vs_mc(self, column, bins, axes=None, extra_weights=None, selection="", transf=lambda x: x, 
                        blinded_box=None, autoblind=True, blinded_cut=None,
                        xlim=[None, None], hist_ylim=[None, None], ratio_ylim=[0, 2], 
                        grouped_bkgs={}, logy=False, norm=False, stacked=True, x_label="",
                        legend_loc="best", legend_ncol=1, sig_scale=0, return_hists=False, extra_hists=[]):
        if not axes:
            fig = plt.figure()
            gs = gridspec.GridSpec(ncols=1, nrows=2, figure=fig, height_ratios=[2, 0.65], hspace=0.08)
            hist_axes = fig.add_subplot(gs[0])
            ratio_axes = fig.add_subplot(gs[1])
        else:
            hist_axes, ratio_axes = axes

        if logy:
            hist_axes.set_yscale("log", nonpositive="clip")
        
        sig_hist = self.sig_hist(
            column, bins, selection=selection, transf=transf,
            extra_weights=extra_weights, norm=norm, scale=sig_scale
        )

        bkg_hist, bkg_stacked = self.bkg_hist(
            column, bins, selection=selection, transf=transf,
            extra_weights=extra_weights, norm=norm, groups=grouped_bkgs
        )

        if blinded_cut is None:
            data_selection = selection
        else:
            data_selection = f"({selection}) and ({blinded_cut})"
        data_hist = self.data_hist(
            column, bins, selection=data_selection, transf=transf, 
            norm=norm
        )

        # Get bkg MC bin counts for automatic blinding
        bkg_counts = bkg_hist.counts.copy()
        bkg_counts[bkg_counts == 0] = 1e-12
        # Find bins to automatically blind
        autoblind_bins = (bkg_counts + sig_hist.counts > bkg_counts + 0.2*bkg_hist.errors)
        # Do automatic blinding
        if autoblind:
            data_hist.counts[autoblind_bins] = 0
            data_hist.errors[autoblind_bins] = 0

        # Get ratio
        ratio_hist = data_hist/bkg_hist
        # Set ratio errors to data relative stat error times the ratio
        data_counts = data_hist.counts.copy()
        data_counts[data_counts == 0] = 1e-12
        ratio_hist._errors = (data_hist.errors/data_counts)*ratio_hist.counts

        # Plot stacked bkg hists
        if stacked:
            yahist.utils.plot_stack(bkg_stacked, ax=hist_axes, histtype="stepfilled")

        # Plot hists and ratio
        bkg_hist.plot(ax=hist_axes, alpha=0.5, zorder=1)
        data_hist.plot(ax=hist_axes, errors=True, errors_binwidth=False, zorder=1.2)
        ratio_hist.plot(ax=ratio_axes, errors=True, errors_binwidth=False, color="k", zorder=1.2)
        for hist_i, hist in enumerate(extra_hists):
            hist.plot(ax=hist_axes, linewidth=2, zorder=hist_i)

        # Plot signal
        if sig_scale > 0:
            sig_hist.plot(ax=hist_axes, linewidth=2)

        # Plot errors
        self.plot_hatched_error(bkg_hist, hist_axes, ratio=False, label="Background unc. [stat]")
        self.plot_hatched_error(bkg_hist, ratio_axes, ratio=True)

        # Plot blinding box if desired
        if blinded_box:
            blind_low, blind_high = blinded_box
            self.plot_blind_box(hist_axes, blind_low, blind_high, ratio_axes=ratio_axes)

        # Plot CMS label
        hep.cms.label(self.cms_label, data=True, lumi=self.lumi, loc=0, ax=hist_axes, fontsize=self.cms_size)

        # Format axes
        hist_axes.set_xticklabels([]) # remove tick labels because they are already on the ratio
        ratio_axes.axhline(y=1, color="k", linestyle="--", alpha=0.75, linewidth=0.75)
        ratio_axes.legend().remove()
        ratio_axes.set_xlabel(x_label)
        ratio_axes.set_ylabel("Data / MC")
        ratio_axes.set_ylim(ratio_ylim)
        if stacked:
            hist_axes.legend(fontsize=14, loc=legend_loc, ncol=legend_ncol).set_zorder(101)
        else:
            hist_axes.legend(loc=legend_loc, ncol=legend_ncol).set_zorder(101)

        ylow, yhigh = hist_ylim
        if ylow is None:
            if not logy:
                hist_axes.set_ylim(bottom=0)
            else:
                hist_axes.set_ylim(bottom=0.01)
                if norm:
                    hist_axes.set_ylim(top=1)
        else:
            hist_axes.set_ylim(bottom=ylow, top=yhigh)

        xlow, xhigh = xlim
        if not (xlow is None) or not (xhigh is None):
            hist_axes.set_xlim(left=xlow, right=xhigh)
            ratio_axes.set_xlim(left=xlow, right=xhigh)

        if norm:
            hist_axes.set_ylabel("Normalized to unity")
        else:
            hist_axes.set_ylabel("Events")

        # Save plot
        if self.plots_dir:
            os.makedirs(self.plots_dir, exist_ok=True)
            # Get plot file name
            plot_file = f"{self.plots_dir}/{column}_data_vs_mc.pdf"
            if norm:
                plot_file = plot_file.replace(".pdf", "_norm.pdf")
            if logy:
                plot_file = plot_file.replace(".pdf", "_log.pdf")
            if selection:
                plot_file = plot_file.replace(".pdf", f"_{PandasPlotter.get_selection_str(selection)}.pdf")
                with open(plot_file.replace("pdf", "txt"), "w") as plot_txt:
                    plot_txt.write(f"SELECTION: {selection}")
            # Write plot .png and .pdf file
            print(f"Wrote plot to {plot_file}")
            plt.savefig(plot_file, bbox_inches="tight")
            plt.savefig(plot_file.replace(".pdf", ".png"), bbox_inches="tight")

        if return_hists:
            return hist_axes, ratio_axes, (data_hist, bkg_hist, ratio_hist)
        else:
            return hist_axes, ratio_axes

    def plot_many_data_vs_mc(self, col_bins_label_tuples, **kwargs):
        for col, bins, x_label in col_bins_label_tuples:
            self.plot_data_vs_mc(col, bins=bins, x_label=x_label, **kwargs)
            plt.close()

    def plot_correlation2D(self, x_col, y_col, x_bins, y_bins, 
                           selection=None, sample=None, flip=False, year=None, rebin=None, show_counts=False,
                           x_label=None, y_label=None, x_lim=None, y_lim=None, 
                           x_transf=lambda x: x, y_transf=lambda y: y):

        x_label = x_label or x_col
        y_label = y_label or y_col

        if sample is None:
            df = self.bkg_df(selection=selection)
        else:
            df = self.sample_df(sample, selection=selection)
            
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
        hep.cms.label("Preliminary", data=False, lumi=lumi, loc=0, ax=axes);
        
        # Save plot
        if self.plots_dir:
            os.makedirs(self.plots_dir, exist_ok=True)
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


    def plot_roc_curve(self, score_column, selection="", name="", signal_tests=[]):
        fig, axes = plt.subplots()

        df = self.df[~self.df.is_data]

        selection = df.eval(selection)

        fpr, tpr, thresh = roc_curve(
            df[selection].is_signal, 
            df[selection][score_column], 
            sample_weight=df[selection].event_weight
        )

        fig, axes = plt.subplots(figsize=(12, 12))
        axes.plot(fpr, tpr, label=f"{name} (AUC = {np.trapz(tpr, fpr):.2f})");

        total_sig = df[selection & (df.is_signal)].event_weight.sum()
        total_bkg = df[selection & (~df.is_signal)].event_weight.sum()

        markers = ["*", "v", "^", "o", "s"]
        colors = ["c", "b", "m", "r", "y"]
        if len(signal_tests) > min(len(markers), len(colors)):
            raise Exception(
                f"PandasPlotter.plot_roc_curve only supports {min(len(markers), len(colors))} signal tests"
            )

        for test_i, n_sig in enumerate(signal_tests):
            test_tpr = n_sig/total_sig
            axes.scatter(
                fpr[tpr >= test_tpr][0], 
                tpr[tpr >= test_tpr][0], 
                marker=markers[test_i], s=100, zorder=100+test_i, color=colors[test_i], 
                label=f"{n_sig} signal ({fpr[tpr >= test_tpr][0]*total_bkg:0.2f} bkg) events"
            )

        axes.tick_params(axis="both", which="both", direction="in", labelsize=32, top=True, right=True)
        axes.set_xlabel("Background efficiency", size=32)
        axes.set_ylabel("Signal efficiency", size=32)
        axes.legend()

        if self.plots_dir:
            plt.savefig(f"{self.plots_dir}/{score_column}_roc.png")
            plt.savefig(f"{self.plots_dir}/{score_column}_roc.pdf")
            print(f"Wrote to {self.plots_dir}/{score_column}_roc.pdf")

            axes.set_xscale("log")
            plt.savefig(f"{self.plots_dir}/{score_column}_roc_logx.png")
            plt.savefig(f"{self.plots_dir}/{score_column}_roc_logx.pdf")
            print(f"Wrote to {self.plots_dir}/{score_column}_roc_logx.pdf")

        hep.cms.label("Preliminary", data=False, lumi=self.lumi, loc=0, ax=axes)
