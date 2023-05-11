//
// Created by Alex Shchelochkov on 22.09.2022.
//

#include "../include/classifier.hpp"

using namespace std;
using namespace tl;
using namespace frameslib;

const set<string> classifier::stopWords = {
        "skydio", "bebop", "drone", "sololink",
};
const set<string> classifier::droneCompanies = {
        "Beijing Fimi Technology Co., Ltd.",
        "PARROT SA",
        "Skydio Inc.",
};
const map<uint64_t, string> classifier::macToCompany = {
        {0x1831BF000000, "ASUSTek COMPUTER INC."}, // 3rd type
        {0x708BCD000000, "ASUSTek COMPUTER INC."},
        {0x74C63B000000, "AzureWave Technology Inc."}, // 3rd type
        {0x6CDFFBE00000, "Beijing Fimi Technology Co., Ltd."}, // 1st type
        {0xE03E44000000, "Broadcom"}, // 3rd type
        {0x10BD18000000, "Cisco Systems, Inc"}, // 3rd type
        {0x5C5015000000, "Cisco Systems, Inc"},
        {0x6400F1000000, "Cisco Systems, Inc"},
        {0xF07D68000000, "D-Link Corporation"}, // 3rd type
        {0x58D56E000000, "D-Link International"}, // 3rd type
        {0xD8FEE3000000, "D-Link International"},
        {0x88DC96000000, "EnGenius Technologies, Inc."}, // 3rd type
        {0xDAA119000000, "Google, Inc."}, // 3rd type
        {0x001882000000, "HUAWEI TECHNOLOGIES CO.,LTD"}, // 3rd type
        {0x001E10000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0x002568000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0x00259E000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0x002EC7000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0x0034FE000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0x00464B000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0x004F1A000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0x005A13000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0x006151000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0xEC5623000000, "HUAWEI TECHNOLOGIES CO.,LTD"},
        {0x48A472000000, "Intel Corporate"}, // 3rd
        {0x48F17F000000, "Intel Corporate"},
        {0x5CE0C5000000, "Intel Corporate"},
        {0x606720000000, "Intel Corporate"},
        {0x6C8814000000, "Intel Corporate"},
        {0x7C7A91000000, "Intel Corporate"},
        {0x7CB27D000000, "Intel Corporate"},
        {0x94659C000000, "Intel Corporate"},
        {0x98AF65000000, "Intel Corporate"},
        {0xBCA8A6000000, "Intel Corporate"},
        {0x28D244000000, "LCFC(HeFei) Electronics Technology co., ltd"}, // 3rd type
        {0x9822EF000000, "Liteon Technology Corporation"}, // 3rd type
        {0x00121C000000, "PARROT SA"}, // 1st type
        {0x00267E000000, "PARROT SA"},
        {0x9003B7000000, "PARROT SA"},
        {0x903AE6000000, "PARROT SA"},
        {0xA0143D000000, "PARROT SA"},
        {0x88A73C000000, "Ragentek Technology Group"}, // 3rd type
        {0x4C5E0C000000, "Routerboard.com"}, // 3rd type
        {0x98AAFC700000, "Shenzhen Hubsan Technology Co.，LTD."}, // 1st type
        {0x381D14000000, "Skydio Inc."}, // 1st type
        {0x706582000000, "Suzhou Hanming Technologies Co., Ltd."}, // 3rd type
        {0xC46E1F000000, "TP-LINK TECHNOLOGIES CO.,LTD."}, // 3rd type
        {0x3C970E000000, "Wistron InfoComm(Kunshan)Co.,Ltd."}, // 3rd type
        {0x80AD16000000, "Xiaomi Communications Co Ltd"},  // 3rd type
};

classifier::SimpleClassifier::SimpleClassifier(shared_ptr<graph::GroupedGraph> network_v) {
    network = std::move(network_v);
    cache = make_unique<utils::Cache<object::SimpleClassifiedObject>>();
}

bool classifier::SimpleClassifier::classifyByMac(uint64_t mac, object::SimpleClassifiedObject& obj) {
    uint64_t mac_mask;
    if (!utils::checkExist(mac & 0xffffff000000, macToCompany)) {
        if (!utils::checkExist(mac & 0xfffffff00000, macToCompany)) {
            return false;
        }
        mac_mask = mac & 0xfffffff00000;
    } else {
        mac_mask = mac & 0xffffff000000;
    }
    const string& company = macToCompany.at(mac_mask);
    obj.setInfo(company);
    if (droneCompanies.find(company) != droneCompanies.end()) {
        obj.setType(object::Drone);
        obj.setFeature(object::MAC);
        return true;
    }
    if (network->checkExistAsHost(mac)) {
        obj.setType(object::Drone);
        obj.setFeature(object::MAC);
        return true;
    }
    if (network->checkExistAsClient(mac)) {
        obj.setType(object::Controller);
        obj.setFeature(object::MAC);
        return true;
    }
    obj.setType(object::AccessPoint);
    obj.setFeature(object::MAC);
    return true;
}

bool classifier::SimpleClassifier::classifyBySSID(tl::optional<string> ssid, object::SimpleClassifiedObject& obj) {
    if (!ssid.has_value()) {
        return false;
    }
    for (const string& word : stopWords) {
        if (utils::strToLower(ssid.value()).find(word) != string::npos) {
            obj.setInfo(ssid.value());
            obj.setType(object::Drone);
            obj.setFeature(object::SSID);
            return true;
        }
    }
    return false;
}

void classifier::SimpleClassifier::setNetwork(shared_ptr<graph::GroupedGraph> network_v) {
    network = std::move(network_v);
}

std::shared_ptr<graph::GroupedGraph> classifier::SimpleClassifier::getNetwork() {
    return network;
}

bool classifier::SimpleClassifier::classify(uint64_t mac, tl::optional<string> ssid) {
    if (mac == graph::BROADCAST) {
        return false;
    }

    object::SimpleClassifiedObject obj = object::SimpleClassifiedObject(mac);

    if (!classifyByMac(mac, obj) && !classifyBySSID(std::move(ssid), obj)) {
        obj.setType(object::Unknown);
        return false;
    }
    cache->push(obj);
    return true;
}

object::DeviceType classifier::SimpleClassifier::classifyByMAC(const uint64_t mac) {
    uint64_t mac_mask = 0x0;
    if (!utils::checkExist(mac & 0xffffff000000, macToCompany)) {
        if (!utils::checkExist(mac & 0xfffffff00000, macToCompany))
            return object::Unknown;
        mac_mask = mac & 0xfffffff00000;
    } else
        mac_mask = mac & 0xffffff000000;
    const string& company = macToCompany.at(mac_mask);
    if (droneCompanies.find(company) != droneCompanies.end())
        return object::Drone;
    return object::AccessPoint;
}

shared_ptr<object::SimpleClassifiedObject> classifier::SimpleClassifier::getLastClassifiedObject() {
    auto obj = cache->top();
    cache->pop();
    return make_shared<object::SimpleClassifiedObject>(obj);
}