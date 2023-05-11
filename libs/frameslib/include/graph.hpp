//
// Created by Alex Shchelochkov on 22.09.2022.
//
#pragma once

#include "utils.hpp"
#include "frame.hpp"

namespace frameslib { namespace graph {
    /// Hexadecimal value of broadcast MAC-address' mask.
    extern const uint64_t BROADCAST;
    /**
     * Connection structure.
     *
     * Decompose connection to transmitter address, receiver address and number of times they connected with each other.
     */
    class Edge {
    private:
        /// TA - transmitter address
        uint64_t from;
        /// RA - receiver address
        uint64_t to;
        /// Number of requests
        uint64_t weight;
    public:
        /**
         * Basic constructor.
         *
         * @param from_v transmitted address.
         * @param to_v receiver address.
         * @param cnt how many times they connected with each other.
         */
        Edge(uint64_t from_v, uint64_t to_v, uint64_t cnt);
        /**
         * String value of edge.
         *
         * @return string representation of Connection.
         */
        std::string toString() const;
        /**
         * Getter for transmitter address.
         *
         * @return transmitter address.
         */
        uint64_t getFrom() const;
        /**
         * Getter for receiver address.
         *
         * @return receiver address.
         */
        uint64_t getTo() const;
        /**
         * Getter for number of connections.
         *
         * @return times of connections.
         */
        uint64_t getWeight() const;
    };
    /**
     * Network structure.
     *
     * Decompose network to edges by transmitter and receiver addresses.
     * Contains number of connection times between each other.
     */
    class Graph {
    private:
        /// Map to get number of connections by transmitter's and receiver's addresses.
        std::map<uint64_t, std::map<uint64_t, uint64_t>> edges;
    public:
        /**
         * Basic default constructor.
         *
         * @param edges_v network's connections with their number.
         * If it's empty init by empty map.
         */
        explicit Graph(std::map<uint64_t, std::map<uint64_t, uint64_t>> edges_v = {});
        /**
         * Alternative constructor.
         *
         * @param edges_v reference to vector of edges.
         */
        explicit Graph(std::vector<Edge> &edges_v);
        /**
         * Convert graph to string value.
         *
         * @return string representation of graph.
         */
        virtual std::string toString();
        /**
         * Append connection.
         *
         * @param e edge equal one time of connection between transmitter and receiver.
         */
        void addEdge(Edge e);
        /**
         * Append connection.
         * @param from transmitter address in number value of MAC-address.
         * @param to receiver address in number value of MAC-address.
         */
        void addEdge(tl::optional<uint64_t> from, tl::optional<uint64_t> to);
    };
    /**
     * Working with network.
     *
     * Contains name `SSID`,
     * MAC-address of hotspot `host` and information about each client `clients`,
     * which connected with.
     */
    class Group {
    private:
        /// Name equals SSID
        std::string SSID;
        /// BSSID MAC-address
        uint64_t host;
        /// The most frequently transmitted client's MAC-address and its frames amount.
        tl::optional<std::pair<uint64_t, uint64_t>> theMostFrequentlyTransmitted = tl::nullopt;
        /// Get number of received and transmitted frames by hexadecimal value of MAC-address.
        std::map<uint64_t, std::pair<uint64_t, uint64_t>> clients;
        ///Update `theMostFrequentlyUsed` param.
        void update(uint64_t client);
    public:
        /**
         * Basic default constructor.
         *
         * @param name SSID value from "Management/Beacon" frame.
         * @param MAC BSSID MAC-address from that frame.
         * @param addresses map with MAC-addresses and number of received and transmitted frames.
         * If it's empty init it by empty map.
         */
        explicit Group(std::string name = "", uint64_t MAC = 0, std::map<uint64_t, std::pair<uint64_t, uint64_t>> addresses = {});
        /**
         * Convert Network to string value.
         *
         * @return string representation of network.
         */
        std::string toString();
        /**
         * Add 1 to amount of received / transmitted frames.
         *
         * @param client client's MAC-address.
         * @param received flag which show if frame was received or transmitted.
         * If frame was received `received` equals True else False.
         */
        void addClient(uint64_t client, bool received); // received = true - получено от ТД | received = false - отправлено к ТД
        /**
         * Return set of clients from current network.
         *
         * @return set of clients.
         */
        std::set <uint64_t> getClients() const;
        /**
         * Return the most frequently used MAC-address.
         *
         * @return address of client
         */
         tl::optional<uint64_t> getMostFrequentlyClient() const;
         /// Get amount of frames received from client.
         uint64_t getReceivedAmount(uint64_t from) const;
        /// Get amount of frames transmitted to client.
         uint64_t getTransmittedAmount(uint64_t to) const;
        /// Check that `mac` exist in clients of current group.
         bool checkExistAsClient(uint64_t mac) const;
    };
    /**
     * Grouped graph into Network with hotspot and clients.
     *
     * Decompose graph to Networks and other simple edges. Make hotspot for Network by "addGroup".
     * Append a directed edge to graph by "addEdge".
     */
    class GroupedGraph: public Graph {
    private:
        /// Get Group by number equals hexadecimal value of hotspot.
        std::map<uint64_t, Group> groups;
    public:
        /**
         * Basic default constructor.
         *
         * @param groups_v map with hotspots and groups. If it's empty init by empty map.
         */
        explicit GroupedGraph(std::map<uint64_t, Group> groups_v = {});
        /**
         * Add Network if it is necessary.
         *
         * @param name SSID value from frame.
         * @param address MAC-address, which equals BSSID value from frame.
         */
        void addGroup(std::string name, uint64_t address);
        /**
         * Add hotspot-client connection.
         *
         * @param from MAC-address of transmitter.
         * @param to MAC-address of receiver.
         */
        void addEdge(uint64_t from, uint64_t to);
        /** Add info about network from frame
         *
         * @param frame reference to frame
         */
        void addFrame(frames::LogFrame &frame);
        /**
         * Convert grapg to string value.
         *
         * @return string representation of graph.
         */
        std::string toString();
        /**
         * Check existing MAC-address as host in networks.
         *
         * @param mac checking address.
         *
         * @return result of checking: "True" if one of network's host equal `mac` else "False".
         */
        bool checkExistAsHost(uint64_t mac) const;
        /**
         * Check existing MAC-address as client in networks.
         *
         * @param mac checking address.
         *
         * @return result of checking: "True" if one of network's client equal `mac` else "False".
         */
        bool checkExistAsClient(uint64_t mac) const;
        /**
         * Return the most frequently used MAC-address which connect with `mac`.
         *
         * @return address of client
         */
        tl::optional<uint64_t> getMostFrequentlyClient(uint64_t mac) const;
    };
} }