import torch
import torch.nn as nn
import torch.nn.functional as F

from utils import print_title

class NeuralNetwork(nn.Module):
    def __init__(self, input_size, n_hidden_layers, hidden_size):
        super().__init__()

        hidden_layers = []
        for layer_i in range(n_hidden_layers):
            if layer_i == 0:
                hidden_layers.append(nn.Linear(input_size, hidden_size))
                hidden_layers.append(nn.ReLU())
            else:
                hidden_layers.append(nn.Linear(hidden_size, hidden_size))
                hidden_layers.append(nn.ReLU())

        self.layers = nn.Sequential(
            *hidden_layers,
            nn.Linear(hidden_size, 1),
            nn.Sigmoid()
        )

        print_title("Neural Network")
        print(self)

    @classmethod
    def from_config(cls, config):
        n_hidden_layers = config.model.n_hidden_layers
        hidden_size = config.model.hidden_size
        if config.discotype == "single":
            input_size = len(config.ingress.features)
            return cls(input_size, n_hidden_layers, hidden_size)
        else:
            input_size1 = len(config.ingress.features1)
            input_size2 = len(config.ingress.features2)
            model1 = cls(input_size1, n_hidden_layers, hidden_size)
            model2 = cls(input_size2, n_hidden_layers, hidden_size)
            return model1, model2

    def forward(self, x):
        return self.layers(x)

class LeakyNeuralNetwork(nn.Module):
    def __init__(self, input_size, n_hidden_layers, hidden_size):
        super().__init__()

        hidden_layers = []
        for layer_i in range(n_hidden_layers):
            if layer_i == 0:
                hidden_layers.append(nn.Linear(input_size, hidden_size))
                hidden_layers.append(nn.LeakyReLU())
            else:
                hidden_layers.append(nn.Linear(hidden_size, hidden_size))
                hidden_layers.append(nn.LeakyReLU())

        self.layers = nn.Sequential(
            *hidden_layers,
            nn.Linear(hidden_size, 1),
            nn.Sigmoid()
        )

        print_title("Neural Network")
        print(self)

    @classmethod
    def from_config(cls, config):
        n_hidden_layers = config.model.n_hidden_layers
        hidden_size = config.model.hidden_size
        if config.discotype == "single":
            input_size = len(config.ingress.features)
            return cls(input_size, n_hidden_layers, hidden_size)
        else:
            input_size1 = len(config.ingress.features1)
            input_size2 = len(config.ingress.features2)
            model1 = cls(input_size1, n_hidden_layers, hidden_size)
            model2 = cls(input_size2, n_hidden_layers, hidden_size)
            return model1, model2

    def forward(self, x):
        return self.layers(x)
