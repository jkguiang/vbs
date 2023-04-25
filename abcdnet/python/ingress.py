import glob
import os
import argparse

import uproot
import torch

from utils import VBSConfig
from datasets import DisCoDataset

def get_outfile(config, tag, ext="pt", subdir=None, msg=None):
    outdir = f"{config.basedir}/{config.name}"
    if subdir:
        outdir = f"{outdir}/{subdir}"

    os.makedirs(outdir, exist_ok=True)
    outfile = f"{outdir}/{config.name}_{tag}.{ext}"

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

def ingress_file(config, root_file, file_i, save=True):
    transforms = config.ingress.get("transforms", {})
    print(f"Loading {root_file}")
    with uproot.open(root_file) as f:
        # Collect branches to ingress
        branches = (
            config.ingress.features 
            + config.ingress.get("label", []) 
            + config.ingress.get("weights", [])
        )
        if config.ingress.get("disco_target", None):
            branches.append(config.ingress.disco_target)

        # Load TTree
        tree = f[config.ingress.ttree_name].arrays(branches, cut=config.ingress.get("selection", None))

        # Assuming that files are named SampleName.root (e.g. QCD.root, ttbar1l.root, etc.)
        sample_name = root_file.split("/")[-1].replace(".root", "")

        # Load features
        features = []
        for feature_branch in config.ingress.features:
            feature = torch.tensor(tree[feature_branch], dtype=torch.float)
            feature = transform(feature, transforms.get(feature_branch, None))
            features.append(feature)

        features = torch.transpose(torch.stack(features), 0, 1)
        n_events = len(features)

        # Set label
        is_signal = False
        if config.ingress.get("signal_file", None):
            if len(config.ingress.signal_file.split("/")) == 1:
                is_signal = (root_file.split("/")[-1] == config.ingress.signal_file)
            else:
                is_signal = (root_file == config.ingress.signal_file)
            labels = torch.ones(n_events)*is_signal
        elif config.ingress.get("label", None):
            labels = torch.tensor(tree[config.ingress.label], dtype=torch.float)
        else:
            raise Exception("No signal file name or label branch provided")

        # Create a unique identifier for this sample (used for splitting evenly)
        sample_number = torch.ones(n_events)*file_i

        # Calculate event weight
        weights = torch.ones(n_events)
        if config.ingress.get("weights", None):
            for branch_i, weight_branch in enumerate(config.ingress.weights):
                weights *= torch.tensor(tree[weight_branch], dtype=torch.float)

        # Load disco target (to be used in SingleDisCo)
        disco_target = None
        if config.ingress.get("disco_target", None):
            disco_target = torch.tensor(tree[config.ingress.disco_target], dtype=torch.float)
            disco_target = transform(disco_target, transforms.get(config.ingress.disco_target, None))

        # Save dataset
        data = DisCoDataset(
            features,
            labels,
            weights,
            sample_number,
            disco_target=disco_target,
            norm=False
        )
        if save:
            data.save(get_outfile(config, tag=sample_name, subdir="datasets", msg="Writing to {}"))
        else:
            return data


def ingress(config, save=True):
    root_files = filter(
        lambda f: "data.root" not in f and "abcdnet" not in f,
        glob.glob(f"{config.ingress.input_dir}/*.root")
    )
    # transforms = config.ingress.get("transforms", {})
    for file_i, root_file in enumerate(root_files):
        ingress_file(config, root_file, file_i, save=True)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Ingress data")
    parser.add_argument("config_json", type=str, help="config JSON")
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)

    ingress(config)
