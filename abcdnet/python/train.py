import os
import json
import time
import argparse

import torch
import torch.optim.lr_scheduler as lr_schedulers
from torch.utils.data import DataLoader
from torch import optim

import models
import ingress
import singledisco
import doubledisco
from utils import VBSConfig, print_title
from datasets import DisCoDataset

def get_outfile(config, epoch=None, tag=None, ext="pt", subdir=None, msg=None):
    outdir = f"{config.base_dir}/{config.name}"
    if subdir:
        outdir = f"{outdir}/{subdir}"

    os.makedirs(outdir, exist_ok=True)
    outfile = (
        f"{outdir}/{config.name}"
        + f"_model{config.model.name}"
        + f"_nhidden{config.model.n_hidden_layers}"
        + f"_hiddensize{config.model.hidden_size}"
        + f"_lr{config.train.scheduler_name}{config.train.learning_rate}"
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

if __name__ == "__main__":
    # CLI
    parser = argparse.ArgumentParser(description="Train ABCDNet")
    parser.add_argument("config_json", type=str, help="config JSON")
    parser.add_argument("-v", "--verbose", action="store_true", help="toggle verbosity")
    parser.add_argument(
        "--no_cuda", action="store_true", default=False,
        help="disables CUDA training"
    )
    parser.add_argument(
        "--log_interval", type=int, default=100, metavar="N",
        help="how many batches to wait before logging training status"
    )
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)

    print_title("Configuration")
    print(config)

    if config.discotype == "single":
        from singledisco.train import train, test
        from losses import SingleDisCoLoss
        from datasets import SingleDisCoDataset
    elif config.discotype == "double":
        from doubledisco.train import train, test
        from losses import DoubleDisCoLoss
        from datasets import DoubleDisCoDataset

    # Write copy of config
    config.write(
        ingress.get_outfile(config, tag=os.environ.get("SLURM_JOB_ID", "local")+"_config", ext="json")
    )

    print_title("Initialization")
    torch.manual_seed(config.train.seed)
    print(f"seed: {config.train.seed}")
    use_cuda = torch.cuda.is_available()
    device = torch.device("cuda" if use_cuda else "cpu")
    print(f"use_cuda: {use_cuda}")

    # Load model(s)
    Model = getattr(models, config.model.name)
    if config.discotype == "single":
        model = Model.from_config(config).to(device)
        params = list(model.parameters())
    elif config.discotype == "double":
        model1, model2 = Model.from_config(config)
        model1 = model1.to(device)
        model2 = model2.to(device)
        params = list(model1.parameters()) + list(model2.parameters())

    print(f"total trainable params: {len(params)}")

    # Initialize optimizer and scheduler
    optimizer = optim.Adam(
        params, 
        lr=config.train.get("learning_rate", 0.001), 
        weight_decay=config.train.get("weight_decay", 0)
    )
    Scheduler = getattr(lr_schedulers, config.train.scheduler_name)
    scheduler = Scheduler(optimizer, **config.train.scheduler_kwargs)

    # Initialize loss function
    if config.discotype == "single":
        criterion = SingleDisCoLoss.from_config(config)
    elif config.discotype == "double":
        criterion = DoubleDisCoLoss.from_config(config)

    # Load data
    print_title("Input data")
    if config.discotype == "single":
        data = SingleDisCoDataset.from_files(
            ingress.get_outfile(config, tag="*", subdir="datasets", msg="Loading files {}"), 
            norm=config.train.get("weight_norm", True)
        )
    elif config.discotype == "double":
        data = DoubleDisCoDataset.from_files(
            ingress.get_outfile(config, tag="*", subdir="datasets", msg="Loading files {}"), 
            norm=config.train.get("weight_norm", True)
        )
    data.plot(config)
    print(f"Before norm: {data.count_label(0)} bkg, {data.count_label(1)} sig (total raw)")

    # Split into test, train, and validation
    train_data, test_data = data.split(config.train.train_frac)
    print("After norm:")
    print(f"{data} (total)")
    print(f"{train_data} (train)")
    print(f"{test_data} (test)")

    # Save datasets
    torch.save(train_data, ingress.get_outfile(config, tag="train", subdir="inputs", msg="Wrote {}"))
    torch.save(test_data, ingress.get_outfile(config, tag="test", subdir="inputs", msg="Wrote {}"))

    # Initialize loaders
    train_batch_size = round(len(train_data)/config.train.n_batches_train)
    train_loader = DataLoader(train_data, batch_size=train_batch_size, shuffle=True, drop_last=True)
    test_batch_size = round(len(test_data)/config.train.n_batches_test)
    test_loader = DataLoader(test_data, batch_size=test_batch_size, shuffle=True, drop_last=True)

    history_json = get_outfile(config, tag="history", ext="json")
    history = {
        "train_loss": [], 
        "train_bce": [], 
        "train_disco": [], 
        "test_loss": [], 
        "test_bce": [], 
        "test_disco": [], 
        "slurm_id": os.environ.get("SLURM_JOB_ID", "local")
    }
    n_epochs = config.train.get("n_epochs", 200)
    for epoch in range(1, n_epochs + 1):
        epoch_t0 = time.time()
        print_title(f"Epoch {epoch}")
        if config.discotype == "single":
            # Run training
            train_results = train(args, config, model, device, train_loader, optimizer, criterion, epoch)
            # Run testing
            test_results = test(model, device, test_loader, criterion)
        elif config.discotype == "double":
            # Run training
            train_results = train(args, config, model1, model2, device, train_loader, optimizer, criterion, epoch)
            # Run testing
            test_results = test(model1, model2, device, test_loader, criterion)

        scheduler.step()

        # Save model(s)
        if epoch % 5 == 0 or epoch == n_epochs:
            if config.discotype == "single":
                torch.save(
                    model.state_dict(), 
                    get_outfile(config, epoch=epoch, tag="model", subdir="models", msg="Wrote {}")
                )
            elif config.discotype == "double":
                torch.save(
                    model1.state_dict(), 
                    get_outfile(config, epoch=epoch, tag="model1", subdir="models", msg="Wrote {}")
                )
                torch.save(
                    model2.state_dict(), 
                    get_outfile(config, epoch=epoch, tag="model2", subdir="models", msg="Wrote {}")
                )

        # Save history
        train_loss, train_bce, train_disco = train_results
        test_loss, test_bce, test_disco = test_results
        history["train_loss"].append(train_loss)
        history["train_bce"].append(train_bce)
        history["train_disco"].append(train_disco)
        history["test_loss"].append(test_loss)
        history["test_bce"].append(test_bce)
        history["test_disco"].append(test_disco)
        if epoch % 100 == 0 or epoch == n_epochs:
            with open(history_json, "w") as f:
                json.dump(history, f)
                print(f"Wrote {history_json}")

        print(f"total runtime: {time.time() - epoch_t0:0.3f}s", flush=True)
        print("\nDone.\n")
