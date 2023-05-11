//
// Created by Alex Shchelochkov on 22.09.2022.
//
#pragma once

#include "frame.hpp"

namespace frameslib { namespace frames { namespace statistics {

    class Statistics {
    private:
        /// Numer of correct frames.
        uint64_t correct;
        /// Number of incorrect frames.
        uint64_t incorrect;
        /// Total number of frames.
        uint64_t whole;
        /// Number of frames without some address information.
        uint64_t noAddress;
    public:
        explicit Statistics(const std::vector<LogFrame> &frames);
        Statistics(uint64_t whole_v, uint64_t correct_v, uint64_t incorrect_v, uint64_t noAddress_v);
        std::string toString() const;
        uint64_t getCorrect() const;
        uint64_t getIncorrect() const;
        uint64_t getWhole() const;
        uint64_t getNoAddress() const;
    };
} } }