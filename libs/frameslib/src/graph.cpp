//
// Created by Alex Shchelochkov on 22.09.2022.
//

#include "../include/graph.hpp"

using namespace std;

const uint64_t frameslib::graph::BROADCAST = 0xffffffffffff;

frameslib::graph::Edge::Edge(uint64_t from_v, uint64_t to_v, uint64_t cnt) {
    from = from_v;
    to = to_v;
    weight = cnt;
}

string frameslib::graph::Edge::toString() const {
    stringstream ss;
    ss << utils::hexToMAC(utils::decToHex(from)) << " -> " << utils::hexToMAC(utils::decToHex(to)) << " : " << weight;
    return ss.str();
}

uint64_t frameslib::graph::Edge::getFrom() const {
    return from;
}

uint64_t frameslib::graph::Edge::getTo() const {
    return to;
}

uint64_t frameslib::graph::Edge::getWeight() const {
    return weight;
}

frameslib::graph::Graph::Graph(map<uint64_t, map<uint64_t, uint64_t>> edges_v) {
    edges = move(edges_v);
}

frameslib::graph::Graph::Graph(vector<Edge> &edges_v) : Graph() {
    for (Edge e : edges_v) {
        addEdge(e);
    }
}

string frameslib::graph::Graph::toString() {
    stringstream ss;
    for (auto& fromToPairs : edges) {
        for (auto& toWeightPair : fromToPairs.second) {
            Edge e = Edge(fromToPairs.first, toWeightPair.first, toWeightPair.second);
            ss << e.toString() << '\n';
        }
    }
    return ss.str();
}

void frameslib::graph::Graph::addEdge(Edge e) {
    uint64_t from = e.getFrom(), to = e.getTo(), cnt = e.getWeight();
    if (edges.find(from) == edges.end()) {
        edges[from] = {};
    }
    if (edges[from].find(to) == edges[from].end()) {
        edges[from][to] = 0;
    }
    edges[from][to] += cnt;
}


void frameslib::graph::Graph::addEdge(tl::optional<uint64_t> from, tl::optional<uint64_t> to) {
    if (!from.has_value() || !to.has_value()) {
        return;
    }
    if (edges.find(from.value()) == edges.end()) {
        edges[from.value()] = {};
    }
    if (edges[from.value()].find(to.value()) == edges[from.value()].end()) {
        edges[from.value()][to.value()] = 0;
    }
    edges[from.value()][to.value()] += 1;
}

frameslib::graph::Group::Group(string name, uint64_t MAC, map<uint64_t, pair<uint64_t, uint64_t>> addresses) {
    SSID = move(name);
    host = MAC;
    clients = move(addresses);
    for (const auto& p : clients)
        if (!theMostFrequentlyTransmitted.has_value() ||
        p.second.second > theMostFrequentlyTransmitted.value().second)
            theMostFrequentlyTransmitted = {p.first, p.second.second};
}

string frameslib::graph::Group::toString() {
    stringstream ss;
    ss << SSID << " : " << utils::hexToMAC(utils::decToHex(host)) << '\n';
    for (auto& client : clients) {
        pair<uint64_t, uint64_t> framesCount = client.second;
        ss << '\t' << utils::hexToMAC(utils::decToHex(client.first)) << " : received " << framesCount.first << " | transmitted " << framesCount.second << '\n';
    }
    return ss.str();
}

void frameslib::graph::Group::addClient(const uint64_t client, const bool received) {
    if (clients.find(client) == clients.end()) {
        clients[client] = pair<uint64_t, uint64_t>(0, 0);
    }
    if (received) {
        clients[client].first += 1;
    } else {
        clients[client].second += 1;
        update(client);
    }
}

set<uint64_t> frameslib::graph::Group::getClients() const {
    set<uint64_t> res;
    for (auto &p : this -> clients) {
        res.insert(p.first);
    }
    return res;
}

tl::optional<uint64_t> frameslib::graph::Group::getMostFrequentlyClient() const {
    return theMostFrequentlyTransmitted.has_value() ?
                tl::optional<uint64_t>(theMostFrequentlyTransmitted.value().first) :
                tl::nullopt;
}

void frameslib::graph::Group::update(uint64_t client) {
    if (!theMostFrequentlyTransmitted.has_value() ||
    clients[client].second > theMostFrequentlyTransmitted.value().second)
        theMostFrequentlyTransmitted = {client, clients[client].second};
}

uint64_t frameslib::graph::Group::getReceivedAmount(uint64_t from) const {
    if (clients.find(from) != clients.end())
        return clients.at(from).first;
    return 0;
}
uint64_t frameslib::graph::Group::getTransmittedAmount(uint64_t to) const {
    if (clients.find(to) != clients.end())
        return clients.at(to).second;
    return 0;
}

bool frameslib::graph::Group::checkExistAsClient(uint64_t mac) const {
    return clients.find(mac) != clients.end();
}

frameslib::graph::GroupedGraph::GroupedGraph(map<uint64_t, Group> groups_v) : Graph() {
    groups = move(groups_v);
}

void frameslib::graph::GroupedGraph::addGroup(string name, uint64_t address) {
    if (groups.find(address) == groups.end()) {
        groups[address] = Group(move(name), address);
    }
}

void frameslib::graph::GroupedGraph::addEdge(uint64_t from, uint64_t to) {
    if (groups.find(from) != groups.end() && to != BROADCAST) {
        groups[from].addClient(to, true);
        return;
    }
    if (groups.find(to) != groups.end() && from != BROADCAST) {
        groups[to].addClient(from, false);
        return;
    }
    Graph::addEdge(from, to);
}

void frameslib::graph::GroupedGraph::addFrame(frameslib::frames::LogFrame &frame) {
    if (frame.getType() == "Management/Beacon") {
        if (frame.getSSID().has_value() && frame.getTA().has_value()) {
            this->addGroup(frame.getSSID().value(), frame.getTA().value());
        }
    }
    if (frame.getTA().has_value() && frame.getRA().has_value()) {
        this->addEdge(frame.getTA().value(), frame.getRA().value());
    }
}

string frameslib::graph::GroupedGraph::toString() {
    stringstream ss;
    for (auto& group : groups) {
        ss << group.second.toString() << '\n';
    }
    ss << Graph::toString();
    return ss.str();
}

bool frameslib::graph::GroupedGraph::checkExistAsHost(uint64_t mac) const {
    return groups.find(mac) != groups.end();
}

bool frameslib::graph::GroupedGraph::checkExistAsClient(uint64_t mac) const {
    return any_of(
            groups.begin(), groups.end(),
            [mac](auto &p) { return p.second.checkExistAsClient(mac); }
    );
}

tl::optional<uint64_t> frameslib::graph::GroupedGraph::getMostFrequentlyClient(uint64_t mac) const {
    if (checkExistAsHost(mac)) {
        return groups.at(mac).getMostFrequentlyClient();
    }
    tl::optional<pair<uint64_t, uint64_t>> res = tl::nullopt;
    for (auto &p : groups) {
        if (p.second.checkExistAsClient(mac)) {
            uint64_t received = p.second.getReceivedAmount(mac);
            if (!res.has_value() || received > res.value().second)
                res = {p.first, received};
        }
    }
    return res.has_value() ? tl::optional<uint64_t>(res.value().first) : tl::nullopt;
}