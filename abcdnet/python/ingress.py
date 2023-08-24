import glob
import os
import argparse

import uproot
import torch

from utils import VBSConfig
from datasets import SingleDisCoDataset, DoubleDisCoDataset

def get_outfile(config, tag, ext="pt", subdir=None, msg=None):
    outdir = f"{config.base_dir}/{config.name}"
    if subdir:
        outdir = f"{outdir}/{subdir}"

    os.makedirs(outdir, exist_ok=True)
    outfile = f"{outdir}/{config.name}_{tag}.{ext}"

    if msg:
        print(msg.format(outfile))
    return outfile

def transform(feature, transf):
    if type(transf) == list:
        transf, params = transf
    if transf == None:
        return feature
    elif transf == "rescale":
        min_value, max_value = params
        return (feature - min_value)/(max_value - min_value)
    elif transf == "log":
        return torch.log(feature)
    elif transf == "log2":
        return torch.log2(feature)
    elif transf == "log10":
        return torch.log10(feature)
    else:
        raise ValueError(f"transformation '{transf}' not supported")

def ingress_file(config, root_file, file_i, is_signal=None, save=True):
    transforms = config.ingress.get("transforms", {})
    if config.discotype == "single":
        feature_names = config.ingress.features
    elif config.discotype == "double":
        feature_names = config.ingress.features1 + config.ingress.features2
    print(f"Loading {root_file}")
    with uproot.open(root_file) as f:
        # Collect branches to ingress
        branches = (
            feature_names
            + config.ingress.get("label", []) 
            + config.ingress.get("weights", [])
        )
        if config.discotype == "single":
            branches.append(config.ingress.disco_target)

        # Load TTree
        tree = f[config.ingress.ttree_name].arrays(branches, cut=config.ingress.get("selection", None))

        # Assuming that files are named SampleName.root (e.g. QCD.root, ttbar1l.root, etc.)
        sample_name = root_file.split("/")[-1].replace(".root", "")

        # Load features
        features = []
        for feature_branch in feature_names:
            feature = torch.tensor(tree[feature_branch], dtype=torch.float)
            feature = transform(feature, transforms.get(feature_branch, None))
            features.append(feature)

        features = torch.transpose(torch.stack(features), 0, 1)
        n_events = len(features)

        # Set truth labels
        if is_signal is None or is_signal is False:
            labels = torch.zeros(n_events)
        elif is_signal is True:
            labels = torch.ones(n_events)
        elif config.ingress.get("label", None):
            labels = torch.tensor(tree[config.ingress.label], dtype=torch.float)
        else:
            raise Exception("The is_signal argument is not a bool and no alternative signal label was provided")

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
        if config.discotype == "single":
            data = SingleDisCoDataset(
                features,
                labels,
                weights,
                sample_number,
                disco_target,
                norm=False
            )
        elif config.discotype == "double":
            data = DoubleDisCoDataset(
                features,
                labels,
                weights,
                sample_number,
                len(config.ingress.features1),
                len(config.ingress.features2),
                norm=False
            )
        if save:
            print(f"Created {data}")
            torch.save(data, get_outfile(config, tag=sample_name, subdir="datasets", msg="Writing to {}"))
        else:
            return data

def ingress(config):
    file_i = 0
    for root_file in config.ingress.sig_files:
        ingress_file(config, f"{config.ingress.input_dir}/{root_file}", file_i, is_signal=True)
        file_i += 1
    for root_file in config.ingress.bkg_files:
        ingress_file(config, f"{config.ingress.input_dir}/{root_file}", file_i, is_signal=False)
        file_i += 1

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Ingress data")
    parser.add_argument("config_json", type=str, help="config JSON")
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)

    ingress(config)
    print("\nDone.\n")
