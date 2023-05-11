//
// Created by Alex Shchelochkov on 22.09.2022.
//
#pragma once

#include <unordered_map>
#include <numeric>
#include <algorithm>
#include <queue>
#include <cmath>
#include <stdexcept>

#include "frame.hpp"
#include "classifier.hpp"
#include "features.hpp"

namespace frameslib { namespace frames { namespace worker {
    class Worker {
    private:
        std::unordered_map<uint64_t, std::shared_ptr<object::PacketClassifiedObject>> objects;
        std::queue<std::shared_ptr<object::PacketClassifiedObject>> queue;
        const size_t packetsAmountThreshold;
    public:
        explicit Worker(size_t packetsAmountThreshold = 200);
        ~Worker();
        void frameHandle(LogFrame &frame);
        int readFramesFromStream(std::istream &in, bool oldFileFormat = true);
        int readFramesFromFile(const std::string &path, bool oldFileFormat = true);
        const std::unordered_map<uint64_t, std::shared_ptr<object::PacketClassifiedObject>> &getObjects() const;
        bool isQueueEmpty() const;
        size_t getQueueSize() const;
        std::shared_ptr<object::PacketClassifiedObject> popFront();
        void clear();
    };
} } }