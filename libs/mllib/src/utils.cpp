//
// Created by Alex Shchelochkov on 27.09.2022.
//

#include "../include/utils.hpp"

using namespace std;

float mllib::utils::fpow(float base, int exp) {
    if (exp == 0) return 1.0;
    if (exp == 1) return base;
    float tmp = fpow(base, exp / 2);
    tmp *= tmp;
    return exp % 2 != 0 ? tmp * base : tmp;
}

float mllib::utils::calc_median(float xs[], size_t size, function<bool(float, float)> cmp) {
    sort(xs, xs + size, move(cmp));
    return size % 2 == 0 ? xs[size/ 2 + 1] : xs[(size + 1)/ 2];
}

uint64_t mllib::utils::count_lines(const string &path) {
    uint64_t cnt = 0;
    ifstream in(path);
    if (in.is_open()) {
        string line;
        while (getline(in, line))
            cnt++;
    }
    in.close();
    return cnt;
}

uint64_t mllib::utils::count_features(const string &path) {
    uint64_t cnt = 0;
    ifstream in(path);
    if (in.is_open()) {
        string line, feature;
        if (getline(in, line)) {
            stringstream row_stream(line);
            while (getline(row_stream, feature, ','))
                cnt++;
        }
    }
    in.close();
    return cnt;
}

vector<string> mllib::utils::split (string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;
    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }
    res.push_back(s.substr(pos_start));
    return res;
}

string mllib::utils::ltrim(const string &s) {
    return regex_replace(s, regex("^\\s+"), "");
}

string mllib::utils::rtrim(const string &s) {
    return regex_replace(s, regex("\\s+$"), "");
}

string mllib::utils::trim(const string &s) {
    return ltrim(rtrim(s));
}