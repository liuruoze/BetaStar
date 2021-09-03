#pragma once

#include "pch.h"
#include "utils.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <MiniDNN.h>

using namespace MiniDNN;


void train(Eigen::MatrixXd X, int num_features, int num_labels, double coef_array[], const std::string model_save_path, int epoch, int batch_size, double lr)
{
	Eigen::MatrixXd features = X.topLeftCorner(X.rows(), num_features).transpose();
	Eigen::MatrixXd labels = X.topRightCorner(X.rows(), num_labels).transpose();

	print_shape(features);
	print_shape(labels);

	Eigen::Map<Eigen::MatrixXd> coef(coef_array, 99, 1);

	normalize_features(features, coef);

	Network net;
	
	Layer* input_layer = new FullyConnected<ReLU>(num_features, 256);
	Layer* layer1 = new FullyConnected<ReLU>(256, 256);
	//Layer* layer3 = new FullyConnected<ReLU>(256, 256);
	Layer* output_layer = new FullyConnected<Identity>(256, num_labels);

	net.add_layer(input_layer);
	net.add_layer(layer1);
	//net.add_layer(layer3);
	net.add_layer(output_layer);

	net.set_output(new RegressionMSE());

	RMSProp opt;
	opt.m_lrate = lr;
	VerboseCallback callback;
	net.set_callback(callback);

	net.init();
	net.fit(opt, features, labels, batch_size, epoch);

	save_model(net, model_save_path);
	load_model(net, model_save_path);

	print_matrix(net.get_parameters());

	show_predictions(net, features, labels, 20);	

	return;
}

void print_shape(Eigen::MatrixXd X)
{
	std::cout << '(' << X.rows() << ", " << X.cols() << ')' << std::endl;
}

void print_matrix(const std::vector<std::vector<Scalar>> & m, const std::size_t block)
{
	for (std::size_t i = 0; i < m.size(); i++) {
		for (std::size_t j = 0; j < m[i].size(); j++) {
			if (j < block)
				std::cout << m[i][j] << " ";
			else
				continue;
		}
		std::cout << std::endl;
		if (i >= block)
			break;
	}
}

void show_predictions(Network & net, Eigen::MatrixXd & features, Eigen::MatrixXd & labels, const int num)
{
	for (int i = 0; i < 10; i++) {
		std::cout << "Prediction:   " << net.predict(features.col(i)).transpose().cast<int>() << '\n'
				  << "Ground Truth: " << labels.col(i).transpose() << '\n'
				  << "----------------------------------------" << std::endl;
	}
}

void normalize_features(Eigen::MatrixXd & features, const Eigen::MatrixXd coef)
{
	if (features.rows() == coef.rows()) { 
		for (int i = 0; i < features.cols(); i++) {
			features.col(i) = features.col(i).cwiseQuotient(coef.col(0));
		}
	}

	return;
}

void save_model(Network & net, const std::string path)
{
	std::vector<std::vector<Scalar>> params = net.get_parameters();
	
	std::ofstream output_file;
	output_file.open(path, std::ios::out);

	std::cout << params.size() << std::endl;

	for (std::size_t i = 0; i < params.size(); i++) {
		for (std::size_t j = 0; j < params[i].size(); j++) {
			output_file << std::to_string(params[i][j]);
			if (j < params[i].size() - 1) {
				output_file << ',';
			}
		}
		output_file << '\n';
		std::cout << params[i].size() << std::endl;
	}

	output_file.close();

	return;
}

void load_model(Network & net, const std::string path)
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
	   
	net.set_parameters(params);

	input_file.close();

	return;
}

Eigen::MatrixXd load_csv(const std::string & path) 
{
	std::ifstream input_file;
	input_file.open(path, std::ios::in);

	std::string line;
	std::vector<double> values;
	std::size_t rows = 0;

	while (std::getline(input_file, line)) {
		std::stringstream line_stream(line);
		std::string cell;
		while (std::getline(line_stream, cell, ',')) {
			values.push_back(std::stod(cell));
		}
		rows++;
	}
	return Eigen::Map<const Eigen::Matrix<Eigen::MatrixXd::Scalar, Eigen::MatrixXd::RowsAtCompileTime, Eigen::MatrixXd::ColsAtCompileTime, Eigen::RowMajor>>(values.data(), rows, values.size() / rows);
}
