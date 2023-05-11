//
// Created by Alex Shchelochkov on 17.11.2022.
//
#pragma once

#include <map>
#include <unordered_set>
#include <numeric>
#include <algorithm>
#include <cmath>

#include "packet.hpp"
#include "utils.hpp"

namespace frameslib { namespace features {
    class StandardFeatures {
    private:
        double standardDeviation;
        double variance;
        double rootMeanSquare;
        double m_square;
        double p_skewness;
        double kurtosys;
        double skewness;
        double min;
        double max;
        double mean;
        double median;
        double medianAD;
    public:
        explicit StandardFeatures(std::vector<double> xs = {});
        std::string toString() const;
        std::vector<double> toVector();
        double getStandardDeviation() const;
        double getVariance() const;
        double getRootMeanSquare() const;
        double getMSquare() const;
        double getPSkewness() const;
        double getKurtosys() const;
        double getSkewness() const;
        double getMin() const;
        double getMax() const;
        double getMean() const;
        double getMedian() const;
        double getMedianAD() const;
    };
    class UniqueFeatures {
    private:
        double pivotSize;
        double PM;
        double PT;
    public:
        explicit UniqueFeatures(const std::vector<packet::Packet>& packets = {});
        std::string toString() const;
        std::vector<double> toVector();
        double getPivotSize() const;
        double getPM() const;
        double getPT() const;
    };
    /**
     * Make vector of features.
     *
     * @param packets reference to vector with packets.
     *
     * @return vector of features.
     */
    std::vector<double> excludeFeaturesFromPackets(const std::vector<packet::Packet> &packets);
    /**
     * Make vector of features with skipping some of them.
     *
     * @param packets reference to vector with packets;
     * @param skips indexes.
     *
     * @return vector of features.
     */
    std::vector<double> excludeFeaturesFromPacketsWithSkips(const std::vector<packet::Packet> &packets,
                                                            const std::unordered_set<size_t> &skips = {});
} }