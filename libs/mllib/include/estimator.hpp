//
// Created by Alex Shchelochkov on 29.08.2022.
//
#pragma once

#include <map>
#include "matrix.hpp"
#include "data.hpp"

namespace mllib { namespace models {
    class Estimator {
    public:
        virtual void save(const std::string &path, std::ios_base::openmode mode) = 0;
        virtual void load(const std::string &path) = 0;
        virtual Estimator *clone() = 0;
        virtual void fit(Data &data) = 0;
        virtual void predict(Data &queries, std::vector<size_t> &result) = 0;
        virtual size_t predict(const std::vector<double> &query) = 0;
        virtual void predict_prob(Data &queries, std::vector<std::map<size_t, double>> &probabilities) = 0;
        virtual bool valid() const = 0;
    };
} }