import os
import glob
import uproot
import torch
from torch.utils.data import Dataset

class DisCoDataset(Dataset):
    def __init__(self, features, labels, weights, sample_labels, disco_target=None):
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

        self.weight_norm = len(self)/torch.sum(self.weights)

    def __add__(self, other):
        return DisCoDataset.__from_tensor(torch.cat((self.data, other.data), dim=1), self.is_single_disco)

    def __len__(self):
        return self.data.size()[-1]

    def __getitem__(self, idx):
        if self.is_single_disco:
            return self.features[:,idx], self.labels[idx], self.weight_norm*self.weights[idx], self.disco_target[idx]
        else:
            return self.features[:,idx], self.labels[idx], self.weight_norm*self.weights[idx]

    @staticmethod
    def get_name(config, tag):
        return f"{config.basedir}/{config.name}_{tag}_dataset.pt"

    @classmethod
    def __from_tensor(cls, data, is_single_disco):
        data = torch.transpose(data, 0, 1)
        if is_single_disco:
            return cls(
                data[:,:-4],               # features
                data[:,-4],                # labels
                data[:,-3],                # weights
                data[:,-2],                # sample_labels
                disco_target=data[:,-1]    # disco_target
            )
        else:
            return cls(
                data[:,:-3],  # features
                data[:,-3],   # labels
                data[:,-2],   # weights
                data[:,-1]    # sample_labels
            )

    @classmethod
    def from_file(cls, pt_file, is_single_disco=True):
        data = torch.load(pt_file)
        return cls.__from_tensor(data, is_single_disco)

    @classmethod
    def from_files(cls, pt_files, is_single_disco=True):
        dataset = None
        if type(pt_files) == str:
            pt_files = glob.glob(pt_files)
        for file_i, pt_file in enumerate(pt_files):
            if file_i == 0:
                dataset = cls.from_file(pt_file, is_single_disco)
            else:
                dataset += cls.from_file(pt_file, is_single_disco)

        return dataset

    def plot(self, config):
        import matplotlib.pyplot as plt
        plots_dir = f"{config.basedir}/plots"
        os.makedirs(plots_dir, exist_ok=True)
        bins = torch.linspace(0, 1, 101)
        for feature_i in range(self.features.size()[0]):
            fig, axes = plt.subplots(figsize=(9, 9))
            is_signal = (self.labels == 1)
            axes.hist(
                self.features[feature_i][~is_signal], 
                bins=bins,
                weights=self.weights[~is_signal],
                histtype="step",
                color="k",
                alpha=0.75,
                label=f"total bkg [{torch.sum(self.weights[~is_signal]).item()}]"
            )
            axes.hist(
                self.features[feature_i][is_signal], 
                bins=bins,
                weights=self.weights[is_signal],
                histtype="step",
                color="r",
                label=f"total sig [{torch.sum(self.weights[is_signal]).item()}]"
            )
            axes.legend()
            feature_name = config.ingress.features[feature_i]
            outname = f"{plots_dir}/{config.name}_{feature_name}.png"
            plt.savefig(outname, bbox_inches="tight")
            print(f"Saved {feature_name} histogram to {outname}")
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
            if s == 0:
                left_data = l_data.clone()
                right_data = r_data.clone()
            else:
                left_data = torch.cat((left_data, l_data), dim=1)
                right_data = torch.cat((right_data, r_data), dim=1)

        return (
            DisCoDataset.__from_tensor(left_data, self.is_single_disco),
            DisCoDataset.__from_tensor(right_data, self.is_single_disco)
        )
