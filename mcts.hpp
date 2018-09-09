#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
#include <utility>
#include <utility>

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
  public:
    node(position_type action, Env env, parent_type parent)
      : action_(action), env_(env), parent_(parent), is_terminal_(false)
    {}

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
      return actions.size();
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
};

template <class Env>
class MCTS {
  public:
    using node_type = node<Env>;
    using position_type = typename Env::position_type;
  private:
    node_type root_;
    node_type* cur_;
    int num_nodes_;
  public:
    MCTS(Env env) 
      : root_(node<Env>{std::make_pair(-1, -1), env, nullptr}),
        cur_(&root_),
        num_nodes_(0)
    {}

    node_type* best_child(node_type* parent) {
      int N = parent->get_n();
      auto& children = parent->get_children();
      double max_score = -std::numeric_limits<double>::infinity();
      std::vector<node_type*> best;
      double sum_q_children = 0;
      for (auto& child : children) {
        sum_q_children += child.get_q();
      }
      double avg_q = sum_q_children / children.size();
      for (auto& child : children) {
        int n = child.get_n();
        double UCB1 = 0;
        if (n == 0) {
          UCB1 = std::numeric_limits<double>::infinity();
        }
        double q = child.get_q();
        UCB1 = q / n + avg_q * std::sqrt(std::log(N) / n);
        if (UCB1 > max_score) {
          max_score = UCB1;
          best.clear();
          best.push_back(&child);
        } else if (UCB1 == max_score) {
          best.push_back(&child);
        }
      }
      std::cout << children.size() << std::endl;
      std::cout << "best_size: " << best.size() << std::endl;
      return best[std::rand() % best.size()];
    }

    node_type* tree_policy(node_type* cur) {
      while (cur->has_children()) {
        cur = best_child(cur);
      }
      int num_children_added = cur->expand();
      num_nodes_ += num_children_added;

      int rand_child_idx = std::rand() % num_children_added;

      if (num_children_added) {
        return &(cur->get_children()[rand_child_idx]);
      }
    }

    double default_policy(node_type* cur) {
      Env env(cur->get_env());
      while (!env.is_game_over()) {
        env.render();
        std::vector<position_type> moves = env.get_possible_moves();
        int rand_move_idx = std::rand() % moves.size();
        position_type pos = moves[rand_move_idx];
        std::cout << pos.first << " " << pos.second << std::endl;
        env.step(pos);
      }
      return env.get_total_reward();
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
          std::cout << "A" << std::endl;
          node_type* leaf = tree_policy(cur_);
          std::cout << "B" << std::endl;
          double reward = default_policy(leaf);
          std::cout << "C" << std::endl;
          backprop(leaf, reward);
        }
        cur_ = best_child(cur_);
      }
      cur_->get_env().render();
      return cur_->get_reward();
    }
};