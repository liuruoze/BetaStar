#pragma once

#include <iostream>
#include <Eigen/Dense>
#include <fstream>
#include <vector>
#include <string>
#include <MiniDNN.h>

using namespace MiniDNN;

Eigen::MatrixXd load_csv(const std::string & path);

void train(Eigen::MatrixXd X, int num_features, int num_labels, double coef_array[], const std::string model_save_path, int epoch = 20, int batch_size = 256, double lr = 0.005);

void print_shape(Eigen::MatrixXd X);

void normalize_features(Eigen::MatrixXd & features, const Eigen::MatrixXd coef);

void save_model(Network & net, const std::string path);

void load_model(Network & net, const std::string path);

void print_matrix(const std::vector<std::vector<Scalar>> & m, const std::size_t block = 5);

void show_predictions(Network & net, Eigen::MatrixXd & features, Eigen::MatrixXd & labels, const int num);
