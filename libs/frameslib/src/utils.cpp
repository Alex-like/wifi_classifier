//
// Created by Alex Shchelochkov on 22.09.2022.
//

#include "../include/utils.hpp"

using namespace std;

string frameslib::utils::decToHex(const uint64_t dec) {
    stringstream ss;
    ss << std::hex << dec;
    return ss.str();
}

string frameslib::utils::hexToMAC(string hex) {
    uint8_t len = hex.length();
    if (len < 12) {
        for (int i = 0; i < 12 - len; i++) {
            hex = '0' + hex;
        }
    }
    string res = hex.substr(0, 2);
    for (int i = 1; i < 6; i++) {
        res += ':' + hex.substr(2 * i, 2);
    }
    return res;
}

uint64_t frameslib::utils::macToHex(const string& mac) {
    string hex;
    const regex regex_digits("([0-9A-Za-z]+):([0-9A-Za-z]+):([0-9A-Za-z]+):([0-9A-Za-z]+):([0-9A-Za-z]+):([0-9A-Za-z]+)");
    smatch digits;
    if (regex_search(mac, digits, regex_digits)) {
        for (string num : digits) {
            if (num.size() < 3) {
                hex += num;
            }
        }
    }
    return stoull(hex, nullptr, 16);
}

void frameslib::utils::getAddressesFromFile(const string& path) {
    ifstream in(path);
    set<string> macs = {};
    if (in.is_open()) {
        smatch match;
        string line;
        while (getline(in, line, '\n')) {
            // skip empty string
            if (regex_match(line, match, regex("^\\s*$"))) {
                continue;
            }

            getAddressesFromStr(line, macs);
        }
    }
    in.close();
    for (const string& mac : macs) {
        std::cout << mac << '\n';
    }
}

void frameslib::utils::getAddressesFromStr(const string& str, set<string> &macs) {
    const regex regex_MAC("[0-9A-Za-z]+(:[0-9A-Za-z]+){5}");
    const vector<smatch> mac{
            sregex_iterator{cbegin(str), cend(str), regex_MAC},
            sregex_iterator{}
    };
    for (const smatch& address : mac) {
        macs.insert(address.str(0));
    }
}

string frameslib::utils::strToLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return tolower(c); });
    return s;
}

bool frameslib::utils::checkRetransmission(uint64_t a, uint64_t b, uint64_t c) {
    return a < c && (a < b && b > c || a > b && b < c) || a > b && b > c;
}

pair<bool, bool> frameslib::utils::getFlagsOfExistence(const string &path) {
    bool hasHeader = path.find("_phy.log", 0) != string::npos;
    bool hasBody = path.find("_parser.log", 0) != string::npos;
    return {hasHeader, hasBody};
}