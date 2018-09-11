#pragma once

#include <chrono>
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
    Env env_;
    std::vector<position_type> moves_;
    bool moves_found_;
    bool is_terminal_;
    double q_;
    int n_;
    int depth_;
    std::size_t node_id_;
  public:
    node(position_type action, Env env, parent_type parent)
      : action_(action), env_(env), parent_(parent), is_terminal_(false),
        q_(0), n_(0), depth_(parent ? parent->depth_ + 1 : 0), node_id_(node_id++),
        moves_({}), moves_found_(false)
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

    node<Env>* expand() {
      if (!moves_found_) {
        moves_ = env_.get_possible_moves();
        moves_found_ = true;
      } 

      if (moves_.empty()) {
        if (children_.empty()) {
          is_terminal_ = true;
          return nullptr;
        } else {
          return nullptr; 
        }
      }

      auto move = moves_.back();
      moves_.pop_back();
      Env env(env_);
      env.step(move);
      node<Env> child(move, env, this);
      children_.push_back(child);
      return &(children_.back());
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

    void make_move(node_type* move) {
      root_ = node_type(move->get_action(), move->get_env(), nullptr);
      cur_ = &root_; 
      num_nodes_ = 0;
    }

    std::string to_gv() const {
      std::stringstream ss;
      ss << "graph {" << std::endl;
      ss << root_.to_gv();
      ss << "}" << std::endl;
      return ss.str();
    }

    double UCB1(node_type* cur) {
      int n = cur->get_n();
      if (n == 0) {
        return std::numeric_limits<double>::infinity(); 
      }
      return cur->get_q() / n + 
        .5 * std::sqrt(std::log(cur->get_parent()->get_n()) / n);
    }

    node_type* best_child(node_type* parent) {
      auto& children = parent->get_children();
      double max_score = -std::numeric_limits<double>::infinity();
      std::vector<node_type*> best;
      for (auto& child : children) {
        double ucb1 = UCB1(&child);
        if (ucb1 > max_score) {
          max_score = ucb1;
          best.clear();
          best.push_back(&child);
        } else if (ucb1 == max_score) {
          best.push_back(&child);
        }
      }
      return best[std::rand() % best.size()];
    }

    node_type* tree_policy(node_type* cur) {
      while (!cur->is_terminal()) {
        node_type* exp = cur->expand();
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

    double default_policy(node_type* cur) {
      Env env(cur->get_env());
      while (!env.is_game_over()) {
        std::vector<position_type> moves = env.get_possible_moves();
        int rand_move_idx = std::rand() % moves.size();
        position_type pos = moves[rand_move_idx];
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
      std::size_t num_iterations = 1e6;
      while (!cur_->is_terminal()) {
        std::chrono::time_point<std::chrono::steady_clock> start = 
          std::chrono::steady_clock::now();

        cur_->get_env().render();

        for (std::size_t i = 0; i < num_iterations; i++) {
          node_type* leaf = tree_policy(cur_);
          double reward = default_policy(leaf);
          backprop(leaf, reward);
        }
                
        std::size_t num_nodes_created = num_nodes_;

        if (cur_->has_children()) {
          make_move(best_child(cur_));
          std::cout << "move made: (" << cur_->get_action().first << ", " 
            << cur_->get_action().second << ")" << std::endl;
        }

        std::chrono::time_point<std::chrono::steady_clock> end = 
          std::chrono::steady_clock::now();
        std::chrono::seconds diff = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        std::cout << "move took: " << diff.count() << "s" << std::endl;
        std::cout << "created " << num_nodes_created << " nodes in " << num_iterations << " iterations" << std::endl;
      }
      return cur_->get_reward();
    }
};
