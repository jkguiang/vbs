import time

import torch

from train import get_outfile

def train(args, config, model1, model2, device, train_loader, optimizer, criterion, epoch):
    model1.train()
    model2.train()
    train_t0 = time.time()
    loss_sum, bce_sum, disco_sum = 0, 0, 0
    n_batches = len(train_loader)
    for batch_i, (features1, features2, labels, weights) in enumerate(train_loader):
        do_logging = (batch_i % args.log_interval == 0 or batch_i == n_batches - 1)

        # Log start
        if do_logging:
            batch_t0 = time.time()
            print(f"[Epoch {epoch}, {batch_i+1}/{n_batches} ({100*batch_i/n_batches:.2g}%)]", flush=True)

        # Load data
        features1 = features1.to(device)
        features2 = features2.to(device)
        labels = labels.to(device)
        weights = weights.to(device)

        # Run inferences
        optimizer.zero_grad()
        inferences1 = model1(features1).squeeze(1)
        inferences2 = model2(features2).squeeze(1)
        if torch.any(torch.isnan(inferences1)) or torch.any(torch.isnan(inferences2)):
            torch.save(
                model1.state_dict(), 
                get_outfile(config, epoch=epoch, tag="DEBUG_MODEL")
            )
            torch.save(
                model2.state_dict(), 
                get_outfile(config, epoch=epoch, tag="DEBUG_MODEL")
            )
            torch.save(inferences1, get_outfile(config, tag="DEBUG_INFERENCES1"))
            torch.save(inferences2, get_outfile(config, tag="DEBUG_INFERENCES2"))
            torch.save(features1, get_outfile(config, tag="DEBUG_FEATURES"))
            torch.save(features2, get_outfile(config, tag="DEBUG_FEATURES"))
            torch.save(labels, get_outfile(config, tag="DEBUG_LABELS"))
            torch.save(weights, get_outfile(config, tag="DEBUG_EVENTWEIGHTS"))
            raise ValueError(
                f"Some (or all) inferences are NaN(s)!"
                + f"\ninferences1 = {inferences1}"
                + f"\ninferences2 = {inferences2}"
                + f"\nfeatures1 = {features1}"
                + f"\nmax(features1) = {features1.max()}"
                + f"\nmin(features1) = {features1.min()}"
                + f"\nfeatures2 = {features2}"
                + f"\nmax(features2) = {features2.max()}"
                + f"\nmin(features2) = {features2.min()}"
            )

        # Calculate loss
        loss, bce, disco = criterion(inferences1, inferences2, labels, weights)
        if torch.isnan(loss):
            torch.save(
                model1.state_dict(), 
                get_outfile(config, epoch=epoch, tag="DEBUG_MODEL")
            )
            torch.save(
                model2.state_dict(), 
                get_outfile(config, epoch=epoch, tag="DEBUG_MODEL")
            )
            torch.save(inferences1, get_outfile(config, tag="DEBUG_INFERENCES1"))
            torch.save(inferences2, get_outfile(config, tag="DEBUG_INFERENCES2"))
            torch.save(features1, get_outfile(config, tag="DEBUG_FEATURES"))
            torch.save(features2, get_outfile(config, tag="DEBUG_FEATURES"))
            torch.save(labels, get_outfile(config, tag="DEBUG_LABELS"))
            torch.save(weights, get_outfile(config, tag="DEBUG_EVENTWEIGHTS"))
            raise ValueError(
                f"Loss is NaN!"
                + f"\ninferences1 = {inferences1}"
                + f"\ninferences2 = {inferences2}"
                + f"\nweights = {weights}"
                + f"\nmax(inferences1) = {inferences1.max()}"
                + f"\nmin(inferences1) = {inferences1.min()}"
                + f"\nmax(inferences2) = {inferences2.max()}"
                + f"\nmin(inferences2) = {inferences2.min()}"
                + f"\nmax(weights) = {weights.max()}"
                + f"\nmin(weights) = {weights.min()}"
            )

        # Wrap up
        loss.backward()
        optimizer.step()
        loss_sum += loss.item()
        bce_sum += bce.item()
        disco_sum += disco.item()

        # Log end
        if do_logging:
            batch_t1 = time.time()
            print(f"Finished batch in {batch_t1 - batch_t0:0.3f}s", flush=True)

    print(f"[Epoch {epoch} summary]", flush=True)
    print(f"train runtime: {time.time() - train_t0:0.3f}s", flush=True)
    print(f"train loss:    {loss_sum/n_batches:0.6f}", flush=True)
    return loss_sum/n_batches, bce_sum/n_batches, disco_sum/n_batches

def test(model1, model2, device, test_loader, criterion):
    model1.eval()
    model2.eval()
    n_batches = len(test_loader)
    loss_sum, bce_sum, disco_sum = 0, 0, 0
    with torch.no_grad():
        for batch_i, (features1, features2, labels, weights) in enumerate(test_loader):
            # Load data
            features1 = features1.to(device)
            features2 = features2.to(device)
            labels = labels.to(device)
            weights = weights.to(device)
            
            # Run GNN inference
            inferences1 = model1(features1).squeeze(1)
            inferences2 = model2(features2).squeeze(1)

            # Compute loss
            loss, bce, disco = criterion(inferences1, inferences2, labels, weights)
            loss_sum += loss.item()
            bce_sum += bce.item()
            disco_sum += disco.item()

    print(f"test loss:     {loss_sum/n_batches:0.6f}")
    return loss_sum/n_batches, bce_sum/n_batches, disco_sum/n_batches
