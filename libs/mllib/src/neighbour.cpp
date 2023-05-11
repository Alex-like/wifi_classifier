//
// Created by Alex Shchelochkov on 06.10.2022.
//

#include "../include/neighbour.hpp"

using namespace std;
using namespace mllib;

const double calculation::PI = 3.14159265358979323846264338327950288;
const double calculation::EPS = 1e-8;

double calculation::_euclidean_dist(const vector<double> &a, const vector <double> &b) {
    double dist = 0;
    for (size_t i = 0; i < a.size(); i++)
        dist += (a[i] - b[i]) * (a[i] - b[i]);
    return sqrt(dist);
}

double calculation::_manhattan_dist(const vector<double> &a, const vector <double> &b) {
    double dist = 0;
    for (int i = 0; i < a.size(); i++)
        dist += abs(a[i] - b[i]);
    return dist;
}

double calculation::_chebyshev_dist(const vector<double> &a, const vector <double> &b) {
    double dist = 0;
    for (int i = 0; i < a.size(); i++)
        dist = max(dist, abs(a[i] - b[i]));
    return dist;
}

double calculation::_uniform_kernel(double x) {
    return abs(x) < 1 ? 0.5 : 0;
}

double calculation::_triangular_kernel(double x) {
    return abs(x) < 1 ? 1 - x : 0;
}

double calculation::_epanechnikov_kernel(double x) {
    return abs(x) < 1 ? 0.75 * (1 - x * x) : 0;
}

double calculation::_quartic_kernel(double x) {
    return abs(x) < 1 ? (15.0 / 16.0) * (1.0 - (x * x)) * (1.0 - (x * x)) : 0;
}

double calculation::_triweight_kernel(double x) {
    return abs(x) < 1 ? (35.0 / 32.0) * (1.0 - (x * x)) * (1.0 - (x * x)) * (1.0 - (x * x)) : 0;
}

double calculation::_tricube_kernel(double x) {
    return abs(x) < 1 ? (70.0 / 81.0) * (1.0 - (x * x * x)) * (1.0 - (x * x * x)) * (1.0 - (x * x * x)) : 0;
}

double calculation::_gaussian_kernel(double x) {
    return exp((-0.5) * x * x) / (sqrt(2 * PI));
}

double calculation::_cosine_kernel(double x) {
    return abs(x) < 1 ? (PI / 4.0) * cos(PI * x / 2.0) : 0;
}

double calculation::_logistic_kernel(double x) {
    return 1.0 / (exp(x) + 2.0 + exp(-x));
}
double calculation::_sigmoid_kernel(double x) {
    return (2.0 / PI) / (exp(x) + exp(-x));
}