//
// Created by Alex Shchelochkov on 30.11.2022.
//

#include "../include/model_checking.hpp"

using namespace std;

double mllib::model_checking::model_check(mllib::models::Estimator &estimator, mllib::Data &data, float test_part) {
    Data train, test;
    random_device rd;
    auto eng = mt19937_64(rd());
    size_t n = data.samples_size();
    vector<size_t> shuffled_ids = data.generate_samples(n, eng);
    shuffle(shuffled_ids.begin(), shuffled_ids.end(), eng);
    auto step = size_t(test_part * float(n));
    size_t cv = n / step;
    vector<double> scores;

    for (size_t i = 0; i < cv; i++) {
        size_t l = step * i, r = min(n, (i + 1) * step);
        if (r <= l) r = n;
        vector<size_t> left(n - (r - l)), right(r - l);
        cross_validation::split(shuffled_ids, l, r, left, right);
        train = get_data_by_ids(data, left);
        test = get_data_by_ids(data, right);
        mllib::models::Estimator * model;
        std::tie(std::ignore, model) = cross_validation::get_best_model_after_cross_val_score(
                estimator, train, "f1", 7, 1);

        vector<size_t> y_true = get_targets_by_ids(data, right), y_pred(r - l);
        model->predict(test, y_pred);

        double f_score = metrics::f1_score(y_true, y_pred);
//        double recall = metrics::recall_score(y_true, y_pred);
//        double precision = metrics::precision_score(y_true, y_pred);

        scores.emplace_back(f_score);
    }
    return accumulate(scores.begin(), scores.end(), 0.0) / double(scores.size());
}