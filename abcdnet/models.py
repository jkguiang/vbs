import torch
import torch.nn as nn
import torch.nn.functional as F

from utils import print_title

class NeuralNetwork(nn.Module):
    def __init__(self, input_size, n_hidden_layers, hidden_size):
        super().__init__()

        hidden_layers = []
        for layer_i in range(n_hidden_layers):
            hidden_layers.append(nn.Linear(hidden_size, hidden_size))
            hidden_layers.append(nn.ReLU())

        self.layers = nn.Sequential(
            nn.Linear(input_size, hidden_size),
            nn.ReLU(),
            *hidden_layers,
            nn.Linear(hidden_size, 1),
            nn.Sigmoid()
        )

        print_title("Neural Network")
        print(self)

    @classmethod
    def from_config(cls, config):
        input_size = len(config.ingress.features)
        n_hidden_layers = config.model.n_hidden_layers
        hidden_size = config.model.hidden_size
        return cls(input_size, n_hidden_layers, hidden_size)

    def forward(self, x):
        return self.layers(x)
