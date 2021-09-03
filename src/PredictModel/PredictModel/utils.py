import os

import numpy as np
import pandas as pd

from tqdm import tqdm

import config as C

def merge_csv(path, output_path):
    output = []
    csv_files = os.listdir(path)

    for f in csv_files:
        file_path = os.path.join(path, f)
        df = pd.read_csv(file_path)
        output.append(df.values[:, 1:])
        
        
    output = np.vstack(output)
    
    print(output.shape)
    print(np.max(output, axis=0).astype(np.int).tolist())
    print(np.min(output, axis=0).astype(np.int).tolist())
    txt2csv(output, output_path)
    # np.savetxt(output_path, output)

def show_prediction(observations, predictions, labels):
    idx = np.random.permutation(predictions.shape[0])

    observations = observations[idx]
    predictions = predictions[idx]
    labels = labels[idx]

    predictions[np.where(predictions < 0)] = 0
    predictions = np.round(predictions).astype(np.int32)

    print(' '.join(['.'.join([str(idx+1), name]) for idx, name in enumerate(C.PREDICTED_UNITS)]))
    print("-" * 50)
    for i in range(20):
        print("Observation:  ", observations[i, :].astype(np.int32))
        print("Prediction:   ", np.ceil(predictions[i, :]).astype(np.int32))
        print("Ground Truth: ", labels[i, :].astype(np.int32))
        print("-" * 50)

def txt2csv(array, output_path):
    df = pd.DataFrame(data=array)
    df.to_csv(output_path, header=False, index=False)

def find_index(feature_names, predict_types):
    observed = []
    for t in predict_types:
        observed.append(feature_names.index(t))

    return observed

def cut_csv(csv_file, num_lines, output_path):
    df = pd.read_csv(csv_file)
    if num_lines > df.shape[0]:
        return
    else:
        df = df[:num_lines]
        # df = df[:df.shape[0] // 2]
        df.to_csv(output_path, header=False, index=False)