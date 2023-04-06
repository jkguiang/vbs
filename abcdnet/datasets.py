import glob
import uproot
import torch
from torch.utils.data import Dataset

import ingress

class SingleDisCoDataset(Dataset):
    def __init__(self, features, labels, weights, disco_target, sample_labels):
        self.data = torch.cat(
            (
                features, 
                labels.unsqueeze(1), 
                weights.unsqueeze(1), 
                disco_target.unsqueeze(1), 
                sample_labels.unsqueeze(1)
            ),
            dim=1
        )
        self.data = torch.transpose(self.data, 0, 1)
        self.features = self.data[:-4]
        self.labels = self.data[-4]
        self.weights = self.data[-3]
        self.disco_target = self.data[-2]
        self.sample_labels = self.data[-1]
        self.weight_norm = len(self)/torch.sum(self.weights)

    def __len__(self):
        # return len(self.labels)
        return self.data.size()[-1]

    def __getitem__(self, idx):
        return self.features[:,idx], self.labels[idx], self.weight_norm*self.weights[idx], self.disco_target[idx]

    @staticmethod
    def __ingress_globber(config, tag):
        return ingress.get_outfile(config, tag).replace("_", "*").replace(".pt", "*.pt")

    @staticmethod
    def __load(config, tag):
        tensors = None
        for file_i, f in enumerate(glob.glob(SingleDisCoDataset.__ingress_globber(config, tag))):
            tensor = torch.load(f)
            if file_i == 0:
                tensors = tensor
            else:
                tensors = torch.cat((tensors, tensor))

        return tensors

    @classmethod
    def from_config(cls, config):
        sample_labels = None
        all_features = None
        for file_i, f in enumerate(glob.glob(cls.__ingress_globber(config, "features"))):
            features = torch.load(f)
            sample_label = torch.ones(features.size()[0])*file_i
            if file_i == 0:
                all_features = features
                sample_labels = sample_label
            else:
                all_features = torch.cat((all_features, features))
                sample_labels = torch.cat((sample_labels, sample_label))

        labels = cls.__load(config, "labels")
        weights = cls.__load(config, "weights")
        disco_target = cls.__load(config, config.train.disco_target)

        return cls(all_features, labels, weights, disco_target, sample_labels)

    @classmethod
    def __from_tensor(cls, data):
        data = torch.transpose(data, 0, 1)
        return cls(
            data[:,:-4],  # features
            data[:,-4],   # labels
            data[:,-3],   # weights
            data[:,-2],   # disco_target
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
