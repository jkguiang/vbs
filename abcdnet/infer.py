#!/bin/env python

import os
import glob
import argparse
from time import time

import uproot
import numpy as np
import torch
from torch.utils.data import DataLoader
from tqdm import tqdm

import models
import ingress
import train
from utils import VBSConfig
from datasets import DisCoDataset

class OutputVBS:
    def __init__(self, file_name):
        self.file_name = file_name
    def write(self):
        raise NotImplementedError()
    def close(self):
        raise NotImplementedError()

class OutputCSV(OutputVBS):
    def __init__(self, file_name):
        super().__init__(file_name)
        self.__f = open(outname, "w")

    def write(self, idx, truth, score):
        self.__f.write(f"{idx},{int(truth)},{float(score)}\n")

    def close(self):
        self.__f.close()

class OutputROOT(OutputVBS):
    def __init__(self, old_baby, new_baby, ttree_name="tree"):
        super().__init__(new_baby)
        self.__scores = []
        self.__old_baby = old_baby
        self.__new_baby = new_baby
        self.__ttree_name = ttree_name

    def write(self, idx, truth, score):
        self.__scores.append(score.item())

    def close(self):
        # Open the existing ROOT file
        with uproot.open(self.__old_baby) as old_baby:
            # Copy the existing TTree
            tree = old_baby[self.__ttree_name].arrays()
            # Add the new branch to the copy
            tree["abcdnet_score"] = np.array(self.__scores)
            # Write the updated TTree to a new ROOT file
            with uproot.recreate(self.__new_baby) as new_baby:
                new_baby[self.__ttree_name] = tree


def infer(model, device, loader, output):
    times = []
    for event_i, (features, labels, weights, disco_target) in enumerate(tqdm(loader)):
        # Load data
        features = features.to(device)
        labels = labels.to(device)
        weights = weights.to(device)
        disco_target = disco_target.to(device)

        start = time()
        inferences = model(features)
        end = time()
        times.append(end - start)

        for truth, score in zip(labels, inferences):
            output.write(event_i, truth, score)

    output.close()
    print(f"Wrote {output.file_name}")

    return times

if __name__ == "__main__":
    # CLI
    parser = argparse.ArgumentParser(description="Run GNN inference")
    parser.add_argument(
        "--epoch", type=int, default=50, metavar="N",
        help="training epoch of model to use for inference (default: 50)"
    )
    parser.add_argument(
        "--export", action="store_true",
        help="write copy of input babies with 'abcdnet_score' branch"
    )
    args = parser.parse_args()

    config = VBSConfig.from_json("config.json")
    models_dir = f"{config.basedir}/trained_models"
    infers_dir = f"{config.basedir}/inferences"
    os.makedirs(infers_dir, exist_ok=True)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    saved_model = f"{models_dir}/{train.get_outfile(config, epoch=args.epoch, tag='model')}"
    Model = getattr(models, config.model.name)
    model = Model.from_config(config).to(device)
    model.load_state_dict(torch.load(saved_model))
    model.eval()

    if args.export:
        times = []
        for pt_file in glob.glob(DisCoDataset.get_name(config, "*")): 
            loader = DataLoader(DisCoDataset.from_file(pt_file))
            name = pt_file.split(config.name+"_")[-1].split("_dataset")[0]
            old_baby = f"{config.ingress.input_dir}/{name}.root"
            new_baby = old_baby.replace(".root", "_abcdnet.root")
            times += infer(model, device, loader, OutputROOT(old_baby, new_baby))
    else:
        csv_name = train.get_outfile(config, epoch=args.epoch, tag="REPACE").replace('.pt', '.csv')
        # Write testing inferences
        test_data = DisCoDataset.from_file(f"{models_dir}/{tran.get_outfile(config, tag='test_dataset')}")
        test_loader = DataLoader(test_data)
        test_csv = f"{infers_dir}/{csv_name.replace('REPLACE', 'test')}"
        times = infer(model, device, test_loader, OutputCSV(test_csv))
        # Write training inferences
        train_data = DisCoDataset.from_file(f"{models_dir}/{train.get_outfile(config, tag='train_dataset')}")
        train_loader = DataLoader(train_data)
        train_csv = f"{infers_dir}/{csv_name.replace('REPLACE', 'train')}"
        times += infer(model, device, test_loader, OutputCSV(train_csv))

    print(f"Avg. inference time: {sum(times)/len(times)}s")
