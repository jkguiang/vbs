#!/bin/env python

import time

import uproot
import numpy as np

from infer import VBSOutput
from utils import SimpleProgress

class OutputCSV(VBSOutput):
    def __init__(self, file_name):
        super().__init__(file_name)
        self.__f = open(file_name, "w")
        self.__f.write("idx,truth,score1,score2,weight\n")

    def write(self, idx, truth, score_1, score_2, weight):
        self.__f.write(f"{idx},{int(truth)},{float(score_1)},{float(score_2)},{float(weight)}\n")

    def close(self):
        self.__f.close()

class OutputROOT(VBSOutput):
    def __init__(self, old_baby, new_baby, selection=None, ttree_name="tree", algo_name="abcdnet"):
        super().__init__(new_baby)
        self.__scores_1 = []
        self.__scores_2 = []
        self.old_baby = old_baby
        self.new_baby = new_baby
        self.ttree_name = ttree_name
        self.selection = selection
        self.algo_name = algo_name

    def write(self, idx, truth, score_1, score_2, weight):
        self.__scores_1.append(score_1.item())
        self.__scores_2.append(score_2.item())

    def close(self):
        # Open the existing ROOT file
        with uproot.open(self.old_baby) as old_baby:
            # Copy the existing TTree
            tree = old_baby[self.ttree_name].arrays(cut=self.selection)
            # Add the new branch to the copy
            tree[f"{algo_name}_score1"] = np.array(self.__scores_1)
            tree[f"{algo_name}_score2"] = np.array(self.__scores_2)
            # Write the updated TTree to a new ROOT file
            with uproot.recreate(self.new_baby) as new_baby:
                new_baby[self.ttree_name] = tree

def infer(model_1, model_2, device, loader, output):
    times = []
    for event_i, (features1, features2, labels, weights) in enumerate(SimpleProgress(loader)):
        # Load data
        features1 = features1.to(device)
        features2 = features2.to(device)
        labels = labels.to(device)
        weights = weights.to(device)

        start = time.time()
        inferences_1 = model_1(features1)
        end = time.time()
        times.append(end - start)

        start = time.time()
        inferences_2 = model_2(features2)
        end = time.time()
        times.append(end - start)

        for truth, score_1, score_2, weight in zip(labels, inferences_1, inferences_2, weights):
            output.write(event_i, truth, score_1, score_2, weight)

    output.close()
    print(f"Wrote {output.file_name}")

    return times
