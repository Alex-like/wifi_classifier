//
// Created by Alex Shchelochkov on 02.09.2022.
//

#include "../include/metrics.hpp"

using namespace std;

mllib::Matrix<size_t> mllib::metrics::build_confusion_matrix(const vector<size_t> &y_true, const vector<size_t> &y_pred) {
    map<size_t, size_t> items;
    size_t n = 0;
    for (auto x : y_true)
        if (items.find(x) == items.end())
            items[x] = n++;
    Matrix<size_t>CM(n, n);
    for (size_t i = 0; i < y_pred.size(); i++)
        CM.set(items[y_true[i]], items[y_pred[i]], 1 + *CM.at(items[y_true[i]], items[y_pred[i]]));
    return CM;
}

double mllib::metrics::precision_score(const vector<size_t>& y_true, const vector<size_t>& y_pred, const bool weighted) {
    Matrix<size_t> CM = build_confusion_matrix(y_true, y_pred);
    size_t n = CM.size().first;
    double precision_score = 0.0, all = 0.0;
    vector<size_t> c(n), p(n);
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)  {
            all += double(*CM.at(i, j));
            c[j] += *CM.at(i, j);
            p[i] += *CM.at(i, j);
        }
    for (int i = 0; i < n; i++) {
        double tp = double(*CM.at(i, i));
        double fp = double(c[i]) - double(*CM.at(i, i));
        precision_score += double(weighted ? p[i] : 1) * (tp + fp != 0 ? double(tp) / (tp + fp) : 0);
    }
    return precision_score / double(n);
}

double mllib::metrics::recall_score(const vector<size_t> &y_true, const vector<size_t> &y_pred, const bool weighted) {
    Matrix<size_t> CM = build_confusion_matrix(y_true, y_pred);
    size_t n = CM.size().first;
    double recall_score = 0.0, all = 0.0;
    vector<size_t> p(n);
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++) {
            all += double(*CM.at(i, j));
            p[i] += *CM.at(i, j);
        }
    for (int i = 0; i < n; i++) {
        double tp = double(*CM.at(i, i));
        double fn = double(p[i]) - double(*CM.at(i, i));
        recall_score += double(weighted ? p[i] : 1) * (tp + fn != 0 ? double(tp) / (tp + fn) : 0);
    }
    return recall_score / (weighted ? all : double(n));
}

double mllib::metrics::f1_score(const vector<size_t> &y_true, const vector<size_t> &y_pred, const bool weighted) {
    Matrix<size_t> CM = build_confusion_matrix(y_true, y_pred);
    size_t n = CM.size().first;
    double f1_score = 0.0, all = 0.0;
    vector<size_t> p(n), c(n);
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++) {
            all += double(*CM.at(i, j));
            p[i] += *CM.at(i, j);
            c[j] += *CM.at(i, j);
        }
    for (int i = 0; i < n; i++) {
        double tp = double(*CM.at(i, i));
        double fn = double(p[i]) - double(*CM.at(i, i));
        double fp = double(c[i]) - double(*CM.at(i, i));
        double precision = tp + fp != 0 ? double(tp) / (tp + fp) : 0;
        double recall = tp + fn != 0 ? double(tp) / (tp + fn) : 0;
        if (precision + recall != 0)
            f1_score += double(weighted ? p[i] : 1) * 2 * precision * recall / (precision + recall);
    }
    return f1_score / (weighted ? all : double(n));
}