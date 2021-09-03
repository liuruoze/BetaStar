import os
import re

import numpy as np

from tqdm import tqdm

MATCH_TYPE = 'PvT'

DATA_PATH = "./dataset/rgd/" + MATCH_TYPE
OUTPUT_PATH = "./dataset/csv/" + MATCH_TYPE

if not os.path.exists(DATA_PATH):
    os.mkdir(DATA_PATH)

if not os.path.exists(OUTPUT_PATH):
    os.mkdir(OUTPUT_PATH)

rgd_files = []

for f in os.listdir(DATA_PATH):
    if ".rgd" in f:
        rgd_files.append(f)

for f in tqdm(rgd_files):
    game_info = []
    
    with open(os.path.join(DATA_PATH,  f), 'r', encoding='utf-8', errors='ignore') as raw_rgd:
        lines = raw_rgd.readlines()

        if re.match(r"Begin", lines[7]) is None:
            continue

        player_0_race = lines[5].strip().split(',')[2].strip()
        
        if player_0_race == "Protoss":
            game_info.append("0," * 4 + "Protoss" + ",0" * 4 + '\n')
        elif player_0_race == "Terran":
            game_info.append("0," * 4 + "Terran" + ",0" * 4 + '\n')
        elif player_0_race == "Zerg":
            game_info.append("0," * 4 + "Zerg" + ",0" * 4 + '\n')

        for idx, line in enumerate(lines[8:-1]):
            action = re.sub(r'[()]', '', line).strip().split(',')

            action_type = action[2]
            
            length = len(action)

            if length < 9:
                zero_fill = 9 - length
                action += ['0'] * zero_fill

            csv_line = ','.join(action) + '\n'

            if length <= 9:
                game_info.append(csv_line)

    with open(os.path.join(OUTPUT_PATH, f.split('.')[0] + '.csv'), 'w', encoding='utf-8') as game_info_writer:
        game_info_writer.writelines(game_info)

print("Conversion has completed. {} file(s) have been dropped".format(len(rgd_files) - len(os.listdir(OUTPUT_PATH))))





