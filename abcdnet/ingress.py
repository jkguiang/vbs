import glob
import os
import argparse

import uproot
import torch

from utils import VBSConfig
from datasets import DisCoDataset

def get_outfile(config, tag, ext="pt", msg=None):
    outfile = f"{config.basedir}/{config.name}_{tag}.{ext}"
    if msg:
        print(msg.format(outfile))
    return outfile

def transform(feature, transf):
    if transf == None:
        return feature
    elif transf == "rescale":
        return (feature - feature.min())/(feature.max() - feature.min())
    elif transf == "log":
        return torch.log(feature)
    elif transf == "log2":
        return torch.log2(feature)
    elif transf == "log10":
        return torch.log10(feature)
    else:
        raise ValueError(f"transformation '{transf}' not supported")

def ingress(config):
    root_files = [f for f in glob.glob(f"{config.ingress.input_dir}/*.root") if "data.root" not in f]
    transforms = config.ingress.get("transforms", {})
    for file_i, root_file in enumerate(root_files):
        print(f"Loading {root_file}")
        with uproot.open(f"{root_file}:{config.ingress.ttree_name}") as tree:

            # Assuming that files are named SampleName.root (e.g. QCD.root, ttbar1l.root, etc.)
            sample_name = root_file.split("/")[-1].replace(".root", "")
            outfile = DisCoDataset.get_name(config, sample_name)

            # Load features
            features = []
            for feature_branch in config.ingress.features:
                feature = torch.tensor(tree[feature_branch].array(), dtype=torch.float)
                feature = transform(feature, transforms.get(feature_branch, None))
                features.append(feature)

            features = torch.transpose(torch.stack(features), 0, 1)
            n_events = features.size()[0]

            # Set label
            is_signal = False
            if config.ingress.get("signal_file", None):
                if len(config.ingress.signal_file.split("/")) == 1:
                    is_signal = (root_file.split("/")[-1] == config.ingress.signal_file)
                else:
                    is_signal = (root_file == config.ingress.signal_file)
                labels = torch.ones(n_events)*is_signal
            elif config.ingress.get("label", None):
                labels = torch.tensor(tree[config.ingress.label].array(), dtype=torch.float)
            else:
                raise Exception("No signal file name or label branch provided")

            # Create a unique identifier for this sample (used for splitting evenly)
            sample_number = torch.ones(n_events)*file_i

            # Calculate event weight
            weights = torch.ones(n_events)
            if config.ingress.get("weights", None):
                for branch_i, weight_branch in enumerate(config.ingress.weights):
                    weights *= torch.tensor(tree[weight_branch].array(), dtype=torch.float)

            # Load disco target (to be used in SingleDisCo)
            disco_target = None
            if config.ingress.get("disco_target", None):
                disco_target = torch.tensor(
                    tree[config.ingress.disco_target].array(), 
                    dtype=torch.float
                )
                disco_target = transform(disco_target, transforms.get(config.ingress.disco_target, None))

            # Save dataset
            data = DisCoDataset(
                features,
                labels,
                weights,
                sample_number,
                disco_target=disco_target
            )
            data.save(outfile)
            print(f"Writing to {outfile}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Ingress data")
    parser.add_argument("config_json", type=str, help="config JSON")
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)
    os.makedirs(config.basedir, exist_ok=True)
    ingress(config)
