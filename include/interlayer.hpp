//
// Created by Alex Shchelochkov on 09.11.2022.
//
#pragma once

#include <map>
#include <sstream>
#include <cfloat>

#include "libs/frameslib/include/worker.hpp"
#include "libs/mllib/include/random_forest.hpp"
#include "libs/mllib/include/neighbour.hpp"
#include "libs/mllib/include/probability.hpp"

#include "GlobalSource.hpp"

namespace WiFiClassifier {
    using MAC_t = uint64_t;
    using Frame = frameslib::frames::LogFrame;
    using DevType = frameslib::object::DeviceType;
    using Network = frameslib::graph::GroupedGraph;
    using MACPredefindEstimator = frameslib::classifier::SimpleClassifier;
    using PacketEstimator = mllib::models::RandomForest;
    using ProbModel = mllib::models::probability_model_t;

    class standard_scale_t {
    private:
        std::vector<double> means = {};
        std::vector<double> stds = {};
    public:
        standard_scale_t() = default;
        standard_scale_t(const std::string &means_path, const std::string &stds_path, bool using_var = false);
        explicit standard_scale_t(std::istream &in);
        void load(const std::string &path);
        void load(std::istream &in);
        void load(const std::string &means_path, const std::string &stds_path, bool using_var = false);
        void save(std::ostream &out) const;
        void save(const std::string &path) const;
        std::vector<double> transform(const std::vector<double> &v) const;
        std::vector<double> inverse_transform(const std::vector<double> &v) const;
    };

    class transformer_t {
    private:
        standard_scale_t scale;
        std::vector<size_t> states = {};
        // TODO: can be Matrix<double> if we have size
        std::vector<std::vector<double>> observations = {};
    public:
        transformer_t(const std::string &states_path,
                      const std::string &observations_path,
                      const std::string &means_path,
                      const std::string &stds_path,
                      bool using_var = false);
        explicit transformer_t(const std::string &path);
        void save(std::ostream &out) const;
        void save(const std::string &path) const;
        size_t calc_observation(const std::vector<double> &ftrs) const;
        bool is_special(size_t observation) const;
    };

    /**
     * Class for working with separated frames from somewhere else
     *
     * Ability of getting list of classified objects
     */
    class WiFiHandler {
    private:
        // NOTE: remember all frames without erasing useless hotspots
        // TODO: Add TTL for hotspots
        std::shared_ptr<Network> network;
        std::unique_ptr<transformer_t> transformer;
        std::unique_ptr<ProbModel> probModel;
        std::unique_ptr<MACPredefindEstimator> macClassifier;
        std::unique_ptr<PacketEstimator> packetClassifier;
        std::unique_ptr<frameslib::frames::worker::Worker> worker;
        std::unordered_map<MAC_t, DevType> alreadyClassified;
        std::vector<size_t> getObservations(const std::shared_ptr<frameslib::object::PacketClassifiedObject> &obj) ;
        void handleFrame(Frame &&frame);
    public:
        explicit WiFiHandler(
                std::unique_ptr<MACPredefindEstimator> macClassifier = std::make_unique<MACPredefindEstimator>(),
                std::unique_ptr<PacketEstimator> packetEstimator = nullptr,
                std::unique_ptr<transformer_t> transformer = nullptr,
                std::unique_ptr<ProbModel> probModel = nullptr);
        ~WiFiHandler();
        WiFiHandler& operator=(WiFiHandler&& other) noexcept;
        void setMACEstimator(std::unique_ptr<MACPredefindEstimator> macClassifier);
        void setPacketClassifier(std::unique_ptr<PacketEstimator> estimator);
        void setTransformer(std::unique_ptr<transformer_t> transformer);
        void setProbModel(std::unique_ptr<ProbModel> probModel);
        void handleFrame(uint64_t ind_v, double Offset_v, uint32_t Size_v,
                         bool FCS_v, tl::optional<std::string> Type_v, tl::optional<std::string> SSID_v,
                         tl::optional<uint64_t> TA_v, tl::optional<uint64_t> RA_v, tl::optional<bool> moreFragments_v,
                         tl::optional<uint64_t> seqNum_v, tl::optional<uint64_t> fragNum_v);
        void handleFrame(const std::string &header, const std::string &body);
        std::string getStrWithClassifiedObjects();
        /// Returning classification results with classifier IDs.
        std::unordered_map<uint64_t, std::pair<std::string, uint8_t>> getClassifiedObjects();
        std::unordered_map<uint64_t, std::pair<std::string, uint8_t>> getNotClassifiedObjects();
        std::unordered_map<uint64_t, std::pair<std::string, uint8_t>> getAllObjects();
    };
}