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
    parser = argparse.ArgumentParser(description="Run inference")
    parser.add_argument("config_json", type=str, help="config JSON")
    parser.add_argument(
        "--epoch", type=int, default=50, metavar="N",
        help="training epoch of model to use for inference (default: 50)"
    )
    parser.add_argument(
        "--export", action="store_true",
        help="write copy of input babies with 'abcdnet_score' branch"
    )
    parser.add_argument(
        "--no_tag", action="store_true",
        help="disabled '_abcdnet' tag for output files"
    )
    parser.add_argument(
        "--name", type=str, default="abcdnet",
        help="name to be used for branches, i.e. NAME_score (default: 'abcdnet')"
    )
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)
    os.makedirs(f"{config.base_dir}/{config.name}", exist_ok=True)

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
        saved_model = train.get_outfile(config, epoch=args.epoch, tag="model", subdir="models", msg="Loading {}")
        model.load_state_dict(torch.load(saved_model, map_location=device))
        model.eval()
    elif config.discotype == "double":
        model1, model2 = Model.from_config(config)
        model1 = model1.to(device)
        model2 = model2.to(device)
        saved_model1 = train.get_outfile(config, epoch=args.epoch, tag="model1", subdir="models", msg="Loading {}")
        saved_model2 = train.get_outfile(config, epoch=args.epoch, tag="model2", subdir="models", msg="Loading {}")
        model1.load_state_dict(torch.load(saved_model1, map_location=device))
        model2.load_state_dict(torch.load(saved_model2, map_location=device))
        model1.eval()
        model2.eval()

    times = []
    if args.export:
        out_dir = f"{config.base_dir}/{config.name}/output"
        os.makedirs(out_dir, exist_ok=True)
        # Process MC
        selection = config.ingress.get("selection", None)
        for pt_file in glob.glob(ingress.get_outfile(config, tag="*", subdir="datasets", msg="Globbing {}")): 
            # Get data
            print(f"Loading {pt_file}")
            if config.discotype == "single":
                data = SingleDisCoDataset.from_file(pt_file, norm=False)
            elif config.discotype == "double":
                data = DoubleDisCoDataset.from_file(pt_file, norm=False)
            print(data)
            loader = DataLoader(data)
            # Make file names
            name = pt_file.split(config.name+"_")[-1].split("_dataset")[0].replace(".pt", "")
            old_root_file = f"{config.ingress.input_dir}/{name}.root"
            if args.no_tag:
                new_root_file = f"{out_dir}/{name}.root"
            else:
                new_root_file = f"{out_dir}/{name}_abcdnet.root"
            # Run inference (and write output)
            output = OutputROOT(
                old_root_file, new_root_file, 
                selection=selection, 
                ttree_name=config.ingress.ttree_name, 
                algo_name=args.name
            )
            if config.discotype == "single":
                times += infer(model, device, loader, output)
            elif config.discotype == "double":
                times += infer(model1, model2, device, loader, output)
        # Run inference on extra files (e.g. data)
        for root_file in config.get("infer", {}).get("extra_files", []):
            # Get data
            loader = DataLoader(ingress.ingress_file(config, root_file, -1, save=False))
            # Make file names
            orig_dir = "/".join(root_file.split("/")[:-1]) or "."
            os.makedirs(f"{orig_dir}/{config.name}", exist_ok=True)
            name = root_file.split("/")[-1].replace(".root", "")
            if args.no_tag:
                new_root_file = f"{out_dir}/{name}.root"
            else:
                new_root_file = f"{out_dir}/{name}_abcdnet.root"
            # Run inference (and write output)
            output = OutputROOT(
                root_file, new_root_file, 
                selection=selection, 
                ttree_name=config.ingress.ttree_name,
                algo_name=args.name
            )
            if config.discotype == "single":
                times += infer(model, device, loader, output)
            elif config.discotype == "double":
                times += infer(model1, model2, device, loader, output)
    else:
        # Load testing and training data
        if config.discotype == "single":
            test_data = SingleDisCoDataset.from_file(
                ingress.get_outfile(config, tag="test", subdir="inputs", msg="Loading {}"), 
                norm=False
            )
            train_data = SingleDisCoDataset.from_file(
                ingress.get_outfile(config, tag="train", subdir="inputs", msg="Loading {}"), 
                norm=False
            )
        if config.discotype == "double":
            test_data = DoubleDisCoDataset.from_file(
                ingress.get_outfile(config, tag="test", subdir="inputs", msg="Loading {}"), 
                norm=False
            )
            train_data = DoubleDisCoDataset.from_file(
                ingress.get_outfile(config, tag="train", subdir="inputs", msg="Loading {}"), 
                norm=False
            )
        test_loader = DataLoader(test_data)
        train_loader = DataLoader(train_data)
        print(test_data)
        print(train_data)
        # Write testing and training inferences
        csv_name = train.get_outfile(config, epoch=args.epoch, tag="REPLACE_inferences", ext="csv", subdir="inferences")
        test_csv = OutputCSV(csv_name.replace("REPLACE", "test"))
        train_csv = OutputCSV(csv_name.replace("REPLACE", "train"))
        if config.discotype == "single":
            times += infer(model, device, test_loader, test_csv)
            times += infer(model, device, train_loader, train_csv)
        elif config.discotype == "double":
            times += infer(model1, model2, device, test_loader, test_csv)
            times += infer(model1, model2, device, train_loader, train_csv)

    print(f"Avg. inference time: {sum(times)/len(times)}s")
    print("\nDone.\n")
