//
// Created by Alex Shchelochkov on 22.09.2022.
//
#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <memory>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <tuple>
#include <functional>
#include <algorithm>
#include <type_traits>

#include "optional.hpp"

namespace frameslib { namespace utils {
    /**
     * Convert decimal to hexadecimal number.
     *
     * @param dec decimal number.
     *
     * @return hexadecimal number equals `dec`.
     */
    std::string decToHex(uint64_t dec);
    /**
     * Convert hexadecimal number to MAC-address.
     *
     * @param hex hexadecimal number.
     *
     * @return string equals MAC-address constructed from `hex`.
     */
    std::string hexToMAC(std::string hex);
    /**
     * Convert MAC-address to hex number.
     *
     * @param mac string with MAC-address.
     *
     * @return hexadecimal value of input MAC-address.
     */
    uint64_t macToHex(const std::string &mac);
    /**
     * Print all MAC-addresses from file.
     *
     * @param path file path.
     */
    void getAddressesFromFile(const std::string& path);
    /**
     * Fill `macs` with MAC-addresses from string.
     *
     * @param str source string.
     * @param macs set which we want to fill.
     */
    void getAddressesFromStr(const std::string &str, std::set<std::string> &macs);
    /**
     * Convert all symbols in string to lower case.
     *
     * @param s source string.
     *
     * @return transformed string.
     */
    std::string strToLower(std::string s);
    bool checkRetransmission(uint64_t a, uint64_t b, uint64_t c);
    /**
     * Get pair of flags which shows existance of header and body in frames from file.
     *
     * @param path file's path.
     *
     * @return pair with boolean flags of existance.
     */
    std::pair<bool, bool> getFlagsOfExistence(const std::string &path);
    /**
     * Check existing of string by key.
     *
     * @param key specific key.
     * @param dict sorted group of pairs with unique keys and values.
     *
     * @return "True" if `dict` contains `key`else return "False".
     */
    template<typename K, typename C>
    bool checkExist(K key, const C &dict);
    template<typename T>
    std::string vectorToString(const std::vector<T> &vec, const std::string &del = " ");
    template<typename T>
    uint64_t hashValue(const std::vector<T> &vec);
    /**
     * Quick power(base, exp).
     *
     * @param base value.
     * @param exp value, only integer.
     *
     * @return power of 'base' into 'exp'.
     */
    template<typename NumericType>
    NumericType fastPow(NumericType base, int exp);
    template<typename NumericType>
    NumericType calcMedian(std::vector<NumericType> xs,
                           std::function<bool(NumericType, NumericType)> cmp,
                           std::function<NumericType(NumericType)> op = nullptr);
    template<class T>
    std::vector<T> filter(const std::vector<T>& vec, std::function<bool(T)> predicate);
    template <class T>
    class Cache {
    private:
        std::stack<T> cache = {};
    public:
        Cache() = default;
        void push(T &obj);
        void pop();
        T& top();
        bool empty();
    };

} }

template<typename K, typename C>
bool frameslib::utils::checkExist(K key, const C &dict) {
    return dict.find(key) != dict.end();
}

template<typename T>
std::string frameslib::utils::vectorToString(const std::vector<T> &vec, const std::string &del) {
    std::stringstream ss;
    size_t cnt = 0;
    if (!vec.empty())
        for (const auto &el: vec)
            ss << el << (++cnt < vec.size() ? del : "");
    return ss.str();
}

template<typename T>
uint64_t frameslib::utils::hashValue(const std::vector<T> &vec) {
    const uint64_t p = 31;
    const uint64_t m = 1e9 + 7;
    uint64_t power = 1;
    uint64_t hash = 0;
    for (const auto &x: vec) {
        hash = (hash + x * power) % m;
        power = (power * p) % m;
    }
    return hash;
}

template<typename NumericType>
NumericType frameslib::utils::fastPow(NumericType base, int exp) {
    if (exp == 0) return NumericType(1);
    if (exp == 1) return base;
    NumericType tmp = fastPow(base, exp / 2);
    tmp *= tmp;
    return exp % 2 != 0 ? tmp * base : tmp;
}


template<typename NumericType>
NumericType frameslib::utils::calcMedian(std::vector<NumericType> xs,
                                         std::function<bool(NumericType, NumericType)> cmp,
                                         std::function<NumericType(NumericType)> op) {
    if (op != nullptr)
        std::transform(xs.cbegin(), xs.cend(), xs.begin(), op);
    std::sort(xs.begin(), xs.end(), std::move(cmp));
    return xs.size() % 2 == 0 ? (xs[xs.size()/2-1] + xs[xs.size()/2]) / 2 : xs[xs.size() / 2];
}
template<class T>
std::vector<T> frameslib::utils::filter(const std::vector<T>& vec,
                                         std::function<bool(T)> predicate) {
    std::vector<T> result;
    copy_if(begin(vec), end(vec), back_inserter(result), std::move(predicate));
    return result;
}

template<class T>
void frameslib::utils::Cache<T>::push(T &obj) {
    cache.push(obj);
}

template<class T>
void frameslib::utils::Cache<T>::pop() {
    cache.pop();
}

template<class T>
T& frameslib::utils::Cache<T>::top() {
    return cache.top();
}

template<class T>
bool frameslib::utils::Cache<T>::empty() {
    return cache.empty();
}