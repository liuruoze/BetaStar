import os

import numpy as np

import config as C
from utils import *

DATA_PATH = "./output/PvP/"
OUTPUT_PATH = "./test/pvp.csv"

if __name__ == "__main__":
    merge_csv(DATA_PATH, OUTPUT_PATH)
    # observations = np.loadtxt("./test/obs.txt")
    # predictions = np.loadtxt("./test/preds.txt")
    # labels = np.loadtxt("./test/label.txt")

    # show_prediction(observations, predictions, labels)  
    # cut_csv(OUTPUT_PATH, 100, "./test/adf.csv")


# import re

# import numpy as np
# import pandas as pd

# from tqdm import tqdm

# DATA_PATH = "./dataset/rgd/"

# rgd_files = []

# for f in os.listdir(DATA_PATH):
#     if ".rgd" in f:
#         rgd_files.append(f)

# is_protoss = 0
# not_protoss = 0

# for f in tqdm(rgd_files):

#     with open(os.path.join(DATA_PATH,  f), 'r', encoding='utf-8', errors='ignore') as raw_rgd:
#         lines = raw_rgd.readlines()
#         print(lines[5].strip())
#         if lines[5].strip().split(',')[2] == "Terran":
#             not_protoss += 1
#         else:
#             is_protoss += 1

# print("is Protoss:", is_protoss)
# print("not Protoss:", not_protoss)
# print("Total:", is_protoss + not_protoss)