#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <vector>

#define P1 0.4
#define P2 0.2

#define N1 4
#define N2 16

static std::size_t node_id = 0;
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(0.0, 1.0);
static std::negative_binomial_distribution<> nb(20, 0.75);
static std::normal_distribution<> norm{100,20};

static std::vector<int> terminal_depths;

template <class T>
double ucb1(const T* cur) {
  int n = cur->get_n();
  if (n == 0) {
    return std::numeric_limits<double>::max();
  }

  double c = std::sqrt(2);
  double q_bar = cur->get_q() / n;
  int parent_n = cur->get_parent() ? cur->get_parent()->get_n() : n;

  return q_bar + c * std::sqrt(std::log(parent_n) / n);
}

class node {
  public:

  private:
    node* parent_;
    std::vector<node> children_;
    bool is_terminal_;
    double q_;
    int n_;
    int depth_;
    int failures_;
    int max_children_;
    std::size_t node_id_;
    double reward_;
  public:
    node(node* parent = nullptr)
      : parent_(parent),
        failures_(parent ? parent->get_failures() : 0),
        depth_(parent ? parent->get_depth() + 1 : 1),
        q_(0), n_(0), node_id_(node_id++)
    {

      bool d1 = dis(gen) > .5;
      double p_thresh = d1 ? P1 : P2;
      double n_thresh = d1 ? N1 : N2;

      if (dis(gen) < p_thresh) {
        failures_++;
      }

      if (failures_ >= n_thresh) {
        max_children_ = 0;
        is_terminal_ = true;
        terminal_depths.push_back(depth_);
      } else {
        max_children_ = std::max(1, nb(gen));
        is_terminal_ = false;
      }

      reward_ = norm(gen);
    }

    std::string to_gv() const {
      std::stringstream ss;
      /*
      ss << " " << node_id_ << " [label=\"r:" << reward_ << std::endl
        << "n:" << n_ << std::endl << "q:" << q_ << "\"]" << std::endl;
      */

      ss << " " << node_id_ << " [label=\"ucb1:" << ucb1(this) << std::endl
        << "max: " << max_children_ << std::endl
        << "n:" << n_ << "\"]" << std::endl;

      for (auto& child : children_) {
        ss << " " << node_id_ << " -- " << child.node_id_ << std::endl;
        ss << child.to_gv() << std::endl;
      }
      return ss.str();
    }

    node* expand() {
      if (num_children() < max_children_) {
        node child(this);
        children_.push_back(child);
        return &(children_.back());
      } else {
        return nullptr;
      }
    }

    int get_failures() const {
      return failures_;
    }

    int get_depth() const {
      return depth_;
    }

    double get_q() const {
      return q_;
    }

    void set_q(double value) {
      q_ = value;
    }

    int get_n() const {
      return n_;
    }

    void set_n(int value) {
      n_ = value;
    }

    std::size_t num_children() const {
      return children_.size();
    }

    int max_children() const {
      return max_children_;
    }

    double get_reward() const {
      return reward_;
    }

    node* get_parent() const {
      return parent_;
    }

    std::vector<node>& get_children() {
      return children_;
    }

    bool is_terminal() const {
      return is_terminal_;
    }

    bool has_children() const {
      return !children_.empty();
    }

};

class mcts {
  public:

  private:
    node root_;
    node* cur_;
    std::size_t num_nodes_;
  public:
    mcts()
      : root_(node{}),
        cur_(&root_),
        num_nodes_(0)
    {
      srand(time(NULL));
    }

    std::string to_gv() const {
      std::stringstream ss;
      ss << "graph {" << std::endl;
      ss << root_.to_gv();
      ss << "}" << std::endl;
      return ss.str();
    }

    std::vector<int>& get_terminal_depths() {
      return terminal_depths;
    }

    node* best_child(node* parent) {
      auto& children = parent->get_children();

      // epsilon greedy
      if (dis(gen) < .3) {
        return &(children[std::rand() % children.size()]);
      }

      double max_score = -std::numeric_limits<double>::max();
      std::vector<node*> best;
      for (auto& child : children) {
        double ucb = 0;
        if (child.get_n() < 5) {
          ucb = std::numeric_limits<double>::max();
        } else {
          ucb = ucb1(&child);
        }
        if (ucb > max_score) {
          max_score = ucb;
          best.clear();
          best.push_back(&child);
        } else if (ucb == max_score) {
          best.push_back(&child);
        }
      }
      return best[std::rand() % best.size()];
    }

    node* tree_policy(node* cur) {
      while (!cur->is_terminal()) {
        node* exp = cur->expand();
        if (exp) {
          num_nodes_++;
          return exp;
        }
        if (cur->has_children()) {
          cur = best_child(cur);
        }
      }
      return cur;
    }

    double default_policy(node* base) {
      node* cur = base;
      bool did_expand = false;

      while (cur && !cur->is_terminal()) {
        cur = cur->expand();
        did_expand = true;
      }

      double reward = cur->get_reward();

      if (did_expand) {
        base->get_children().pop_back();
      }

      return reward;
    }

    void backprop(node* cur, double q) {
      while (cur) {
        cur->set_q(cur->get_q() + q);
        cur->set_n(cur->get_n() + 1);
        cur = cur->get_parent();
      }
    }


    void search_aio(int iterations) {
      for (std::size_t i = 0; i < iterations; i++) {
        node* leaf = tree_policy(cur_);
        double reward = default_policy(leaf);
        backprop(leaf, reward);
      }
      std::cout << "num_node: " << num_nodes_ << std::endl;
    }

};
