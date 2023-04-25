import os
import glob
import uproot
import torch
from torch.utils.data import Dataset

class DisCoDataset(Dataset):
    def __init__(self, features, labels, weights, sample_labels, disco_target=None, norm=True):
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
        if disco_target is None:
            self.is_single_disco = False
        else:
            self.data = torch.cat((self.data, torch.transpose(disco_target.unsqueeze(1), 0, -1)))
            self.disco_target = self.data[-1]
            self.is_single_disco = True

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
        return DisCoDataset.__from_tensor(torch.cat((self.data, other.data), dim=1), self.is_single_disco)

    def __len__(self):
        return self.data.size()[-1]

    def __getitem__(self, idx):
        if self.is_single_disco:
            return self.features[:,idx], self.labels[idx], self.labels_norm[idx]*self.weight_norm[idx]*self.weights[idx], self.disco_target[idx]
        else:
            return self.features[:,idx], self.labels[idx], self.labels_norm[idx]*self.weight_norm[idx]*self.weights[idx]

    @classmethod
    def __from_tensor(cls, data, is_single_disco, **kwargs):
        data = torch.transpose(data, 0, 1)
        if is_single_disco:
            return cls(
                data[:,:-4],               # features
                data[:,-4],                # labels
                data[:,-3],                # weights
                data[:,-2],                # sample_labels
                disco_target=data[:,-1],   # disco_target
                **kwargs
            )
        else:
            return cls(
                data[:,:-3],  # features
                data[:,-3],   # labels
                data[:,-2],   # weights
                data[:,-1],   # sample_labels
                **kwargs
            )

    @classmethod
    def from_file(cls, pt_file, is_single_disco=True, **kwargs):
        data = torch.load(pt_file)
        return cls.__from_tensor(data, is_single_disco, **kwargs)

    @classmethod
    def from_files(cls, pt_files, is_single_disco=True, **kwargs):
        dataset = None
        if type(pt_files) == str:
            pt_files = glob.glob(pt_files)
        for file_i, pt_file in enumerate(pt_files):
            if file_i == 0:
                dataset = cls.from_file(pt_file, is_single_disco, **kwargs)
            else:
                dataset += cls.from_file(pt_file, is_single_disco, **kwargs)

        return dataset

    def n_label(self, label):
        return torch.sum(self.labels == label).item()

    def plot(self, config, norm=True):
        import matplotlib.pyplot as plt
        plots_dir = f"{config.basedir}/{config.name}/plots"
        os.makedirs(plots_dir, exist_ok=True)
        for feature_i in range(self.features.size()[0]):
            fig, axes = plt.subplots(figsize=(10, 10))
            is_signal = (self.labels == 1)
            sig_vals = self.features[feature_i][is_signal]
            sig_wgts = self.weights[is_signal]
            bkg_vals = self.features[feature_i][~is_signal]
            bkg_wgts = self.weights[~is_signal]

            # Compute binning
            bin_max = self.features[feature_i].abs().max().item()
            bin_width = round(bin_max/100) if bin_max >= 100 else bin_max/100

            if torch.all(sig_vals >= 0) and torch.all(bkg_vals >= 0):
                bins = torch.linspace(0, 100*bin_width, 101)
            else:
                bins = torch.linspace(-100*bin_width, 100*bin_width, 101)

            centers = 0.5*(bins[1:] + bins[:-1])

            # Clip values s.t. first bin is underflow and last bin is overflow
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
            sig_events = bkg_wgts.sum().item()
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

            feature_name = config.ingress.features[feature_i]
            axes.set_title(feature_name)

            # Save plot
            outname = f"{plots_dir}/{config.name}_{feature_name}.png"
            plt.savefig(outname, bbox_inches="tight")
            print(f"Wrote {outname}")
            plt.close()

    def save(self, outfile):
        torch.save(self.data, outfile)

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
            DisCoDataset.__from_tensor(left_data, self.is_single_disco),
            DisCoDataset.__from_tensor(right_data, self.is_single_disco)
        )