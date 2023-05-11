//
// Created by Alex Shchelochkov on 05.10.2022.
//
#pragma once

#include "worker.hpp"
#include "thread_pool.hpp"

namespace frameslib { namespace frames { namespace syncworker {
    class SyncPacket : public packet::Packet {
    protected:
        std::set<uint64_t> frags;
    public:
        explicit SyncPacket(LogFrame frame);
        void addFragment(LogFrame *frame) override;
    };

    class SyncPacketWorker {
    protected:
        bool needCut = true;
        bool readyToWork = false;
        std::vector<bool> readyPackets;
        std::vector<int> expectedFragsSizes;
        std::vector<packet::Packet> packets;
        std::recursive_mutex _lock;
        size_t packetsAmountThreshold;

        int count();
        void tryCut();
    public:
        explicit SyncPacketWorker(size_t packetsAmountThreshold = 20);
        bool addFragment(LogFrame frame);
        bool isReadyToWork() const;
        std::vector<packet::Packet> getPackets();
        void reset();
    };

    class SyncFrameWorker {
    protected:
        int threads_cnt;
        std::recursive_mutex _lock;
        std::map<uint64_t, SyncPacketWorker> packetsByAddress;
        std::map <uint64_t, bool> alreadyAddToQueue;
    public:
        explicit SyncFrameWorker(int threads_cnt = 1);
        void frameHandling(std::vector<std::string> lines, std::queue<SyncPacketWorker *> &queue, bool oldFileFormat = true);
        int readFramesFromFile(const std::string &path, std::queue<SyncPacketWorker *> &queue, bool oldFileFormat = true);
        void reset();
    };
} } }