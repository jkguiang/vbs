import os
import glob
import matplotlib.pyplot as plt
import torch
from torch.utils.data import Dataset

class DisCoDataset(Dataset):
    def __init__(self, features, labels, weights, sample_labels, norm=True):
        self.data = torch.cat(
            (
                features, 
                labels.unsqueeze(1), 
                weights.unsqueeze(1), 
                sample_labels.unsqueeze(1)
            ),
            dim=1
        )
        self.data = torch.transpose(self.data, 0, 1)
        self.features = self.data[:-3]
        self.labels = self.data[-3]
        self.weights = self.data[-2]
        self.sample_labels = self.data[-1]

        self.norm = norm
        self.weight_norm = torch.ones(self.labels.size())
        self.labels_norm = torch.ones(self.labels.size())
        if self.norm:
            self.weight_norm[self.labels == 0] = torch.sum(self.labels == 0)/torch.sum(self.weights[self.labels == 0])
            self.weight_norm[self.labels == 1] = torch.sum(self.labels == 1)/torch.sum(self.weights[self.labels == 1])
            self.labels_norm[self.labels == 1] = torch.sum(self.labels == 0)/torch.sum(self.labels == 1)

    def __str__(self):
        n_sig = torch.sum(self.weight_norm[self.labels == 1]*self.labels_norm[self.labels == 1]*self.weights[self.labels == 1])
        n_bkg = torch.sum(self.weight_norm[self.labels == 0]*self.labels_norm[self.labels == 0]*self.weights[self.labels == 0])
        if self.norm:
            return f"DisCoDataset: {n_sig:0.1f} sig, {n_bkg:0.1f} bkg (normed)"
        else:
            return f"DisCoDataset: {n_sig:0.1f} sig, {n_bkg:0.1f} bkg"

    def __add__(self, other):
        if self.norm != other.norm:
            raise ValueError(f"Inconsistent normalization schemes between {self} and {other}")
        
        return self.get_filled_copy(torch.cat((self.data, other.data), dim=1))

    def __len__(self):
        return self.data.size()[-1]

    def __getitem__(self, idx):
        """
        Must be filled out for each child class. Returns the data for a single event, e.g.
        ```
        def __getitem__(self, idx):
            return (
                self.features[:,idx], 
                self.labels[idx], 
                self.weights[idx]*self.labels_norm[idx]*self.weight_norm[idx]
            )
        ```
        """
        raise NotImplementedError("DisCoDataset serves as a template base class, so it is not itself useable")

    def get_filled_copy(self, data=None, norm=None):
        """
        Must be filled out for each child class. Returns a 'filled' copy of the data, e.g.
        ```
        def get_filled_copy(self, data=None, norm=None):
            if data is None:
                data = self.data.clone()
            if norm is None:
                norm = self.norm
            data = torch.transpose(data, 0, 1)
            return ExampleDisCoDataset(
                data[:,:-4],  # features
                data[:,-4],   # labels
                data[:,-3],   # weights
                data[:,-2],   # sample_labels
                norm=norm
            )
        ```
        """
        raise NotImplementedError("DisCoDataset serves as a template base class, so it is not itself useable")

    def count_label(self, label):
        return torch.sum(self.labels == label).item()

    def plot_variable(self, values, name, plots_dir, norm=True):
        fig, axes = plt.subplots(figsize=(10, 10))
        is_signal = (self.labels == 1)
        sig_vals = values[is_signal]
        sig_wgts = self.weights[is_signal]
        bkg_vals = values[~is_signal]
        bkg_wgts = self.weights[~is_signal]

        # Compute binning
        bin_max = values.abs().max().item()
        bin_width = round(bin_max/100) if bin_max >= 100 else bin_max/100

        if torch.any(values < 0):
            bins = torch.linspace(-100*bin_width, 100*bin_width, 101)
        else:
            bins = torch.linspace(0, 100*bin_width, 101)

        centers = 0.5*(bins[1:] + bins[:-1])

        # Clamp values s.t. first bin is underflow and last bin is overflow
        sig_vals = sig_vals.clamp(min=centers[0], max=centers[-1])
        bkg_vals = bkg_vals.clamp(min=centers[0], max=centers[-1])

        # Get bkg histogram counts
        bkg_events = torch.sum(bkg_wgts).item()
        bkg_counts, _ = torch.histogram(bkg_vals, bins=bins, weight=bkg_wgts)
        if norm:
            bkg_counts /= torch.sum(bkg_counts)

        # Plot bkg histogram
        axes.hist(
            centers, 
            bins=bins, 
            weights=bkg_counts,
            color="k",
            alpha=0.75,
            label=f"total bkg [{bkg_events:0.1f} events]"
        )

        # Get sig histogram counts
        sig_events = sig_wgts.sum().item()
        sig_counts, _ = torch.histogram(sig_vals, bins=bins, weight=sig_wgts)
        if norm:
            sig_counts /= sig_counts.sum()

        # Plot sig histogram
        axes.hist(
            centers,
            bins=bins,
            weights=sig_counts,
            histtype="step",
            color="r",
            label=f"total sig [{sig_events:0.1f} events]"
        )

        # Format axes
        axes.legend()
        axes.autoscale()
        if norm:
            axes.set_ylabel("a.u.")
        else:
            axes.set_ylabel("Events")

        axes.set_xlabel(name)

        # Save plot
        outname = f"{plots_dir}/{name}.png"
        plt.savefig(outname, bbox_inches="tight")
        print(f"Wrote {outname}")
        plt.close()

    def plot(self, config, norm=True):
        plots_dir = f"{config.base_dir}/{config.name}/plots"
        os.makedirs(plots_dir, exist_ok=True)
        # Plot features
        for feature_i in range(len(self.features)):
            self.plot_variable(
                self.features[feature_i],
                config.ingress.features[feature_i],
                plots_dir,
                norm=norm
            )

    def split(self, fraction):
        left_data = None
        right_data = None
        for s in torch.unique(self.sample_labels):
            # Find sample in data
            is_sample = torch.where(self.sample_labels == s)[0]
            # Split events
            n_events = is_sample.size()[0]
            idxs = torch.randperm(n_events)
            splt = round(n_events*fraction)
            # Collect data left of split
            l_idxs = idxs[:splt]
            l_data = self.data[:,is_sample][:,l_idxs]
            # Collect data right of split
            r_idxs = idxs[splt:]
            r_data = self.data[:,is_sample][:,r_idxs]
            if left_data is None and right_data is None:
                left_data = l_data.clone()
                right_data = r_data.clone()
            else:
                left_data = torch.cat((left_data, l_data), dim=1)
                right_data = torch.cat((right_data, r_data), dim=1)

        return (
            self.get_filled_copy(left_data),
            self.get_filled_copy(right_data)
        )

    @staticmethod
    def from_file(pt_file, norm=True):
        dataset = torch.load(pt_file)
        if norm != dataset.norm:
            return dataset.get_filled_copy(norm=norm)
        else:
            return dataset

    @classmethod
    def from_files(cls, pt_files, norm=True):
        dataset = None
        if type(pt_files) == str:
            pt_files = glob.glob(pt_files)
        for file_i, pt_file in enumerate(pt_files):
            if file_i == 0:
                dataset = cls.from_file(pt_file, norm=norm)
            else:
                dataset += cls.from_file(pt_file, norm=norm)

        return dataset

class SingleDisCoDataset(DisCoDataset):
    def __init__(self, features, labels, weights, sample_labels, disco_target, norm=True):
        super().__init__(features, labels, weights, sample_labels, norm=norm)
        self.data = torch.cat((self.data, torch.transpose(disco_target.unsqueeze(1), 0, -1)))
        self.disco_target = self.data[-1]

    def __getitem__(self, idx):
        return (
            self.features[:,idx], 
            self.labels[idx], 
            self.weights[idx]*self.labels_norm[idx]*self.weight_norm[idx], 
            self.disco_target[idx]
        )

    def get_filled_copy(self, data=None, norm=None):
        if data is None:
            data = self.data.clone()
        if norm is None:
            norm = self.norm
        data = torch.transpose(data, 0, 1)
        return SingleDisCoDataset(
            data[:,:-4],  # features
            data[:,-4],   # labels
            data[:,-3],   # weights
            data[:,-2],   # sample_labels
            data[:,-1],   # disco_target
            norm=norm
        )

    def plot(self, config, norm=True):
        super().plot(config, norm=norm)
        plots_dir = f"{config.base_dir}/{config.name}/plots"
        self.plot_variable(
            self.disco_target,
            config.ingress.disco_target,
            plots_dir,
            norm=norm
        )

class DoubleDisCoDataset(DisCoDataset):
    def __init__(self, features, labels, weights, sample_labels, n_features1, n_features2, norm=True):
        super().__init__(features, labels, weights, sample_labels, norm=norm)
        if self.features.size()[0] != n_features1 + n_features2:
            raise Exception("something is wrong: len(features) != len(features1) + len(features2)")
        self.features1 = self.features[:n_features1]
        self.features2 = self.features[n_features1:]
        self.n_features1 = n_features1
        self.n_features2 = n_features2

    def __getitem__(self, idx):
        return (
            self.features1[:,idx], 
            self.features2[:,idx], 
            self.labels[idx], 
            self.weights[idx]*self.labels_norm[idx]*self.weight_norm[idx]
        )

    def get_filled_copy(self, data=None, norm=None):
        if data is None:
            data = self.data.clone()
        if norm is None:
            norm = self.norm
        data = torch.transpose(data, 0, 1)
        return DoubleDisCoDataset(
            data[:,:-3],  # features
            data[:,-3],   # labels
            data[:,-2],   # weights
            data[:,-1],   # sample_labels
            self.n_features1,
            self.n_features2,
            norm=norm
        )

    def plot(self, config, norm=True):
        plots_dir = f"{config.base_dir}/{config.name}/plots"
        feature_names = config.ingress.features1 + config.ingress.features2
        os.makedirs(plots_dir, exist_ok=True)
        # Plot features
        for feature_i in range(len(self.features)):
            self.plot_variable(
                self.features[feature_i],
                feature_names[feature_i],
                plots_dir,
                norm=norm
            )
