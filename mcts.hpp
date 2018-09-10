#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>
#include <utility>
#include <vector>

static std::size_t node_id = 0;

template <class Env>
class node {
  public:
    using parent_type = node<Env>*;
    using children_type = std::vector<node<Env>>;
    using position_type = typename Env::position_type;
  private:
    parent_type parent_;
    children_type children_;
    position_type action_;
    bool is_terminal_;
    Env env_;
    double q_;
    int n_;
    int depth_;
    std::size_t node_id_;
  public:
    node(position_type action, Env env, parent_type parent)
      : action_(action), env_(env), parent_(parent), is_terminal_(false),
        q_(0), n_(0), depth_(parent ? parent->depth_ + 1 : 0), node_id_(node_id++)
    {}

    std::string to_gv() const {
      std::stringstream ss;
      ss << "  " << node_id_ << " [label=\"(" << action_.first << ", " 
        << action_.second << ")\n n: " << n_ << " q: " << q_ << ")\"]" << std::endl;
      for (auto& child : children_) {
        ss << "  " << node_id_ << " -- " << child.node_id_ << std::endl;
        ss << child.to_gv() << std::endl;
      } 
      return ss.str();
    }

    int expand() {
      if (this->has_children()) {
        std::cerr << "Tried to expand a node which was already expanded" << std::endl;
        return 0;
      }
      std::vector<position_type> actions = env_.get_possible_moves();
      if (!actions.size()) {
        is_terminal_ = true;
        return 0;
      }
      for (auto& action : actions) {
        Env env(env_);
        env.step(action);
        node<Env> child(action, env, this);
        children_.push_back(child);
      }
      return children_.size();
    }

    Env& get_env() {
      return env_;
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

    int get_reward() const {
      return env_.get_total_reward();
    }

    parent_type get_parent() {
      return parent_;
    }

    children_type& get_children() {
      return children_;
    }

    bool has_children() const {
      return !children_.empty();  
    }

    bool is_terminal() const {
      return is_terminal_;
    }

    position_type get_action() const {
      return action_;
    }

    int get_depth() const {
      return depth_;
    }

    std::size_t get_id() const {
      return node_id_;
    }
};

template <class Env>
class MCTS {
  public:
    using node_type = node<Env>;
    using position_type = typename Env::position_type;
  private:
    node_type root_;
    node_type* cur_;
    std::size_t num_nodes_;
  public:
    MCTS(Env env) 
      : root_(node<Env>{std::make_pair(-1, -1), env, nullptr}),
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

    node_type* best_child(node_type* parent) {
      int N = parent->get_n();
      auto& children = parent->get_children();
      double max_score = -std::numeric_limits<double>::infinity();
      std::vector<node_type*> best;
      double sum_q_children = 0;
      for (auto& child : children) {
        sum_q_children += child.get_q();
      }
      double avg_q = sum_q_children / N;
      for (auto& child : children) {
        int n = child.get_n();
        double UCB1 = 0;
        if (n == 0) {
          UCB1 = std::numeric_limits<double>::infinity();
        } else {
          double q = child.get_q();
          UCB1 = q / n + std::sqrt(2) * std::sqrt(std::log(N) / n);
        }
        if (UCB1 > max_score) {
          max_score = UCB1;
          best.clear();
          best.push_back(&child);
        } else if (UCB1 == max_score) {
          best.push_back(&child);
        }
      }
      return best[std::rand() % best.size()];
    }

    node_type* tree_policy(node_type* cur) {
      while (cur->has_children()) {
        cur = best_child(cur);
      }

      if (cur->get_n() == 0) {
        return cur;
      }

      int num_children_added = cur->expand();
      num_nodes_ += num_children_added;
      if (num_children_added) {
        int rand_child_idx = std::rand() % num_children_added;
        return &(cur->get_children()[rand_child_idx]);
      } else {
        return cur;
      }
    }

    double default_policy(node_type* cur) {
      Env env(cur->get_env());
      while (!env.is_game_over()) {
        std::vector<position_type> moves = env.get_possible_moves();
        int rand_move_idx = std::rand() % moves.size();
        position_type pos = moves[rand_move_idx];
        env.step(pos);
      }
      if (env.get_total_reward() > -900) {
        return 1;
      } else {
        return 0;
      }
    }

    void backprop(node_type* cur, double q) {
      while (cur) {
        cur->set_q(cur->get_q() + q);
        cur->set_n(cur->get_n() + 1);
        cur = cur->get_parent();
      }
    }

    double search(int iterations) {
      while (!cur_->is_terminal()) {
        cur_->get_env().render();
        for (int i = 0; i < iterations; i++) {
          node_type* leaf = tree_policy(cur_);
          double reward = default_policy(leaf);
          backprop(leaf, reward);
        }
        cur_ = best_child(cur_);
        std::cout << "move made: (" << cur_->get_action().first << ", " 
          << cur_->get_action().second << ")" << std::endl;
        std::cout << "# nodes: " << num_nodes_ << std::endl;
      }
      cur_->get_env().render();
      return cur_->get_reward();
    }
};
