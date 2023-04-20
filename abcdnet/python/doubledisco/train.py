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
from losses import DoubleDisCoLoss
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

def train(args, model_1, model_2, device, train_loader, optimizer, criterion, epoch):
    model_1.train()
    model_2.train()
    train_t0 = time.time()
    loss_sum = 0
    n_batches = len(train_loader)
    for batch_i, (features, labels, weights) in enumerate(train_loader):
        do_logging = (batch_i % args.log_interval == 0 or batch_i == n_batches - 1)

        # Log start
        if do_logging:
            batch_t0 = time.time()
            print(f"[Epoch {epoch}, {batch_i+1}/{n_batches} ({100*batch_i/n_batches:.2g}%)]", flush=True)

        # Load data
        features = features.to(device)
        labels = labels.to(device)
        weights = weights.to(device)

        # Run inferences
        optimizer.zero_grad()
        inferences_1 = model_1(features).squeeze(1)
        inferences_2 = model_2(features).squeeze(1)
        if torch.any(torch.isnan(inferences)):
            raise ValueError(
                f"Output contains NaN values!"
                + f"\ninferences = {inferences}"
                + f"\nfeatures = {features}"
                + f"\nmax(features) = {features.max()}"
                + f"\nmin(features) = {features.min()}"
            )

        # Calculate loss
        loss = criterion(inferences_1, inferences_2, labels, weights)
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

def validate(model_1, model_2, device, val_loader, criterion):
    model_1.eval()
    model_2.eval()
    n_batches = len(val_loader)
    thresh_sum_1, thresh_sum_2 = 0, 0
    accs_sum_1, accs_sum_2 = 0, 0
    loss_sum = 0
    for (features, labels, weights) in val_loader:
        features = features.to(device)
        labels = labels.to(device)
        weights = weights.to(device)

        inferences_1 = model_1(features).squeeze(1)
        inferences_2 = model_2(features).squeeze(1)
        loss_sum += criterion(inferences_1, inferences_2, labels, weights)

        # define optimal threshold (thresh) where TPR = TNR
        diff_1, opt_thresh_1, opt_acc_1 = 100, 0, 0
        diff_2, opt_thresh_2, opt_acc_2 = 100, 0, 0
        best_tpr, best_tnr = 0, 0
        for thresh in np.arange(0.001, 0.5, 0.001):
            TP, TN, FP, FN = roc_numbers(labels, inferences_1, thresh)
            acc = (TP+TN)/(TP+TN+FP+FN)
            TPR = TP/(TP+FN)
            TNR = TN/(TN+FP)
            delta = abs(TPR-TNR)
            if (delta < diff):
                diff_1, opt_thresh_1, opt_acc_1 = delta, thresh, acc

            TP, TN, FP, FN = roc_numbers(labels, inferences_2, thresh)
            acc = (TP+TN)/(TP+TN+FP+FN)
            TPR = TP/(TP+FN)
            TNR = TN/(TN+FP)
            delta = abs(TPR-TNR)
            if (delta < diff):
                diff_2, opt_thresh_2, opt_acc_2 = delta, thresh, acc
        
        thresh_sum_1 += opt_thresh_1
        thresh_sum_2 += opt_thresh_2
        accs_sum_1 += opt_acc_1
        accs_sum_2 += opt_acc_2

    print(f"val loss:       {loss_sum/n_batches:0.6f}")
    print(f"val accuracy_1: {accs_sum_1/n_batches:0.6f}")
    print(f"val accuracy_2: {accs_sum_2/n_batches:0.6f}")
    return thresh_sum/n_batches

def test(model_1, model_2, device, test_loader, criterion, thresh_1, thresh_2):
    model_1.eval()
    model_2.eval()
    n_batches = len(train_loader)
    losses, accs = [], []
    loss_sum = 0
    accs_sum_1 = 0
    accs_sum_2 = 0
    with torch.no_grad():
        for batch_i, (features, labels, weights) in enumerate(train_loader):
            # Load data
            features = features.to(device)
            labels = labels.to(device)
            weights = weights.to(device)
            
            # Run GNN inference
            inferences_1 = model_1(features).squeeze(1)
            inferences_2 = model_2(features).squeeze(1)

            # Compute accuracy
            TP, TN, FP, FN = roc_numbers(labels, inferences_1, thresh_1)
            acc = (TP+TN)/(TP+TN+FP+FN)
            accs_sum_1 += acc
            TP, TN, FP, FN = roc_numbers(labels, inferences_2, thresh_2)
            acc = (TP+TN)/(TP+TN+FP+FN)
            accs_sum_2 += acc

            # Compute loss
            loss = criterion(inferences_1, inferences_2, labels, weights)
            loss_sum += loss.item()

    print(f"test loss:       {loss_sum/n_batches:0.6f}")
    print(f"test accuracy 1: {accs_sum_1/n_batches:0.6f}")
    print(f"test accuracy 2: {accs_sum_2/n_batches:0.6f}")
    return loss_sum/n_batches, accs_sum_1/n_batches, accs_sum_2/n_batches

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
    model_1 = Model.from_config(config).to(device)
    model_2 = Model.from_config(config).to(device)
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
    criterion = DoubleDisCoLoss.from_config(config)

    # Load data
    print_title("Input data")
    data = DisCoDataset.from_files(
        ingress.get_outfile(config, tag="*", msg="Loading files {}"), 
        is_single_disco=(config.ingress.get("disco_target", None) != None)
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

    output = {"train_loss": [], "test_loss": [], "test_acc_1": [], "test_acc_2": []}
    for epoch in range(1, args.n_epochs + 1):
        epoch_t0 = time.time()
        print_title(f"Epoch {epoch}")
        # Run training
        train_loss = train(args, model_1, model_2, device, train_loader, optimizer, criterion, epoch)
        # Run validation
        thresh_1, thresh_2 = validate(model_1, model_2, device, val_loader, criterion)
        print(f"optimal threshold 1: {thresh_1:0.6f}")
        print(f"optimal threshold 2: {thresh_2:0.6f}")
        # Run testing
        test_loss, test_acc_1, test_acc_2 = test(model_1, model_2, device, test_loader, criterion, thresh=thresh)
        scheduler.step()

        if epoch % 5 == 0:
            torch.save(model_1.state_dict(), get_outfile(config, epoch=epoch, tag="model1", msg="Wrote {}"))
            torch.save(model_2.state_dict(), get_outfile(config, epoch=epoch, tag="model2", msg="Wrote {}"))

        output["train_loss"].append(train_loss)
        output["test_loss"].append(test_loss)
        output["test_acc_1"].append(test_acc_1)
        output["test_acc_2"].append(test_acc_2)
        print(f"total runtime: {time.time() - epoch_t0:0.3f}s", flush=True)

    with open(get_outfile(config, tag="history", ext="json", msg="Wrote {}"), "w") as f_out:
        json.dump(output, f_out)

    print(output)
