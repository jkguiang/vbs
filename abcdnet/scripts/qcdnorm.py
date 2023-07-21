import os
import glob
import argparse

import uproot
import torch
import numpy as np

from utils import VBSConfig

def get_event_count(root_file, ttree_name, weight_branches, selection=None):
    with uproot.open(root_file) as f:
        tree = f[ttree_name].arrays(weight_branches, cut=selection)
        weights = None
        for branch_i, weight_branch in enumerate(weight_branches):
            if branch_i == 0:
                weights  = torch.tensor(tree[weight_branch], dtype=torch.float)
            else:
                weights *= torch.tensor(tree[weight_branch], dtype=torch.float)

        return weights.sum().item()

def get_qcdnorm(config, qcd_file, data_file):
    n_qcd = get_event_count(
        qcd_file, config.ingress.ttree_name, config.ingress.weights, selection=config.ingress.selection
    )
    n_other = 0
    for other_file in config.ingress.bkg_files:
        other_file = f"{config.ingress.input_dir}/{other_file}"
        if other_file not in [qcd_file, data_file]:
            n_other += get_event_count(
                other_file, config.ingress.ttree_name, config.ingress.weights, selection=config.ingress.selection
            )
    n_data = get_event_count(
        data_file, config.ingress.ttree_name, config.ingress.weights, selection=config.ingress.selection
    )
    return (n_data - n_other)/n_qcd

if __name__ == "__main__":
    # CLI
    parser = argparse.ArgumentParser(description="Normalize QCD to (data - non-QCD)")
    parser.add_argument("config_json", type=str, help="config JSON")
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)
    indir = config.ingress.input_dir
    outdir = f"{indir}/{config.name}_qcdnorm"
    os.makedirs(outdir, exist_ok=True)

    qcd_file = f"{indir}/QCD.root"
    data_file = f"{indir}/data.root"
    qcdnorm = get_qcdnorm(config, qcd_file, data_file)

    for baby in glob.glob(f"{indir}/*.root"):
        # Open the existing ROOT file
        with uproot.open(baby) as old_baby:
            print(f"Loading {baby}")
            # Copy the existing TTree
            tree = old_baby[config.ingress.ttree_name].arrays(cut=config.ingress.selection)
            n_events = len(tree["xsec_sf"])
            # Add QCD normalization weight
            if baby == qcd_file:
                tree["qcdnorm_sf"] = np.ones(n_events)*qcdnorm
            else:
                tree["qcdnorm_sf"] = np.ones(n_events)
            # Write the updated TTree to a new ROOT file
            with uproot.recreate(baby.replace(indir, outdir)) as new_baby:
                new_baby[config.ingress.ttree_name] = tree
                print(f"Wrote {baby.replace(indir, outdir)}")

    print("\nDone.\n")
