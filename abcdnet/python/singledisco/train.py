import time

import torch

from train import get_outfile

def train(args, config, model, device, train_loader, optimizer, criterion, epoch):
    model.train()
    train_t0 = time.time()
    loss_sum, bce_sum, disco_sum = 0, 0, 0
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
            torch.save(
                model.state_dict(), 
                get_outfile(config, epoch=epoch, tag="DEBUG_MODEL")
            )
            torch.save(inferences, get_outfile(config, tag="DEBUG_INFERENCES"))
            torch.save(features, get_outfile(config, tag="DEBUG_FEATURES"))
            torch.save(labels, get_outfile(config, tag="DEBUG_LABELS"))
            torch.save(disco_target, get_outfile(config, tag="DEBUG_DISCOTARGETS"))
            torch.save(weights, get_outfile(config, tag="DEBUG_EVENTWEIGHTS"))
            raise ValueError(
                f"Some (or all) inferences are NaN(s)!"
                + f"\ninferences = {inferences}"
                + f"\nfeatures = {features}"
                + f"\nmax(features) = {features.max()}"
                + f"\nmin(features) = {features.min()}"
            )

        # Calculate loss
        loss, bce, disco = criterion(inferences, labels, disco_target, weights)
        if torch.isnan(loss):
            torch.save(
                model.state_dict(), 
                get_outfile(config, epoch=epoch, tag="DEBUG_MODEL")
            )
            torch.save(inferences, get_outfile(config, tag="DEBUG_INFERENCES"))
            torch.save(features, get_outfile(config, tag="DEBUG_FEATURES"))
            torch.save(labels, get_outfile(config, tag="DEBUG_LABELS"))
            torch.save(disco_target, get_outfile(config, tag="DEBUG_DISCOTARGETS"))
            torch.save(weights, get_outfile(config, tag="DEBUG_EVENTWEIGHTS"))
            raise ValueError(
                f"Loss is NaN!"
                + f"\ninferences = {inferences}"
                + f"\ndisco_target = {disco_target}"
                + f"\nweights = {weights}"
                + f"\nmax(inferences) = {inferences.max()}"
                + f"\nmin(inferences) = {inferences.min()}"
                + f"\nmax(disco_target) = {disco_target.max()}"
                + f"\nmin(disco_target) = {disco_target.min()}"
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

def test(model, device, test_loader, criterion):
    model.eval()
    n_batches = len(test_loader)
    loss_sum, bce_sum, disco_sum = 0, 0, 0
    with torch.no_grad():
        for batch_i, (features, labels, weights, disco_target) in enumerate(test_loader):
            # Load data
            features = features.to(device)
            labels = labels.to(device)
            weights = weights.to(device)
            disco_target = disco_target.to(device)
            
            # Run GNN inference
            inferences = model(features).squeeze(1)

            # Compute loss
            loss, bce, disco = criterion(inferences, labels, disco_target, weights)
            loss_sum += loss.item()
            bce_sum += bce.item()
            disco_sum += disco.item()

    print(f"test loss:     {loss_sum/n_batches:0.6f}")
    return loss_sum/n_batches, bce_sum/n_batches, disco_sum/n_batches
