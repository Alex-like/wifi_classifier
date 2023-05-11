//
// Created by Alex Shchelochkov on 05.10.2022.
//

#include "../include/syncworker.hpp"

using namespace std;
using namespace frameslib;

frames::syncworker::SyncPacket::SyncPacket(LogFrame frame)
: packet::Packet(frame.getSeqNum(), frame.getSize(), frame.getOffset(), {{frame.getSeqNum(), frame.getOffset()}}) {
    this->frags = {frame.getFragNum()};
}

void frames::syncworker::SyncPacket::addFragment(LogFrame* frame) {
    if (number != frame->getSeqNum() || frags.find(frame->getFragNum()) != frags.end())
        return;
    fragments.emplace_back(frame->getSize(), frame->getOffset());
    frags.insert(frame->getFragNum());
    size += frame->getSize();
    offset = max(offset, frame->getOffset());
}

frames::syncworker::SyncPacketWorker::SyncPacketWorker(const size_t packetsAmountThreshold)
    : packetsAmountThreshold(packetsAmountThreshold) {
    packets = {};
    expectedFragsSizes = {};
    readyPackets = {};
}

bool frames::syncworker::SyncPacketWorker::addFragment(LogFrame frame) {
    std::unique_lock<std::recursive_mutex> _ul(_lock);
    if (readyToWork) return readyToWork;
    if (packets.empty()) {
        SyncPacket p(frame);
        packets.emplace_back(p);
        expectedFragsSizes.emplace_back(!frame.getMoreFrags() ? frame.getFragNum() + 1 : frame.getFragNum() + 2);
        readyPackets.emplace_back(p.getFragSize() - 1 == frame.getFragNum() && !frame.getMoreFrags());
        return false;
    }

//    // Try to write normal structure
//    const uint64_t seqNum = frame.getSeqNum();
//    const float offset = frame.getOffset();
//    int ind_offset = 0;
//    for (; ind_offset < packets.size() && packets[ind_offset].getOffset() < offset; ind_offset++);
//
//    if (ind_offset == 0 && !needCut) return readyToWork;
//
//    // Check extreme cases
//    if (ind_offset == 0 || ind_offset == packets.size()) {
//        if (ind_offset == packets.size()) ind_offset--;
//        if (packets[ind_offset].getSeqNum() == seqNum) {
//            packets[ind_offset].addFragment(&frame);
//            expectedFragsSizes[ind_offset] = !frame.getMoreFrags() ?
//                                             int(frame.getFragNum() + 1) :
//                                             std::max(expectedFragsSizes[ind_offset], int(frame.getFragNum() + 2));
//            readyPackets[ind_offset] = expectedFragsSizes[ind_offset] == packets[ind_offset].getFragSize();
//            tryCut();
//            if (!readyToWork)
//                readyToWork = !needCut && count() >= global_vars::packetsAmountNeedForClassifier;
//            return readyToWork;
//        }
//        // Try to find packet with seqNum in packets
//        int ind_seq = 0;
//        for (; ind_seq < packets.size() && packets[ind_seq].getSeqNum() != seqNum; ind_seq++);
//        if (ind_seq == packets.size()) {
//            if (ind_offset == 0 && !needCut) return readyToWork;
//            SyncPacket p(frame);
//            packets.insert(packets.begin() + ind_offset, p);
//            expectedFragsSizes.insert(expectedFragsSizes.begin() + ind_offset,
//                                      !frame.getMoreFrags() ? frame.getFragNum() + 1 : frame.getFragNum() + 2);
//            readyPackets.insert(readyPackets.begin() + ind_offset,
//                                p.getFragSize() - 1 == frame.getFragNum() && !frame.getMoreFrags());
//            tryCut();
//            if (!readyToWork)
//                readyToWork = !needCut && count() >= global_vars::packetsAmountNeedForClassifier;
//            return readyToWork;
//        } else {
//            packets[ind_seq].addFragment(&frame);
//            expectedFragsSizes[ind_seq] = !frame.getMoreFrags() ?
//                                             int(frame.getFragNum() + 1) :
//                                             std::max(expectedFragsSizes[ind_seq], int(frame.getFragNum() + 2));
//            readyPackets[ind_seq] = expectedFragsSizes[ind_seq] == packets[ind_seq].getFragSize();
//            tryCut();
//            if (!readyToWork)
//                readyToWork = !needCut && count() >= global_vars::packetsAmountNeedForClassifier;
//            return readyToWork;
//        }
//    } else {
//        // Check [0..x] cur [x+1...]
//        // Check ∈ [x + 1]
//        if (packets[ind_offset].getSeqNum() == seqNum) {
//            packets[ind_offset].addFragment(&frame);
//            expectedFragsSizes[ind_offset] = !frame.getMoreFrags() ?
//                                             int(frame.getFragNum() + 1) :
//                                             std::max(expectedFragsSizes[ind_offset], int(frame.getFragNum() + 2));
//            readyPackets[ind_offset] = expectedFragsSizes[ind_offset] == packets[ind_offset].getFragSize();
//            tryCut();
//            if (!readyToWork)
//                readyToWork = !needCut && count() >= global_vars::packetsAmountNeedForClassifier;
//            return readyToWork;
//        } else
//            // Check ∈ [x]
//            if (packets[ind_offset - 1].getSeqNum() == seqNum) {
//            packets[ind_offset - 1].addFragment(&frame);
//            expectedFragsSizes[ind_offset - 1] = !frame.getMoreFrags() ?
//                                             int(frame.getFragNum() + 1) :
//                                             std::max(expectedFragsSizes[ind_offset - 1], int(frame.getFragNum() + 2));
//            readyPackets[ind_offset - 1] = expectedFragsSizes[ind_offset - 1] == packets[ind_offset - 1].getFragSize();
//            tryCut();
//            if (!readyToWork)
//                readyToWork = !needCut && count() >= global_vars::packetsAmountNeedForClassifier;
//            return readyToWork;
//        } else {
//            // Try to find packet with seqNum in packets
//            int ind_seq = 0;
//            for (; ind_seq < packets.size() && packets[ind_seq].getSeqNum() < seqNum; ind_seq++);
//
//        }
//    }

    // Need more useful and powerful structure
    const uint64_t seqNum = frame.getSeqNum();
    int ind = 0;
    for (; ind < packets.size() && packets[ind].getSeqNum() < seqNum; ind++);

    // We cut first packets and now try to insert in begin
    if (ind == 0 && !needCut) return readyToWork;

    if (ind == packets.size() || packets[ind].getSeqNum() > seqNum) {
        SyncPacket p(frame);
        packets.insert(packets.begin() + ind, p);
        expectedFragsSizes.insert(expectedFragsSizes.begin() + ind,
                                  !frame.getMoreFrags() ? frame.getFragNum() + 1 : frame.getFragNum() + 2);
        readyPackets.insert(readyPackets.begin() + ind,
                            p.getFragSize() - 1 == frame.getFragNum() && !frame.getMoreFrags());
        tryCut();
        if (!readyToWork)
            readyToWork = !needCut && count() >= packetsAmountThreshold;
        return readyToWork;
    }
    packets[ind].addFragment(&frame);
    expectedFragsSizes[ind] = !frame.getMoreFrags() ?
                              int(frame.getFragNum() + 1) :
                              std::max(expectedFragsSizes[ind], int(frame.getFragNum() + 2));
    readyPackets[ind] = expectedFragsSizes[ind] == packets[ind].getFragSize();
    tryCut();
    if (!readyToWork)
        readyToWork = !needCut && count() >= packetsAmountThreshold;
    return readyToWork;
}

void frames::syncworker::SyncPacketWorker::tryCut() {
//    std::unique_lock<std::recursive_mutex> _ul(_lock);
    if (!needCut) return;
    if (packets.size() < 2) return;
    uint64_t ind = 1;
    if (!readyPackets[0]) return;
    for (; ind < packets.size() && readyPackets[ind] && packets[ind].getSize() >= packets[ind - 1].getSize(); ind++);
    if (ind == packets.size())
        return;
    if (ind >= 1 && packets[ind].getSize() < packets[ind - 1].getSize()) {
        packets.erase(packets.begin(), packets.begin() + ind - 1);
        readyPackets.erase(readyPackets.begin(), readyPackets.begin() + ind - 1);
        expectedFragsSizes.erase(expectedFragsSizes.begin(), expectedFragsSizes.begin() + ind - 1);
        needCut = false;
    }
}

int frames::syncworker::SyncPacketWorker::count() {
//    std::unique_lock<std::recursive_mutex> _ul(_lock);
    int ind = 0;
    for (; ind < readyPackets.size() && readyPackets[ind]; ind++);
    return ind;
}

vector<packet::Packet> frames::syncworker::SyncPacketWorker::getPackets() {
    std::unique_lock<std::recursive_mutex> _ul(_lock);
    vector<packet::Packet>res;
    res.reserve(packetsAmountThreshold);
    for (int i = 0; i < packetsAmountThreshold; i++)
        res.emplace_back(packets[i]);
    return res;
}

bool frames::syncworker::SyncPacketWorker::isReadyToWork() const {
    return readyToWork;
}

void frames::syncworker::SyncPacketWorker::reset() {
    std::unique_lock<std::recursive_mutex> _ul(_lock);
    needCut = true;
    readyToWork = false;
}

frames::syncworker::SyncFrameWorker::SyncFrameWorker(int threads_cnt) : threads_cnt(threads_cnt) {}

int frames::syncworker::SyncFrameWorker::readFramesFromFile(const string &path, queue<SyncPacketWorker *> &queue, bool oldFileFormat) {
    int count = 0;
    int expectedCount = oldFileFormat ? 3 : 2;
    vector<string> lines(expectedCount);
    ThreadPool threadPool(threads_cnt);
    ifstream in(path);
    if (in.is_open()) {
        string line;
        while (getline(in, line, '\n')) {
            // skip empty string
            smatch match;
            if (regex_match(line, match, regex("^\\s*$"))) {
                continue;
            }

            // remeber string
            lines[count++] = line;
            if (count == expectedCount) {
                count = 0;
                threadPool.add_job(std::bind(&SyncFrameWorker::frameHandling, this, vector<string> (lines),queue , oldFileFormat));
            }
        }
    } else return -1;
    in.close();
    return 0;
}

void frames::syncworker::SyncFrameWorker::frameHandling(const vector<string> lines, std::queue<SyncPacketWorker *> &queue, bool oldFileFormat) {
    LogFrame frame = parse(lines, true, true, oldFileFormat);
    if (!frame.getTA().has_value()
        || !frame.getType().has_value()
        || frame.getType().value().find("Data", 0) == string::npos) return;
    auto TA = frame.getTA().value();
    if (packetsByAddress[TA].addFragment(frame)) {
        std::unique_lock<std::recursive_mutex> _ul(_lock);
        if (alreadyAddToQueue.find(TA) == alreadyAddToQueue.end() || !alreadyAddToQueue[TA]) {
            queue.push(&packetsByAddress[TA]);
            alreadyAddToQueue[TA] = true;
        }
    }
}

void frames::syncworker::SyncFrameWorker::reset() {
    packetsByAddress.clear();
    alreadyAddToQueue.clear();
}
