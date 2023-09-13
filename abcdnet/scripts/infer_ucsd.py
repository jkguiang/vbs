#!/bin/env python

import os
import glob
import argparse

import uproot
import numpy as np
import torch
from torch.utils.data import DataLoader

import models
import ingress
import train
from utils import VBSConfig
from datasets import DisCoDataset

class VBSOutput:
    def __init__(self, file_name):
        self.file_name = file_name
    def write(self):
        raise NotImplementedError()
    def close(self):
        raise NotImplementedError()

if __name__ == "__main__":
    # CLI
    parser = argparse.ArgumentParser(
        description="Run inference at UCSD (not all models available, only part of config relevant)"
    )
    parser.add_argument("config_json", type=str, help="config JSON")
    parser.add_argument("model", type=str, help="model PyTorch file")
    parser.add_argument("baby_dir", type=str, help="directory containing analysis babies")
    parser.add_argument(
        "--other_model", type=str, default="",
        help="other model PyTorch file (for double DisCo)"
    )
    parser.add_argument(
        "--name", type=str, default="abcdnet",
        help="name to be used for branches, i.e. NAME_score (default: 'abcdnet')"
    )
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)

    if config.discotype == "single":
        from singledisco.infer import infer, OutputCSV, OutputROOT
        from datasets import SingleDisCoDataset
    elif config.discotype == "double":
        from doubledisco.infer import infer, OutputCSV, OutputROOT
        from datasets import DoubleDisCoDataset

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    Model = getattr(models, config.model.name)
    if config.discotype == "single":
        model = Model.from_config(config).to(device)
        model.load_state_dict(torch.load(args.model, map_location=device))
        model.eval()
    elif config.discotype == "double":
        model1, model2 = Model.from_config(config)
        model1 = model1.to(device)
        model2 = model2.to(device)
        model1.load_state_dict(torch.load(args.model, map_location=device))
        model2.load_state_dict(torch.load(args.other_model, map_location=device))
        model1.eval()
        model2.eval()

    times = []
    out_dir = f"{args.baby_dir}/inferences"
    os.makedirs(out_dir, exist_ok=True)
    # Run inference on extra files (e.g. data)
    for root_file in glob.glob(f"{args.baby_dir}/*.root"):
        # Get data
        loader = DataLoader(ingress.ingress_file(config, root_file, -1, save=False))
        # Make file names
        name = root_file.split("/")[-1].replace(".root", "")
        new_root_file = f"{out_dir}/{name}.root"
        # Run inference (and write output)
        output = OutputROOT(
            root_file, new_root_file, 
            selection=config.ingress.get("selection", None), 
            ttree_name=config.ingress.ttree_name,
            algo_name=args.name
        )
        if config.discotype == "single":
            times += infer(model, device, loader, output)
        elif config.discotype == "double":
            times += infer(model1, model2, device, loader, output)

    print(f"Avg. inference time: {sum(times)/len(times)}s")
    print("\nDone.\n")
