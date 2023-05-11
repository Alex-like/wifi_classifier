//
// Created by Alex Shchelochkov on 05.04.2023.
//

#include "../include/probability.hpp"

using namespace std;
using namespace mllib;

models::probability_model_t::probability_model_t(size_t states_amount, size_t observations_amount) :
states_amount(states_amount),
observations_amount(observations_amount)
{
    this->p.assign(states_amount, 0);
    this->a = Matrix<double>(states_amount, states_amount);
    this->b = Matrix<double>(states_amount, observations_amount);
}

void models::probability_model_t::fit(const std::vector<std::vector<size_t>> &O, const std::vector<std::vector<size_t>> &S) {
//    if (S.empty()) baum_welch(O);
    for (const auto& v : S)
        p[v[0]]++;
    for_each(p.begin(), p.end(), [n=S.size()](double& x){ x/=n; });

    vector<uint64_t> y(states_amount);
    for (const auto& v : S) {
        for (size_t i = 0; i+1 < v.size(); i++) {
            a(v[i], v[i + 1])++;
            y[v[i]]++;
        }
        y[v.back()]++;
    }
    for (size_t i = 0; i < states_amount; i++)
        for (size_t j = 0; j < states_amount; j++)
            a(i, j) /= y[i] > 0 ? y[i] : 1;

    for (size_t i = 0; i < O.size(); i++)
        for (size_t t = 0; t < O[i].size(); t++)
            b(S[i][t], O[i][t])++;
    for (size_t i = 0; i < states_amount; i++)
        for (size_t k = 0; k < observations_amount; k++)
            b(i, k) /= y[i] > 0 ? y[i] : 1;
}
// TODO: Need optimisation, Need special double for extra low exponent
size_t models::probability_model_t::predict_state(const vector<size_t> &O) const {
    size_t T = O.size();
    vector<vector<double>> alpha(T, vector<double>(states_amount, 0)),
    beta(T, vector<double>(states_amount, 0)),
    gamma(T, vector<double>(states_amount, 0));
    for (size_t i = 0; i < states_amount; i++) {
        alpha[0][i] = p[i] * b(i, O[0]);
        beta[T-1][i] = 1;
    }
    for (size_t t = 1; t < T; t++)
        for (size_t j = 0; j < states_amount; j++) {
            double sum = 0;
            for (size_t i = 0; i < states_amount; i++)
                sum += alpha[t - 1][i] * a(i, j);
            alpha[t][j] = b(j, O[t]) * sum;
        }
    double P = accumulate(alpha[T-1].begin(), alpha[T-1].end(), 0.0);
    for (size_t t = T-1; t > 0; t--)
        for (size_t i = 0; i < states_amount; i++)
            for (size_t j = 0; j < states_amount; j++)
                beta[t-1][i] += beta[t][j] * a(i, j) * b(j, O[t-1]);
    for (size_t t = 0; t < T; t++)
        for (size_t i = 0; i < states_amount; i++)
            gamma[t][i] = alpha[t][i] * beta[t][i] / P;
    return utils::arg_max(gamma.back());
}

void models::probability_model_t::save(const string &path) const {
    ofstream out(path, ios::out);
    if (out.is_open()) {
        // Header
        out << "states=" << states_amount << " observations=" << observations_amount << '\n';
        // ∏
        out << "p\n";
        for (const auto& x : p)
            out << x << ' ';
        out << '\n';
        // A
        out << "a\n" << a.to_string();
        // B
        out << "b\n" << b.to_string();
    }
    out.close();
}

void models::probability_model_t::load(const string &path) {
    const regex regex_states("states=(\\d+)");
    const regex regex_observations("observations=(\\d+)");

    bool header = false, reading_p = false, reading_a = false, reading_b = false;
    size_t cnt = 0;

    ifstream in(path);
    if (in.is_open()) {
        string line;
        while (getline(in, line, '\n')) {
            // skip empty string
            smatch matcher_1, matcher_2;
            if (regex_match(line, matcher_1, regex("^\\s*$"))) continue;
            // check header
            if (!header
            && regex_search(line, matcher_1, regex_states)
            && regex_search(line, matcher_2, regex_observations)) {
                this->states_amount = stoull(matcher_1[1].str());
                this->observations_amount = stoull(matcher_2[1].str());
                this->p.clear();
                this->p.reserve(states_amount);
                this->a = Matrix<double>(states_amount, states_amount);
                this->b = Matrix<double>(states_amount, observations_amount);
                header = true;
                continue;
            }
            line = utils::trim(line);
            // check p
            if (line == "p") {
                reading_p = true;
                continue;
            }
            if (reading_p && !reading_a && !reading_b) {
                for (const auto& s : utils::split(line, " "))
                    p.emplace_back(stod(s));
                reading_p = false;
                continue;
            }
            // check a
            if (line == "a") {
                reading_a = true;
                continue;
            }
            if (!reading_p && reading_a && !reading_b) {
                size_t i = 0;
                for (const auto& s : utils::split(line, " "))
                    a(cnt, i++) = stod(s);
                cnt++;
                if (cnt >= states_amount) {
                    reading_a = false;
                    cnt = 0;
                }
                continue;
            }
            // check b
            if (line == "b") {
                reading_b = true;
                continue;
            }
            if (!reading_p && !reading_a && reading_b) {
                size_t i = 0;
                for (const auto& s : utils::split(line, " "))
                    b(cnt, i++) = stod(s);
                cnt++;
                if (cnt >= states_amount) {
                    reading_b = false;
                    cnt = 0;
                }
                continue;
            }
        }
    }
    in.close();
}

