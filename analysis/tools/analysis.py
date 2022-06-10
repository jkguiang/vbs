import uproot
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def clip(np_array, bins):
    clip_low = 0.5 * (bins[0] + bins[1])
    clip_high = 0.5 * (bins[-2] + bins[-1])
    return np.clip(np_array, clip_low, clip_high)

class PandasAnalysis:
    def __init__(self, sig_root_files, bkg_root_files, ttree_name="Events", 
                 weight_columns=[]):
        dfs = []
        # Load signal
        for root_file in sig_root_files:
            name = root_file.split("/")[-1].replace(".root", "")
            with uproot.open(root_file) as f:
                df = f.get(ttree_name).arrays(library="pd")
                df["name"] = name
                df["is_signal"] = True
                dfs.append(df)
        # Load background
        for root_file in bkg_root_files:
            name = root_file.split("/")[-1].replace(".root", "")
            with uproot.open(root_file) as f:
                df = f.get(ttree_name).arrays(library="pd")
                df["name"] = name
                df["is_signal"] = False
                dfs.append(df)

        self.df = pd.concat(dfs)
        self.df["name"] = self.df.name.astype("category")

        if weight_columns:
            self.df["event_weight"] = self.df[weight_columns[0]]
            for weight_col in weight_columns[1:]:
                self.df["event_weight"] *= self.df[weight_col]

        self.selections = set()

    def sig_df(self, selection=None):
        if not selection:
            return self.df[self.df.is_signal]
        else:
            if selection and type(selection) == str:
                selection = self.df.eval(selection)
            return self.df[self.df.is_signal & selection]

    def bkg_df(self, selection=None):
        if not selection:
            return self.df[~self.df.is_signal]
        else:
            if selection and type(selection) == str:
                selection = self.df.eval(selection)
            return self.df[~self.df.is_signal & selection]

    def get_event_counts(self, selection=None, raw=False):
        bkg_df = self.bkg_df(selection=selection)
        sig_df = self.sig_df(selection=selection)
        if raw:
            return len(sig_df), len(bkg_df)
        else:
            return sig_df.event_weight.sum(), bkg_df.event_weight.sum()

    def set_split_column(self, ratio=0.5):
        self.df["split"] = (np.random.rand(len(self.df)) < ratio)

    def plot_sig_vs_bkg(self, column, bins, selection="", raw=False, xlabel=""):
        fig, axes = plt.subplots(figsize=(12, 9))

        bkg_df = self.bkg_df(selection=selection)
        sig_df = self.sig_df(selection=selection)

        if raw:
            bkg_weights, sig_weights = np.ones(len(bkg_df)), np.ones(len(sig_df))
        else:
            bkg_weights, sig_weights = bkg_df.event_weight, sig_df.event_weight

        axes.hist(
            clip(bkg_df[column], bins),
            bins=bins,
            weights=bkg_weights,
            alpha=0.5,
            label=f"total background [{sum(bkg_weights):0.1f} events]"
        )

        axes.hist(
            clip(sig_df[column], bins),
            bins=bins,
            weights=sig_weights,
            histtype="step",
            color="r",
            label=f"total signal [{sum(sig_weights):0.1f} events]"
        )
        
        axes.set_xlabel(xlabel, size=18)
        axes.set_ylabel("Events", size=18)
        axes.legend(fontsize=16)

    def mini_sig_scan(self, variable, working_points, operator=">", base_selection=""):
        print("sel,sig,bkg")
        for wp in working_points:
            if base_selection:
                sel = f"{base_selection} and {variable} {operator} {wp}"
            else:
                sel = f"{variable} {operator} {wp}"
                
            sig, bkg = vbswh.get_event_counts(selection=sel)
            print(f"{sel},{sig},{bkg}")
