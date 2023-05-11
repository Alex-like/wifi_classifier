//
// Created by Alex Shchelochkov on 05.04.2023.
//
#pragma once

#include <cstdint>
#include <numeric>
#include "utils.hpp"
#include "matrix.hpp"

namespace mllib { namespace models {
    class probability_model_t {
    private:
        size_t states_amount;
        size_t observations_amount;
        std::vector<double> p;
        Matrix<double> a;
        Matrix<double> b;
//        void baum_welch(const std::vector<std::vector<size_t>> &O);
    public:
        explicit probability_model_t(size_t states_amount = 0, size_t observations_amount = 0);
        void save(const std::string &path) const;
        void load(const std::string &path);
        void fit(const std::vector<std::vector<size_t>> &O, const std::vector<std::vector<size_t>> &S);
        size_t predict_state(const std::vector<size_t> &O) const;
//        std::vector<double> predict_states(const std::vector<size_t> &O) const;
    };
} }