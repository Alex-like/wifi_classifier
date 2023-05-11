//
// Created by Alex Shchelochkov on 22.09.2022.
//
#pragma once

#include "object.hpp"
#include "graph.hpp"

namespace frameslib { namespace classifier {
    /// MACS which belongs to definite company.
    extern const std::map<uint64_t, std::string> macToCompany;
    /// Words which help to identify device by SSID.
    extern const std::set<std::string> stopWords;
    /// Companies which makes real.
    extern const std::set<std::string> droneCompanies;

    /**
     * Classify device by MAC-address.
     */
    class SimpleClassifier {
    private:
        /// Reference to cache with last classified objects
        std::unique_ptr<utils::Cache<object::SimpleClassifiedObject>> cache;
        /// Current network.
        std::shared_ptr<graph::GroupedGraph> network;
        /**
         * Classify device by MAC.
         *
         * @param mac MAC-address of device.
         * @param obj reference to device definition.
         *
         * @return if device has specific MAC-address return "True", else return "False".
         */
        bool classifyByMac(uint64_t mac, object::SimpleClassifiedObject& obj);
        /**
         * Classify device by SSID.
         *
         * @param ssid SSID of device.
         * @param obj reference to device definition.
         *
         * @return if device has specific SSID return "True", else return "False".
         */
        static bool classifyBySSID(tl::optional<std::string> ssid, object::SimpleClassifiedObject& obj);
    public:
        /**
         * Basic default constructor.
         *
         * @param network_v current network.
         */
        explicit SimpleClassifier(
                std::shared_ptr<graph::GroupedGraph> network_v= std::make_shared<graph::GroupedGraph>());
        /**
         * Set current network.
         *
         * @param network_v current network.
         */
        void setNetwork(std::shared_ptr<graph::GroupedGraph> network_v);
        /**
         * Get current network.
         *
         * @return current network.
         */
        std::shared_ptr<graph::GroupedGraph> getNetwork();
        /**
         * Classify device by MAC-address or SSID.
         *
         * @param mac device's MAC-address.
         * @param ssid device's SSID if it exists.
         *
         * @return true if MAC was classified otherwise false.
         * Classified object will be push to stack, use `getLastClassifiedObject` to get it.
         */
        bool classify(uint64_t mac, tl::optional<std::string> ssid);
        static object::DeviceType classifyByMAC(uint64_t mac);
        std::shared_ptr<object::SimpleClassifiedObject> getLastClassifiedObject();
    };
} }