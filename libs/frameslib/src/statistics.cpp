//
// Created by Alex Shchelochkov on 22.09.2022.
//

#include "../include/statistics.hpp"

using namespace std;

frameslib::frames::statistics::Statistics::Statistics(uint64_t whole_v, uint64_t correct_v, uint64_t incorrect_v, uint64_t noAddress_v) {
    whole = whole_v;
    correct = correct_v;
    incorrect = incorrect_v;
    noAddress = noAddress_v;
}

frameslib::frames::statistics::Statistics::Statistics(const vector<LogFrame> &frames) {
    whole = frames.size();
    correct = 0;
    incorrect = 0;
    noAddress = 0;
    for (LogFrame frame : frames) {
        if (frame.isCorrect()) {
            correct++;
        } else {
            incorrect++;
        }
        if (frame.isCorrect() && (!frame.getTA().has_value() || !frame.getRA().has_value())) {
            noAddress++;
        }
    }
}

string frameslib::frames::statistics::Statistics::toString() const {
    stringstream s;
    s << "Whole frames' number: " << whole
    << "\nCorrect frames: " << correct
    << "\nIncorrect frames: " << incorrect
    << "\nFrames without some address information: " << noAddress
    << "\nPercentage correct frames of the total: " << ((double)correct / (double)whole * 100.0) << '%' << '\n';
    return s.str();
}

uint64_t frameslib::frames::statistics::Statistics::getCorrect() const {
    return correct;
}

uint64_t frameslib::frames::statistics::Statistics::getIncorrect() const {
    return incorrect;
}

uint64_t frameslib::frames::statistics::Statistics::getWhole() const {
    return whole;
}

uint64_t frameslib::frames::statistics::Statistics::getNoAddress() const {
    return noAddress;
}
