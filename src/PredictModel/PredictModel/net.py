import os
import torch

import numpy as np

from torch import nn

class FCNet(nn.Module):

    def __init__(self, input_dim, n_hidden1, n_hidden2, output_dim):
        super().__init__()
        self.layer1 = nn.Sequential(nn.Linear(input_dim, n_hidden1), 
                                    # nn.BatchNorm1d(n_hidden1), 
                                    nn.ReLU(inplace=True))
        self.layer2 = nn.Sequential(nn.Linear(n_hidden1, n_hidden2), 
                                    # nn.BatchNorm1d(n_hidden2), 
                                    nn.ReLU(inplace=True))
        self.layer3 = nn.Sequential(nn.Linear(n_hidden2, output_dim))

    def forward(self, x):
        x = self.layer1(x)
        x = self.layer2(x)
        x = self.layer3(x)

        return x