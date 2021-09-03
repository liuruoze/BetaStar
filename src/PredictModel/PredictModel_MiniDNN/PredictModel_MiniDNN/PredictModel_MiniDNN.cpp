#include "pch.h"
#include <iostream>
#include <MiniDNN.h>
#include "utils.h"
#include "MiniDNNModel.h"

using namespace MiniDNN;

const std::string data_path = "./dataset/pvp.csv";
//const std::string model_path = "./model_weight.csv";
const std::string enemy_race = "P";

int main()
{
	Eigen::MatrixXd dataset = load_csv(data_path);

	if (enemy_race == "T") {
		double terran_coef[] = { 64800, 31394, 11222, 95166, 37534, 400, 400, 56, 57, 110, 17, 15, 32, 23, 22, 16,
								 37, 35, 24, 40, 9, 22, 44, 10, 40, 5, 3, 45, 8, 13, 3, 2, 2, 2, 2, 20, 2, 3, 3,
								 3, 3, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, 12, 61,
			                     45, 14, 94, 50, 52, 99, 15, 11, 3, 11, 5, 49, 7, 2, 4, 11, 6, 9, 7, 17, 72, 6, 2,
								 7, 23, 10, 8, 2, 10, 4, 1, 118, 74, 14, 10, 15, 60, 66, 59, 11, 52, 8, 12, 21 };

		train(dataset, 97, 13, terran_coef, "./pvt_params.csv");
	}
	else if (enemy_race == "Z") {
		double zerg_coef[] = { 64800, 41403, 19050, 87646, 35876, 400, 400, 42, 59, 116, 22, 14, 18, 5, 38, 9, 17,
							   69, 26, 29, 11, 8, 34, 8, 19, 4, 3, 66, 6, 10, 3, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 1,
							   1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 11, 80, 92, 26,
							   30, 142, 12, 28, 15, 46, 38, 19, 29, 8, 2, 4, 8, 1, 18, 2, 3, 3, 4, 2, 2, 2, 16, 24,
							   2, 146, 56, 196, 104, 33, 32, 32, 16, 58, 20, 47, 11 };

		train(dataset, 93, 12, zerg_coef, "./pvz_params.csv");
	}
	else
	{
		double protoss_coef[] = { 64800, 15456, 7514, 90760, 35086, 414, 400, 41, 53, 96, 16, 11, 22, 18, 15, 9, 16,
								  58, 36, 18, 11, 10, 42, 9, 28, 3, 2, 75, 4, 11, 2, 1, 2, 2, 2, 6, 1, 3, 3, 3, 3, 1,
								  1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 37, 44, 93, 9, 9, 32,
								  10, 6, 8, 4, 26, 11, 16, 8, 8, 28, 8, 19, 3, 3, 50, 2, 6, 1, 1, 1, 2, 1, 2, 1, 44,
								  53, 105, 11, 10, 32, 13, 6, 10, 4, 69, 15, 24, 12 };

		train(dataset, 93, 14, protoss_coef, "./pvp_params.csv");
	}
	
	
	//MiniDNNModel model("./model_weight.csv");

	//Eigen::MatrixXd feature_matrix = load_csv("./dataset/adf.csv").transpose();
	//print_shape(feature_matrix);

	//int line = 50;
	//std::vector<double> features(feature_matrix.col(line).data(), feature_matrix.col(line).data() + 99);
	//std::vector<double> labels(feature_matrix.col(line).data() + 99, feature_matrix.col(line).data() + 112);

	//for (auto i = 0; i < features.size(); i++) {
	//	std::cout << features[i] << ", ";
	//}
	//std::cout << "\n" << std::endl;

	//std::cout << features.size() << std::endl;
	//for (auto i = 0; i < 13; i++) {
	//	std::cout << *(feature_matrix.data() + 99 + i) << ", ";
	//}
	//std::cout << "\n" << std::endl;

	//std::vector<int> pred = model.get_predictions(features);

	//std::cout << "Predictions:  ";
	//for (std::size_t i = 0; i < pred.size(); i++) {
	//	std::cout << pred[i] << ", ";
	//}
	//std::cout << "\n" << std::endl;

	//std::cout << "Ground Truth: ";
	//for (std::size_t i = 0; i < labels.size(); i++) {
	//	std::cout << labels[i] << ", ";
	//}
	//std::cout << "\n" << std::endl;

	return 0;
}
