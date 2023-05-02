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
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)
    os.makedirs(f"{config.basedir}/{config.name}", exist_ok=True)

    if config.ingress.get("disco_target", None):
        discotype = "singledisco"
        from singledisco.infer import infer, OutputCSV, OutputROOT
    else:
        discotype = "doubledisco"
        from doubledisco.infer import infer, OutputCSV, OutputROOT

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    Model = getattr(models, config.model.name)
    if discotype == "singledisco":
        saved_model = train.get_outfile(config, epoch=args.epoch, tag="model", subdir="models", msg="Loading {}")
        model = Model.from_config(config).to(device)
        model.load_state_dict(torch.load(saved_model, map_location=device))
        model.eval()
    elif discotype == "doubledisco":
        saved_model1 = train.get_outfile(config, epoch=args.epoch, tag="model1", subdir="models", msg="Loading {}")
        saved_model2 = train.get_outfile(config, epoch=args.epoch, tag="model2", subdir="models", msg="Loading {}")
        model1 = Model.from_config(config).to(device)
        model2 = Model.from_config(config).to(device)
        model1.load_state_dict(torch.load(saved_model1, map_location=device))
        model2.load_state_dict(torch.load(saved_model2, map_location=device))
        model1.eval()
        model2.eval()

    times = []
    if args.export:
        os.makedirs(f"{config.ingress.input_dir}/{config.name}", exist_ok=True)
        # Process MC
        selection = config.ingress.get("selection", None)
        for pt_file in glob.glob(ingress.get_outfile(config, tag="*", subdir="datasets", msg="Globbing {}")): 
            print(f"Loading {pt_file}")
            data = DisCoDataset.from_file(pt_file, norm=False)
            print(data)
            loader = DataLoader(data)
            name = pt_file.split(config.name+"_")[-1].split("_dataset")[0].replace(".pt", "")
            old_baby = f"{config.ingress.input_dir}/{name}.root"
            new_baby = f"{config.ingress.input_dir}/{config.name}/{name}_abcdnet.root"
            output = OutputROOT(old_baby, new_baby, selection=selection)
            if discotype == "singledisco":
                times += infer(model, device, loader, output)
            elif discotype == "doubledisco":
                times += infer(model1, model2, device, loader, output)
        # Run inference on data
        loader = DataLoader(ingress.ingress_file(config, f"{config.ingress.input_dir}/data.root", -1, save=False))
        old_baby = f"{config.ingress.input_dir}/data.root"
        new_baby = f"{config.ingress.input_dir}/{config.name}/data_abcdnet.root"
        output = OutputROOT(old_baby, new_baby, selection=selection)
        if discotype == "singledisco":
            times += infer(model, device, loader, output)
        elif discotype == "doubledisco":
            times += infer(model, device, loader, output)
    else:
        csv_name = train.get_outfile(config, epoch=args.epoch, tag="REPLACE_inferences", ext="csv", subdir="inferences")
        # Write testing inferences
        test_data = DisCoDataset.from_file(
            ingress.get_outfile(config, tag="test", subdir="inputs", msg="Loading {}"), 
            norm=False
        )
        print(test_data)
        test_loader = DataLoader(test_data)
        test_csv = OutputCSV(csv_name.replace("REPLACE", "test"))
        # Write training inferences
        train_data = DisCoDataset.from_file(
            ingress.get_outfile(config, tag="train", subdir="inputs", msg="Loading {}"), 
            norm=False
        )
        print(train_data)
        train_loader = DataLoader(train_data)
        train_csv = OutputCSV(csv_name.replace("REPLACE", "train"))
        # Write validation inferences
        val_data = DisCoDataset.from_file(
            ingress.get_outfile(config, tag="val", subdir="inputs", msg="Loading {}"), 
            norm=False
        )
        print(val_data)
        val_loader = DataLoader(val_data)
        val_csv = OutputCSV(csv_name.replace("REPLACE", "val"))
        if discotype == "singledisco":
            times += infer(model, device, test_loader, test_csv)
            times += infer(model, device, train_loader, train_csv)
            times += infer(model, device, val_loader, val_csv)
        elif discotype == "doubledisco":
            times += infer(model1, model2, device, test_loader, test_csv)
            times += infer(model1, model2, device, train_loader, train_csv)
            times += infer(model1, model2, device, val_loader, val_csv)

    print(f"Avg. inference time: {sum(times)/len(times)}s")
