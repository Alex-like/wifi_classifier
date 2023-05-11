//
// Created by Alex Shchelochkov on 17.11.2022.
//
#pragma once

#include "frame.hpp"
#include "packet.hpp"

namespace frameslib { namespace object {
    using MAC_t = uint64_t;

    /**
     * Type of device which MAC-address belongs to.
     *
     * Use to define which type of device is that.
     */
    enum DeviceType {
        /// Drone.
        Drone = 1,
        /// Remote controller.
        Controller,
        /// Access point.
        AccessPoint,
        /// Other device.
        Unknown
    };
    /**
     * Convert type to string.
     *
     * @param dev device type.
     *
     * @return string representation of `dev`.
     */
    std::string toString(DeviceType dev);
    /**
     * Get opposite device of current.
     *
     * @param cur device type.
     *
     * @return opposite device type.
     */
     DeviceType getOpposite(DeviceType cur);

    /**
     * Abstract class for classified objects (Drone, RC, AccessPoint)
     */
    class ClassifiedObject {
    public:
        virtual MAC_t getAddress() const = 0;
        virtual DeviceType getType() const = 0;
        virtual std::string getDefinition() const = 0;
    };

    /**
     * Features for detection.
     *
     * Kind of feature which helped to detect type of device
     */
    enum SimpleFeature {
        /// MAC-address which is belong to specific company.
        MAC = 1,
        /// Name of frame which contains company's name.
        SSID
    };
    /**
     * Convert feature to string.
     *
     * @param ftr feature.
     *
     * @return string representation of `ftr`.
     */
    std::string toString(SimpleFeature ftr);

    /**
     * Class for objects which was classified by predefined mac address
     * which was belonged to specified company
     */
    class SimpleClassifiedObject : public ClassifiedObject {
    private:
        /// Specific MAC-address.
        MAC_t mac;
        /// Type of device whose MAC-address equals to this.
        DeviceType type;
        /// Set with features which was detected.
        SimpleFeature feature;
        /// Extra information.
        tl::optional<std::string> info;
    public:
        explicit SimpleClassifiedObject(MAC_t mac, DeviceType type = Unknown);
        MAC_t getAddress() const override;
        DeviceType getType() const override;
        std::string getDefinition() const override;
        void setType(DeviceType newType);
        void setInfo(const std::string& info);
        /**
         * Set detected feature.
         *
         * @param ftr detected feature.
         */
        void setFeature(SimpleFeature ftr);
    };

    class PacketCollection {
    private:
        std::vector<packet::Packet> packets = {};
    public:
        explicit PacketCollection() = default;
        explicit PacketCollection(size_t packetsAmountThreshold);
        explicit PacketCollection(PacketCollection *collection);
        PacketCollection(PacketCollection&& collection) noexcept;
        PacketCollection& operator=(const PacketCollection& collection);
        PacketCollection& operator=(PacketCollection&& collection) noexcept;
        ~PacketCollection();
        bool empty();
        void addPacket(packet::Packet &pack);
        void removeByIndex(size_t id);
        void removeFirstPackets(size_t cnt);
        const std::vector<packet::Packet>& getPackets() const;
        packet::Packet& getPacket(size_t id);
    };

    /**
     * Class for objects which was classified by packet classifier model
     * which was trained on special dataset
     */
    class PacketClassifiedObject : public ClassifiedObject {
    private:
        MAC_t mac;
        DeviceType type;
        bool ready = false;
        bool needCut = true;
        uint32_t amountCutPackets = 0;
        std::unique_ptr<PacketCollection> packets;
    public:
        explicit PacketClassifiedObject(MAC_t mac = 0xffffffffffff,
                                        DeviceType type = Unknown,
                                        size_t packetsAmountThreshold = 200);
        PacketClassifiedObject(PacketClassifiedObject&& obj) noexcept ;
        PacketClassifiedObject(const PacketClassifiedObject& obj) noexcept;
        PacketClassifiedObject& operator=(PacketClassifiedObject&& obj) noexcept ;
        PacketClassifiedObject& operator=(const PacketClassifiedObject& obj) noexcept;
        ~PacketClassifiedObject();
        MAC_t getAddress() const override;
        DeviceType getType() const override;
        std::string getDefinition() const override;
        bool isPacketsEmpty() const;
        bool isReady() const;
        bool isNeedCut() const;
        std::vector<packet::Packet> getPackets() const;
        packet::Packet& getPacket(size_t id);
        void setType(DeviceType newType);
        void setReady(bool ready);
        void setNeedCut(bool needCut);
        void setAmountCutPackets(uint32_t cut);
        void addPacket(packet::Packet &pack);
        void removeByIndex(size_t id);
        void removeFirstPackets(size_t cnt);
    };
} }