//
// Created by Alex Shchelochkov on 22.09.2022.
//

#include "../include/worker.hpp"

using namespace std;
using namespace frameslib;

frames::worker::Worker::Worker(const size_t packetsAmountThreshold)
: packetsAmountThreshold(packetsAmountThreshold) {
    objects = {};
    queue = {};
}

frames::worker::Worker::~Worker() {
    clear();
}

void frames::worker::Worker::frameHandle(LogFrame &frame) {
    // Check frame's type
    if (frame.getType().has_value()
        && frame.getTA().has_value() && frame.getTA().value() != graph::BROADCAST
        && frame.getType().value().find("Data", 0) != string::npos) {
        // Check that the object is ready
        uint64_t TA = frame.getTA().value();
        // Make new object if necessary
        if (objects.find(TA) == objects.end())
            objects[TA] = make_shared<object::PacketClassifiedObject>(TA, object::Unknown, packetsAmountThreshold);
        std::shared_ptr<object::PacketClassifiedObject>& obj = objects[TA];
        if (obj->isReady())
            return;
        // Add frame at the packet if necessary
        if (obj->isPacketsEmpty() || frame.getSeqNum() != obj->getPackets().back().getSeqNum()) {
            // Check that amount of packets equals to packetsAmountNeedForClassifier
            if (!obj->isNeedCut() && obj->getPackets().size() >= packetsAmountThreshold) {
                obj->setReady(true);
                queue.push(obj);
                return;
            }
            // Check that we need to cut first packets
            if (obj->isNeedCut()) {
                uint64_t predLast = obj->getPackets().size() - 2;
                if (obj->getPackets().size() > 1 &&
                    obj->getPackets()[predLast + 1].getSize() < obj->getPackets()[predLast].getSize()) {
                    // Cut first packets
                    double time;
                    if (predLast == 0)
                        time = obj->getPackets()[predLast + 1].getOffset();
                    else
                        time = obj->getPackets()[predLast + 1].getOffset() - obj->getPackets()[predLast].getOffset();
                    obj->getPacket(predLast + 1).setArrivalTime(time);
                    obj->setAmountCutPackets(predLast);
                    obj->removeFirstPackets(predLast);
                    obj->setNeedCut(false);
                }
            }
            packet::Packet new_p(frame.getSeqNum(),
                                 frame.getSize(),
                                 frame.getOffset(),
                                 {{frame.getSize(), frame.getOffset()}});
            obj->addPacket(new_p);
        } else {
            uint64_t last = obj->getPackets().size() - 1;
            obj->getPacket(last).addFragment(&frame);
        }
        if (obj->getPackets().size() > 2) {
            uint32_t sz = obj->getPackets().size();
            uint32_t l = sz - 3, m = sz - 2, r = sz - 1;
            if (utils::checkRetransmission(obj->getPackets()[l].getSeqNum(),
                                           obj->getPackets()[m].getSeqNum(),
                                           obj->getPackets()[r].getSeqNum()))
                obj->removeByIndex(m);
        }
    }
}

int frames::worker::Worker::readFramesFromStream(istream &in,
                         bool oldFileFormat) {
    int count = 0;
    int expectedCount = oldFileFormat ? 3 : 2;
    vector<string> lines(expectedCount);
    string line;
    while (getline(in, line, '\n')) {
        // skip isPacketsEmpty string
        smatch match;
        if (regex_match(line, match, regex("^\\s*$"))) continue;
        // remeber string
        lines[count++] = line;
        if (count == expectedCount) {
            count = 0;
            LogFrame frame = parse(lines, true, true, oldFileFormat);
            frameHandle(frame);
        }
    }
    if (0 < count && count < expectedCount) return -2;
    return 0;
}

int frames::worker::Worker::readFramesFromFile(const string &path,
                                       bool oldFileFormat) {
    ifstream in(path);
    int exitCode;
    if (in.is_open())
        exitCode = readFramesFromStream(in, oldFileFormat);
    else return -1;
    in.close();
    return exitCode;
}

const unordered_map<uint64_t, shared_ptr<object::PacketClassifiedObject>> &frames::worker::Worker::getObjects() const {
    return objects;
}

bool frames::worker::Worker::isQueueEmpty() const {
    return queue.empty();
}

size_t frames::worker::Worker::getQueueSize() const {
    return queue.size();
}

std::shared_ptr<object::PacketClassifiedObject> frames::worker::Worker::popFront() {
    std::shared_ptr<object::PacketClassifiedObject> obj = queue.front();
    queue.pop();
    return obj;
}

void frames::worker::Worker::clear() {
    while (!queue.empty())
        queue.pop();
    objects.clear();
}