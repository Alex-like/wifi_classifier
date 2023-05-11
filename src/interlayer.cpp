//
// Created by Alex Shchelochkov on 16.11.2022.
//

#include "../include/interlayer.hpp"

using namespace std;
using namespace WiFiClassifier;
using namespace frameslib;
using namespace mllib;

standard_scale_t::standard_scale_t(const string &means_path,
                                   const string &stds_path,
                                   bool using_var) {
    load(means_path, stds_path, using_var);
}

standard_scale_t::standard_scale_t(istream &in) {
    load(in);
}

void standard_scale_t::save(std::ostream &out) const {
    out << "means=[" << frameslib::utils::vectorToString(means, ", ") << "]\n";
    out << "stds=[" << frameslib::utils::vectorToString(stds, ", ") << "]\n";
}

void standard_scale_t::save(const string &path) const {
    ofstream out(path, ios::out);
    if (out.is_open())
        save(out);
    out.close();
}

void standard_scale_t::load(const std::string &path) {
    ifstream in(path);
    if (in.is_open())
        load(in);
    in.close();
}

void standard_scale_t::load(istream &in) {
    const regex regex_means(R"(means=\[(.*)\])");
    const regex regex_stds(R"(stds=\[(.*)\])");
    string line;
    auto reader = [](vector<double>& v, const string& in) {
        for (const auto& s : mllib::utils::split(mllib::utils::trim(in), ", "))
            v.emplace_back(stod(s));
    };
    while (getline(in, line, '\n')) {
        smatch match;
        if (regex_match(line, match, regex("^\\s*$"))) continue;
        if (regex_match(line, match, regex_means))
            reader(means, match[1].str());
        if (regex_match(line, match, regex_stds))
            reader(stds, match[1].str());
    }
}

void standard_scale_t::load(const string &means_path, const string &stds_path, bool using_var) {
    ifstream means_in(means_path);
    if (means_in.is_open()) {
        string line, x;
        getline(means_in, line); // skip header
        getline(means_in, line); // data
        stringstream row_stream(line);
        while (getline(row_stream, x, ','))
            means.emplace_back(stod(x));
    }
    means_in.close();
    ifstream stds_in(stds_path);
    if (stds_in.is_open()) {
        string line, x;
        getline(stds_in, line); // skip header
        getline(stds_in, line); // data
        stringstream row_stream(line);
        while (getline(row_stream, x, ','))
            stds.emplace_back(!using_var ? stod(x) : sqrt(stod(x)));
    }
    stds_in.close();
}

vector<double> standard_scale_t::transform(const vector<double> &v) const {
    if (v.size() != means.size() || v.size() != stds.size()) return {};
    size_t n = v.size();
    vector<double> res(n);
    for (size_t i = 0; i < n; i++)
        res[i] = (v[i] - means[i]) / stds[i];
    return res;
}

vector<double> standard_scale_t::inverse_transform(const vector<double> &v) const {
    if (v.size() != means.size() || v.size() != stds.size()) return {};
    size_t n = v.size();
    vector<double> res(n);
    for (size_t i = 0; i < n; i++)
        res[i] = v[i] * stds[i] + means[i];
    return res;
}

transformer_t::transformer_t(const string &states_path,
                             const string &observations_path,
                             const string &means_path,
                             const string &stds_path,
                             bool using_var) {
    // read scale
    scale.load(means_path, stds_path, using_var);
    // read observations
    ifstream obs_in(observations_path);
    if (obs_in.is_open()) {
        string line, x;
        getline(obs_in, line); // skip header
        while (getline(obs_in, line)) { // read data
            observations.emplace_back();
            stringstream row_stream(line);
            while (getline(row_stream, x, ','))
                observations.back().emplace_back(stod(x));
        }
    }
    obs_in.close();
    // read states of observations
    ifstream sts_in(states_path);
    if (sts_in.is_open()) {
        string line, x;
        getline(sts_in, line);
        stringstream row_stream(line);
        while (getline(row_stream, x, ','))
            states.emplace_back(stoul(x));
    }
    sts_in.close();
}

transformer_t::transformer_t(const std::string &path) {
    ifstream in(path);
    if (in.is_open()) {
        scale.load(in);
        in.clear();
        in.seekg(0, ios::beg);
        const regex regex_states(R"(states=\[(.*)\])");
        string line;
        while (getline(in, line, '\n')) {
            smatch match;
            if (regex_match(line, match, regex("^\\s*$"))) continue;
            if (regex_match(line, match, regex_states)) {
                for (const auto& s : mllib::utils::split(mllib::utils::trim(match[1].str()), ", "))
                    states.emplace_back(stoul(s));
                continue;
            }
            if (regex_search(line, match, regex("^(\\s*)?[^0-9-]"))) continue;
            observations.emplace_back();
            for (const auto& s : mllib::utils::split(mllib::utils::trim(line), ", "))
                observations.back().emplace_back(stod(s));
        }
    }
    in.close();
}

void transformer_t::save(ostream &out) const {
    scale.save(out);
    out << "states=[" << frameslib::utils::vectorToString(states, ", ") << "]\n";
    out << "observations_centers\n";
    for (const auto& v : observations)
        out << frameslib::utils::vectorToString(v, ", ") << '\n';
}

void transformer_t::save(const std::string &path) const {
    ofstream out(path, ios::out);
    if (out.is_open())
        save(out);
    out.close();
}

size_t transformer_t::calc_observation(const vector<double> &ftrs) const {
    // return special observation for NaN (state is AP)
    if (std::any_of(ftrs.begin(), ftrs.end(), [](auto x) { return isnan(x); }))
        return observations.size();
    auto scaled_ftrs = scale.transform(ftrs);
    size_t res = observations.size();
    double m = DBL_MAX;
    for (size_t i = 0; i < observations.size(); i++) {
        double dist = calculation::_euclidean_dist(scaled_ftrs, observations[i]);
        if (dist < m) {
            res = i;
            m = dist;
        }
    }
    return res;
}

bool transformer_t::is_special(size_t observation) const {
    return observation == observations.size();
}


WiFiHandler::WiFiHandler(unique_ptr<MACPredefindEstimator> macClassifier,
                         unique_ptr<PacketEstimator> packetEstimator,
                         unique_ptr<transformer_t> transformer,
                         unique_ptr<ProbModel> probModel)
: macClassifier(std::move(macClassifier)) {
    network = this->macClassifier->getNetwork();
    worker = make_unique<frames::worker::Worker>(global_vars::packetsAmountThreshold);
    if (packetEstimator != nullptr)
        packetClassifier = std::move(packetEstimator);
    else {
        packetClassifier = std::make_unique<PacketEstimator>();
    }
}

WiFiHandler::~WiFiHandler() {
    alreadyClassified.clear();
}

WiFiHandler& WiFiHandler::operator=(WiFiHandler&& other) noexcept {
    this->network = std::move(other.network);
    this->macClassifier = std::move(other.macClassifier);
    this->packetClassifier = std::move(other.packetClassifier);
    this->worker = std::move(other.worker);
    this->alreadyClassified = std::move(other.alreadyClassified);
    return *this;
}

void WiFiHandler::setMACEstimator(unique_ptr<MACPredefindEstimator> macClassifier) {
    this->macClassifier = std::move(macClassifier);
    this->network = this->macClassifier->getNetwork();
}

void WiFiHandler::setPacketClassifier(unique_ptr<PacketEstimator> estimator) {
    this->packetClassifier = std::move(estimator);
}

void WiFiHandler::handleFrame(uint64_t ind_v, double Offset_v, uint32_t Size_v,
                              bool FCS_v, tl::optional<string> Type_v, tl::optional<string> SSID_v,
                              tl::optional<uint64_t> TA_v, tl::optional<uint64_t> RA_v,
                              tl::optional<bool> moreFragments_v, tl::optional<uint64_t> seqNum_v,
                              tl::optional<uint64_t> fragNum_v) {
    handleFrame(Frame(ind_v, Offset_v, "", "", Size_v,
                      "",
                      "", FCS_v, std::move(Type_v), std::move(SSID_v), TA_v, RA_v, moreFragments_v, seqNum_v, fragNum_v));
}

vector<size_t> WiFiHandler::getObservations(const shared_ptr<object::PacketClassifiedObject> &obj) {
    auto packs = obj->getPackets();
    vector<size_t> O;
    O.reserve(packs.size());
    for (size_t sz = 1; sz <= packs.size(); sz++) {
        vector<packet::Packet> v(packs.begin(), packs.begin()+sz);
        auto ftrs =  features::excludeFeaturesFromPacketsWithSkips(v, global_vars::skippedFeatures);
        O.emplace_back(transformer->calc_observation(ftrs));
    }
    return O;
}


void WiFiHandler::handleFrame(Frame &&frame) {
    if (!frame.getTA().has_value()) return;
    network->addFrame(frame);
    MAC_t mac = frame.getTA().value();
    if (frameslib::utils::checkExist(mac, alreadyClassified)) return;
    if (macClassifier->classify(mac, frame.getSSID())) {
        alreadyClassified.insert({mac, macClassifier->getLastClassifiedObject()->getType()});
        return;
    }
    worker->frameHandle(frame);
}

void WiFiHandler::handleFrame(const string &header, const string &body) {
    handleFrame(frames::parse({header, body}, true, true, false));
}

std::unordered_map<uint64_t, std::pair<std::string, uint8_t>> WiFiHandler::getClassifiedObjects() {
    unordered_map<uint64_t , pair<string, uint8_t>> res_dict;
    if (!worker->isQueueEmpty()) {
        size_t n_rows = worker->getQueueSize();
        vector<object::PacketClassifiedObject> result;
        result.reserve(n_rows);
        Matrix<double> tmp(n_rows, global_vars::n_features);
        for (size_t i = 0; !worker->isQueueEmpty();  i++) {
            shared_ptr<object::PacketClassifiedObject> query = worker->popFront();
            result.emplace_back(*query);
            vector<double> curFeatures = features::excludeFeaturesFromPackets(result.back().getPackets());
            for (size_t j = 0; j < global_vars::n_features; j++)
                tmp.set(i, j, curFeatures[j]);
        }
        Data queries(n_rows, global_vars::n_features, tmp, nullptr);
        vector<size_t> res(n_rows);
        packetClassifier->predict(queries, res);
        for (size_t i = 0; i < n_rows; i++) {
            result[i].setType(object::DeviceType(res[i]));
            alreadyClassified.insert({ result[i].getAddress(), result[i].getType() });
            res_dict.insert({ result[i].getAddress(), {object::toString(result[i].getType()), 3}});
        }
    }
    // Добавление классификации объектов с недостаточным количеством пакетов
    for (const auto& p : worker->getObjects())
        if (p.first != graph::BROADCAST && !frameslib::utils::checkExist(p.first, alreadyClassified)) {
            DevType pred = DevType(probModel->predict_state(getObservations(p.second)));
            res_dict[p.first] = {object::toString(pred), 2};
            alreadyClassified[p.first] = pred;
        }
    // Добавление классификации собеседников
    unordered_map<MAC_t, pair<string, uint8_t>> addition = {};
    for (const auto& p: alreadyClassified) {
        if (p.second == frameslib::object::Unknown) continue;
        tl::optional<uint64_t> client = network->getMostFrequentlyClient(p.first);
        if (!client.has_value()) continue;
        if (client.value() != graph::BROADCAST && !frameslib::utils::checkExist(client.value(), alreadyClassified)) {
            uint8_t algo = 1;
            if (frameslib::utils::checkExist(p.first, res_dict)) algo = res_dict[p.first].second;
            addition.insert({client.value(), {object::toString(object::getOpposite(p.second)), algo}});
        }
    }

    // Агрегация результатов
    for (const auto& p: alreadyClassified)
        if (!frameslib::utils::checkExist(p.first, res_dict))
            res_dict[p.first] = {object::toString(p.second), 1};
    return std::move(res_dict);
}

std::unordered_map<uint64_t, std::pair<std::string, uint8_t>> WiFiHandler::getNotClassifiedObjects() {
    unordered_map<uint64_t , pair<string, uint8_t>> res;
    unordered_map<uint64_t, shared_ptr<object::PacketClassifiedObject>> packetObjects = worker->getObjects();
    for (const auto& obj : alreadyClassified)
        if (frameslib::utils::checkExist(obj.first, packetObjects))
            packetObjects.erase(obj.first);
    res.reserve(packetObjects.size());
    stringstream ss;
    for (const auto& p: packetObjects)
        res[p.first] = {object::toString(p.second->getType()) + ", недостаточно данных", 0};
    return std::move(res);
}

std::unordered_map<uint64_t, std::pair<std::string, uint8_t>> WiFiHandler::getAllObjects() {
    unordered_map<uint64_t , pair<string, uint8_t>> res = getClassifiedObjects();
    unordered_map<uint64_t , pair<string, uint8_t>> tmp = getNotClassifiedObjects();
    res.reserve(res.size() + tmp.size());
    res.insert(tmp.begin(), tmp.end());
    return std::move(res);
}

string WiFiHandler::getStrWithClassifiedObjects() {
    stringstream ss;
    for (const auto& p: getClassifiedObjects())
        ss << frameslib::utils::hexToMAC(frameslib::utils::decToHex(p.first))
        << " : "
        << p.second.second << '\n';
    return ss.str();
}

void WiFiHandler::setTransformer(unique_ptr<transformer_t> transformer) {
    WiFiHandler::transformer = std::move(transformer);
}

void WiFiHandler::setProbModel(unique_ptr<ProbModel> probModel) {
    WiFiHandler::probModel = std::move(probModel);
}
