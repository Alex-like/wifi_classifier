//
// Created by Alex Shchelochkov on 17.11.2022.
//

#include "../include/packet.hpp"

using namespace std;
using namespace frameslib;

packet::Packet::Packet(uint64_t id, uint64_t size_v, double time, vector<pair<uint64_t, double>> &&frags) {
    number = id;
    size = size_v;
    offset = time;
    fragments = std::move(frags);
}

packet::Packet::~Packet() {
    fragments.clear();
}

void packet::Packet::addFragment(frames::LogFrame* frame) {
    if (number != frame->getSeqNum() || fragments.size() - 1 == frame->getFragNum())
        return;
    fragments.emplace_back(frame->getSize(), frame->getOffset());
    size += frame->getSize();
    offset = max(offset, frame->getOffset());
}

uint32_t packet::Packet::getFragSize() const {
    return fragments.size();
}

uint64_t packet::Packet::getSeqNum() const {
    return number;
}

uint64_t packet::Packet::getSize() const {
    return size;
}

double packet::Packet::getOffset() const {
    return offset;
}

double packet::Packet::getArrivalTime() const {
    return arrivalTime;
}

void packet::Packet::setArrivalTime(double time) {
    arrivalTime = time;
}

map<uint64_t, vector<packet::Packet>> packet::collectPacketsByTA(const vector<frames::LogFrame> &frames) {
    // filter data frames
    vector<frames::LogFrame> dataFrames = utils::filter<frames::LogFrame>(frames, [](auto f) {
        return f.getType().has_value() && f.getType().value().find("Data", 0) != string::npos;
    });
    // grouped frames by TA
    map<uint64_t, vector<frames::LogFrame*>> transmissions;
    for (auto & dataFrame : dataFrames) {
        tl::optional<uint64_t> tmp_num = dataFrame.getTA();
        if (!tmp_num.has_value())
            continue;
        uint64_t TA = tmp_num.value();
        transmissions[TA].emplace_back(&dataFrame);
    }
    // sort groups by offset
    for (auto &p : transmissions) {
        sort(p.second.begin(), p.second.end(), [](auto a, auto b) { return a->getOffset() < b->getOffset(); });
    }
    // collect transmissions to packets grouped by TA
    map<uint64_t, vector<Packet>> D;
    for (auto &p : transmissions) {
        D[p.first] = vector<Packet>();
        for (int i = 0; i < p.second.size(); i++) {
            if (i == 0 || p.second[i]->getSeqNum() != p.second[i - 1]->getSeqNum()) {
                Packet new_p(p.second[i]->getSeqNum(),
                             p.second[i]->getSize(),
                             p.second[i]->getOffset(),
                             {{p.second[i]->getSize(), p.second[i]->getOffset()}});
                D[p.first].emplace_back(new_p);
                continue;
            }
            uint64_t last = D[p.first].size() - 1;
            D[p.first][last].addFragment(p.second[i]);
        }

        if (D[p.first].size() > 3)
            for (uint32_t i = D[p.first].size() - 2; i > 0; i--) {
                uint64_t a = D[p.first][i - 1].getSeqNum();
                uint64_t b = D[p.first][i].getSeqNum();
                uint64_t c = D[p.first][i + 1].getSeqNum();
                if (utils::checkRetransmission(a, b, c))
                    D[p.first].erase(D[p.first].begin() + i);
            }
    }
    return D;
}

map<uint64_t, map<uint64_t, vector<packet::Packet>>> packet::collectPackets(const vector<frames::LogFrame> &frames) {
    // filter data frames
    vector<frames::LogFrame> correctFrames = utils::filter<frames::LogFrame>(frames, [](auto f) {
        return f.isCorrect();
    });
    // grouped frames by TA
    map<uint64_t, map<uint64_t, vector<frames::LogFrame*>>> transmissions;
    for (auto & frame : correctFrames) {
        tl::optional<uint64_t> tmp_num = frame.getTA();
        if (!tmp_num.has_value())
            continue;
        uint64_t TA = tmp_num.value();
        tmp_num = frame.getRA();
        if (!tmp_num.has_value())
            continue;
        uint64_t RA = tmp_num.value();
        transmissions[TA][RA].emplace_back(&frame);
    }
    // sort groups by offset
    for (auto &dict : transmissions)
        for (auto &p : dict.second)
            sort(p.second.begin(), p.second.end(), [](auto a, auto b) { return a->getOffset() < b->getOffset(); });
    // collect transmissions to packets grouped by TA & RA
    map<uint64_t, map<uint64_t, vector<Packet>>> D;
    for (auto &dict : transmissions)
        for (auto &p : dict.second) {
            D[dict.first][p.first] = vector<Packet>();
            for (int i = 0; i < p.second.size(); i++) {
                if (i == 0 || p.second[i]->getSeqNum() != p.second[i - 1]->getSeqNum()) {
                    Packet new_p(p.second[i]->getSeqNum(),
                                 p.second[i]->getSize(),
                                 p.second[i]->getOffset(),
                                 {{p.second[i]->getSize(), p.second[i]->getOffset()}});
                    D[dict.first][p.first].emplace_back(new_p);
                    continue;
                }
                uint64_t last = D[dict.first][p.first].size() - 1;
                D[dict.first][p.first][last].addFragment(p.second[i]);
            }
            if (D[dict.first][p.first].size() > 3)
                for (uint32_t i = D[dict.first][p.first].size() - 2; i > 0; i--) {
                    uint64_t a = D[dict.first][p.first][i - 1].getSeqNum();
                    uint64_t b = D[dict.first][p.first][i].getSeqNum();
                    uint64_t c = D[dict.first][p.first][i + 1].getSeqNum();
                    if (utils::checkRetransmission(a, b, c))
                        D[dict.first][p.first].erase(D[dict.first][p.first].begin() + i);
                }
        }
    return D;
}

map<uint64_t, vector<packet::Packet>> packet::cutFirstMTUPackets(const map<uint64_t, vector<Packet>> &D,
                                                                 const size_t packetsAmountThreshold) {
    map<uint64_t, vector<Packet>> SM;
    for (auto p : D) {
        if (p.second.size() < packetsAmountThreshold)
            SM[p.first] = p.second;
        else {
            size_t left = 0;
            for (; left < p.second.size() - 1
                   && p.second[left].getSize() <= p.second[left + 1].getSize()
                   && p.second.size() - left > packetsAmountThreshold;
                   left++);
            size_t right = min(left + packetsAmountThreshold, p.second.size());
            SM[p.first] = vector<Packet>();
            for (size_t i = left; i < right; i++)
                SM[p.first].emplace_back(p.second[i]);
        }
    }
    return SM;
}