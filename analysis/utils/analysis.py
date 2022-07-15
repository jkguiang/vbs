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
import yahist

from utils.cutflow import Cut, Cutflow, CutflowCollection

def clip(np_array, bins):
    clip_low = 0.5 * (bins[0] + bins[1])
    clip_high = 0.5 * (bins[-2] + bins[-1])
    return np.clip(np_array, clip_low, clip_high)

class PandasAnalysis:
    def __init__(self, sig_root_files=None, bkg_root_files=None, data_root_files=None, 
                 ttree_name="Events", weight_columns=None):
        dfs = []
        # Load signal
        if sig_root_files:
            for root_file in sig_root_files:
                name = root_file.split("/")[-1].replace(".root", "")
                with uproot.open(root_file) as f:
                    df = f.get(ttree_name).arrays(library="pd")
                    df["name"] = name
                    df["is_signal"] = True
                    df["is_data"] = False
                    dfs.append(df)
        # Load background
        if bkg_root_files:
            for root_file in bkg_root_files:
                name = root_file.split("/")[-1].replace(".root", "")
                with uproot.open(root_file) as f:
                    df = f.get(ttree_name).arrays(library="pd")
                    df["name"] = name
                    df["is_signal"] = False
                    df["is_data"] = False
                    dfs.append(df)
        # Load data
        if data_root_files:
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
        self.__update_cutflows("base")

    def __update_cutflows(self, selection):
        selection = selection.replace(">", "gt")
        selection = selection.replace("<", "lt")
        selection = selection.replace(">=", "geq")
        selection = selection.replace("<=", "leq")
        selection = selection.replace("==", "eq")
        selection = selection.replace("!=", "neq")
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
        for name in self.df[self.df.is_data | self.df.is_signal].name.unique():
            self.cutflows["TotalBkg"] -= self.cutflows[name]


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

    def data_error(self, selection=None, raw=False):
        df = self.bkg_df(selection=selection)
        return np.sqrt(len(df))

class Optimization(PandasAnalysis):

    def get_event_counts(self, selection=None, raw=False):
        sig_count = self.sig_count(selection=selection, raw=raw)
        bkg_count = self.bkg_count(selection=selection, raw=raw)
        return sig_count, bkg_count

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

    def plot_correlation(self, column, bins, base_selection=None, selections=None, x_label="", 
                         logy=False, norm=True, transf=lambda x: x):

        fig = plt.figure(figsize=(6.4*1.5, 4.8*1.25*1.5))
        gs = gridspec.GridSpec(ncols=1, nrows=2, figure=fig, height_ratios=[2, 0.65], hspace=0.1)
        hist_axes = fig.add_subplot(gs[0])
        ratio_axes = fig.add_subplot(gs[1])

        if not base_selection:
            base_selection = "name != ''"
        if not selections:
            return hist_axes, ratio_axes

        denom_df = self.bkg_df(selection=base_selection)
        denom = yahist.Hist1D(
            denom_df[column],
            bins=bins, 
            weights=denom_df.event_weight
        );
        if norm:
            denom = denom.normalize()

        denom.plot(ax=hist_axes, label="before", color="k")

        for selection in selections:
            numer_df = self.bkg_df(selection=f"({base_selection}) and ({selection})")
            numer = yahist.Hist1D(
                numer_df[column],
                bins=bins, 
                weights=numer_df.event_weight
            );
            if norm:
                numer = numer.normalize()

            numer.plot(ax=hist_axes, label=f"after {selection}")
            (numer/denom).plot(ax=ratio_axes, errors=True)

        hist_axes.legend(fontsize=16)
        if norm:
            hist_axes.set_ylabel("a.u.", size=18)
        else:
            hist_axes.set_ylabel("Events", size=18)

        ratio_axes.set_xlabel(x_label, size=18)
        ratio_axes.set_ylabel("after/before", size=18)
        ratio_axes.set_ylim([0.5, 2.0])

        return hist_axes, ratio_axes

    def plot_sig_vs_bkg(self, column, bins, selection="", transf=lambda x: x, raw=False, 
                        x_label="", logy=False, axes=None, norm=False, stacked=False):
        if not axes:
            fig, axes = plt.subplots(figsize=(12, 9))

        bkg_df = self.bkg_df(selection=selection)
        sig_df = self.sig_df(selection=selection)

        if raw:
            bkg_weights, sig_weights = np.ones(len(bkg_df)), np.ones(len(sig_df))
        else:
            bkg_weights, sig_weights = bkg_df.event_weight, sig_df.event_weight

        # Get stacked backgrounds
        bkg_hists = []
        if stacked:
            for name in bkg_df.name.unique():
                weights = bkg_weights[bkg_df.name == name]
                if norm:
                    weights /= sum(bkg_weights)
                hist = yahist.Hist1D(
                    transf(bkg_df[bkg_df.name == name][column]),
                    bins=bins, 
                    weights=weights,
                    label=f"{name} [{sum(weights):.1f} events]"
                )
                bkg_hists.append(hist)

        # Get total background
        bkg_hist = yahist.Hist1D(
            transf(bkg_df[column]),
            bins=bins, 
            weights=bkg_weights,
            label=f"total background [{sum(bkg_weights):0.1f} events]",
            color="k"
        )

        # Get signal
        sig_hist = yahist.Hist1D(
            transf(sig_df[column]),
            bins=bins, 
            weights=sig_weights,
            label=f"total signal [{sum(sig_weights):0.1f} events]",
            color="r"
        )
        if norm:
            sig_hist = sig_hist.normalize()

        # Plot everything
        if bkg_hists:
            bkg_hists.sort(key=lambda h: sum(h.counts), reverse=True)
            yahist.utils.plot_stack(bkg_hists, ax=axes, histtype="stepfilled", alpha=0.75)
        bkg_hist.plot(ax=axes, alpha=0.75)
        sig_hist.plot(ax=axes, linewidth=2)
        
        axes.set_xlabel(x_label, size=18)
        axes.set_ylabel("Events", size=18)
        axes.set_ylim(bottom=0)
        axes.legend(fontsize=14)

        return axes

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

class Validation(PandasAnalysis):

    def get_event_counts(self, selection=None, raw=False):
        data_count = self.data_count(selection=selection, raw=raw)
        mc_count = self.bkg_count(selection=selection, raw=raw)
        return data_count, mc_count

    def get_event_errors(self, selection=None, raw=False):
        data_error = self.data_error(selection=selection, raw=raw)
        mc_error = self.bkg_error(selection=selection, raw=raw)
        return data_error, mc_error

    def plot_data_vs_mc(self, column, bins, selection="", x_label="", logy=False, 
                        transf=lambda x: x, norm=False):

        fig = plt.figure(figsize=(6.4*1.5, 4.8*1.25*1.5))
        gs = gridspec.GridSpec(ncols=1, nrows=2, figure=fig, height_ratios=[2, 0.65], hspace=0.1)
        hist_axes = fig.add_subplot(gs[0])
        ratio_axes = fig.add_subplot(gs[1])

        # Get numerator and denominator
        denom_df = self.bkg_df(selection=selection)
        denom = yahist.Hist1D(
            transf(denom_df[column]),
            bins=bins, 
            weights=denom_df.event_weight,
            label=f"MC [{denom_df.event_weight.sum():0.1f} events]"
        );
        numer_df = self.data_df(selection=selection)
        numer = yahist.Hist1D(
            transf(numer_df[column]),
            bins=bins, 
            weights=numer_df.event_weight, 
            label=f"data [{len(numer_df)} events]", 
            color="k"
        );
        if norm:
            numer = numer.normalize()
            denom = denom.normalize()

        # Get ratio
        ratio = numer/denom
        # Get ratio errors
        numer_counts = numer.counts
        numer_counts[numer_counts == 0] = 1e-12
        denom_counts = denom.counts
        denom_counts[denom_counts == 0] = 1e-12
        rel_errors = np.sqrt(
            (numer.errors/numer_counts)**2 + (denom.errors/denom_counts)**2
        )

        # Plot hists and ratio
        denom.plot(ax=hist_axes)
        numer.plot(ax=hist_axes, errors=True)
        ratio.plot(ax=ratio_axes, errors=True, color="k")
        # Plot relative errors
        err_points = np.repeat(denom.edges, 2)[1:-1]
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

        hist_axes.legend(fontsize=16)
        hist_axes.xaxis.set_minor_locator(AutoMinorLocator())
        hist_axes.yaxis.set_minor_locator(AutoMinorLocator())
        if norm:
            hist_axes.set_ylabel("a.u.", size=18)
        else:
            hist_axes.set_ylabel("Events", size=18)

        ratio_axes.axhline(y=1, color="k", linestyle="--", alpha=0.75, linewidth=0.75)
        ratio_axes.xaxis.set_minor_locator(AutoMinorLocator())
        ratio_axes.yaxis.set_minor_locator(AutoMinorLocator())
        ratio_axes.set_xlabel(x_label, size=18)
        ratio_axes.set_ylabel("data/MC", size=18)
        ratio_axes.set_ylim([0.5, 2.0])

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

    def ABCD(self, h_right, v_up, base_selection, h_dir="right", v_dir="up", label=""):
        """
        Compute yields for an ABCD extrapolation in the following configuration:

               D   |   C
            -------|-------
               A   |   B

        """
        h_left = f"(not ({h_right}))"
        v_down = f"(not ({v_up}))"
        # Assemble regions
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
        A_sig_raw, A_bkg_raw = self.get_event_counts(selection=A_sel, raw=True)
        A_sig_err, A_bkg_err = self.get_event_errors(selection=A_sel)
        B_sig_wgt, B_bkg_wgt = self.get_event_counts(selection=B_sel)
        B_sig_raw, B_bkg_raw = self.get_event_counts(selection=B_sel, raw=True)
        B_sig_err, B_bkg_err = self.get_event_errors(selection=B_sel)
        C_sig_wgt, C_bkg_wgt = self.get_event_counts(selection=C_sel)
        C_sig_raw, C_bkg_raw = self.get_event_counts(selection=C_sel, raw=True)
        C_sig_err, C_bkg_err = self.get_event_errors(selection=C_sel)
        D_sig_wgt, D_bkg_wgt = self.get_event_counts(selection=D_sel)
        D_sig_raw, D_bkg_raw = self.get_event_counts(selection=D_sel, raw=True)
        D_sig_err, D_bkg_err = self.get_event_errors(selection=D_sel)
        # Calculate extrapolation factors
        AtoB_extp = B_bkg_wgt/A_bkg_wgt
        AtoB_err = np.sqrt((B_bkg_err/B_bkg_wgt)**2 + (A_bkg_err/A_bkg_wgt)**2)
        BtoC_extp = C_bkg_wgt/B_bkg_wgt
        BtoC_err = np.sqrt((C_bkg_err/C_bkg_wgt)**2 + (B_bkg_err/B_bkg_wgt)**2)
        CtoD_extp = D_bkg_wgt/C_bkg_wgt
        CtoD_err = np.sqrt((D_bkg_err/D_bkg_wgt)**2 + (C_bkg_err/C_bkg_wgt)**2)
        # Print table
        print("cut,region,bkg_raw,bkg_wgt,bkg_err,sig_raw,sig_wgt,sig_err")
        print(f"{A_sel},A,{A_bkg_raw},{A_bkg_wgt},{A_bkg_err},{A_sig_raw},{A_sig_wgt},{A_sig_err}")
        print(f"{B_sel},B,{B_bkg_raw},{B_bkg_wgt},{B_bkg_err},{B_sig_raw},{B_sig_wgt},{B_sig_err}")
        print(f"{C_sel},C,{C_bkg_raw},{C_bkg_wgt},{C_bkg_err},{C_sig_raw},{C_sig_wgt},{C_sig_err}")
        print(f"{D_sel},D,{D_bkg_raw},{D_bkg_wgt},{D_bkg_err},{D_sig_raw},{D_sig_wgt},{D_sig_err}")
        print("")
        print("name,extp,rel_err")
        print(f"AtoB,{AtoB_extp},{AtoB_err}")
        print(f"BtoC,{BtoC_extp},{BtoC_err}")
        print(f"CtoD,{CtoD_extp},{CtoD_err}")
