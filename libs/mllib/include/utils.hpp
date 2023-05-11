//
// Created by Alex Shchelochkov on 29.08.2022.
//
#pragma once

#include <map>
#include <random>
#include <sstream>
#include <fstream>
#include <string>
#include <regex>
#include <functional>
#include <algorithm>

namespace mllib { namespace utils {
    float fpow(float base, int exp);
    float calc_median(float xs[], size_t size, std::function<bool(float, float)> cmp);
    uint64_t count_lines(const std::string &path);
    uint64_t count_features(const std::string &path);
    std::vector<std::string> split (std::string s, std::string delimiter = ", ");
    std::string ltrim(const std::string &s);
    std::string rtrim(const std::string &s);
    std::string trim(const std::string &s);
    template<typename T, typename E>
    extern std::string map_to_str(std::map<T, E> &dict);
    template<typename T, typename E>
    extern std::map<T, E> str_to_map(const std::string &dict,
                                     std::function<T(std::string)> parse_T,
                                     std::function<E(std::string)> parse_E);
    template <typename T, typename A>
    extern size_t arg_max(const std::vector<T, A> &vec);
    template <typename T, typename A>
    extern size_t arg_min(const std::vector<T, A> &vec);
} }

template <typename T, typename A>
size_t  mllib::utils::arg_max(const std::vector<T, A> &vec) {
    return static_cast<size_t>(std::distance(vec.begin(), max_element(vec.begin(), vec.end())));
}

template <typename T, typename A>
size_t mllib::utils::arg_min(const std::vector<T, A>& vec) {
    return static_cast<size_t>(std::distance(vec.begin(), min_element(vec.begin(), vec.end())));
}

template<typename T, typename E>
std::string mllib::utils::map_to_str(std::map<T, E> &dict) {
    std::stringstream ss;
    size_t n = 0;
    ss << "{";
    for (const auto &p : dict)
        ss << p.first << ':' << p.second << (++n < dict.size() ? ", " : "");
    ss << "}";
    return ss.str();
}

template<typename T, typename E>
std::map<T, E> mllib::utils::str_to_map(const std::string &dict,
                          std::function<T(std::string)> parse_T,
                          std::function<E(std::string)> parse_E) {
    std::map<T, E> res;
    std::regex regex_brackets("^\\{(.*)\\}$");
    std::regex regex_any("(.+):(.+)");
    std::smatch match;
    std::string new_dict = dict;
    if (std::regex_search(dict, match, regex_brackets))
        new_dict = match[1].str();
    for (auto &s: split(new_dict)) {
        if (std::regex_search(s, match, regex_any)) {
            T key = parse_T(match[1].str());
            E val = parse_E(match[2].str());
            res[key] = val;
        }
    }
    return res;
}