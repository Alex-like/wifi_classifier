//
// Created by Alex Shchelochkov on 17.11.2022.
//
#pragma once

#include <map>

#include "frame.hpp"
#include "utils.hpp"

namespace frameslib { namespace packet {
    class Packet {
    protected:
        uint64_t number;
        uint64_t size;
        double offset;
        double arrivalTime;
        std::vector<std::pair<uint64_t, double>> fragments;
    public:
        explicit Packet(uint64_t id = 0, uint64_t size_v = 0, double time = 0,
                        std::vector<std::pair<uint64_t, double>> &&frags = {});
        ~Packet();
        virtual void addFragment(frames::LogFrame* frame);
        uint32_t getFragSize() const;
        uint64_t getSeqNum() const;
        uint64_t getSize() const;
        double getArrivalTime() const;
        double getOffset() const;
        void setArrivalTime(double time);
    };
    /**
    * Collect transmitions into packets and group them by TA.
    *
    * @param frames reference to vector of LogFrames.
    *
    * @return map with packets
    * key: TA (MAC-address);
    * value: sorted vector of Packets.
    */
    std::map<uint64_t, std::vector<Packet>> collectPacketsByTA(const std::vector<frames::LogFrame> &frames);
    /**
     * Collect transmissions into packets and group them by TA & RA.
     *
     * @param frames reference to vector of LogFrames.
     *
     * @return map with packets
     * key: TA (MAC-address);
     * key: RA (MAC-address);
     * value: sorted vector of Packets.
     */
     std::map<uint64_t, std::map<uint64_t, std::vector<Packet>>> collectPackets(const std::vector<frames::LogFrame> &frames);
    /**
     * Cut first "MTU" packets and save not more than packetsAmountThreshold.
     *
     * @param D map with all packets from each MAC-address.
     * @param packetsAmountThreshold packets' amount need for worker
     *
     * @return SM cut map of packets.
     */
    std::map<uint64_t, std::vector<Packet>> cutFirstMTUPackets(const std::map<uint64_t, std::vector<Packet>> &D,
                                                               size_t packetsAmountThreshold);
} }