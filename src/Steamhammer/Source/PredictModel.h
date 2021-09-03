#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <MiniDNN.h>
#include <BWAPI.h>

// by Haifeng Guo, 2019-07-13

using namespace MiniDNN;

class PredictModel
{
	BWAPI::Race enemy_race;

	Network net;

	double* coef_array;

public:
	void normalize_features(Eigen::MatrixXd & features);

	PredictModel();

	PredictModel(BWAPI::Race enemy_race);

	static PredictModel & Instance(BWAPI::Race enemy_race);

	void load_model(const std::string path);

	std::vector<int> get_predictions(std::vector<double> features);

	int get_upgrade_feature(BWAPI::UpgradeType upgrade);

	int get_research_feature(BWAPI::TechType research);

	void set_enemy_race(BWAPI::Race race);
};