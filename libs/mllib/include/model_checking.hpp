//
// Created by Alex Shchelochkov on 30.11.2022.
//
#pragma once

#include "cross_validation.hpp"

namespace mllib { namespace model_checking {
    double model_check(models::Estimator &estimator, Data &data, float test_part = 0.2);
} }