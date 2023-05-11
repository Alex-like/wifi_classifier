//
// Created by Alex Shchelochkov on 02.09.2022.
//
#pragma once

#include <vector>
#include <map>
#include "matrix.hpp"

namespace mllib { namespace metrics {
    Matrix<size_t> build_confusion_matrix(const std::vector<size_t> &y_true, const std::vector<size_t> &y_pred);
    double precision_score(const std::vector<size_t> &y_true, const std::vector<size_t> &y_pred, bool weighted = false);
    double recall_score(const std::vector<size_t> &y_true, const std::vector<size_t> &y_pred, bool weighted = false);
    double f1_score(const std::vector<size_t> &y_true, const std::vector<size_t> &y_pred, bool weighted = false);
} }
