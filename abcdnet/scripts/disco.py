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

def disco(loader):
    discos = []
    for batch_i, (features, labels, weights, disco_target) in enumerate(tqdm(loader)):
        # Load data
        features = features.to(device)
        labels = labels.to(device)
        weights = weights.to(device)
        disco_target = disco_target.to(device)

        inferences = model(features)

        disco = SingleDisCoLoss.dCorr(
            inferences[labels == 0], 
            disco_target[labels == 0], 
            weights[labels == 0], 
            power=2
        )
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

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    saved_model = train.get_outfile(config, epoch=args.epoch, tag="model", subdir="models")
    Model = getattr(models, config.model.name)
    model = Model.from_config(config).to(device)
    model.load_state_dict(torch.load(saved_model, map_location=device))
    model.eval()

    test_data = DisCoDataset.from_file(
        ingress.get_outfile(config, tag="test", subdir="datasets", msg="Loading {}"), 
        norm=False
    )
    test_loader = DataLoader(test_data, batch_size=config.train.test_batch_size)
    print(f"Test disco: {disco(test_loader):0.3f}")