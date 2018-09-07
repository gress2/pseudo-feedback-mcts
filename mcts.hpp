#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>
#include <vector>
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
    Env* env_;
    double q_;
    int n_;
  public:
    node(position_type action, Env* env, parent_type parent)
      : action_(action), env_(env), parent_(parent)
    {}

    int expand() {
      if (this->has_children()) {
        std::cerr << "Tried to expand a node which was already expanded" << std::endl;
        return 0;
      }
      std::vector<position_type> actions = env_->get_possible_actions(false);
      if (!actions.size()) {
        is_terminal_ = true;
        return 0;
      }
      for (auto& action : actions) {
        Env env(*env_);
        env.step(action);
        node<Env> child(action, &env, this);
        children_.push_back(child);
      }
      return actions.size();
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
      return env_->get_reward();
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
  private:
    node_type root_;
    node_type* cur_;
    int num_nodes_;
  public:
    MCTS(Env* env) 
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
      return best[std::rand() % best.size() + 1];
    }

    node_type* tree_policy(node_type* cur) {
      while (cur->has_children()) {
        cur = best_child(cur);
      }
      int num_children_added = cur->expand();
      num_nodes_ += num_children_added;
      if (num_children_added) {
        return cur->children()[std::rand() % num_children_added + 1];
      }
    }
};