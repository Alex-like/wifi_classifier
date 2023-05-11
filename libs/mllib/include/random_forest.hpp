//
// Created by Alex Shchelochkov on 29.08.2022.
//
#pragma once

#include <set>
#include <climits>
#include "estimator.hpp"
#include "thread_pool.hpp"

namespace mllib { namespace calculation {
    size_t _sqrt(size_t x);
    size_t _log2(size_t x);
    size_t _none(size_t x);
    std::map<size_t, size_t> compute_labels(std::vector<size_t> &samples,
                                            Data &data);
    double compute_gini_impurity(std::map<size_t, size_t> &labels);
    double compute_gini(std::map<size_t, size_t> &left, std::map<size_t, size_t> &right);
    double compute_IG(std::map<size_t, size_t> &left, std::map<size_t, size_t> &right);
    double compute_entropy(std::map<size_t, size_t> &labels);
    std::map<size_t, double> compute_probabilities(std::map<size_t, size_t> &labels);
} }

namespace mllib { namespace models {
    class DecisionTree : public Estimator {
    private:
        struct node {
            bool is_leaf = false;
            size_t depth = 0;
            size_t feature_idx = 0;
            double threshold = 0.0;
            double h_value = 0.0;
            std::map<size_t, double> probabilities = {};
            node *left = nullptr;
            node *right = nullptr;

            node() = default;
            node(node &&n) noexcept;
            ~node();
            node& operator=(node &&n) noexcept;
            std::string node_def(size_t cur_id);
            std::tuple<size_t, size_t, size_t> parse(const std::vector<std::string> &lines);
        };

        bool lower_better;
        size_t max_depth;
        size_t min_samples_leaf;
        size_t min_samples_split;
        size_t seed;
        std::mt19937_64 eng;
        std::string criterion_func_name;
        std::string max_feature_func_name;
        std::function<size_t(size_t)> get_feature_amount;
        std::function<double(std::map<size_t, size_t> &)> node_value_func;
        std::function<double(std::map<size_t, size_t> &, std::map<size_t, size_t> &)> criterion_func;
        node *root = nullptr;

        static void split_samples_by_threshold(size_t &feature_idx,
                                               double &threshold,
                                               std::vector<size_t> &samples,
                                               std::vector<size_t> &left,
                                               std::vector<size_t> &right,
                                               Data &data);
        void choose_best_split(node *node,
                               std::vector<size_t> &samples,
                               Data &data);
        node *construct_node(size_t depth,
                             std::vector<size_t> &samples,
                             Data &data);
        std::map<size_t, double> get_probabilities(const std::vector<double> &query) const;
        std::map<size_t, double> get_probabilities(size_t sample_idx, Data &data) const;
        void generate_definition_for_graphviz(node *node, size_t id, size_t parent_id, std::ostream &out);
        void get_tree_def(node *node, size_t id, std::queue<std::string> &queue);
        size_t parse_header(const std::string &header);
    public:
        explicit DecisionTree(size_t max_depth = 0,
                              size_t min_samples_leaf = 1,
                              size_t min_samples_split = 2,
                              const std::string &criterion = "gini",
                              const std::string &max_features = "none",
                              size_t seed = 0);
        DecisionTree(DecisionTree &&m) noexcept;
        ~DecisionTree();
        DecisionTree& operator=(DecisionTree &&m) noexcept;
        std::string get_saved_def();
        void save(const std::string &path, std::ios_base::openmode mode = std::ios::out) override;
        void load_from_stream(std::istream &in);
        void load(const std::string &path) override;
        DecisionTree *clone() override;
        void fit(Data &data) override;
        void predict(Data &queries, std::vector<size_t> &result) override;
        size_t predict(const std::vector<double> &query) override;
        void predict_prob(Data &queries, std::vector<std::map<size_t, double>> &probabilities) override;
        bool valid() const override;
        void draw(const std::string &path);
        size_t get_seed() const;
    };

    class RandomForest : public Estimator {
    private:
        size_t n_estimators;
        size_t n_jobs;
        size_t max_depth;
        size_t min_samples_leaf;
        size_t min_samples_split;
        std::string criterion;
        std::string max_features;
        std::vector<DecisionTree> trees;
        std::size_t *seeds = nullptr;

        void norm(std::map<size_t, double> &result) const;
        void parse_header(const std::string &header);
    public:
        explicit RandomForest(size_t n_estimators = 10,
                     size_t n_jobs = 1,
                     size_t max_depth = 0,
                     size_t min_samples_leaf = 1,
                     size_t min_samples_split = 2,
                     const std::string &criterion = "gini",
                     const std::string &max_features = "none");
        RandomForest(RandomForest &&m) noexcept;
        ~RandomForest();
        RandomForest& operator=(RandomForest &&m) noexcept;
        std::string get_saved_def();
        void save(const std::string &path, std::ios_base::openmode mode = std::ios::out) override;
        void load_from_stream(std::istream &in);
        void load(const std::string &path) override;
        RandomForest *clone() override;
        void fit(Data &data) override;
        void predict(Data &queries, std::vector<size_t> &result) override;
        size_t predict(const std::vector<double> &query) override;
        void predict_prob(Data &queries, std::vector<std::map<size_t, double>> &probabilities) override;
        bool valid() const override;
    };
} }