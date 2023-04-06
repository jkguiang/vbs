import glob
import os
import uproot
import torch

from utils import VBSConfig

def get_outfile(config, tag, msg=None):
    outfile = f"{config.basedir}/{config.name}_{tag}.pt"
    if msg:
        print(msg.format(outfile))
    return outfile

def ingress(config):
    root_files = [f for f in glob.glob(f"{config.ingress.input_dir}/*.root") if "data.root" not in f]
    for file_i, root_file in enumerate(root_files):
        print(f"Loading {root_file}")
        with uproot.open(f"{root_file}:{config.ingress.ttree_name}") as tree:
            # Load features
            features = []
            for feature_branch in config.ingress.features:
                feature = torch.tensor(tree[feature_branch].array(), dtype=torch.float)
                features.append(feature)

            features = torch.transpose(torch.stack(features), 0, 1)
            features -= features.min(1, keepdim=True)[0]
            features /= features.max(1, keepdim=True)[0]
            n_events = features.size()[0]

            # Calculate event weight
            weights = torch.ones(n_events)
            for branch_i, weight_branch in enumerate(config.ingress.weights):
                weights *= torch.tensor(tree[weight_branch].array(), dtype=torch.float)

            # Load extra features (to be used in SingleDisCo)
            extras = []
            for extra_branch in config.ingress.extras:
                extra = torch.tensor(tree[extra_branch].array(), dtype=torch.float)
                extra -= extra.min()
                extra /= extra.max()
                extras.append(extra)

            # Set label
            if "VBSVVH" in root_file:
                labels = torch.ones(n_events)
            else:
                labels = torch.zeros(n_events)

            fname = root_file.split("/")[-1].replace(".root", "")
            torch.save(features, get_outfile(config, f"{fname}_features", msg="Writing to {}"))
            torch.save(weights, get_outfile(config, f"{fname}_weights", msg="Writing to {}"))
            torch.save(labels, get_outfile(config, f"{fname}_labels", msg="Writing to {}"))
            for extra_name, extra in zip(config.ingress.extras, extras):
                torch.save(
                    extra, get_outfile(config, f"{fname}_{extra_name}", msg="Writing to {}")
                )

if __name__ == "__main__":
    config = VBSConfig.from_json("config.json")
    os.makedirs(config.basedir, exist_ok=True)
    ingress(config)
