#include "PredictModel.h"

// by Haifeng Guo, 2019-07-13

using namespace MiniDNN;


PredictModel::PredictModel()
{
}

PredictModel::PredictModel(BWAPI::Race race)
{
	int input_size = 0;
	int output_size = 0;
	if (race == BWAPI::Races::Protoss) {
		input_size = 93;
		output_size = 14;
	}
	else if (race == BWAPI::Races::Terran) {
		input_size = 97;
		output_size = 13;
	}
	else if (race == BWAPI::Races::Zerg) {
		input_size = 93;
		output_size = 12;
	}

	Layer* input_layer = new FullyConnected<ReLU>(input_size, 256);
	Layer* h_layer1 = new FullyConnected<ReLU>(256, 256);
	Layer* output_layer = new FullyConnected<ReLU>(256, output_size);

	net.add_layer(input_layer);
	net.add_layer(h_layer1);
	net.add_layer(output_layer);
	net.set_output(new RegressionMSE());
	net.init();

	set_enemy_race(race);
	
	if (race == BWAPI::Races::Protoss) {
		load_model("bwapi-data/AI/pvp_params.csv");
	}
	else if (race == BWAPI::Races::Terran) {
		load_model("bwapi-data/AI/pvt_params.csv");
	}
	else if (race == BWAPI::Races::Zerg) {
		load_model("bwapi-data/AI/pvz_params.csv");
	}
}

PredictModel & PredictModel::Instance(BWAPI::Race enemy_race)
{

	static PredictModel instance(enemy_race);
	return instance;
}

void PredictModel::load_model(const std::string path)
{
	std::vector<std::vector<Scalar>> params;

	std::ifstream input_file;
	input_file.open(path, std::ios::in);

	std::string line;

	while (std::getline(input_file, line)) {
		std::vector<double> values;
		std::stringstream line_stream(line);
		std::string cell;

		while (std::getline(line_stream, cell, ',')) {
			values.push_back(std::stod(cell));
		}
		params.push_back(values);
	}
	std::cout << params[0].size() << std::endl;
	net.set_parameters(params);
	input_file.close();

	return;
}

void PredictModel::normalize_features(Eigen::MatrixXd & features)
{
	// normalization parameters extracted from the dataset (.rgd files).

	double terran_coef[] = { 64800, 31394, 11222, 95166, 37534, 400, 400, 56, 57, 110, 17, 15, 32, 23, 22, 16,
						 	 37, 35, 24, 40, 9, 22, 44, 10, 40, 5, 3, 45, 8, 13, 3, 2, 2, 2, 2, 20, 2, 3, 3,
							 3, 3, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, 12, 61,
							 45, 14, 94, 50, 52, 99, 15, 11, 3, 11, 5, 49, 7, 2, 4, 11, 6, 9, 7, 17, 72, 6, 2,
						 	 7, 23, 10, 8, 2, 10, 4, 1, 118, 74, 14, 10, 15, 60, 66, 59, 11, 52, 8, 12, 21 };

	double zerg_coef[] = { 64800, 41403, 19050, 87646, 35876, 400, 400, 42, 59, 116, 22, 14, 18, 5, 38, 9, 17,
						   69, 26, 29, 11, 8, 34, 8, 19, 4, 3, 66, 6, 10, 3, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 1,
						   1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 11, 80, 92, 26,
						   30, 142, 12, 28, 15, 46, 38, 19, 29, 8, 2, 4, 8, 1, 18, 2, 3, 3, 4, 2, 2, 2, 16, 24,
						   2, 146, 56, 196, 104, 33, 32, 32, 16, 58, 20, 47, 11 };

	double protoss_coef[] = { 64800, 15456, 7514, 90760, 35086, 414, 400, 41, 53, 96, 16, 11, 22, 18, 15, 9, 16,
							  58, 36, 18, 11, 10, 42, 9, 28, 3, 2, 75, 4, 11, 2, 1, 2, 2, 2, 6, 1, 3, 3, 3, 3, 1,
							  1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 37, 44, 93, 9, 9, 32,
							  10, 6, 8, 4, 26, 11, 16, 8, 8, 28, 8, 19, 3, 3, 50, 2, 6, 1, 1, 1, 2, 1, 2, 1, 44,
							  53, 105, 11, 10, 32, 13, 6, 10, 4, 69, 15, 24, 12 };

	double* coef_array = nullptr;
	int shape = 0;

	if (enemy_race == BWAPI::Races::Terran) {
		coef_array = terran_coef;
		shape = 97;
	}
	else if (enemy_race == BWAPI::Races::Zerg) {
		coef_array = zerg_coef;
		shape = 93;
	}
	else if (enemy_race == BWAPI::Races::Protoss) {
		coef_array = protoss_coef;
		shape = 93;
	}

	Eigen::Map<Eigen::MatrixXd> coef(coef_array, shape, 1);

	//UAB_ASSERT(features.rows() == coef.rows(), "ERROR: Invalid feature dimensions.");

	for (auto i = 0; i < features.cols(); i++) {
		features.col(i) = features.col(i).cwiseQuotient(coef.col(0));
	}

	return;
}

std::vector<int> PredictModel::get_predictions(std::vector<double> features)
{
	Eigen::MatrixXd feature_matrix = Eigen::Map<Eigen::MatrixXd>(features.data(), features.size(), 1);
	normalize_features(feature_matrix);
	Eigen::MatrixXd prediction_matrix = net.predict(feature_matrix);

	std::vector<int> predictions;

	for (auto i = 0; i < prediction_matrix.size(); i++) {
		double pred = prediction_matrix(i, 0) > 0 ? std::round(prediction_matrix(i, 0)) : 0.0;

		predictions.push_back(static_cast<int>(pred));
	}

	return predictions;
}

int PredictModel::get_upgrade_feature(BWAPI::UpgradeType upgrade)
{
	int upgrade_level = BWAPI::Broodwar->self()->getUpgradeLevel(upgrade);
	if (BWAPI::Broodwar->self()->isUpgrading(upgrade)) {
		upgrade_level++;
	}
	return upgrade_level;
}

int PredictModel::get_research_feature(BWAPI::TechType research)
{	
	return (BWAPI::Broodwar->self()->hasResearched(research) || BWAPI::Broodwar->self()->isResearching(research));
}

void PredictModel::set_enemy_race(BWAPI::Race race)
{
	this->enemy_race = race;
}
