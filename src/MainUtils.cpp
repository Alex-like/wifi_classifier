//
// Created by Alex Shchelochkov on 12.07.2022.
//

#include "../include/MainUtils.hpp"

using namespace std;
using namespace frameslib;

void WiFiClassifier::getStatistics(vector<frames::LogFrame> &frames) {
    frames::statistics::Statistics stat(frames);
    cout << stat.toString() << '\n';
}

void WiFiClassifier::countFrameTypes(vector<frames::LogFrame> &frames) {
    map<string, uint64_t> types;
    for (frames::LogFrame frame : frames) {
        if (frame.getType().has_value()) {
            if (types.find(frame.getType().value()) == types.end()) {
                types[frame.getType().value()] = 0;
            }
            types[frame.getType().value()]++;
        }
    }
    for (auto &type : types) {
        cout << type.first << " : " << type.second << '\n';
    }
}

void WiFiClassifier::makeSimpleGraph(vector<frames::LogFrame> &frames) {
    graph::Graph graph = graph::Graph();
    for (frames::LogFrame frame : frames) {
        graph.addEdge(frame.getTA(), frame.getRA());
    }
    cout << graph.toString() << "\n";
}

void WiFiClassifier::getNetworks(vector<frames::LogFrame> &frames) {
    graph::GroupedGraph graph = graph::GroupedGraph();
    for (frames::LogFrame frame : frames) {
        graph.addFrame(frame);
    }
    cout << graph.toString() << '\n';
}

void WiFiClassifier::getAddressesFromFrames(vector<frames::LogFrame> &frames) {
    set<string> macs;
    for (frames::LogFrame frame : frames) {
        utils::getAddressesFromStr(frame.getData(), macs);
    }
    for (const string& mac : macs) {
        cout << mac << '\n';
    }
    cout << '\n';
}

void WiFiClassifier::getSSIDFromFrames(vector<frames::LogFrame> &frames) {
    set<string> ssids;
    for (frames::LogFrame frame : frames) {
        tl::optional<string> ssid = frame.getSSID();
        if (ssid.has_value()) {
            ssids.insert(ssid.value());
        }
    }
    for (const string& ssid : ssids) {
        cout << ssid << '\n';
    }
    cout << '\n';
}

void WiFiClassifier::classifyAllMacFromFrames(vector<frames::LogFrame> &frames) {
    graph::GroupedGraph graph = graph::GroupedGraph();
    for (frames::LogFrame frame : frames) {
        if (frame.getType() == "Management/Beacon") {
            if (frame.getSSID().has_value() && frame.getTA().has_value()) {
                graph.addGroup(frame.getSSID().value(), frame.getTA().value());
            }
        }
        if (frame.getTA().has_value() && frame.getRA().has_value()) {
            graph.addEdge(frame.getTA().value(), frame.getRA().value());
        }
    }
    cout << graph.toString() << '\n';

    map<uint64_t, string> macAndSSID;
    for (frames::LogFrame frame : frames) {
        if (frame.getTA().has_value() && !utils::checkExist(frame.getTA().value(), macAndSSID)) {
            string ssid;
            if (frame.getSSID().has_value()) {
                ssid = frame.getSSID().value();
            }
            macAndSSID[frame.getTA().value()] = ssid;
        }
    }

    set<string> macs;
    for (frames::LogFrame frame : frames) {
        utils::getAddressesFromStr(frame.getData(), macs);
    }

    classifier::SimpleClassifier classifier = classifier::SimpleClassifier(std::make_shared<graph::GroupedGraph>(graph));
    vector<object::SimpleClassifiedObject> objs;
    for (const string& mac : macs) {
        uint64_t mac_v = utils::macToHex(mac);
        if (classifier.classify(mac_v, macAndSSID[mac_v])) {
            objs.push_back(*classifier.getLastClassifiedObject());
        }
    }
    for (const object::SimpleClassifiedObject& obj : objs) {
        cout << obj.getDefinition() << '\n';
    }
}

void WiFiClassifier::printFramesTypes(vector<frames::LogFrame> &frames) {
    map<uint64_t, map<uint64_t, vector<string>>> types;
    for (frames::LogFrame frame : frames) {
        tl::optional<uint64_t> tmp_num = frame.getTA();
        if (!tmp_num.has_value())
            continue;
        uint64_t TA = tmp_num.value();
        tmp_num = frame.getRA();
        if (!tmp_num.has_value())
            continue;
        uint64_t RA = tmp_num.value();
        tl::optional<string> tmp_str = frame.getType();
        if (!tmp_str.has_value())
            continue;
        string type = tmp_str.value();
        types[TA][RA].emplace_back(type);
    }
    for (auto &p1 : types)
        for (auto &p2 : p1.second) {
            cout << p1.first << " -> " << p2.first << " : ";
            for (const string& type : p2.second)
                cout << type << ' ';
            cout << '\n';
        }
}

void WiFiClassifier::excludeDataForTrainingSet(vector<frames::LogFrame> &frames) {
    // collect transmissions to packets grouped by TA
    map<uint64_t, vector<packet::Packet>> D = packet::collectPacketsByTA(frames);
    // regroup "MTU" packets into groups with size not more than 20 packets
    map<uint64_t, vector<vector<packet::Packet>>> SM;
    for (auto &p : D) {
        if (p.second.size() < global_vars::packetsAmountThreshold)
            SM[p.first].emplace_back(p.second);
        else {
            size_t left = 0;
            while (left < p.second.size()) {
                for (; left < p.second.size() - 1
                        && p.second[left].getSize() <= p.second[left + 1].getSize()
                        && p.second.size() - left > global_vars::packetsAmountThreshold;
                    left++);
                size_t right = min(left + global_vars::packetsAmountThreshold, p.second.size());
                SM[p.first].emplace_back(vector<packet::Packet>());
                double time;
                if (left == 0)
                    time = p.second[left].getOffset();
                else
                    time = p.second[left].getOffset() - p.second[left - 1].getOffset();
                p.second[left].setArrivalTime(time);
                for (size_t i = left; i < right; i++)
                    SM[p.first].back().emplace_back(p.second[i]);
                left = right;
            }
        }
    }
    // create Features DataSet
    vector<vector<double>> DS;
    for (auto &p : SM) {
        for (auto &packets: p.second) {
            if (packets.size() < global_vars::packetsAmountThreshold)
                continue;
            vector<double> curFeatures = features::excludeFeaturesFromPackets(packets);
            if (count_if(curFeatures.begin(), curFeatures.end(), [](float x) { return isnan(x); }) > 0)
                continue;
            DS.emplace_back(curFeatures);
        }
        // TODO
//        if (!DS.empty())
//            printToFile("../data/data_tmp.log", DS, p.first, 0);
//        DS.clear();
    }
}

void WiFiClassifier::workWithDataFrames(vector<frames::LogFrame> &frames) {
    // collect transmissions to packets grouped by TA
    map<uint64_t, vector<packet::Packet>> D = packet::collectPacketsByTA(frames);
    // cut first "MTU" packets from begin and save not more than 20 packets
    map<uint64_t, vector<packet::Packet>> SM = cutFirstMTUPackets(D, global_vars::packetsAmountThreshold);
    // create Features DataSet
    map<uint64_t, vector<double>> DS;
    for (auto &p : SM) {
        if (p.second.size() < global_vars::packetsAmountThreshold)
            continue;
        vector<double> curFeatures = features::excludeFeaturesFromPackets(p.second);
        if (count_if(curFeatures.begin(), curFeatures.end(), [](float x){ return isnan(x); }) > 0)
            continue;
        DS[p.first] = curFeatures;
    }
    for (auto &p : DS)
        cout << utils::hexToMAC(utils::decToHex(p.first)) << " : " << utils::vectorToString(p.second) << '\n';
//    printToFile("../data/data_tmp.log", DS, 2);
}

void
WiFiClassifier::workWithDefiniteFile(const string &path,
                                     const function<void(vector<frames::LogFrame> &)> &action) {
    vector<frames::LogFrame> frames;
    if (readFromFile(path, frames, true, true, true) != 0) {
        cerr << "Can't read file: " << path << '\n';
        return;
    }
    action(frames);
}

void WiFiClassifier::workWithAllFiles(const function<void(vector<frames::LogFrame> &)>& action) {
    vector<string> paths {
            "../data/drones/3dr_solo/dsss/frames.log",
            "../data/drones/3dr_solo/wifi-ofdm-20/frames.log",
            "../data/drones/fimi_x8_me_2020/1wifi_fc_5825000000_fs_12000000.pcm.result/frames.log",
            "../data/drones/fimi_x8_me_2020/2wifi_fc_5825000000_fs_12000000.pcm.result/frames.log",
            "../data/drones/fimi_x8_me_2020/wifi_fc_5825000000_fs_12000000.pcm.result/frames.log",
            "../data/drones/hubsan_zino_2/Vega_2021-03-30_15-13-49-781_1_5785000000_10000000_11764706.log",
            "../data/drones/nelk_b6/NELK_B6_Downlink_5220.213483MHz_46625.000000KHz.pcm.log/frames.log",
            "../data/drones/parrot_bebop2/on_the_ground_gps-dsss/frames.log",
            "../data/drones/parrot_bebop2/on_the_ground_gps-ofdm-20/frames.log",
            "../data/drones/skydio2/unsafe_space/frames.log",
            "../data/drones/skydio2/office/frames.log"
    };
    vector<frames::LogFrame> frames;
    for (const string &path : paths) {
        cout << path << '\n';
        if (readFromFile(path, frames, true, true, true) != 0) {
            cerr << "Can't read file: " << path << '\n';
            return;
        }
        action(frames);
        frames.clear();
    }
}

void WiFiClassifier::workWithSeparatedFiles(const function<void(vector<frames::LogFrame> &)>& action) {
    vector<string> paths {
            "../data/drones/droidcam_voip/frames_parser.log",
            "../data/drones/droidcam_voip/frames_phy.log",
            
            "../data/drones/part2/dji_mavic_air/handshake-work-goodbye.pcm/frames_parser.log",
            "../data/drones/part2/dji_mavic_air/handshake-work-goodbye.pcm/frames_phy.log",
            "../data/drones/part2/dji_phantom_3/06.07.2017_13_39_32/frames_parser.log",
            "../data/drones/part2/dji_phantom_3/06.07.2017_13_39_32/frames_phy.log",
            "../data/drones/part2/dji_phantom_3/11.09.2017_10_20_15/frames_parser.log",
            "../data/drones/part2/dji_phantom_3/11.09.2017_10_20_15/frames_phy.log",
            "../data/drones/part2/dji_spark/08.02.2018_16_26_47/frames_parser.log",
            "../data/drones/part2/dji_spark/08.02.2018_16_26_47/frames_phy.log",
            "../data/drones/part2/dji_spark/08.02.2018_16_40_01/frames_parser.log",
            "../data/drones/part2/dji_spark/08.02.2018_16_40_01/frames_phy.log",
            "../data/drones/part2/dji_tello/18_01_45 2427MHz 23312.5KHz.pcm/frames_parser.log",
            "../data/drones/part2/dji_tello/18_01_45 2427MHz 23312.5KHz.pcm/frames_phy.log",
            "../data/drones/part2/hubsan_x4_air/20.06.2019_11_12_28/frames_parser.log",
            "../data/drones/part2/hubsan_x4_air/20.06.2019_11_12_28/frames_phy.log",
            "../data/drones/part2/hubsan_x4_air/20.06.2019_11_26_28/frames_parser.log",
            "../data/drones/part2/hubsan_x4_air/20.06.2019_11_26_28/frames_phy.log",
            "../data/drones/part2/hubsan_x4_air/20.06.2019_11_27_08/frames_parser.log",
            "../data/drones/part2/hubsan_x4_air/20.06.2019_11_27_08/frames_phy.log",
            "../data/drones/part2/mjx_bugs_12eis/16_08_26/frames_parser.log",
            "../data/drones/part2/mjx_bugs_12eis/16_08_26/frames_phy.log",
            "../data/drones/part2/syma_x5_sw/10_24_27 2439.346405MHz 93250.000000KHz.pcm/frames_parser.log",
            "../data/drones/part2/syma_x5_sw/10_24_27 2439.346405MHz 93250.000000KHz.pcm/frames_phy.log",
            "../data/drones/part2/syma_x5_uw/10_31_11 2439.346405MHz 93250.000000KHz.pcm/frames_parser.log",
            "../data/drones/part2/syma_x5_uw/10_31_11 2439.346405MHz 93250.000000KHz.pcm/frames_phy.log",
            "../data/drones/part2/syma_x8_pro/07.12.2017_17_24_06/frames_parser.log",
            "../data/drones/part2/syma_x8_pro/07.12.2017_17_24_06/frames_phy.log",
            "../data/drones/part2/syma_x8_pro/07.12.2017_17_24_25/frames_parser.log",
            "../data/drones/part2/syma_x8_pro/07.12.2017_17_24_25/frames_phy.log",
            "../data/drones/part2/syma_x8_pro/07.12.2017_17_24_30/frames_parser.log",
            "../data/drones/part2/syma_x8_pro/07.12.2017_17_24_30/frames_phy.log",
            "../data/drones/part2/syma_x8_pro/07.12.2017_17_24_33/frames_parser.log",
            "../data/drones/part2/syma_x8_pro/07.12.2017_17_24_33/frames_phy.log",
            "../data/drones/part2/syma_x8_pro/20.11.2018_19_39_38/frames_parser.log",
            "../data/drones/part2/syma_x8_pro/20.11.2018_19_39_38/frames_phy.log",
            "../data/drones/part2/syma_x8_pro/20.11.2018_19_40_26/frames_parser.log",
            "../data/drones/part2/syma_x8_pro/20.11.2018_19_40_26/frames_phy.log",
            "../data/drones/part2/udirc_falcon_u842/26.08.2019_18_47_53/frames_parser.log",
            "../data/drones/part2/udirc_falcon_u842/26.08.2019_18_47_53/frames_phy.log",
            "../data/drones/part2/walkera_vitus_320/12_40_29/frames_parser.log",
            "../data/drones/part2/walkera_vitus_320/12_40_29/frames_phy.log",
            "../data/drones/part2/wifi/MCS_0_27.03.2018_16_34_48/frames_parser.log",
            "../data/drones/part2/wifi/MCS_0_27.03.2018_16_34_48/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/25.10.2016_09_48_32_40000KHz/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/25.10.2016_09_48_32_40000KHz/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/25.10.2016_09_48_32_93250KHz/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/25.10.2016_09_48_32_93250KHz/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/29.11.2016_11_09_57/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/29.11.2016_11_09_57/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/AT_Alpha_AT600/18.10.2016_12_41_37/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/AT_Alpha_AT600/18.10.2016_12_41_37/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/AT_Alpha_AT600/18.10.2016_12_55_13/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/AT_Alpha_AT600/18.10.2016_12_55_13/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/AT_Alpha_AT600/18.10.2016_12_58_57/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/AT_Alpha_AT600/18.10.2016_12_58_57/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/AT_Intel_AX200/18.10.2016_13_01_21/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/AT_Intel_AX200/18.10.2016_13_01_21/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/pdf/08.11.2016_10_39_53/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/pdf/08.11.2016_10_39_53/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/pdf/08.11.2016_10_47_11/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/pdf/08.11.2016_10_47_11/frames_phy.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/pdf/08.11.2016_11_02_15/frames_parser.log",
            "../data/drones/part2/wifi/Mikrotik_hAPac3_AlphaNetworks/pdf/08.11.2016_11_02_15/frames_phy.log",
            "../data/drones/part2/wltoys_q242/10_39_38 2439.346405MHz 93250.000000KHz(Video).pcm/frames_parser.log",
            "../data/drones/part2/wltoys_q242/10_39_38 2439.346405MHz 93250.000000KHz(Video).pcm/frames_phy.log",
            "../data/drones/part2/xiaomi_mi_drone_4k/13_40_03 5765.117978MHz 13321.428571KHz.pcm/frames_parser.log",
            "../data/drones/part2/xiaomi_mi_drone_4k/13_40_03 5765.117978MHz 13321.428571KHz.pcm/frames_phy.log",
            
            "../data/drones/radio/test_03.08.2022/frames_parser.log",
            "../data/drones/radio/test_03.08.2022/frames_phy.log",
            "../data/drones/radio/test_copy_from_radiostore/frames_parser.log",
            "../data/drones/radio/test_copy_from_radiostore/frames_phy.log",
            "../data/drones/radio/test_download_app_from_appstore_with_terminations_and_pause/frames_parser.log",
            "../data/drones/radio/test_download_app_from_appstore_with_terminations_and_pause/frames_phy.log",
            "../data/drones/radio/test_pdf/frames_parser.log",
            "../data/drones/radio/test_pdf/frames_phy.log",
            "../data/drones/radio/test_ping/frames_parser.log",
            "../data/drones/radio/test_ping/frames_phy.log",
            "../data/drones/radio/test_twitch/frames_parser.log",
            "../data/drones/radio/test_twitch/frames_phy.log",
            "../data/drones/radio/test_yandex_maps/frames_parser.log",
            "../data/drones/radio/test_yandex_maps/frames_phy.log",
            "../data/drones/radio/test_youtube/frames_parser.log",
            "../data/drones/radio/test_youtube/frames_phy.log"
    };
    vector<frames::LogFrame> frames;
    map<uint64_t, frames::LogFrame> frameByInd;
    string header, body;
    bool hasHeader, hasBody;
    for (int i = 0; i < paths.size(); i += 2) {
        cout << paths[i] << '\n';

        std::tie(hasHeader, hasBody) = utils::getFlagsOfExistence(paths[i]);
        header = hasHeader ? paths[i] : paths[i+1];
        body = hasBody ? paths[i] : paths[i+1];
        readFramesFromSeparatedFiles(header, body, frames);

        action(frames);
        frames.clear();
    }
}

void WiFiClassifier::reportAdditionalTaskWriter() {
    vector<string> paths {
            "../data/drones/part2/dji_mavic_air/handshake-work-goodbye.pcm/frames_parser.log",
            "../data/drones/part2/dji_mavic_air/handshake-work-goodbye.pcm/frames_phy.log",
            "../data/drones/part2/dji_tello/18_01_45 2427MHz 23312.5KHz.pcm/frames_parser.log",
            "../data/drones/part2/dji_tello/18_01_45 2427MHz 23312.5KHz.pcm/frames_phy.log",
            "../data/drones/part2/syma_x5_sw/10_24_27 2439.346405MHz 93250.000000KHz.pcm/frames_parser.log",
            "../data/drones/part2/syma_x5_sw/10_24_27 2439.346405MHz 93250.000000KHz.pcm/frames_phy.log",
            "../data/drones/part2/syma_x5_uw/10_31_11 2439.346405MHz 93250.000000KHz.pcm/frames_parser.log",
            "../data/drones/part2/syma_x5_uw/10_31_11 2439.346405MHz 93250.000000KHz.pcm/frames_phy.log",
            "../data/drones/part2/wltoys_q242/10_39_38 2439.346405MHz 93250.000000KHz(Video).pcm/frames_parser.log",
            "../data/drones/part2/wltoys_q242/10_39_38 2439.346405MHz 93250.000000KHz(Video).pcm/frames_phy.log",
            "../data/drones/part2/xiaomi_mi_drone_4k/13_40_03 5765.117978MHz 13321.428571KHz.pcm/frames_parser.log",
            "../data/drones/part2/xiaomi_mi_drone_4k/13_40_03 5765.117978MHz 13321.428571KHz.pcm/frames_phy.log"
    };
    vector<frames::LogFrame> frames;
    map<uint64_t, frames::LogFrame> frameByInd;
    string header, body;
    bool hasHeader, hasBody;
    for (size_t i = 0; i < paths.size(); i += 2) {
        cout << paths[i] << '\n' << paths[i + 1] << '\n';

        std::tie(hasHeader, hasBody) = utils::getFlagsOfExistence(paths[i]);
        header = hasHeader ? paths[i] : paths[i+1];
        body = hasBody ? paths[i] : paths[i+1];
        readFramesFromSeparatedFiles(header, body, frames);

        // collect transmitions to packets grouped by TA
        map<uint64_t, vector<packet::Packet>> D = packet::collectPacketsByTA(frames);
        // cut first "MTU" packets from begin and save not more than 20 packets
        map<uint64_t, uint32_t> packets_cnt;
        map<uint64_t, vector<packet::Packet>> SM;
        for (auto &p : D) {
            if (p.second.size() < global_vars::packetsAmountThreshold) {
                packets_cnt[p.first] = p.second.size();
                SM[p.first] = p.second;
            } else {
                size_t left = 0;
                for (; left < p.second.size() - 1
                        && p.second[left].getSize() <= p.second[left + 1].getSize()
                        && p.second.size() - left > global_vars::packetsAmountThreshold;
                    left++);
                size_t right = min(left + global_vars::packetsAmountThreshold, p.second.size());
                SM[p.first] = vector<packet::Packet>();
                packets_cnt[p.first] = right - left;
                for (size_t j = left; j < right; j++)
                    SM[p.first].emplace_back(p.second[j]);
            }
        }
        // create Features DataSet
        map<uint64_t, vector<double>> DS;
        for (auto &p : SM) {
            if (p.second.size() < global_vars::packetsAmountThreshold)
                continue;
            vector<double> curFeatures = features::excludeFeaturesFromPackets(p.second);
            if (count_if(curFeatures.begin(), curFeatures.end(), [](float x){ return isnan(x); }) > 0)
                continue;
            DS[p.first] = curFeatures;
        }
        // Print MAC: predicted class
        // TODO
//        vector<double> query;
//        for (auto &p : DS) {
//            query = p.second;
//            normalizeRowByZScore(query, global_vars::LOOModel.meanAndVariance);
//            cout << hexToMAC(decToHex(p.first)) << " : predicted class " << global_vars::LOOModel.predict(query) << '\n';
//            cout << hexToMAC(decToHex(p.first)) << " : packets amount needed to make prediction " << packets_cnt[p.first] << '\n';
//            cout << hexToMAC(decToHex(p.first)) << " : MTU size " << int(p.second[0] / p.second[1]) << '\n';
//            uint32_t pivots_cnt = 0;
//            for (auto &packet : SM[p.first])
//                pivots_cnt += packet.getSize() == int(p.second[0]) ? 1 : 0;
//            cout << hexToMAC(decToHex(p.first)) << " : pivots count " << pivots_cnt << '\n';
//            cout << endl;
//        }
        frames.clear();
    }
}

void WiFiClassifier::reportTaskWriter() {
    vector<string> paths {
            "../data/drones/3dr_solo/dsss/frames.log",
            "../data/drones/3dr_solo/wifi-ofdm-20/frames.log",
            "../data/drones/fimi_x8_me_2020/1wifi_fc_5825000000_fs_12000000.pcm.result/frames.log",
            "../data/drones/fimi_x8_me_2020/2wifi_fc_5825000000_fs_12000000.pcm.result/frames.log",
            "../data/drones/fimi_x8_me_2020/wifi_fc_5825000000_fs_12000000.pcm.result/frames.log",
            "../data/drones/hubsan_zino_2/Vega_2021-03-30_15-13-49-781_1_5785000000_10000000_11764706.log",
            "../data/drones/nelk_b6/NELK_B6_Downlink_5220.213483MHz_46625.000000KHz.pcm.log/frames.log",
            "../data/drones/parrot_bebop2/on_the_ground_gps-dsss/frames.log",
            "../data/drones/parrot_bebop2/on_the_ground_gps-ofdm-20/frames.log",
            "../data/drones/skydio2/unsafe_space/frames.log",
            "../data/drones/skydio2/office/frames.log"
    };
    vector<frames::LogFrame> frames;
    for (const string &path : paths) {
        cout << path << '\n';
        if (readFromFile(path, frames, true, true, true) != 0) {
            cerr << "Can't read file: " << path << '\n';
            return;
        }
        // collect transmitions to packets grouped by TA
        map<uint64_t, vector<packet::Packet>> D = packet::collectPacketsByTA(frames);
        // cut first "MTU" packets from begin and save not more than 20 packets
        map<uint64_t, uint32_t> packets_cnt;
        map<uint64_t, vector<packet::Packet>> SM;
        for (auto &p : D) {
            if (p.second.size() < global_vars::packetsAmountThreshold) {
                packets_cnt[p.first] = p.second.size();
                SM[p.first] = p.second;
            } else {
                size_t left = 0;
                for (; left < p.second.size() - 1
                        && p.second[left].getSize() <= p.second[left + 1].getSize()
                        && p.second.size() - left > global_vars::packetsAmountThreshold;
                    left++);
                size_t right = min(left + global_vars::packetsAmountThreshold, p.second.size());
                SM[p.first] = vector<packet::Packet>();
                packets_cnt[p.first] = right - left;
                for (size_t i = left; i < right; i++)
                    SM[p.first].emplace_back(p.second[i]);
            }
        }
        // create Features DataSet
        map<uint64_t, vector<double>> DS;
        for (auto &p : SM) {
            if (p.second.size() < global_vars::packetsAmountThreshold)
                continue;
            vector<double> curFeatures = features::excludeFeaturesFromPackets(p.second);
            if (count_if(curFeatures.begin(), curFeatures.end(), [](float x){ return isnan(x); }) > 0)
                continue;
            DS[p.first] = curFeatures;
        }
        // Print MAC: predicted class
        // TODO
//        vector<double> query;
//        for (auto &p : DS) {
//            query = p.second;
//            normalizeRowByZScore(query, global_vars::LOOModel.meanAndVariance);
//            cout << hexToMAC(decToHex(p.first)) << " : predicted class " << global_vars::LOOModel.predict(query) << '\n';
//            cout << hexToMAC(decToHex(p.first)) << " : packets amount needed to make prediction " << packets_cnt[p.first] << '\n';
//            cout << hexToMAC(decToHex(p.first)) << " : MTU size " << int(p.second[0] / p.second[1]) << '\n';
//            uint32_t pivots_cnt = 0;
//            for (auto &packet : SM[p.first])
//                pivots_cnt += packet.getSize() == int(p.second[0]) ? 1 : 0;
//            cout << hexToMAC(decToHex(p.first)) << " : pivots count " << pivots_cnt << '\n';
//            cout << endl;
//        }
        frames.clear();
    }
}

int WiFiClassifier::readFramesFromSeparatedFiles(const string &headerPath, const string &bodyPath,
                                 vector<frameslib::frames::LogFrame> &frames) {
    vector<frames::LogFrame> tmp;
    map<uint64_t, frames::LogFrame> frameByInd;
    auto flags = utils::getFlagsOfExistence(headerPath);

    int readFileExitCode = readFromFile(headerPath, tmp, flags.first, flags.second, false);
    if (readFileExitCode != 0) {
        cerr << "Can't read file: " << headerPath << '\n';
        return readFileExitCode;
    }

    for (const frames::LogFrame& frame : tmp)
        frameByInd[frame.getInd()] = frame;
    tmp.clear();
    flags = utils::getFlagsOfExistence(bodyPath);
    readFileExitCode = readFromFile(bodyPath, tmp, flags.first, flags.second, false);
    if (readFileExitCode != 0) {
        cerr << "Can't read file: " << bodyPath << '\n';
        return readFileExitCode;
    }

    for (frames::LogFrame &frame : tmp) {
        if (frameByInd.find(frame.getInd()) == frameByInd.end()) continue;
        if (flags.first && !flags.second) {
            if (!frameByInd[frame.getInd()].isCorrect())
                frame.setFCS(frameByInd[frame.getInd()].isCorrect());
            frame.setFCS(frameByInd[frame.getInd()].isCorrect());
            frame.setType(frameByInd[frame.getInd()].getType());
            frame.setSSID(frameByInd[frame.getInd()].getSSID());
            frame.setTA(frameByInd[frame.getInd()].getTA());
            frame.setRA(frameByInd[frame.getInd()].getRA());
            frame.setMoreFrags(frameByInd[frame.getInd()].getMoreFrags());
            frame.setSeqNum(frameByInd[frame.getInd()].getSeqNum());
            frame.setFragNum(frameByInd[frame.getInd()].getFragNum());
        } else {
            frame.setOffset(frameByInd[frame.getInd()].getOffset());
            frame.setBW(frameByInd[frame.getInd()].getBW());
            frame.setMCS(frameByInd[frame.getInd()].getMCS());
            frame.setSize(frameByInd[frame.getInd()].getSize());
            frame.setFrame(frameByInd[frame.getInd()].getFrame());
        }
        frames.emplace_back(frame);
    }
    return 0;
}

int WiFiClassifier::transformer(const string& path_1, const string& path_2, const string& output_path) {
    vector<string> lines_1, lines_2;
    auto flags = frameslib::utils::getFlagsOfExistence(path_1);
    ifstream in(path_1);
    if (in.is_open()) {
        int cnt = 0;
        string line;
        while (getline(in, line, '\n'))
            lines_1.emplace_back(line);
    } else return 1;
    in.close();
    flags = frameslib::utils::getFlagsOfExistence(path_2);
    in = ifstream(path_2);
    if (in.is_open()) {
        int cnt = 0;
        string line;
        while (getline(in, line, '\n'))
            lines_2.emplace_back(line);
    } else return 2;
    in.close();
    ofstream out(output_path, ios::out);
    if (out.is_open()) {
        if (lines_1.size() != lines_2.size()) return 4;
        for (int i = 0; i < lines_1.size(); i++)
            if (flags.first)
                out << lines_2[i] << '\n' << lines_1[i] << '\n';
            else
                out << lines_1[i] << '\n' << lines_2[i] << '\n';
    } else return 3;
    out.close();
    return 0;
}