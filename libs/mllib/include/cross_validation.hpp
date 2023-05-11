//
// Created by Alex Shchelochkov on 02.09.2022.
//
#pragma once

#include <numeric>

#include "metrics.hpp"
#include "estimator.hpp"
#include "thread_pool.hpp"

namespace mllib { namespace cross_validation {
    std::tuple<double, models::Estimator *> get_best_model_after_cross_val_score(
            models::Estimator &estimator, Data &data,
            const std::string& scoring = "f1", size_t cv = 7, size_t n_jobs = 1);
    double cross_val_score(models::Estimator &estimator, Data &data,
                           const std::string& scoring = "f1", size_t cv = 5, size_t n_jobs = 1);
    void split(const std::vector<size_t> &base,
               size_t left, size_t right,
               std::vector<size_t> &left_v, std::vector<size_t> &right_v);
} }