#!/bin/env python

import os
import time

import uproot
import numpy as np

from infer import VBSOutput
from utils import SimpleProgress

class OutputCSV(VBSOutput):
    def __init__(self, file_name):
        super().__init__(file_name)
        self.__f = open(file_name, "w")
        self.__f.write("idx,truth,score,weight\n")

    def write(self, idx, truth, score, weight):
        self.__f.write(f"{idx},{int(truth)},{float(score)},{float(weight)}\n")

    def close(self):
        self.__f.close()

class OutputROOT(VBSOutput):
    def __init__(self, old_baby, new_baby, selection=None, ttree_name="tree"):
        super().__init__(new_baby)
        self.__scores = []
        self.__old_baby = old_baby
        self.__new_baby = new_baby
        self.__ttree_name = ttree_name
        self.__selection = selection

    def write(self, idx, truth, score, weight):
        self.__scores.append(score.item())

    def close(self):
        # Open the existing ROOT file
        with uproot.open(self.__old_baby) as old_baby:
            # Copy the existing TTree
            tree = old_baby[self.__ttree_name].arrays(cut=self.__selection)
            # Add the new branch to the copy
            tree["abcdnet_score"] = np.array(self.__scores)
            # Write the updated TTree to a new ROOT file
            with uproot.recreate(self.__new_baby) as new_baby:
                new_baby[self.__ttree_name] = tree

def infer(model, device, loader, output):
    times = []
    for event_i, (features, labels, weights, disco_target) in enumerate(SimpleProgress(loader)):
        # Load data
        features = features.to(device)
        labels = labels.to(device)
        weights = weights.to(device)
        disco_target = disco_target.to(device)

        start = time.time()
        inferences = model(features)
        end = time.time()
        times.append(end - start)

        for truth, score, weight in zip(labels, inferences, weights):
            output.write(event_i, truth, score, weight)

    output.close()
    print(f"Wrote {output.file_name}")

    return times
