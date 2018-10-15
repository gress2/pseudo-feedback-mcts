#pragma once

#include <algorithm>
#include <cstdio>
#include <iostream> 
#include <fstream>
#include <queue> 
#include <random>
#include <set>
#include <stack>

static long int node_id = 0;

template <class T>
class node {
  private:
    T env_;
    long int id_;
    long int parent_id_;
    std::vector<long int> seq_;
  public:
    node(T env)
      : env_(env),
        id_(node_id++),
        parent_id_(-1),
        seq_({id_})
    {}

    node(const node& other)
      : env_(other.env_),
        id_(node_id++),
        parent_id_(other.id_),
        seq_(other.seq_)
    {
      seq_.push_back(id_); 
    }

    std::vector<typename T::move_type> get_possible_moves() {
      return env_.get_possible_moves();
    }

    void step(typename T::move_type move) {
      env_.step(move);
    }

    long int get_id() const {
      return id_;
    }

    long int get_parent_id() const {
      return parent_id_;
    }

    double get_reward() {
      return env_.get_total_reward();
    }

    double get_curr_reward() {
      return env_.get_curr_reward();
    }

    double get_depth() const {
      return env_.get_num_steps();
    }

    const T& get_env() const {
      return env_;
    }

    const std::vector<long int>& get_seq() const {
      return seq_;
    }
};

struct metadata {
  int depth;
  int parent_arity;
  double reward;
};

template <class T>
auto first(T& ds) -> decltype(ds.front()) {
  return ds.front();
}

template <class T>
auto first(T& ds) -> decltype(ds.top()) {
  return ds.top();
}

template <class T>
std::pair<double, double> get_mean_and_var(T& container) {
  double mean = 0;
  int n = container.size();

  for (auto& elem : container) {
    mean += elem;
  } 

  mean /= n;

  double var = 0;
  for (auto& elem : container) {
    var += (elem - mean) * (elem - mean);    
  }

  var /= n;

  return std::make_pair(mean, var);
}

struct info {
  std::vector<double> rewards;
  int depth;
};

template <class T, template <class...> class Container>
void search(T& env, int num_rounds, std::size_t max_iters) {
  std::random_device rd;
  std::mt19937 twister(rd());
  
  std::string ti("tree_info");
  std::remove(ti.c_str());

  std::vector<info> infos;

  for (int i = 0; i < num_rounds; i++) {
    std::cout << "round (" << i << "/" << num_rounds << ")" << std::endl;
    Container<node<T>> s;
    node<T> root(env);
    s.push(root);
    std::size_t iters = 0;

    while (!s.empty() && iters < max_iters) {
      node<T> v = first(s);
      s.pop();

      auto moves = v.get_possible_moves();
      std::shuffle(moves.begin(), moves.end(), twister);
      
      std::size_t num_moves = moves.size();
      
      info inf;

      std::vector<double> child_rewards;
      inf.depth = v.get_depth();

      for (auto& move : moves) {
        node<T> child(v);
        child.step(move);
        s.push(child);
        child_rewards.push_back(child.get_curr_reward());
        iters++;
      }

      inf.rewards = child_rewards;
      infos.push_back(inf);
    }    
  }

  std::ofstream ti_f(ti);
  for (auto& inf : infos) {
    auto it = inf.rewards.begin();
    if (it != inf.rewards.end()) {
      ti_f << *it;
    }
    while (it != inf.rewards.end()) {
      ti_f << "," << *it;
      it++;
    }
    ti_f << ";";
    ti_f << inf.depth << std::endl;
  }
  ti_f.close();
}

template <class T>
void bfs(T& env, int num_rounds = 1, std::size_t max_iters = 1e6) {
  search<T, std::queue>(env, num_rounds, max_iters);
}

template <class T>
void dfs(T& env, int num_rounds = 1, std::size_t max_iters = 1e6) {
  search<T, std::stack>(env, num_rounds, max_iters);
}

