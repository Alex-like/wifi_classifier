//
// Created by Alex Shchelochkov on 31.08.2022.
//

#include "../include/data.hpp"

using namespace std;

mllib::Data::Data() {
    this->samples_amount = 0;
    this->features_amount = 0;
}

mllib::Data::Data(size_t samples_amount, size_t features_amount) {
    this->samples_amount = samples_amount;
    this->features_amount = features_amount;
}

mllib::Data::Data(const std::vector<std::vector<double>> &v) {
    this->features = Matrix<double>(v);
    this->samples_amount = features.size().first;
    this->features_amount = features.size().second;
    this->targets = new size_t[samples_amount];
    std::fill(this->targets, this->targets+samples_amount, 0);
}

mllib::Data::Data(size_t samples_amount, size_t features_amount, Matrix<double> &features, size_t *targets) {
    this->samples_amount = samples_amount;
    this->features_amount = features_amount;
    this->features = std::move(features);
    this->targets = targets;
}

mllib::Data::Data(Data &&d) noexcept :
samples_amount(std::move(d.samples_amount)),
features_amount(std::move(d.features_amount)),
targets(std::move(d.targets)),
features(std::move(d.features))
{
    d.samples_amount = 0;
    d.features_amount = 0;
    d.targets = nullptr;
    d.features = Matrix<double>();
}

mllib::Data& mllib::Data::operator=(Data &&d) noexcept {
    this->samples_amount = std::move(d.samples_amount);
    this->features_amount = std::move(d.features_amount);
    this->targets = std::move(d.targets);
    this->features = std::move(d.features);
    d.samples_amount = 0;
    d.features_amount = 0;
    d.targets = nullptr;
    d.features = Matrix<double>();
    return *this;
}

mllib::Data::~Data() {
    delete[] targets;
}

double mllib::Data::get_feature(size_t row, size_t column) const {
    return *features.at(row, column);
}

size_t mllib::Data::get_target(size_t row) const {
    return targets[row];
}

size_t mllib::Data::samples_size() const {
    return samples_amount;
}

size_t mllib::Data::features_size() const {
    return features_amount;
}

void mllib::Data::read(const string &path, bool has_header, size_t label_idx) {
    read_with_skip_features(path, has_header, label_idx, {});
}

void mllib::Data::read_with_skip_features(const string &path, bool has_header, size_t label_idx,
                                          const unordered_set<size_t> &skip_ids) {
    samples_amount = utils::count_lines(path);
    if (has_header)
        samples_amount--;
    features_amount = utils::count_features(path) - 1 - skip_ids.size();
    targets = new size_t[samples_amount];
    features = Matrix<double>(samples_amount, features_amount);
    ifstream in(path);
    if (in.is_open()) {
        string line, feature;
        size_t i = 0, j, real_j;
        while (getline(in, line)) {
            if (has_header) {
                has_header = false;
                continue;
            }
            stringstream row_stream(line);
            j = 0, real_j = 0;
            bool has_target = true;
            while (getline(row_stream, feature, ',')) {
                if (has_target && j == label_idx) {
                    targets[i] = stoul(feature);
                    has_target = false;
                } else {
                    if (skip_ids.count(j++) > 0) continue;
                    features.set(i, real_j, stod(feature));
                    real_j++;
                }
            }
            i++;
        }
    }
    in.close();
}

vector<size_t> mllib::Data::generate_samples(size_t cnt, mt19937_64 &eng) const {
    unordered_set<size_t> tmp;
    uniform_int_distribution<size_t> distribution;
    while (cnt > 0) {
        size_t x = distribution(eng) % samples_amount;
        auto res = tmp.insert(x);
        if (res.second) cnt--;
    }
    return {tmp.begin(), tmp.end()};
}

vector<size_t> mllib::Data::generate_balanced_samples(double part, mt19937_64 &eng) const {
    part = min(1., part);
    unordered_set<size_t> tmp;
    map<size_t, vector<size_t>> target_ids;
    uniform_int_distribution<size_t> distribution;
    for (size_t i = 0; i < samples_amount; i++)
        target_ids[targets[i]].emplace_back(i);
    size_t n = samples_amount;
    for (const auto &p : target_ids)
        n = min(n, p.second.size());
    n = size_t(double(n) * part);
    for (const auto &p : target_ids) {
        size_t cnt = n, size = p.second.size();
        while (cnt > 0) {
            size_t x = distribution(eng) % size;
            auto res = tmp.insert(p.second[x]);
            if (res.second) cnt--;
        }
    }
    return {tmp.begin(), tmp.end()};
}

vector<size_t> mllib::Data::get_difference(const vector<size_t> &ids) const {
    unordered_set<size_t> tmp;
    unordered_set<size_t> already_exist(ids.begin(), ids.end());
    for (size_t i = 0; i < samples_amount; i++) {
        if (already_exist.find(i) != already_exist.end())
            continue;
        tmp.insert(i);
        if (tmp.size() + already_exist.size() == samples_amount)
            break;
    }
    return {tmp.begin(), tmp.end()};
}

vector<size_t> mllib::Data::generate_features(function<size_t(size_t)> &get_cnt, mt19937_64 &eng) const {
    size_t cnt = get_cnt(features_amount);
    unordered_set<size_t> tmp;
    uniform_int_distribution<size_t> distribution;
    while (cnt > 0) {
        size_t x = distribution(eng) % features_amount;
        auto res = tmp.insert(x);
        if (res.second) cnt--;
    }
    return {tmp.begin(), tmp.end()};
}

string mllib::Data::features_to_string() {
    return features.to_string();
}

mllib::Data mllib::get_data_by_ids(const mllib::Data &data, const vector<size_t> &ids) {
    size_t rows = ids.size(), columns = data.features_size();
    mllib::Matrix<double> m(rows, columns);
    auto *targets = new size_t[rows];
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++)
            m.set(i, j, data.get_feature(ids[i], j));
        targets[i] = data.get_target(ids[i]);
    }
    return {rows, columns, m, targets};
}

vector<size_t> mllib::get_targets_by_ids(const mllib::Data &data, const vector<size_t> &ids) {
    vector<size_t> targets;
    targets.reserve(ids.size());
    for (auto &id : ids)
        targets.emplace_back(data.get_target(id));
    return targets;
}

void mllib::write(mllib::Data &data, const string &path) {
    ofstream out(path, ios::out);
    if (out.is_open()) {
        out << "label";
        for (size_t j = 0; j < data.features_size(); j++) {
            out << ',' << j + 1;
        }
        out << '\n';
        for (size_t i = 0; i < data.samples_size(); i++) {
            out << data.get_target(i);
            for (size_t j = 0; j < data.features_size(); j++) {
                out << ',' << data.get_feature(i, j);
            }
            out << '\n';
        }
    }
    out.close();
}