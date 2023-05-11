//
// Created by Alex Shchelochkov on 31.08.2022.
//

#include "../include/random_forest.hpp"

using namespace std;

size_t mllib::calculation::_sqrt(size_t x) {
    return size_t(sqrtl(x));
}

size_t mllib::calculation::_log2(size_t x) {
    return size_t(log2l(x));
}

size_t mllib::calculation::_none(size_t x) {
    return x;
}

map<size_t, size_t> mllib::calculation::compute_labels(vector<size_t> &samples, Data &data) {
    map<size_t, size_t> labels;
    for (auto &x : samples)
        labels[data.get_target(x)]++;
    return labels;
}

double mllib::calculation::compute_gini_impurity(map<size_t, size_t> &labels) {
    size_t sum = 0.0, n = 0;
    for (auto &p : labels) {
        n += p.second;
        sum += p.second * p.second;
    }
    if (sum > 0)
        return 1.0 - double(sum) / double(n * n);
    return 1.0;
}

double mllib::calculation::compute_gini(map<size_t, size_t> &left,
                                 map<size_t, size_t> &right) {
    size_t n = 0, n_left = 0.0, n_right = 0.0;
    for (auto &p : left)
        n += p.second;
    n_left = n;
    double left_gini = compute_gini_impurity(left);
    for (auto &p : right)
        n += p.second;
    n_right = n - n_left;
    double right_gini = compute_gini_impurity(right);
    return left_gini * (double(n_left) / double(n)) + right_gini * (double(n_right) / double(n));
}

double mllib::calculation::compute_IG(map<size_t, size_t> &left,
                               map<size_t, size_t> &right) {
    size_t n = 0;
    double n_left, n_right;
    map<size_t, size_t> labels;
    for (auto &p : left) {
        labels[p.first] += p.second;
        n += p.second;
    }
    n_left = double(n);
    for (auto &p : right) {
        labels[p.first] += p.second;
        n += p.second;
    }
    n_right = double(n) - n_left;
    double h_parent = calculation::compute_entropy(labels);
    double h_left = calculation::compute_entropy(left);
    double h_right = calculation::compute_entropy(right);
    return h_parent - ((n_left / double(n)) * h_left + (n_right / double(n)) * h_right);
}

double mllib::calculation::compute_entropy(map<size_t, size_t> &labels) {
    size_t n = 0;
    double sum = 0.0;
    for (auto &p: labels)
        n += p.second;
    for (auto &p : labels)
        if (p.second > 0)
            sum += (double(p.second) / double(n)) * log2(double(p.second) / double(n));
    return -sum;
}

map<size_t, double> mllib::calculation::compute_probabilities(map<size_t, size_t> &labels) {
    size_t n = 0;
    map<size_t, double> res;
    for (auto &p : labels) {
        n += p.second;
        res[p.first] = double(p.second);
    }
    if (n == 0)
        return res;
    for (auto &p : res)
        p.second /= double(n);
    return res;
}

mllib::models::DecisionTree::node::node(node &&n) noexcept :
is_leaf(n.is_leaf),
depth(n.depth),
feature_idx(n.feature_idx),
threshold(n.threshold),
h_value(n.h_value),
probabilities(std::move(n.probabilities)),
left(n.left),
right(n.right)
{
    n.is_leaf = false;
    n.depth = 0;
    n.feature_idx = 0;
    n.threshold = 0;
    n.h_value = 0;
    n.probabilities.clear();
    n.left = nullptr;
    n.right = nullptr;
}

mllib::models::DecisionTree::node::~node() {
    delete left;
    delete right;
}

mllib::models::DecisionTree::node& mllib::models::DecisionTree::node::operator=(mllib::models::DecisionTree::node &&n) noexcept {
    this->is_leaf = n.is_leaf;
    this->depth = n.depth;
    this->feature_idx = n.feature_idx;
    this->threshold = n.threshold;
    this->h_value = n.h_value;
    this->probabilities = std::move(n.probabilities);
    this->left = n.left;
    this->right = n.right;
    n.is_leaf = false;
    n.depth = 0;
    n.feature_idx = 0;
    n.threshold = 0;
    n.h_value = 0;
    n.probabilities.clear();
    n.left = nullptr;
    n.right = nullptr;
    return *this;
}

string mllib::models::DecisionTree::node::node_def(size_t cur_id) {
    stringstream ss;
    ss << "id=" << cur_id << ',';
    ss << "leaf=" << is_leaf << ',';
    ss << "depth=" << depth << '\n';
    if (is_leaf) {
        ss << "h=" << h_value << ',';
        ss << "probs=" << utils::map_to_str(probabilities) << '\n';
    } else {
        ss << "feature=" << feature_idx << ',';
        ss << "threshold=" << threshold << ',';
        ss << "left=" << cur_id * 2 + 1 << ',';
        ss << "right=" << cur_id * 2 + 2 << '\n';
    }
    return ss.str();
}

tuple<size_t, size_t, size_t> mllib::models::DecisionTree::node::parse(const vector<string> &lines) {
    size_t id = 0, left_id = 0, right_id = 0;
    // Parse Node defenition
    const regex regex_node_id("id=([0-9]+)");
    const regex regex_node_leaf("leaf=([^,]+)");
    const regex regex_node_depth("depth=([0-9]+)");
    const regex regex_node_h("h=([-+]?([0-9]*[.])?[0-9]+([eE][-+]?[0-9]+)?)");
    const regex regex_node_probs("probs=(\\{[^}]+\\})");
    const regex regex_node_feature("feature=([0-9]+)");
    const regex regex_node_threshold("threshold=([-+]?([0-9]*[.])?[0-9]+([eE][-+]?[0-9]+)?)");
    const regex regex_node_left("left=([0-9]+)");
    const regex regex_node_right("right=([0-9]+)");
    smatch match;
    if (regex_search(lines[0], match, regex_node_id))
        id = stoull(match[1].str());
    if (regex_search(lines[0], match, regex_node_leaf))
        is_leaf = match[1].str() == "1";
    if (regex_search(lines[0], match, regex_node_leaf))
        depth = stoull(match[1].str());
    if (is_leaf) {
        if (regex_search(lines[1], match, regex_node_h))
            h_value = stod(match[1].str());
        if (regex_search(lines[1], match, regex_node_probs))
            probabilities =
                    utils::str_to_map<size_t, double>(match[1].str(),
                                                      [] (string s) { return stoull(s); },
                                                      [] (string s) { return stod(s); });
    } else {
        if (regex_search(lines[1], match, regex_node_feature))
            feature_idx = stoull(match[1].str());
        if (regex_search(lines[1], match, regex_node_threshold))
            threshold = stod(match[1].str());
        if (regex_search(lines[1], match, regex_node_left))
            left_id = stoull(match[1].str());
        if (regex_search(lines[1], match, regex_node_right))
            right_id = stoull(match[1].str());
    }
    return {id, left_id, right_id};
}

void mllib::models::DecisionTree::split_samples_by_threshold(size_t &feature_idx,
                                              double &threshold,
                                              vector<size_t> &samples,
                                              vector<size_t> &left,
                                              vector<size_t> &right,
                                              Data &data) {
    left.clear();
    right.clear();
    for (auto &idx : samples)
        if (data.get_feature(idx, feature_idx) > threshold)
            right.push_back(idx);
        else
            left.push_back(idx);
}

void mllib::models::DecisionTree::choose_best_split(node *node,
                       vector<size_t> &samples,
                       Data &data) {
    vector<size_t> features_ids = data.generate_features(this->get_feature_amount, this->eng);
    size_t best_feature_idx = features_ids[0];
    double best_threshold = 0.0, best_score = lower_better ? 1e9 : -1e9, threshold;
    map<size_t, size_t> labels = calculation::compute_labels(samples, data);
    vector<pair<size_t, double>> sample_feature_values;
    sample_feature_values.reserve(samples.size());
    for (auto &idx : samples)
        sample_feature_values.emplace_back(idx, 0.0);
    for (auto &feature_idx : features_ids) {
        for (auto &p : sample_feature_values)
            p.second = data.get_feature(p.first, feature_idx);
        sort(sample_feature_values.begin(), sample_feature_values.end(),
                  [](auto a, auto b) { return a.second < b.second; });
        map<size_t, size_t> left, right = labels;
        for (size_t i = 0; i < samples.size() - 1;) {
            threshold = (sample_feature_values[i].second + sample_feature_values[i+1].second) / 2.0;
            while (i < samples.size() && sample_feature_values[i].second <= threshold) {
                size_t target = data.get_target(sample_feature_values[i].first);
                left[target]++;
                right[target]--;
                i++;
            }
            if (i == samples.size()) continue;
            if (i > 0 && sample_feature_values[i-1].second == threshold)
                threshold = (sample_feature_values[i-1].second + sample_feature_values[i].second) / 2.0;
            double value = criterion_func(left, right);
            if (lower_better && value <= best_score || !lower_better && value >= best_score) {
                best_score = value;
                best_threshold = threshold;
                best_feature_idx = feature_idx;
            }
        }
    }
    node->feature_idx = best_feature_idx;
    node->threshold = best_threshold;
    node->h_value = node_value_func(labels);
}

mllib::models::DecisionTree::node* mllib::models::DecisionTree::construct_node(size_t depth,
                     vector<size_t> &samples,
                     Data &data) {
    node* node = new DecisionTree::node();
    node->depth = depth;
    auto labels = calculation::compute_labels(samples, data);
    double h = calculation::compute_entropy(labels);
    if (depth == max_depth || samples.size() < min_samples_split || samples.size() == 1 || h == 0) {
        node->is_leaf = true;
        node->h_value = node_value_func(labels);
        node->probabilities = calculation::compute_probabilities(labels);
    } else {
        choose_best_split(node, samples, data);
        vector<size_t> left, right;
        split_samples_by_threshold(node->feature_idx, node->threshold, samples, left, right, data);
        if (left.size() < min_samples_leaf || right.size() < min_samples_leaf) {
            node->is_leaf = true;
            node->h_value = node_value_func(labels);
            node->probabilities = calculation::compute_probabilities(labels);
        } else {
            node->left = construct_node(depth + 1, left, data);
            node->right = construct_node(depth + 1, right, data);
        }
    }
    return node;
}

map<size_t, double> mllib::models::DecisionTree::get_probabilities(const vector<double> &query) const {
    auto node = root;
    while (!node->is_leaf)
        if (query[node->feature_idx] > node->threshold)
            node = node->right;
        else
            node = node->left;
    return node->probabilities;
}

map<size_t, double> mllib::models::DecisionTree::get_probabilities(size_t sample_idx, Data &data) const {
    auto node = root;
    while (!node->is_leaf)
        if (data.get_feature(sample_idx, node->feature_idx) > node->threshold)
            node = node->right;
        else
            node = node->left;
    return node->probabilities;
}

void mllib::models::DecisionTree::get_tree_def(node *node, size_t id, queue<string> &queue) {
    if (node == nullptr)
        return;
    queue.push(node->node_def(id));
    get_tree_def(node->left, id * 2 + 1, queue);
    get_tree_def(node->right, id * 2 + 2, queue);
}

mllib::models::DecisionTree::DecisionTree(size_t max_depth,
                           size_t min_samples_leaf,
                           size_t min_samples_split,
                           const string &criterion,
                           const string &max_features,
                           size_t seed) {
    criterion_func_name = criterion;
    if (criterion == "entropy") {
        criterion_func = calculation::compute_IG;
        node_value_func = calculation::compute_entropy;
        lower_better = false;
    } else {
        criterion_func = calculation::compute_gini;
        node_value_func = calculation::compute_gini_impurity;
        lower_better = true;
    }
    max_feature_func_name = max_features;
    if (max_features == "auto" || max_features == "sqrt")
        get_feature_amount = calculation::_sqrt;
    else if (max_features == "log2")
        get_feature_amount = calculation::_log2;
    else
        get_feature_amount = calculation::_none;
    if (seed == 0) {
        random_device rd;
        seed = rd();
    }
    this->seed = seed;
    this->eng = mt19937_64(seed);
    this->min_samples_leaf = min_samples_leaf;
    this->min_samples_split = min_samples_split;
    this->max_depth = max_depth == 0 ? SIZE_MAX : max_depth;
}

mllib::models::DecisionTree::DecisionTree(mllib::models::DecisionTree &&m) noexcept :
lower_better(m.lower_better),
max_depth(m.max_depth),
min_samples_leaf(m.min_samples_leaf),
min_samples_split(m.min_samples_split),
seed(m.seed),
eng(m.eng),
criterion_func_name(std::move(m.criterion_func_name)),
max_feature_func_name(std::move(m.max_feature_func_name)),
get_feature_amount(std::move(m.get_feature_amount)),
node_value_func(std::move(m.node_value_func)),
criterion_func(std::move(m.criterion_func)),
root(m.root)
{
    m.lower_better = false;
    m.max_depth = 0;
    m.min_samples_leaf = 0;
    m.min_samples_split = 0;
    m.seed = 0;
    m.eng = mt19937_64(m.seed);
    m.root = nullptr;
}

mllib::models::DecisionTree::~DecisionTree() {
    delete root;
}

mllib::models::DecisionTree &mllib::models::DecisionTree::operator=(mllib::models::DecisionTree &&m) noexcept {
    this->lower_better = m.lower_better;
    this->max_depth = m.max_depth;
    this->min_samples_leaf = m.min_samples_leaf;
    this->min_samples_split = m.min_samples_split;
    this->seed = m.seed;
    this->eng = m.eng;
    this->criterion_func_name = std::move(m.criterion_func_name);
    this->max_feature_func_name = std::move(m.max_feature_func_name);
    this->get_feature_amount = std::move(m.get_feature_amount);
    this->node_value_func = std::move(m.node_value_func);
    this->criterion_func = std::move(m.criterion_func);
    this->root = m.root;
    m.lower_better = false;
    m.max_depth = 0;
    m.min_samples_leaf = 0;
    m.min_samples_split = 0;
    m.seed = 0;
    m.eng = mt19937_64(m.seed);
    m.root = nullptr;
    return *this;
}

mllib::models::DecisionTree* mllib::models::DecisionTree::clone() {
    auto *new_tree = new DecisionTree(
            max_depth,
            min_samples_leaf,
            min_samples_split);
    new_tree->seed = seed;
    new_tree->eng = mt19937_64(seed);
    new_tree->node_value_func = node_value_func;
    new_tree->criterion_func = criterion_func;
    new_tree->lower_better = lower_better;
    new_tree->get_feature_amount = get_feature_amount;
    return new_tree;
}

void mllib::models::DecisionTree::fit(Data &data) {
    vector<size_t> samples = data.generate_samples(data.samples_size(), this->eng);
    root = construct_node(0, samples, data);
}

void mllib::models::DecisionTree::predict(Data &queries, vector<size_t> &result) {
    for (size_t i = 0; i < result.size(); i++) {
        map<size_t, double> labels = get_probabilities(i, queries);
        size_t max_label = 0;
        double max_prob = 0.0;
        for (auto &p : labels)
            if (max_prob < p.second) {
                max_label = p.first;
                max_prob = p.second;
            }
        result[i] = max_label;
    }
}

size_t mllib::models::DecisionTree::predict(const vector<double> &query) {
    map<size_t, double> labels = get_probabilities(query);
    size_t max_label = 0;
    double max_prob = 0.0;
    for (auto &p : labels)
        if (max_prob < p.second) {
            max_label = p.first;
            max_prob = p.second;
        }
    return max_label;
}

void mllib::models::DecisionTree::predict_prob(Data &queries, vector<map<size_t, double>> &probabilities) {
    if (queries.samples_size() == 0) return;
    for (auto &dict : probabilities)
        if (!dict.empty()) dict.clear();
    size_t n = queries.samples_size();
    for (size_t i = 0; i < n; i++) {
        map<size_t, double> labels = get_probabilities(i, queries);
        for (auto &p : labels)
            probabilities[i][p.first] += p.second;
    }
}

bool mllib::models::DecisionTree::valid() const {
    return root != nullptr;
}

void mllib::models::DecisionTree::generate_definition_for_graphviz(mllib::models::DecisionTree::node *node, size_t id, size_t parent_id, ostream &out) {
    string leaf = "#32CD32";
    string left = "#66CDAA";
    string rght = "#FFA500";
    if (!node->is_leaf)
        out << id
            << " [label=\"feature_" << node->feature_idx << " <= " << node->threshold << "\\n"
            << "value = " << node->h_value << "\", "
            << "fillcolor=\"" << (id % 2 ? rght : left) << "\"] ;\n";
    else
        out << id
            << " [label=\"value = " << node->h_value << "\\n"
            << "probs = " << utils::map_to_str(node->probabilities) << "\", "
            << "fillcolor=\"" << leaf << "\"] ;\n";
    if (id != parent_id)
        out << parent_id << " -> " << id << " ;\n";
    if (node->is_leaf) return;
    generate_definition_for_graphviz(node->left, id * 2 + 1, id, out);
    generate_definition_for_graphviz(node->right, (id + 1) * 2, id, out);
}

void mllib::models::DecisionTree::draw(const string &path) {
    ofstream out(path, ios::out);
    if (out.is_open()) {
        out << "digraph Tree {\n";
        out << "node [shape=box, style=\"filled\", color=\"black\", fontname=\"helvetica\"] ;\n";
        out << "edge [fontname=\"helvetica\"] ;\n";
        generate_definition_for_graphviz(root, 0, 0, out);
        out << "}";
    }
    out.close();
    size_t found_slash = path.find_last_of("/\\"), found_dot = path.find_last_of('.');
    string dir = path.substr(0, found_slash);
    string file = path.substr(found_slash+1);
    string file_name = path.substr(found_slash+1, found_dot-found_slash-1);
    string cmd = "cd " + dir + "; dot -Tsvg " + file + " -o " + file_name + ".svg;";
    system(cmd.c_str());
}

string mllib::models::DecisionTree::get_saved_def() {
    stringstream ss;
    ss << "max_depth=" << max_depth << ',';
    ss << "min_samples_leaf=" << min_samples_leaf << ',';
    ss << "min_samples_split=" << min_samples_split << ',';
    ss << "seed=" << seed << ',';
    ss << "criterion=" << criterion_func_name << ',';
    ss << "max_features=" << max_feature_func_name << ',';
    ss << "root_id=" << 0 << '\n';
    queue<string> tree_def;
    get_tree_def(root, 0, tree_def);
    while (!tree_def.empty()) {
        ss << tree_def.front();
        tree_def.pop();
    }
    return ss.str();
}

void mllib::models::DecisionTree::save(const string &path, ios_base::openmode mode) {
    ofstream out(path, mode);
    if (out.is_open()) {
        out << get_saved_def();
    }
    out.close();
}

size_t mllib::models::DecisionTree::parse_header(const string &header) {
    // Parse Tree header
    const regex regex_max_depth("max_depth=([0-9]+)");
    const regex regex_min_samples_leaf("min_samples_leaf=([0-9]+)");
    const regex regex_min_samples_split("min_samples_split=([0-9]+)");
    const regex regex_seed("seed=([0-9]+)");
    const regex regex_criterion("criterion=(\\w+)");
    const regex regex_max_features("max_features=(\\w+)");
    const regex regex_root_id("root_id=([0-9]+)");
    smatch match;
    if (regex_search(header, match, regex_max_depth))
        max_depth = stoull(match[1].str());
    if (regex_search(header, match, regex_min_samples_leaf))
        min_samples_leaf = stoull(match[1].str());
    if (regex_search(header, match, regex_min_samples_split))
        min_samples_split = stoull(match[1].str());
    if (regex_search(header, match, regex_seed)) {
        seed = stoull(match[1].str());
        eng = mt19937_64(seed);
    }
    if (regex_search(header, match, regex_criterion)) {
        criterion_func_name = match[1].str();
        if (criterion_func_name == "entropy") {
            criterion_func = calculation::compute_IG;
            node_value_func = calculation::compute_entropy;
            lower_better = false;
        } else {
            criterion_func = calculation::compute_gini;
            node_value_func = calculation::compute_gini_impurity;
            lower_better = true;
        }
    }
    if (regex_search(header, match, regex_max_features)) {
        max_feature_func_name = match[1].str();
        if (max_feature_func_name == "auto" || max_feature_func_name == "sqrt")
            get_feature_amount = calculation::_sqrt;
        else if (max_feature_func_name == "log2")
            get_feature_amount = calculation::_log2;
        else
            get_feature_amount = calculation::_none;
    }
    size_t root_id = 0;
    if (regex_search(header, match, regex_root_id))
        root_id = stoull(match[1].str());
    return root_id;
}

void mllib::models::DecisionTree::load_from_stream(istream &in) {
    string line;
    vector<string> lines(2);
    map<size_t, tuple<node*, size_t, size_t>> nodes;
    getline(in, line, '\n');
    size_t root_id = parse_header(line);
    size_t i = 0;
    while(getline(in, line, '\n')) {
        smatch match;
        if (regex_match(line, match, regex("^\\s*$"))) {
            break;
        }
        lines[i++] = line;
        if (i == 2) {
            i = 0;
            node *new_node = new node();
            size_t id, left_if, right_id;
            std::tie(id, left_if, right_id) = new_node->parse(lines);
            nodes[id] = {new_node, left_if, right_id};
        }
    }
    for (auto &p : nodes) {
        node *n = get<0>(p.second);
        if (!n->is_leaf) {
            n->left = get<0>(nodes[get<1>(p.second)]);
            n->right = get<0>(nodes[get<2>(p.second)]);
        }
    }
    root = get<0>(nodes[root_id]);
}

void mllib::models::DecisionTree::load(const string &path) {
    ifstream in(path);
    if (in.is_open()) {
        load_from_stream(in);
    }
    in.close();
}

size_t mllib::models::DecisionTree::get_seed() const {
    return seed;
}

void mllib::models::RandomForest::norm(map<size_t, double> &result) const {
    for (auto &p : result)
        p.second /= double(n_estimators);
}

mllib::models::RandomForest::RandomForest(size_t n_estimators,
                           size_t n_jobs,
                           size_t max_depth,
                           size_t min_samples_leaf,
                           size_t min_samples_split,
                           const string& criterion,
                           const string& max_features) {
    this->n_estimators = n_estimators;
    this->n_jobs = n_jobs;
    this->max_depth = max_depth;
    this->min_samples_leaf = min_samples_leaf;
    this->min_samples_split = min_samples_split;
    this->criterion = criterion;
    this->max_features = max_features;
    this->trees.reserve(n_estimators);
    this->seeds = new size_t[n_jobs];
    random_device rd;
    for (int i = 0; i < n_jobs; i++)
        seeds[i] = rd();
    seed_seq seed(seeds, seeds+n_jobs);
    seed.generate(seeds, seeds+n_jobs);
}

mllib::models::RandomForest::RandomForest(mllib::models::RandomForest &&m) noexcept :
n_estimators(m.n_estimators),
n_jobs(m.n_jobs),
max_depth(m.max_depth),
min_samples_leaf(m.min_samples_leaf),
min_samples_split(m.min_samples_split),
criterion(std::move(m.criterion)),
max_features(std::move(m.max_features)),
trees(std::move(m.trees)),
seeds(m.seeds)
{
    m.n_estimators = 0;
    m.n_jobs = 0;
    m.max_depth = 0;
    m.min_samples_leaf = 0;
    m.min_samples_split = 0;
    m.criterion.clear();
    m.max_features.clear();
    m.trees.clear();
    m.seeds = nullptr;
}

mllib::models::RandomForest::~RandomForest() {
    delete seeds;
}

mllib::models::RandomForest &mllib::models::RandomForest::operator=(mllib::models::RandomForest &&m) noexcept {
    this->n_estimators = m.n_estimators;
    this->n_jobs = m.n_jobs;
    this->max_depth = m.max_depth;
    this->min_samples_leaf = m.min_samples_leaf;
    this->min_samples_split = m.min_samples_split;
    this->criterion = std::move(m.criterion);
    this->max_features = std::move(m.max_features);
    this->trees = std::move(m.trees);
    this->seeds = m.seeds;
    m.n_estimators = 0;
    m.n_jobs = 0;
    m.max_depth = 0;
    m.min_samples_leaf = 0;
    m.min_samples_split = 0;
    m.criterion.clear();
    m.max_features.clear();
    m.trees.clear();
    m.seeds = nullptr;
    return *this;
}

mllib::models::RandomForest* mllib::models::RandomForest::clone() {
    return new RandomForest(n_estimators,
                            n_jobs,
                            max_depth,
                            min_samples_leaf,
                            min_samples_split,
                            criterion,
                            max_features);
}

void mllib::models::RandomForest::fit(Data &data) {
    if (!trees.empty()) trees.clear();
    ThreadPool pool(n_jobs);
    for (int i = 0; i < n_estimators; i++) {
        pool.add_job([&, i] {
            DecisionTree tree(max_depth,
                              min_samples_leaf,
                              min_samples_split,
                              criterion,
                              max_features,
                              seeds[i % n_jobs]);
            tree.fit(data);
            trees.emplace_back(std::move(tree));
        });
    }
}

size_t mllib::models::RandomForest::predict(const std::vector<double> &query) {
    vector<size_t> results;
    results.reserve(n_estimators);
    {
        ThreadPool pool(n_jobs);
        for (int i = 0; i < n_estimators; i++)
            pool.add_job([&, i] { results.emplace_back(trees[i].predict(query)); });
    }
    map<size_t, size_t> class_cnt;
    for (size_t j = 0; j < n_estimators; j++)
        class_cnt[results[j]]++;
    size_t max_cnt = 0, label = 0;
    for (auto &p : class_cnt)
        if (p.second > max_cnt) {
            max_cnt = p.second;
            label = p.first;
        }
    return label;
}

void mllib::models::RandomForest::predict(Data &queries, vector<size_t> &result) {
    if (queries.samples_size() == 0) return;
    vector<vector<size_t>> results;
    results.reserve(n_estimators);
    size_t n_obj = queries.samples_size();
    {
        ThreadPool pool(n_jobs);
        for (int i = 0; i < n_estimators; i++) {
            pool.add_job([&, i] {
                vector<size_t> tree_result(n_obj);
                trees[i].predict(queries, tree_result);
                results.emplace_back(tree_result);
            });
        }
    }
    map<size_t, size_t> class_cnt;
    for (size_t i = 0; i < n_obj; i++) {
        class_cnt.clear();
        for (size_t j = 0; j < n_estimators; j++)
            class_cnt[results[j][i]]++;
        size_t max_cnt = 0, label = 0;
        for (auto &p : class_cnt)
            if (p.second > max_cnt) {
                max_cnt = p.second;
                label = p.first;
            }
        result[i] = label;
    }
}

void mllib::models::RandomForest::predict_prob(Data &queries, vector<map<size_t, double>> &probabilities) {
    if (queries.samples_size() == 0) return;
    vector<vector<map<size_t, double>>> results;
    for (auto &dict : probabilities)
        if (!dict.empty()) dict.clear();
    size_t n_obj = queries.samples_size();
    {
        ThreadPool pool(n_jobs);
        for (int i = 0; i < n_estimators; i++) {
            pool.add_job([&, i] {
                vector<map<size_t, double>> result(n_obj, map<size_t, double> ());
                trees[i].predict_prob(queries, result);
                results.emplace_back(result);
            });
        }
    }
    size_t n = queries.samples_size();
    for (auto &res : results)
        for (size_t i = 0; i < n; i++)
            for (auto &p : res[i])
                probabilities[i][p.first] += p.second;
    for (auto &dict : probabilities)
        norm(dict);
}

bool mllib::models::RandomForest::valid() const {
    if (trees.size() != n_estimators) return false;
    for (const auto& tree : trees)
        if (!tree.valid()) return false;
    return true;
}

string mllib::models::RandomForest::get_saved_def() {
    stringstream ss;
    ss << "n_estimators=" << n_estimators << ',';
    ss << "n_jobs=" << n_jobs << ',';
    ss << "max_depth=" << max_depth << ',';
    ss << "min_samples_leaf=" << min_samples_leaf << ',';
    ss << "min_samples_split=" << min_samples_split << ',';
    ss << "criterion=" << criterion << ',';
    ss << "max_features=" << max_features << '\n';
    size_t id = 0;
    for (auto &tree : trees) {
        ss << tree.get_saved_def();
        ss << '\n';
    }
    return ss.str();
}

void mllib::models::RandomForest::save(const string &path, ios_base::openmode mode) {
    ofstream out(path, mode);
    if (out.is_open()) {
        out << get_saved_def();
        out << '\n';
    }
    out.close();
}

void mllib::models::RandomForest::parse_header(const string &header) {
    // Parse Tree header
    const regex regex_n_estimators("n_estimators=([0-9]+)");
    const regex regex_n_jobs("n_jobs=([0-9]+)");
    const regex regex_max_depth("max_depth=([0-9]+)");
    const regex regex_min_samples_leaf("min_samples_leaf=([0-9]+)");
    const regex regex_min_samples_split("min_samples_split=([0-9]+)");
    const regex regex_criterion("criterion=(\\w+)");
    const regex regex_max_features("max_features=(\\w+)");
    smatch match;
    if (regex_search(header, match, regex_n_estimators)) {
        n_estimators = stoull(match[1].str());
        trees.reserve(n_estimators);
    }
    if (regex_search(header, match, regex_n_jobs)) {
        n_jobs = stoull(match[1].str());
        seeds = new size_t[n_jobs];
    }
    if (regex_search(header, match, regex_max_depth))
        max_depth = stoull(match[1].str());
    if (regex_search(header, match, regex_min_samples_leaf))
        min_samples_leaf = stoull(match[1].str());
    if (regex_search(header, match, regex_min_samples_split))
        min_samples_split = stoull(match[1].str());
    if (regex_search(header, match, regex_criterion))
        criterion = match[1].str();
    if (regex_search(header, match, regex_max_features))
        max_features = match[1].str();
}

void mllib::models::RandomForest::load_from_stream(istream &in) {
    string line;
    getline(in, line, '\n');
    parse_header(line);
    size_t i = 0;
    int64_t last_pos = in.tellg();
    while(getline(in, line, '\n')) {
        smatch match;
        if (regex_match(line, match, regex("^\\s*$"))) {
            last_pos = in.tellg();
        }
        in.seekg(last_pos);
        i++;
        DecisionTree tree;
        tree.load_from_stream(in);
        trees.emplace_back(std::move(tree));
        if (i == n_estimators)
            break;
    }
    random_device rd;
            for (int id = 0; id < n_jobs; id++) {
        if (id < n_estimators)
            seeds[id] = trees[id].get_seed();
        else
            seeds[id] = rd();
    }
}

void mllib::models::RandomForest::load(const string &path) {
    ifstream in(path);
    if (in.is_open()) {
        load_from_stream(in);
    }
    in.close();
}