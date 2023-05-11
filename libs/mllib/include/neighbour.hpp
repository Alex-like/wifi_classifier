//
// Created by Alex Shchelochkov on 06.10.2022.
//
#pragma once

#include "estimator.hpp"
#include "thread_pool.hpp"

namespace mllib { namespace calculation {
    extern const double PI;
    extern const double EPS;

    double _euclidean_dist(const std::vector<double> &a, const std::vector <double> &b);
    double _manhattan_dist(const std::vector<double> &a, const std::vector <double> &b);
    double _chebyshev_dist(const std::vector<double> &a, const std::vector <double> &b);
    
    double _uniform_kernel(double x);
    double _triangular_kernel(double x);
    double _epanechnikov_kernel(double x);
    double _quartic_kernel(double x);
    double _triweight_kernel(double x);
    double _tricube_kernel(double x);
    double _gaussian_kernel(double x);
    double _cosine_kernel(double x);
    double _logistic_kernel(double x);
    double _sigmoid_kernel(double x);
} }

namespace mllib { namespace models {
    class KNeighbors : public Estimator {
    public:
        explicit KNeighbors();
        KNeighbors(KNeighbors &&m);
        ~KNeighbors();
        KNeighbors& operator= (KNeighbors &&m);
        void save(const std::string &path, std::ios_base::openmode mode = std::ios::out) override;
        void load(const std::string &path) override;
        KNeighbors *clone() override;
        void fit(Data &data) override;
        void predict(Data &queries, std::vector<size_t> &result) override;
        void predict_prob(Data &queries, std::vector<std::map<size_t, double>> &probabilities) override;
        bool valid() const override;
    };
} }