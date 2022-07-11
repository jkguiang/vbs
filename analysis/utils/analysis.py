import uproot
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib import gridspec
from matplotlib import colors
from matplotlib import pyplot as plt
from matplotlib.ticker import AutoMinorLocator
from matplotlib.colors import LogNorm, Normalize
from mpl_toolkits.axes_grid1 import ImageGrid
import scipy.stats

from utils.cutflow import Cut, Cutflow, CutflowCollection

def clip(np_array, bins):
    clip_low = 0.5 * (bins[0] + bins[1])
    clip_high = 0.5 * (bins[-2] + bins[-1])
    return np.clip(np_array, clip_low, clip_high)

class PandasAnalysis:
    def __init__(self, sig_root_files=[], bkg_root_files=[], data_root_files=[], 
                 ttree_name="Events", weight_columns=[]):
        dfs = []
        # Load signal
        for root_file in sig_root_files:
            name = root_file.split("/")[-1].replace(".root", "")
            with uproot.open(root_file) as f:
                df = f.get(ttree_name).arrays(library="pd")
                df["name"] = name
                df["is_signal"] = True
                df["is_data"] = False
                dfs.append(df)
        # Load background
        for root_file in bkg_root_files:
            name = root_file.split("/")[-1].replace(".root", "")
            with uproot.open(root_file) as f:
                df = f.get(ttree_name).arrays(library="pd")
                df["name"] = name
                df["is_signal"] = False
                df["is_data"] = False
                dfs.append(df)
        # Load data
        for root_file in data_root_files:
            name = root_file.split("/")[-1].replace(".root", "")
            with uproot.open(root_file) as f:
                df = f.get(ttree_name).arrays(library="pd")
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

        self.cutflows = CutflowCollection(
            cutflows={name: Cutflow() for name in self.df.name.unique()}
        )
        self.__update_cutflows("Base")

    def __update_cutflows(self, selection):
        selection = selection.replace(">", "gt")
        selection = selection.replace("<", "lt")
        selection = selection.replace(">=", "geq")
        selection = selection.replace("<=", "leq")
        selection = selection.replace(".", "p")
        selection = selection.replace("-", "m")
        selection = selection.replace(" ", "_")
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

        self.cutflows["TotalBkg"] = self.cutflows.sum()
        self.cutflows["TotalBkg"] -= self.cutflows["data"]


    def print_cutflow(self):
        print("\n".join(self.cutflows.get_csv(self.cutflows.terminal_cut_names[-1])))

    def make_selection(self, selection):
        if selection and type(selection) == str:
            self.df = self.df[self.df.eval(selection)].copy()
            self.__update_cutflows(selection)

    def sig_df(self, selection=None):
        if not selection:
            return self.df[~self.df.is_data & self.df.is_signal]
        else:
            if selection and type(selection) == str:
                selection = self.df.eval(selection)
            return self.df[~self.df.is_data & self.df.is_signal & selection]

    def bkg_df(self, selection=None):
        if not selection:
            return self.df[~self.df.is_data & ~self.df.is_signal]
        else:
            if selection and type(selection) == str:
                selection = self.df.eval(selection)
            return self.df[~self.df.is_data & ~self.df.is_signal & selection]

    def data_df(self, selection=None):
        if not selection:
            return self.df[self.df.is_data]
        else:
            if selection and type(selection) == str:
                selection = self.df.eval(selection)
            return self.df[self.df.is_data & selection]

    @staticmethod
    def clopper_pearson_error(passed, total, level=0.6827):
        """Matching TEfficiency::ClopperPearson()
           stolen from Nick Amin: https://github.com/aminnj/yahist
        """
        alpha = 0.5*(1.0 - level)
        low = scipy.stats.beta.ppf(alpha, passed, total - passed + 1)
        high = scipy.stats.beta.ppf(1 - alpha, passed + 1, total - passed)
        return low, high

    @staticmethod
    def ratio_plot(numer, denom, bin_edges, numer_err=None, denom_err=None, xerr=0, yerr=0, 
                   axes=None, label="", fmt="ko", markersize=3):
        # Get bin dimensions
        bin_widths = bin_edges[1:] - bin_edges[:-1]
        bin_centers = bin_edges[:-1] + 0.5*bin_widths
        # Aviod divide-by-zero
        denom[denom == 0] = 10^(-8)
        # Get ratios
        ratios = 1.0*numer/denom
        # Get errors on ratio
        if not yerr:
            bothzero = (numer == 0) & (denom == 0)
            err_down, err_up = PandasAnalysis.clopper_pearson_error(numer, denom)
            # these are actually the positions for down and up, but we want the errors
            # wrt to the central value
            err_up = err_up - ratios
            err_down = ratios - err_down
            errors = 0.5*(err_down + err_up)  # nominal errors are avg of up and down
            # For consistency with TEfficiency, up error is 1 if we have 0/0
            err_up[bothzero] = 1.0
            yerr = errors
        # Plot ratios
        axes.errorbar(
            bin_centers, 
            ratios, 
            xerr=xerr, 
            yerr=yerr, 
            fmt=fmt, 
            label=label, 
            markersize=markersize
        )
        # Get relative errors
        if not type(numer_err) == type(None) and not type(denom_err) == type(None):
            rel_errors = (
                (numer_err/denom)**2.0 + (denom_err*numer/denom**2.0)**2.0
            )**0.5
            # Plot relative errors
            err_points = np.repeat(bin_edges, 2)[1:-1]
            err_high = np.repeat(1 + rel_errors, 2)
            err_low = np.repeat(1 - rel_errors, 2)
            axes.fill_between(
                err_points, err_high, err_low, 
                step="mid", 
                hatch="///////", 
                facecolor="none",
                edgecolor=(0.75,0.75,0.75), 
                linewidth=0.0, 
                linestyle="-",
                zorder=2
            )
        # Formatting
        axes.margins(x=0) # Remove axis margins
        
        return

    @staticmethod
    def errorbar_hist(vals, weights=None, bins=None, yerr=None, fmt="ko", label="", 
                      markersize=4, fig=None, axes=None):
        if not fig or not axes:
            fig = plt.figure(figsize=(6.4,4.8))
            axes = plt.subplot(111)
        # Get histogram values
        counts, bin_edges = np.histogram(vals, weights=weights, bins=bins)
        # Get bin dimensions
        bin_widths = bin_edges[1:] - bin_edges[:-1]
        bin_centers = bin_edges[:-1] + 0.5*bin_widths
        # Plot error bars
        axes.errorbar(
            bin_centers, 
            counts, 
            yerr=yerr, 
            fmt=fmt, 
            label=label,
            markersize=markersize
        )
        
        return counts, bin_edges

    @staticmethod
    def efficiency_plot(numer_df, denom_df, column, denom_label="", numer_label="",
                        numer_weights=None, denom_weights=None, ratio_label="", 
                        x_label="", transf=lambda x: x, bins=None):

        fig = plt.figure(figsize=(6.4*1.5,4.8*1.25*1.5))
        gs = gridspec.GridSpec(ncols=1, nrows=2, figure=fig, height_ratios=[2,0.65], hspace=0.05)
        hist_axes = fig.add_subplot(gs[0])
        ratio_axes = fig.add_subplot(gs[1])

        # Plot denominator
        denom_counts, _, _ = hist_axes.hist(
            clip(transf(denom_df[column]), bins), 
            bins=bins,
            color="yellow", 
            histtype="stepfilled", 
            edgecolor="k",
            zorder=1,
            label=denom_label,
            weights=denom_weights
        )
        # Get denominator errors
        denom_err = np.sqrt(denom_counts)
        denom_err_points = np.repeat(bins, 2)[1:-1]
        denom_err_high = np.repeat(denom_counts + denom_err, 2)
        denom_err_low = np.repeat(denom_counts - denom_err, 2)
        # Plot denominator errors
        hist_axes.fill_between(
            denom_err_points, 
            denom_err_high, 
            denom_err_low, 
            step="mid", 
            hatch="///////", 
            facecolor="none",
            edgecolor=(0.75,0.75,0.75), 
            linewidth=0.0, 
            linestyle="-",
            zorder=2
        );

        # Plot numerator
        numer_counts, _ = PandasAnalysis.errorbar_hist(
            clip(transf(numer_df[column]), bins), 
            bins=bins,
            fig=fig, 
            axes=hist_axes, 
            label=numer_label,
            weights=numer_weights
        )
        # Get numerator errors
        numer_err = np.sqrt(numer_counts)
        # Formatting
        hist_axes.set_ylim(bottom=0)
        hist_axes.legend(fontsize=14);
        hist_axes.set_ylabel("Counts", size=18)
        hist_axes.margins(x=0) # Remove axis margins
        hist_axes.xaxis.set_minor_locator(AutoMinorLocator())
        hist_axes.yaxis.set_minor_locator(AutoMinorLocator())
        hist_axes.tick_params(labelbottom=False)

        # Plot ratio
        PandasAnalysis.ratio_plot(
            numer_counts, 
            denom_counts, 
            bins, 
            numer_err=numer_err, 
            denom_err=denom_err, 
            axes=ratio_axes
        );
        ratio_axes.axhline(y=1, color="k", linestyle="--", alpha=0.75, linewidth=0.75)
        # Formatting
        ratio_axes.xaxis.set_minor_locator(AutoMinorLocator())
        ratio_axes.yaxis.set_minor_locator(AutoMinorLocator())
        ratio_axes.set_ylabel(ratio_label, size=18);
        ratio_axes.set_ylim([0.5,1.5]);
        ratio_axes.set_xlim([bins[0], bins[-1]]);
        ratio_axes.set_xlabel(x_label, size=18)
        
        return hist_axes, ratio_axes

class Optimization(PandasAnalysis):

    def get_event_counts(self, selection=None, raw=False):
        bkg_df = self.bkg_df(selection=selection)
        sig_df = self.sig_df(selection=selection)
        if raw:
            return len(sig_df), len(bkg_df)
        else:
            return sig_df.event_weight.sum(), bkg_df.event_weight.sum()

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

    def plot_sig_vs_bkg(self, column, bins, selection="", abs=False, raw=False, 
                        x_label="", logy=False):

        fig, axes = plt.subplots(figsize=(12, 9))

        bkg_df = self.bkg_df(selection=selection)
        sig_df = self.sig_df(selection=selection)

        if raw:
            bkg_weights, sig_weights = np.ones(len(bkg_df)), np.ones(len(sig_df))
        else:
            bkg_weights, sig_weights = bkg_df.event_weight, sig_df.event_weight

        bkg_data = bkg_df[column] if not abs else np.abs(bkg_df[column])
        axes.hist(
            clip(bkg_data, bins),
            bins=bins,
            weights=bkg_weights,
            alpha=0.5,
            label=f"total background [{sum(bkg_weights):0.1f} events]",
            log=logy
        )

        sig_data = sig_df[column] if not abs else np.abs(sig_df[column])
        axes.hist(
            clip(sig_data, bins),
            bins=bins,
            weights=sig_weights,
            histtype="step",
            color="r",
            label=f"total signal [{sum(sig_weights):0.1f} events]",
            log=logy
        )
        
        axes.set_xlabel(x_label, size=18)
        axes.set_ylabel("Events", size=18)
        axes.legend(fontsize=16)

        return axes

    def plot_sig_vs_bkg_2D(self, xcolumn, ycolumn, xbins, ybins, selection="", xabs=False, 
                           yabs=False, raw=False, x_label="", y_label="", logz=False):

        fig, axes = plt.subplots(1, 3, figsize=(24, 9), gridspec_kw={"width_ratios": [20, 20, 1]})

        bkg_df = self.bkg_df(selection=selection)
        sig_df = self.sig_df(selection=selection)

        if raw:
            bkg_weights, sig_weights = np.ones(len(bkg_df)), np.ones(len(sig_df))
        else:
            bkg_weights, sig_weights = bkg_df.event_weight, sig_df.event_weight

        bkg_xdata = bkg_df[xcolumn] if not xabs else np.abs(bkg_df[xcolumn])
        bkg_ydata = bkg_df[ycolumn] if not yabs else np.abs(bkg_df[ycolumn])
        bkg_counts, _, _ = np.histogram2d(
            clip(bkg_xdata, xbins),
            clip(bkg_ydata, ybins),
            bins=[xbins, ybins],
            weights=bkg_weights,
        )
        axes[0].set_title(f"total background [{sum(bkg_weights):0.1f} events]", size=18)

        sig_xdata = sig_df[xcolumn] if not xabs else np.abs(sig_df[xcolumn])
        sig_ydata = sig_df[ycolumn] if not yabs else np.abs(sig_df[ycolumn])
        sig_counts, _, _ = np.histogram2d(
            clip(sig_xdata, xbins),
            clip(sig_ydata, ybins),
            bins=[xbins, ybins],
            weights=sig_weights
        )
        axes[1].set_title(f"total signal [{sum(sig_weights):0.1f} events]", size=18)

        if logz:
            nonzero_counts = np.concatenate(
                [bkg_counts[bkg_counts > 0].flatten(), sig_counts[sig_counts > 0].flatten()]
            )
            norm = LogNorm(
                vmin=np.min(nonzero_counts),
                vmax=np.max([bkg_counts, sig_counts])
            )
        else:
            norm = Normalize(
                vmin=np.min([bkg_counts, sig_counts]), 
                vmax=np.max([bkg_counts, sig_counts])
            )

        bkg_mesh = axes[0].pcolormesh(xbins, ybins, bkg_counts.T, norm=norm)
        sig_mesh = axes[1].pcolormesh(xbins, ybins, sig_counts.T, norm=norm)

        axes[0].set_xlim(xbins[0], xbins[-1])
        axes[0].set_ylim(ybins[0], ybins[-1])
        axes[1].set_xlim(xbins[0], xbins[-1])
        axes[1].set_ylim(ybins[0], ybins[-1])

        fig.colorbar(sig_mesh, cax=axes[2])
        
        axes[0].set_xlabel(x_label, size=18)
        axes[0].set_ylabel(y_label, size=18)
        axes[1].set_xlabel(x_label, size=18)

        return axes

    def fom_scan(self, variable, working_points, abs=False, show_raw=False, operator=">", 
                 base_selection="", fom=lambda s, b: s/np.sqrt(b)):
        if abs:
            variable = f"abs({variable})"

        if not show_raw:
            print("sel,sig,bkg,fom")
        else:
            print("sel,sig,,bkg,,fom")

        for wp in working_points:
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

class Validation(PandasAnalysis):

    def get_event_counts(self, selection=None, raw=False):
        bkg_df = self.bkg_df(selection=selection)
        data_df = self.data_df(selection=selection)
        if raw:
            return len(data_df), len(bkg_df)
        else:
            return data_df.event_weight.sum(), bkg_df.event_weight.sum()

    def plot_data_vs_mc(self, column, bins, selection="", x_label="", logy=False, 
                        transf=lambda x: x):

        mc_df = self.bkg_df(selection=selection)
        data_df = self.data_df(selection=selection)

        PandasAnalysis.efficiency_plot(
            data_df, 
            mc_df, 
            column, 
            numer_label=f"total data [{len(data_df)} events]",
            denom_label=f"total background [{mc_df.event_weight.sum():.1f} events]", 
            denom_weights=mc_df.event_weight, 
            ratio_label="data/mc", 
            transf=transf,
            x_label=x_label,
            bins=bins
        )
