#include "pch.h"
#include "MiniDNNModel.h"

using namespace MiniDNN;

MiniDNNModel::MiniDNNModel()
{
}

MiniDNNModel::MiniDNNModel(std::string path)
{
	Layer* input_layer = new FullyConnected<ReLU>(99, 256);
	Layer* h_layer1 = new FullyConnected<ReLU>(256, 256);
	Layer* output_layer = new FullyConnected<ReLU>(256, 13);

	net.add_layer(input_layer);
	net.add_layer(h_layer1);
	net.add_layer(output_layer);
	net.set_output(new RegressionMSE());
	net.init();

	load_model(path);
}

void MiniDNNModel::load_model(const std::string path)
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
	model_weight = params;
	net.set_parameters(params);
	input_file.close();

	return;	
}

void MiniDNNModel::normalize_features(Eigen::MatrixXd & features)
{
	double coef_array[] = { 64800, 31394, 11308, 95086, 37494, 400, 400, 56, 57, 110,
							17, 15, 33, 23, 22, 16, 37, 165, 35, 24, 31, 9, 2, 22, 44,
							10, 40, 5, 3, 45, 8, 13, 3, 2, 2, 2, 2, 20, 2, 3, 3, 3, 3,
							1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
							1, 1, 8, 12, 60, 48, 14, 95, 49, 52, 99, 15, 11, 2, 11, 5,
							46, 6, 2, 4, 11, 6, 9, 7, 18, 72, 6, 2, 7, 23, 10, 8, 2, 10, 4, 1 };
	Eigen::Map<Eigen::MatrixXd> coef(coef_array, 99, 1);

	assert(features.rows() == coef.rows());

	for (auto i = 0; i < features.cols(); i++) {
		features.col(i) = features.col(i).cwiseQuotient(coef.col(0));
	}

	return;
}

std::vector<int> MiniDNNModel::get_predictions(std::vector<double> features)
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
