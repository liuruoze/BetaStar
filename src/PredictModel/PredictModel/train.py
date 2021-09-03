import os
import torch

import numpy as np
import pandas as pd

from tqdm import tqdm
from torch import nn, optim
from torch.utils.data import Dataset, DataLoader
from torch.utils.data import random_split
from torch.utils.data.sampler import SubsetRandomSampler

from net import FCNet
import config as C
import utils as U

HAS_GPU = torch.cuda.is_available()
DEVICE = torch.device("cuda:0" if HAS_GPU else "cpu")

MATCH_TYPE = "PvT"

DATA_PATH = "./test/"
DATA_FILE = os.path.join(DATA_PATH, "aaa.csv")

BATCH_SIZE = 256
SPLIT_RATIO = 0.5
LEARNING_RATE = 1e-3
EPOCH = 20
HIDDEN_UNITS = 256
NUM_FEATURES = len(C.FEATURE_NAMES) - len(C.PREDICTED_UNITS)

OBS_UNIT_INDEX = U.find_index(C.FEATURE_NAMES, C.PREDICTED_UNITS)
print(OBS_UNIT_INDEX)

np.random.seed(0)


class CustomDataset(Dataset):

    def __init__(self, file_path):
        super().__init__()
        self.data = pd.read_csv(file_path).values.astype(np.float32)
        self.normalize_features()
        

    def __getitem__(self, index):
        features = self.data[index, :NUM_FEATURES]
        labels = self.data[index, NUM_FEATURES:]

        return features, labels

    def __len__(self):
        return self.data.shape[0]
    

    def normalize_features(self):
        features = self.data[:, :NUM_FEATURES]
        # denominator = np.max(features, axis=0) - np.min(features, axis=0)
        denominator = np.array([64800, 31394, 11308, 95086, 37494, 400, 400, 56, 57, 110,
                       17, 15, 33, 23, 22, 16, 37, 165, 35, 24, 31, 9, 2, 22, 44,
                       10, 40, 5, 3, 45, 8, 13, 3, 2, 2, 2, 2, 20, 2, 3, 3, 3, 3,
                       1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                       1, 1, 8, 12, 60, 48, 14, 95, 49, 52, 99, 15, 11, 2, 11, 5,
                       46, 6, 2, 4, 11, 6, 9, 7, 18, 72, 6, 2, 7, 23, 10, 8, 2, 10, 4, 1])
        print(np.min(features, axis=0).astype(np.int).tolist())
        features = (features - np.min(features, axis=0)) / np.expand_dims(denominator, axis=0)
        print(features.shape)

        self.data[:, :NUM_FEATURES] = features


def load_data(data_file, split_ratio=0.5):
    dataset = CustomDataset(data_file)
    train_size = int(len(dataset) * split_ratio)
    val_size = int((len(dataset) - train_size) * 0.4)
    test_size = len(dataset) - val_size - train_size
    train_data, val_data, test_data = random_split(
        dataset, [train_size, val_size, test_size])

    train_loader = DataLoader(train_data, batch_size=BATCH_SIZE, shuffle=False)
    val_loader = DataLoader(val_data, batch_size=BATCH_SIZE, shuffle=False)
    test_loader = DataLoader(test_data, batch_size=BATCH_SIZE, shuffle=False)

    return train_loader, val_loader, test_loader


def train_model(train_loader, val_loader, test_loader, num_epoches):
    model = FCNet(NUM_FEATURES, HIDDEN_UNITS,
                  HIDDEN_UNITS, len(C.PREDICTED_UNITS))
    model.to(DEVICE)

    criterion = nn.MSELoss()
    # optimizer = optim.RMSprop(model.parameters(), lr=LEARNING_RATE)
    optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE)

    for epoch in range(num_epoches):
        model.train()
        train_loss = 0.0
        for i, (feature, label) in enumerate(tqdm(train_loader)):
            feature = feature.to(DEVICE)
            label = label.to(DEVICE)

            output = model(feature)
            loss = criterion(output, label)

            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

            train_loss += loss.item()

        val_loss = eval_model(model, criterion, val_loader)
        print("Epoch: {}, Training Loss: {:.4f}, Val Loss: {:.4f}".format(
            epoch, train_loss / len(train_loader), val_loss))

    test_loss = eval_model(model, criterion, test_loader, test_mode=True)
    print("Training finished, Test Loss = {:.4f}".format(test_loss))


def eval_model(model, criterion, data_loader, test_mode=False):
    model.eval()
    eval_loss = 0.0

    # obs = []
    predictions = []
    labels = []
    for feature, label in data_loader:
        feature = feature.to(DEVICE)
        label = label.to(DEVICE)

        output = model(feature)
        loss = criterion(output, label)

        eval_loss += loss.item()
        # print(feature.cpu().detach().numpy().shape)
        # print(feature.cpu().detach().numpy()[:, OBS_UNIT_INDEX])
        # obs.append(feature.cpu().detach().numpy()[:, OBS_UNIT_INDEX])
        predictions.append(output.cpu().detach().numpy())
        labels.append(label.cpu().detach().numpy())

    if test_mode:
        # obs = np.vstack(obs)
        predictions = np.vstack(predictions)
        labels = np.vstack(labels)
        # np.savetxt("./test/obs.txt", obs)
        np.savetxt("./test/preds.txt", predictions)
        np.savetxt("./test/label.txt", labels)

    return eval_loss / len(data_loader)


if __name__ == "__main__":
    train_loader, val_loader, test_loader = load_data(DATA_FILE, SPLIT_RATIO)
    train_model(train_loader, val_loader, test_loader, EPOCH)
    
