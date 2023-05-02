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

def train(args, model1, model2, device, train_loader, optimizer, criterion, epoch):
    model1.train()
    model2.train()
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
        inferences1 = model1(features).squeeze(1)
        inferences2 = model2(features).squeeze(1)
        if torch.any(torch.isnan(inferences)):
            raise ValueError(
                f"Output contains NaN values!"
                + f"\ninferences = {inferences}"
                + f"\nfeatures = {features}"
                + f"\nmax(features) = {features.max()}"
                + f"\nmin(features) = {features.min()}"
            )

        # Calculate loss
        loss = criterion(inferences1, inferences2, labels, weights)
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

def test(model1, model2, device, test_loader, criterion):
    model1.eval()
    model2.eval()
    n_batches = len(test_loader)
    loss_sum = 0
    with torch.no_grad():
        for batch_i, (features, labels, weights) in enumerate(test_loader):
            # Load data
            features = features.to(device)
            labels = labels.to(device)
            weights = weights.to(device)
            
            # Run GNN inference
            inferences1 = model1(features).squeeze(1)
            inferences2 = model2(features).squeeze(1)

            # Compute loss
            loss = criterion(inferences1, inferences2, labels, weights)
            loss_sum += loss.item()

    print(f"test loss:     {loss_sum/n_batches:0.6f}")
    return loss_sum/n_batches
