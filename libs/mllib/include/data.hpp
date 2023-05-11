//
// Created by Alex Shchelochkov on 31.08.2022.
//
#pragma once

#include <unordered_set>
#include <functional>
#include "utils.hpp"
#include "matrix.hpp"

namespace mllib {
    class Data {
    private:
        size_t samples_amount;
        size_t features_amount;
        size_t *targets = nullptr;
        Matrix<double> features;
    public:
        Data();
        Data(size_t samples_amount, size_t features_amount);
        explicit Data(const std::vector<std::vector<double>> &v);
        Data(size_t samples_amount, size_t features_amount, Matrix<double> &features, size_t *targets);
        Data(Data &&d) noexcept;
        Data& operator=(Data &&d) noexcept;
        ~Data();
        double get_feature(size_t row, size_t column) const;
        size_t get_target(size_t row) const;
        size_t samples_size() const;
        size_t features_size() const;
        void read(const std::string &path, bool has_header = true, size_t label_idx = 0);
        void read_with_skip_features(const std::string &path, bool has_header = true, size_t label_idx = 0,
                                     const std::unordered_set<size_t> &skip_ids = {});
        std::vector<size_t> generate_samples(size_t cnt, std::mt19937_64 &eng) const;
        std::vector<size_t> generate_balanced_samples(double part, std::mt19937_64 &eng) const;
        std::vector<size_t> get_difference(const std::vector<size_t> &ids) const;
        std::vector<size_t> generate_features(std::function<size_t(size_t)> &get_cnt, std::mt19937_64 &eng) const;
        std::string features_to_string();
    };

    Data get_data_by_ids(const Data &data, const std::vector<size_t> &ids);
    std::vector<size_t> get_targets_by_ids(const Data &data, const std::vector<size_t> &ids);
    void write(Data &data, const std::string &path);
}