//
// Created by Alex Shchelochkov on 02.09.2022.
//

#include "../include/cross_validation.hpp"

using namespace std;

void mllib::cross_validation::split(const vector<size_t> &base,
                             size_t left, size_t right,
                             vector<size_t> &left_v, vector<size_t> &right_v) {
    for (size_t i = 0; i < left; i++)
        left_v[i] = base[i];
    for (size_t i = left; i < right; i++)
        right_v[i-left] = base[i];
    for (size_t i = right; i < base.size(); i++)
        left_v[i - (right - left)] = base[i];
}

tuple<double, mllib::models::Estimator *> mllib::cross_validation::get_best_model_after_cross_val_score(
        models::Estimator &estimator, Data &data,
        const string& scoring, size_t cv, size_t n_jobs) {
    vector<double> results;
    results.reserve(cv);
    vector<size_t> samples;
    size_t n = data.samples_size();
    random_device rd;
    auto eng = mt19937_64(rd());
    samples = data.generate_samples(n, eng);
    shuffle(samples.begin(), samples.end(), eng);
    size_t step = n / cv + (n % cv ? 1 : 0);
    vector<models::Estimator *> models(cv);
    for (auto &x : models)
        x = estimator.clone();
    double best_score = 0.0;
    models::Estimator *res = nullptr;
    {
        ThreadPool pool(n_jobs);
        for (size_t i = 0; i < cv; i++) {
            pool.add_job([&, i] {
                size_t n = data.samples_size();
                size_t l = step * i, r = min(n, (i + 1) * step);
                if (r <= l) r = n;
                vector<size_t> left(n - (r - l)), right(r - l);
                cross_validation::split(samples, l, r, left, right);
                Data train = get_data_by_ids(data, left);
                models[i]->fit(train);
                Data test = get_data_by_ids(data, right);
                vector<size_t> y_true = get_targets_by_ids(data, right), y_pred(r - l);
                models[i]->predict(test, y_pred);
                double score;
                if (scoring == "precision")
                    score = metrics::precision_score(y_true, y_pred);
                else if (scoring == "precision_weighted")
                    score = metrics::precision_score(y_true, y_pred, true);
                else if (scoring == "recall")
                    score = metrics::recall_score(y_true, y_pred);
                else if (scoring == "recall_weighted")
                    score = metrics::recall_score(y_true, y_pred, true);
                else if (scoring == "f1_weighted")
                    score = metrics::f1_score(y_true, y_pred, true);
                else
                    score = metrics::f1_score(y_true, y_pred);
                results.emplace_back(score);
                if (score > best_score) {
                    best_score = score;
                    res = std::move(models[i]);
                }
            });
        }
    }

    double sum = accumulate(results.begin(), results.end(), 0.0);
    return {sum / double(cv), std::move(res)};
}

double mllib::cross_validation::cross_val_score (models::Estimator &estimator, Data &data,
                                          const string& scoring, size_t cv, size_t n_jobs) {
    return get<0>(get_best_model_after_cross_val_score(estimator, data, scoring, cv, n_jobs));
}