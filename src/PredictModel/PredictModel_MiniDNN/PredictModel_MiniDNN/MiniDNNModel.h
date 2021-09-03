#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <MiniDNN.h>

using namespace MiniDNN;

class MiniDNNModel
{
private:

	std::vector<std::vector<Scalar>> model_weight;

	Network net;
	
public:
	MiniDNNModel();
	
	MiniDNNModel(std::string path);

	void load_model(const std::string path);

	void normalize_features(Eigen::MatrixXd & features);

	std::vector<int> get_predictions(std::vector<double> features);	
};