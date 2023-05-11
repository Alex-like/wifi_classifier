//
// Created by Alex Shchelochkov on 13.07.2022.
//
#pragma once

#include <thread>
#include <cstdint>
#include <string>
#include <algorithm>
#include <unordered_set>

namespace WiFiClassifier { namespace global_vars {
    static const size_t packetsAmountThreshold = 200;
    static const size_t n_features = 27;
    static const std::string modelDataPath = "/data_raw/features.csv";
    static const std::string modelParamsPath = "/models/rf.log";
    static const std::string probabilityModelPath = "/models/prob.log";
    static const std::string transformerPath = "/models/transformer.log";
    static const std::unordered_set<size_t> skippedFeatures = { 2, 15 };
    static const std::string spModelParamsPath = "/models/rf_sp.log";
    static const std::string tmpDataSource = "/data_raw/data_tmp.log";
    static const size_t extraThreadsAmount = std::max(size_t(1), size_t(std::thread::hardware_concurrency() - 1));
} }