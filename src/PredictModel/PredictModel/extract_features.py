import os 
import time
import random
import logging
import platform
import argparse

import numpy as np
import pandas as pd
import multiprocessing as mp

from tqdm import tqdm 

import config as C

# random.seed(4)
parser = argparse.ArgumentParser()

parser.add_argument("-d", "--debug", help="Enter debug mode.", action="store_true")
parser.add_argument("-m", "--matchtype", choices=['PvP', 'PvT', 'PvZ'], default='PvT', help="Specify match type.")

args = parser.parse_args()

MATCH_TYPE = args.matchtype
DEBUG_MODE = args.debug

DATA_PATH = "./dataset/csv/" + MATCH_TYPE
OUTPUT_PATH = "./output/" + MATCH_TYPE

if not os.path.exists(OUTPUT_PATH):
    os.mkdir(OUTPUT_PATH)

ON_SERVER = 'Linux' in platform.platform()
NUM_PROCESSES = 24 if ON_SERVER else 1
INTERFRAME = 500

if MATCH_TYPE == 'PvT':
    ENEMY_UNITS = C.TERRAN_UNITS
    ENEMY_BUILDINGS = C.TERRAN_BUILDINGS
    PREDICTED_UNITS = C.TERRAN_PREDICTIONS
elif MATCH_TYPE == 'PvZ':
    ENEMY_UNITS = C.ZERG_UNITS
    ENEMY_BUILDINGS = C.ZERG_BUILDINGS
    PREDICTED_UNITS = C.ZERG_PREDICTIONS
elif MATCH_TYPE == 'PvP':
    ENEMY_UNITS = C.PROTOSS_UNITS
    ENEMY_BUILDINGS = C.PROTOSS_BUILDINGS
    PREDICTED_UNITS = C.PROTOSS_PREDICTONS

NUM_UNIT_TYPES = len(C.PROTOSS_UNITS + C.PROTOSS_BUILDINGS)
NUM_RESEARCHES = len(C.PROTOSS_UPGRADES + C.PROTOSS_RESEARCH)
NUM_OBSERVATIONS = len(ENEMY_UNITS + ENEMY_BUILDINGS)
NUM_PREDICTIONS = len(PREDICTED_UNITS)


FEATURE_NAMES = ['Frame', 'Mineral', 'Gas',
                 'Gathered Mineral', 'Gathered Gas', 'Supply Used',
                 'Total Supply'] + \
                 C.PROTOSS_UNITS + C.PROTOSS_BUILDINGS + C.PROTOSS_UPGRADES + \
                 C.PROTOSS_RESEARCH + ENEMY_UNITS + ENEMY_BUILDINGS + PREDICTED_UNITS

if DEBUG_MODE:
    print(NUM_UNIT_TYPES + NUM_RESEARCHES)
    print(NUM_OBSERVATIONS)
    print(NUM_PREDICTIONS)
    print(len(FEATURE_NAMES))

def load_rgd(path):
    names = ['frame', 'playerID', 'action',
              'attribute_1', 'attribute_2', 'attribute_3',
              'attribute_4', 'attribute_5', 'attribute_6']
    df = pd.read_csv(path, names=names)

    return df


def split_player_info(df):
    player_0_race = df.iloc[0].attribute_2
    max_frame = df.tail(1).frame.values[0]
    
    df = df.drop(index=0)

    if player_0_race == "Protoss":
        player = df[df.playerID == 0].drop(columns="playerID")
        opponent = df[df.playerID == 1].drop(columns="playerID")
    else:
        player = df[df.playerID == 1].drop(columns="playerID")
        opponent = df[df.playerID == 0].drop(columns="playerID")

    return player, opponent, max_frame


def count_units(row, unit_list):
    feature = np.zeros((len(unit_list),))

    action = row.action
    unit_type = row.attribute_2

    if unit_type == 'Terran Siege Tank Siege Mode' or unit_type == 'Terran Siege Tank Tank Mode':
        unit_type = 'Terran Siege Tank'

    if unit_type in unit_list:
        if action != 'Morph':
            feature[unit_list.index(unit_type)] = 1
        elif unit_type in C.MORPHED_UNITS:
            feature[unit_list.index(unit_type)] = 1
            if unit_type in C.MORPHED_ZERG:
                feature[unit_list.index(get_pre_unit(unit_type))] = -1

        if action == 'Destroyed':
            feature = -feature
    # else:
    #     print(unit_type)

    return feature


def acc_unit_num(df, unit_list):
    unit_info = df[(df.action == 'Created') | (df.action == 'Destroyed') | (df.action == 'Morph')]
    # unit_info = unit_info[unit_info.attribute_2 in unit_list]
    units_in_frame = np.zeros((len(unit_list),))

    for idx, row in unit_info.iterrows():
        units_in_frame += count_units(row, unit_list)
    
    return units_in_frame


def get_research_level(df, current_info):
    research_info = df[(df.action == 'FinishUpgrade') | (df.action == 'FinishResearch')]
    research_names = C.PROTOSS_UPGRADES + C.PROTOSS_RESEARCH

    for idx, row in research_info.iterrows():
        action = row.action
        research_type = row.attribute_1
        research_level = row.attribute_2

        if research_type in research_names:
            if action == 'FinishUpgrade':
                current_info[research_names.index(research_type)] = research_level
            elif action == 'FinishResearch':
                current_info[research_names.index(research_type)] = 1
        # else:
        #     print(research_type)

    return current_info


def get_observed_enemy(df, current_info, unit_id):
    obs_info = df[(df.action == 'Discovered') | (df.action == 'Destroyed')]
    obs_names = ENEMY_UNITS + ENEMY_BUILDINGS

    for idx, row in obs_info.iterrows():
        action = row.action
        uid = row.attribute_1
        unit_type = row.attribute_2

        if unit_type == 'Terran Siege Tank Siege Mode' or unit_type == 'Terran Siege Tank Tank Mode':
            unit_type = 'Terran Siege Tank'

        if unit_type in obs_names:
            num_units = len(unit_id)
            if DEBUG_MODE and uid == '136':
                print(current_info[2], action, uid, unit_id.get(uid), row.frame)
            # if DEBUG_MODE and unit_type == "Zerg Drone":
            #     print(current_info[2], action, uid, unit_id.get(uid), row.frame)
            if action == 'Discovered':
                if uid not in unit_id.keys():
                    unit_id[uid] = unit_type
                else:
                    existed_unit = unit_id[uid]
                    unit_id[uid] = unit_type

                if len(unit_id) != num_units:
                    current_info[obs_names.index(unit_type)] += 1
                elif unit_type in C.MORPHED_ZERG and existed_unit != unit_type:
                    current_info[obs_names.index(unit_type)] += 1
                    current_info[obs_names.index(existed_unit)] -= 1
                            
            elif action == 'Destroyed':
                if uid in unit_id.keys():
                    del unit_id[uid]
                    current_info[obs_names.index(unit_type)] -= 1

    return current_info

def get_pre_unit(unit_type):
    pre_type = None

    if unit_type == 'Zerg Lair':
        pre_type = 'Zerg Hatchery'
    elif unit_type == 'Zerg Hive':
        pre_type = 'Zerg Lair'
    elif unit_type == 'Zerg Lurker':
        pre_type = 'Zerg Hydralisk'
    elif unit_type == 'Zerg Devourer':
        pre_type = 'Zerg Mutalisk'
    elif unit_type == 'Zerg Guardian':
        pre_type = 'Zerg Mutalisk'
    elif unit_type == 'Zerg Sunken Colony' or 'Zerg Spore Colony':
        pre_type = 'Zerg Creep Colony'
    else:
        pre_type = 'Zerg Drone'
    
    return pre_type


def extract_features(df):
    player, opponent, max_frame = split_player_info(df)

    player = player.applymap(lambda x: pd.to_numeric(x, errors='ignore'))
    res_info = player[player.action == 'R']

    player_unit_info = np.zeros((NUM_UNIT_TYPES,))
    player_research_info = np.zeros((NUM_RESEARCHES,))    
    observation = np.zeros((NUM_OBSERVATIONS,))
    opponent_info = np.zeros((NUM_PREDICTIONS))
    unit_id = dict()

    features = []

    for i in range(25 * random.randint(0, INTERFRAME // 25 - 1), max_frame + 1, INTERFRAME):
        player_cframes = player[(player.frame > i - INTERFRAME) & (player.frame <= i)]
        opponent_cframes = opponent[(opponent.frame > i - INTERFRAME) & (opponent.frame <= i)]
        
        frame = np.array([i]) # current frame
        res_features = np.array(res_info[res_info.frame == i].values[0][2:])
        player_unit_info += acc_unit_num(player_cframes, C.PROTOSS_UNITS + C.PROTOSS_BUILDINGS)
        # print(player_unit_info)
        player_research_info = get_research_level(player_cframes, player_research_info)

        observation = get_observed_enemy(opponent_cframes, observation, unit_id)
        opponent_info += acc_unit_num(opponent_cframes, PREDICTED_UNITS)
        # print(opponent_info)

        features.append(np.concatenate([frame, res_features, player_unit_info, player_research_info, observation, opponent_info]))
    
    features = np.vstack(features)
    features = pd.DataFrame(features, columns=FEATURE_NAMES)
    
    return features


def extractor(files):
    for f in tqdm(files):
        try:
            df = load_rgd(os.path.join(DATA_PATH, f))
            features = extract_features(df)
            features.to_csv(os.path.join(OUTPUT_PATH, f))
        except Exception as e:
            print(e, "occurred in", f)


if __name__ == "__main__":
    input_files = os.listdir(DATA_PATH)
    dumped_files = os.listdir(OUTPUT_PATH)

    if DEBUG_MODE:
        # input_files = [input_files[random.randint(0, len(input_files))]]
        input_files = ["TL867.csv"]
        extractor(input_files)
    else:
        for dumped_file in dumped_files:
            input_files.remove(dumped_file)

        length = np.ceil((len(input_files) / NUM_PROCESSES)).astype(np.int)

        input_lists = [input_files[i : i+length] for i in range(0, len(input_files), length)]

        print(list(map(len, input_lists)))
        for i in range(NUM_PROCESSES):
            # print("Create extracting process {}.".format(i))
            p = mp.Process(target=extractor, args=(input_lists[i],))
            p.start()

    


