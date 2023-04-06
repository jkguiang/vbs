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
        if disco_target == None:
            self.is_single_disco = False
        else:
            self.data = torch.cat((self.data, torch.transpose(disco_target.unsqueeze(1), 0, -1)))
            self.disco_target = self.data[-1]
            self.is_single_disco = True

        self.weight_norm = len(self)/torch.sum(self.weights)

    @staticmethod
    def get_name(config, tag):
        return f"{config.basedir}/{config.name}_{tag}_dataset.pt"

    def __len__(self):
        return self.data.size()[-1]

    def __getitem__(self, idx):
        if is_single_disco:
            return self.features[:,idx], self.labels[idx], self.weight_norm*self.weights[idx], self.disco_target[idx]
        else:
            return self.features[:,idx], self.labels[idx], self.weight_norm*self.weights[idx]

    @classmethod
    def __from_tensor(cls, data):
        data = torch.transpose(data, 0, 1)
        if self.is_single_disco:
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
    def from_file(cls, pt_file):
        data = torch.load(pt_file)
        return cls.__from_tensor(data)

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
            SingleDisCoDataset.__from_tensor(left_data),
            SingleDisCoDataset.__from_tensor(right_data)
        )
