import argparse

import torch
from torch.utils.data import DataLoader
from tqdm import tqdm

import models
import ingress
import singledisco.train as train
from losses import SingleDisCoLoss
from utils import VBSConfig
from datasets import DisCoDataset

def disco(loader, disco_lambda):
    discos = []
    for batch_i, (features, labels, weights, disco_target) in enumerate(tqdm(loader)):
        # Load data
        features = features.to(device)
        labels = labels.to(device)
        weights = weights.to(device)
        disco_target = disco_target.to(device)

        inferences = model(features).squeeze(1)

        disco = disco_lambda*SingleDisCoLoss.dCorr(inferences[labels == 0], disco_target[labels == 0], weights[labels == 0], power=2)
        discos.append(disco)
    
    return sum(discos)/len(discos)

if __name__ == "__main__":
    # CLI
    parser = argparse.ArgumentParser(description="Run inference and calculate DisCo")
    parser.add_argument("config_json", type=str, help="config JSON")
    parser.add_argument(
        "--epoch", type=int, default=50, metavar="N",
        help="training epoch of model to use for inference (default: 50)"
    )
    args = parser.parse_args()

    config = VBSConfig.from_json(args.config_json)
    disco_lambda = config.train.get("disco_lambda", 0)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    saved_model = train.get_outfile(config, epoch=args.epoch, tag="model", subdir="models")
    Model = getattr(models, config.model.name)
    model = Model.from_config(config).to(device)
    model.load_state_dict(torch.load(saved_model, map_location=device))
    model.eval()

    train_data = DisCoDataset.from_file(
        ingress.get_outfile(config, tag="train", subdir="inputs", msg="Loading {}"), 
        norm=True
    )
    print(train_data)
    train_batch_size = round(len(train_data)/config.train.n_batches_train)
    train_loader = DataLoader(train_data, batch_size=train_batch_size, shuffle=True, drop_last=True)
    print(f"Train disco: {disco(train_loader, disco_lambda):0.3f}")

    test_data = DisCoDataset.from_file(
        ingress.get_outfile(config, tag="test", subdir="inputs", msg="Loading {}"), 
        norm=True
    )
    print(test_data)
    test_batch_size = round(len(test_data)/config.train.n_batches_test)
    test_loader = DataLoader(test_data, batch_size=test_batch_size, shuffle=True, drop_last=True)
    print(f"Test disco: {disco(test_loader, disco_lambda):0.3f}")
