#!/bin/env python

import os
import argparse
from time import time
import numpy as np
import torch
from torch.utils.data import DataLoader

import models
from utils import VBSConfig
from train import get_outfile
from datasets import DisCoDataset

def infer(model, device, loader, output_csv):
    f = open(output_csv, "w")
    f.write("idx,truth,score\n")
    times = []
    for event_i, (features, labels, weights, disco_target) in enumerate(loader):
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
            f.write(f"{event_i},{int(truth)},{float(score)}\n")

    f.close()
    print(f"Wrote {output_csv}")

    return times

if __name__ == "__main__":
    # CLI
    parser = argparse.ArgumentParser(description="Run GNN inference")
    parser.add_argument(
        "--epoch", type=int, default=50, metavar="N",
        help="training epoch of model to use for inference (default: 50)"
    )
    args = parser.parse_args()

    config = VBSConfig.from_json("config.json")
    models_dir = f"{config.basedir}/trained_models"
    infers_dir = f"{config.basedir}/inferences"
    os.makedirs(infers_dir, exist_ok=True)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    saved_model = f"{models_dir}/{get_outfile(config, epoch=args.epoch, tag='model')}"
    Model = getattr(models, config.model.name)
    model = Model.from_config(config).to(device)
    model.load_state_dict(torch.load(saved_model))
    model.eval()

    test_data = DisCoDataset.from_file(f"{models_dir}/{get_outfile(config, tag='test_dataset')}")
    test_loader = DataLoader(test_data)
    times = infer(
        model, device, test_loader, 
        f"{infers_dir}/{get_outfile(config, epoch=args.epoch, tag='test').replace('.pt', '.csv')}"
    )
    train_data = DisCoDataset.from_file(f"{models_dir}/{get_outfile(config, tag='train_dataset')}")
    train_loader = DataLoader(train_data)
    times += infer(
        model, device, train_loader, 
        f"{infers_dir}/{get_outfile(config, epoch=args.epoch, tag='train').replace('.pt', '.csv')}"
    )
    print(f"Avg. inference time: {sum(times)/len(times)}s")
