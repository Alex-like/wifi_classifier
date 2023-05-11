//
// Created by Alex Shchelochkov on 17.11.2022.
//

#include "../include/features.hpp"

using namespace std;
using namespace frameslib;

features::StandardFeatures::StandardFeatures(vector<double> xs) {
    auto n = double(xs.size());
    // min, max
    min = *std::min_element(xs.begin(), xs.end());
    max = *std::max_element(xs.begin(), xs.end());
    // median, medianAD
    median = utils::calcMedian<double>(xs, [](double a, double b) { return a < b; });
    medianAD = utils::calcMedian<double>(xs,
                                         [](double a, double b) { return a < b; },
                                         [m = median](double a) { return std::fabs(a - m); });
    // mean
    mean = std::accumulate(xs.begin(), xs.end(), 0.0) / n;
    auto m_i = [m = mean, n](vector<double> xs, int i) {
        return std::accumulate(xs.begin(), xs.end(), 0.0, [m, i](double acc, double x) {
            return acc + utils::fastPow(x - m, i);
        }) / n;
    };
    // variance
    variance = m_i(xs, 2);
    // standard deviation
    standardDeviation = std::sqrt(variance);
    // m_square
    m_square = std::accumulate(xs.begin(), xs.end(), 0.0, [](double acc, double x) { return acc + x * x; }) / n;
    // root-mean-square
    rootMeanSquare = std::sqrt(m_square);
    // p_skewness
    p_skewness = 3.0 * (mean - median) / standardDeviation;
    // kurtosis
    kurtosys = m_i(xs, 4) / utils::fastPow(variance, 2);
    // skewness
    skewness = m_i(xs, 3) / utils::fastPow(standardDeviation, 3);
}

string features::StandardFeatures::toString() const {
    stringstream ss;
    ss.precision(5);
    ss << min << ' ' << max << ' ' << mean << ' '
       << median << ' ' << medianAD  << ' ' << skewness << ' '
       << kurtosys << ' ' << p_skewness << ' ' << m_square << ' '
       << rootMeanSquare << ' ' << variance << ' ' << standardDeviation;
    return ss.str();
}

std::vector<double> features::StandardFeatures::toVector() {
    return {
            min, max, mean, median, medianAD ,skewness, kurtosys, p_skewness,
            m_square, rootMeanSquare, variance, standardDeviation

    };
}

double features::StandardFeatures::getStandardDeviation() const {
    return standardDeviation;
}

double features::StandardFeatures::getVariance() const {
    return variance;
}

double features::StandardFeatures::getRootMeanSquare() const {
    return rootMeanSquare;
}

double features::StandardFeatures::getMSquare() const {
    return m_square;
}

double features::StandardFeatures::getPSkewness() const {
    return p_skewness;
}

double features::StandardFeatures::getKurtosys() const {
    return kurtosys;
}

double features::StandardFeatures::getSkewness() const {
    return skewness;
}

double features::StandardFeatures::getMin() const {
    return min;
}

double features::StandardFeatures::getMax() const {
    return max;
}

double features::StandardFeatures::getMean() const {
    return mean;
}

double features::StandardFeatures::getMedian() const {
    return median;
}

double features::StandardFeatures::getMedianAD() const {
    return medianAD;
}

features::UniqueFeatures::UniqueFeatures(const vector<packet::Packet>& packets) {
    // find pivot
    map<uint64_t, uint16_t> sizeAmounts;
    uint64_t totalSize = 0, MTU = 0;
    for (const auto &packet: packets) {
        uint64_t size = packet.getSize();
        sizeAmounts[size]++;
        totalSize += size;
        MTU = max(MTU, size);
    }
    pivotSize = double(MTU);
    for (const auto &p : packets)
        if (p.getSize() != MTU) {
            pivotSize = double(p.getSize());
            break;
        }
    for (const auto &p : sizeAmounts)
        if (p.first != MTU && p.second > sizeAmounts[uint64_t(pivotSize)])
            pivotSize = double(p.first);
    // pivot size / MTU size
    PM = pivotSize / double(MTU);
    // pivot size / total sample size
    PT = pivotSize / double(totalSize);
}

string features::UniqueFeatures::toString() const {
    stringstream ss;
    ss.precision(5);
    ss << pivotSize << ' ' << PM << ' ' << PT;
    return ss.str();
}

std::vector<double> features::UniqueFeatures::toVector() {
    return {pivotSize, PM, PT};
}

double features::UniqueFeatures::getPivotSize() const {
    return pivotSize;
}

double features::UniqueFeatures::getPM() const {
    return PM;
}

double features::UniqueFeatures::getPT() const {
    return PT;
}

vector<double> features::excludeFeaturesFromPacketsWithSkips(const vector<packet::Packet> &packets,
                                                             const unordered_set<size_t> &skips) {
    vector<double> tmp, curFeatures;
    tmp = UniqueFeatures(packets).toVector();
    curFeatures.insert(curFeatures.end(), tmp.begin(), tmp.end());
    tmp.clear();
    transform(packets.begin(), packets.end(), back_inserter(tmp), [](auto p) { return p.getSize(); });
    tmp = StandardFeatures(tmp).toVector();
    curFeatures.insert(curFeatures.end(), tmp.begin(), tmp.end());
    tmp.clear();
    transform(packets.begin(), packets.end(), back_inserter(tmp), [](auto p) { return p.getOffset(); });
    for (uint32_t i = tmp.size() - 1; i > 0; i--)
        tmp[i] -= tmp[i - 1];
    tmp[0] = packets[0].getArrivalTime();
    tmp = StandardFeatures(tmp).toVector();
    curFeatures.insert(curFeatures.end(), tmp.begin(), tmp.end());
    tmp.clear();
    tmp = curFeatures;
    curFeatures.clear();
    for (size_t i = 0; i < tmp.size(); i++)
        if (!utils::checkExist(i, skips)) curFeatures.emplace_back(tmp[i]);
    return curFeatures;
}

vector<double> features::excludeFeaturesFromPackets(const vector<packet::Packet> &packets) {
    return excludeFeaturesFromPacketsWithSkips(packets);
}