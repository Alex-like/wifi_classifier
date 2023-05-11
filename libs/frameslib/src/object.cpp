//
// Created by Alex Shchelochkov on 17.11.2022.
//

#include "../include/object.hpp"

using namespace std;
using namespace frameslib;

string object::toString(DeviceType dev) {
    switch (dev) {
        case Drone:         return "Drone";
        case Controller:    return "Remote controller";
        case AccessPoint:   return "Access point / Client / Something else";
        case Unknown:       return "Unknown device";
        default:            return "";
    }
}

object::DeviceType object::getOpposite(DeviceType cur) {
    switch (cur) {
        case Drone:         return Controller;
        case Controller:    return Drone;
        case AccessPoint:   return AccessPoint;
        default:            return Unknown;
    }
}

string object::toString(SimpleFeature ftr) {
    switch (ftr) {
        case MAC:   return "specified MAC";
        case SSID:  return "specified SSID";
        default:    return "";
    }
}

object::SimpleClassifiedObject::SimpleClassifiedObject(MAC_t mac,
                                                       DeviceType type) {
    this->mac = mac;
    this->type = type;
}

object::MAC_t object::SimpleClassifiedObject::getAddress() const {
    return mac;
}

object::DeviceType object::SimpleClassifiedObject::getType() const {
    return type;
}

string object::SimpleClassifiedObject::getDefinition() const {
    stringstream ss;
    ss << utils::hexToMAC(utils::decToHex(mac)) << " : ";
    ss << object::toString(type);
    if (info.has_value())
        ss << "\t(" << info.value() << ')';
    return ss.str();
}

void object::SimpleClassifiedObject::setInfo(const std::string& info) {
    this->info = info;
}

void object::SimpleClassifiedObject::setType(DeviceType newType) {
    this->type = newType;
}

void object::SimpleClassifiedObject::setFeature(SimpleFeature ftr) {
    feature = ftr;
}

object::PacketCollection::PacketCollection(size_t packetsAmountThreshold) :
PacketCollection()
{
    packets.reserve(packetsAmountThreshold);
}

object::PacketCollection::PacketCollection(PacketCollection *collection) :
packets(vector<packet::Packet>(collection->packets))
{}

object::PacketCollection::PacketCollection(PacketCollection&& collection) noexcept :
packets(std::move(collection.packets)) 
{
    collection.packets.clear();
}

object::PacketCollection::~PacketCollection() {
    packets.clear();
}

object::PacketCollection& object::PacketCollection::operator=(const PacketCollection& collection) {
    this->packets = vector<packet::Packet>(collection.packets);
    return *this;
}

object::PacketCollection& object::PacketCollection::operator=(PacketCollection&& collection) noexcept {
    this->packets = std::move(collection.packets);
    collection.packets.clear();
    return *this;
}

bool object::PacketCollection::empty() {
    return packets.empty();
}

void object::PacketCollection::addPacket(packet::Packet &pack) {
    packets.emplace_back(pack);
}

void object::PacketCollection::removeByIndex(size_t id) {
    packets.erase(packets.begin() + id);
}

void object::PacketCollection::removeFirstPackets(size_t cnt) {
    packets.erase(packets.begin(), packets.begin() + cnt);
}

const vector<packet::Packet>& object::PacketCollection::getPackets() const {
    return packets;
}

packet::Packet& object::PacketCollection::getPacket(size_t id) {
    return packets[id];
}

object::PacketClassifiedObject::PacketClassifiedObject(MAC_t mac, DeviceType type, size_t packetsAmountThreshold) :
mac(mac),
type(type)
{
    this->packets = make_unique<PacketCollection>(packetsAmountThreshold);
}

object::PacketClassifiedObject::PacketClassifiedObject(PacketClassifiedObject&& obj) noexcept :
mac(obj.mac),
type(obj.type),
ready(obj.ready),
needCut(obj.needCut),
amountCutPackets(obj.amountCutPackets),
packets(std::move(obj.packets))
{
    obj.packets = nullptr;
}

object::PacketClassifiedObject::PacketClassifiedObject(const object::PacketClassifiedObject& obj) noexcept : 
mac(obj.mac),
type(obj.type),
ready(obj.ready),
needCut(obj.needCut),
amountCutPackets(obj.amountCutPackets) 
{
    packets = make_unique<PacketCollection>(new PacketCollection(obj.packets.get()));
}

object::PacketClassifiedObject::~PacketClassifiedObject() {
    packets.reset(nullptr);
}

object::PacketClassifiedObject& object::PacketClassifiedObject::operator=(PacketClassifiedObject&& obj) noexcept {
    this->mac = obj.mac;
    this->type = obj.type;
    this->ready = obj.ready;
    this->needCut = obj.needCut;
    this->amountCutPackets = obj.amountCutPackets;
    this->packets = std::move(obj.packets);
    obj.packets = nullptr;
    return *this;
}

object::PacketClassifiedObject& object::PacketClassifiedObject::operator=(const object::PacketClassifiedObject& obj) noexcept {
    this->mac = obj.mac;
    this->type = obj.type;
    this->ready = obj.ready;
    this->needCut = obj.needCut;
    this->amountCutPackets = obj.amountCutPackets;
    this->packets = make_unique<PacketCollection>(obj.packets.get());
    return *this;
}

object::MAC_t object::PacketClassifiedObject::getAddress() const {
    return mac;
}

object::DeviceType object::PacketClassifiedObject::getType() const {
    return type;
}

string object::PacketClassifiedObject::getDefinition() const {
    stringstream ss;
    ss << utils::hexToMAC(utils::decToHex(mac)) << " : ";
    ss << object::toString(type);
    return ss.str();
}

bool object::PacketClassifiedObject::isPacketsEmpty() const {
    return packets->empty();
}

bool object::PacketClassifiedObject::isReady() const {
    return ready;
}

bool object::PacketClassifiedObject::isNeedCut() const {
    return needCut;
}

vector<packet::Packet> object::PacketClassifiedObject::getPackets() const {
    return packets->getPackets();
}

packet::Packet& object::PacketClassifiedObject::getPacket(size_t id) {
    return packets->getPacket(id);
}

void object::PacketClassifiedObject::setType(object::DeviceType newType) {
    type = newType;
}

void object::PacketClassifiedObject::setReady(bool ready) {
    this->ready = ready;
}

void object::PacketClassifiedObject::setNeedCut(bool needCut) {
    this->needCut = needCut;
}

void object::PacketClassifiedObject::setAmountCutPackets(uint32_t cut) {
    this->amountCutPackets = cut;
}

void object::PacketClassifiedObject::addPacket(packet::Packet &pack) {
    packets->addPacket(pack);
}

void object::PacketClassifiedObject::removeByIndex(size_t id) {
    packets->removeByIndex(id);
}

void object::PacketClassifiedObject::removeFirstPackets(size_t cnt) {
    packets->removeFirstPackets(cnt);
}