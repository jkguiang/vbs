import os
import json
import time
import argparse

import numpy as np
import torch
import torch.optim.lr_scheduler as lr_schedulers
import torch.nn.functional as F
from torch.utils.data import DataLoader
from torch import optim

import models
import ingress
from utils import VBSConfig, print_title
from losses import SingleDisCoLoss
from datasets import DisCoDataset

def get_outfile(config, epoch=None, tag=None, ext="pt", msg=None):
    outfile = (
        f"{config.basedir}/{config.name}/{config.name}"
        + f"_model{config.model.name}"
        + f"_nhidden{config.model.n_hidden_layers}"
        + f"_hiddensize{config.model.hidden_size}"
        + f"_lr{config.train.learning_rate}"
        + f"_discolambda{config.train.disco_lambda}"
    )

    if epoch:
        outfile += f"_epoch{epoch}"
    if tag:
        outfile += f"_{tag}"

    outfile += f".{ext}"
    if msg:
        print(msg.format(outfile))

    return outfile

def train(args, model, device, train_loader, optimizer, criterion, epoch):
    model.train()
    train_t0 = time.time()
    loss_sum = 0
    n_batches = len(train_loader)
    for batch_i, (features, labels, weights, disco_target) in enumerate(train_loader):
        do_logging = (batch_i % args.log_interval == 0 or batch_i == n_batches - 1)

        # Log start
        if do_logging:
            batch_t0 = time.time()
            print(f"[Epoch {epoch}, {batch_i+1}/{n_batches} ({100*batch_i/n_batches:.2g}%)]", flush=True)

        # Load data
        features = features.to(device)
        labels = labels.to(device)
        weights = weights.to(device)
        disco_target = disco_target.to(device)

        # Run inferences
        optimizer.zero_grad()
        inferences = model(features).squeeze(1)
        if torch.any(torch.isnan(inferences)):
            raise ValueError(
                f"Output contains NaN values!"
                + f"\ninferences = {inferences}"
                + f"\nfeatures = {features}"
                + f"\nmax(features) = {features.max()}"
                + f"\nmin(features) = {features.min()}"
            )

        # Calculate loss
        loss = criterion(inferences, labels, disco_target, weights)
        if torch.isnan(loss):
            raise ValueError(
                f"Loss is NaN!"
                + f"\ninferences = {inferences}"
                + f"\ndisco_target = {disco_target}"
                + f"\nmax(disco_target) = {disco_target.max()}"
                + f"\nmin(disco_target) = {disco_target.min()}"
            )

        # Wrap up
        loss.backward()
        optimizer.step()
        loss_sum += loss.item()

        # Log end
        if do_logging:
            batch_t1 = time.time()
            print(f"Finished batch in {batch_t1 - batch_t0:0.3f}s", flush=True)

    print(f"[Epoch {epoch} summary]", flush=True)
    print(f"train runtime: {time.time() - train_t0:0.3f}s", flush=True)
    print(f"train loss:    {loss_sum/n_batches:0.6f}", flush=True)
    return loss_sum/n_batches

def roc_numbers(labels, inferences, thresh):
    TP = torch.sum((labels == 1) & (inferences >= thresh)).item()
    TN = torch.sum((labels == 0) & (inferences <  thresh)).item()
    FP = torch.sum((labels == 0) & (inferences >= thresh)).item()
    FN = torch.sum((labels == 1) & (inferences <  thresh)).item()
    return TP, TN, FP, FN

def validate(model, device, val_loader, criterion):
    model.eval()
    n_batches = len(val_loader)
    thresh_sum = 0
    accs_sum = 0
    loss_sum = 0
    for (features, labels, weights, disco_target) in val_loader:
        features = features.to(device)
        labels = labels.to(device)
        weights = weights.to(device)
        disco_target = disco_target.to(device)

        inferences = model(features).squeeze(1)
        loss_sum += criterion(inferences, labels, disco_target, weights)

        # define optimal threshold (thresh) where TPR = TNR
        diff, opt_thresh, opt_acc = 100, 0, 0
        best_tpr, best_tnr = 0, 0
        for thresh in np.arange(0.001, 0.5, 0.001):
            TP, TN, FP, FN = roc_numbers(labels, inferences, thresh)
            acc = (TP+TN)/(TP+TN+FP+FN)
            TPR, TNR = TP/(TP+FN), TN/(TN+FP)
            delta = abs(TPR-TNR)
            if (delta < diff):
                diff, opt_thresh, opt_acc = delta, thresh, acc
        
        thresh_sum += opt_thresh
        accs_sum += opt_acc

    print(f"val loss:      {loss_sum/n_batches:0.6f}")
    print(f"val accuracy:  {accs_sum/n_batches:0.6f}")
    return thresh_sum/n_batches

def test(model, device, test_loader, criterion, thresh=0.5):
    model.eval()
    n_batches = len(train_loader)
    losses, accs = [], []
    loss_sum = 0
    accs_sum = 0
    with torch.no_grad():
        for batch_i, (features, labels, weights, disco_target) in enumerate(train_loader):
            # Load data
            features = features.to(device)
            labels = labels.to(device)
            weights = weights.to(device)
            disco_target = disco_target.to(device)
            
            # Run GNN inference
            inferences = model(features).squeeze(1)

            # Compute accuracy
            TP, TN, FP, FN = roc_numbers(labels, inferences, thresh)
            acc = (TP+TN)/(TP+TN+FP+FN)
            accs_sum += acc

            # Compute loss
            loss = criterion(inferences, labels, disco_target, weights)
            loss_sum += loss.item()

    print(f"test loss:     {loss_sum/n_batches:0.6f}")
    print(f"test accuracy: {accs_sum/n_batches:0.6f}")
    return loss_sum/n_batches, accs_sum/n_batches

if __name__ == "__main__":
    # CLI
    parser = argparse.ArgumentParser(description="Train GNN")
    parser.add_argument("config_json", type=str, help="config JSON")
    parser.add_argument("-v", "--verbose", action="store_true", help="toggle verbosity")
    parser.add_argument(
        "--no_cuda", action="store_true", default=False,
        help="disables CUDA training"
    )
    parser.add_argument(
        "--n_epochs", type=int, default=50, metavar="N",
        help="number of epochs to train"
    )
    parser.add_argument(
        "--log_interval", type=int, default=100, metavar="N",
        help="how many batches to wait before logging training status"
    )
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)
    os.makedirs(f"{config.basedir}/{config.name}", exist_ok=True)

    print_title("Configuration")
    print(config)

    print_title("Initialization")
    torch.manual_seed(config.train.seed)
    print(f"seed: {config.train.seed}")
    use_cuda = torch.cuda.is_available()
    device = torch.device("cuda" if use_cuda else "cpu")
    print(f"use_cuda: {use_cuda}")

    # Load model
    Model = getattr(models, config.model.name)
    model = Model.from_config(config).to(device)
    total_trainable_params = sum(p.numel() for p in model.parameters())
    print(f"total trainable params: {total_trainable_params}")

    optimizer = optim.Adam(
        model.parameters(), 
        lr=config.train.get("learning_rate", 0.001), 
        weight_decay=config.train.get("weight_decay", 0)
    )
    Scheduler = getattr(lr_schedulers, config.train.scheduler_name)
    scheduler = Scheduler(optimizer, **config.train.scheduler_kwargs)

    # Initialize loss function
    criterion = SingleDisCoLoss.from_config(config)

    # Load data
    print_title("Input data")
    data = DisCoDataset.from_files(
        ingress.get_outfile(config, tag="*", msg="Loading files {}"), 
        is_single_disco=(config.ingress.get("disco_target", None) != None),
        norm=config.ingress.get("weight_norm", True)
    )
    data.plot(config)
    print(f"Before norm: {data.n_label(0)} bkg, {data.n_label(1)} sig (total raw)")

    # Split into test, train, and validation
    train_data, leftover_data = data.split(config.train.train_frac)
    test_data, val_data = leftover_data.split(config.train.test_frac/(1 - config.train.train_frac))
    print("After norm:")
    print(f"{data} (total)")
    print(f"{train_data} (train)")
    print(f"{test_data} (test)")
    print(f"{val_data} (val)")

    # Save datasets
    train_data.save(ingress.get_outfile(config, tag="train", msg="Wrote {}"))
    test_data.save(ingress.get_outfile(config, tag="test", msg="Wrote {}"))
    val_data.save(ingress.get_outfile(config, tag="val", msg="Wrote {}"))

    # Initialize loaders
    train_loader = DataLoader(train_data, batch_size=config.train.train_batch_size, shuffle=True)
    test_loader = DataLoader(test_data, batch_size=config.train.test_batch_size, shuffle=True)
    val_loader = DataLoader(val_data, batch_size=config.train.val_batch_size, shuffle=True)

    output = {
        "train_loss": [], 
        "test_loss": [], 
        "test_acc": [], 
        "slurm_id": os.environ.get("SLURM_JOB_ID", "local")
    }
    for epoch in range(1, args.n_epochs + 1):
        epoch_t0 = time.time()
        print_title(f"Epoch {epoch}")
        # Run training
        train_loss = train(args, model, device, train_loader, optimizer, criterion, epoch)
        # Run validation
        thresh = validate(model, device, val_loader, criterion)
        print(f"optimal threshold: {thresh:0.6f}")
        # Run testing
        test_loss, test_acc = test(model, device, test_loader, criterion, thresh=thresh)
        scheduler.step()

        if epoch % 5 == 0:
            torch.save(model.state_dict(), get_outfile(config, epoch=epoch, tag="model", msg="Wrote {}"))

        output["train_loss"].append(train_loss)
        output["test_loss"].append(test_loss)
        output["test_acc"].append(test_acc)
        print(f"total runtime: {time.time() - epoch_t0:0.3f}s", flush=True)

    with open(get_outfile(config, tag="history", ext="json", msg="Wrote {}"), "w") as f_out:
        json.dump(output, f_out)

    print(output)