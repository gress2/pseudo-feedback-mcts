#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

struct pair_hash {
  template <class T1, class T2>
  std::size_t operator() (const std::pair<T1, T2>& pair) const {
    auto h1 = std::hash<T1>{}(pair.first);
    auto h2 = std::hash<T2>{}(pair.second);
    return h1 ^ h2;
  }
};

class same_game_env {
  public:
    using board_type = std::vector<std::vector<short>>;
    using position_type = std::pair<short, short>;
    using move_to_adj_map = std::unordered_map<position_type, std::vector<position_type>, pair_hash>;
  private:
    const int num_colors_ = 5;
    int total_reward_ = 0;
    board_type board_;
    move_to_adj_map moves_and_connected_ = {};
  public:
    same_game_env(int random_seed = 32) {
      std::srand(random_seed);
      for (int x = 0; x < 15; x++) {
        board_.push_back({});
        for (int y = 0; y < 15; y++) {
          board_[x].push_back((std::rand() % 5) + 1);
        }
      }
      get_possible_moves();
    }
    
    same_game_env(const same_game_env& other) 
      : board_(other.board_), total_reward_(other.total_reward_),
        moves_and_connected_(other.moves_and_connected_)
    {}

    double get_total_reward() const {
      return total_reward_;
    }
      
    void render() {
      std::cout << "*********************************" << std::endl;
      for (int y = 14; y >= 0; y--) {
        std::cout << "| ";
        for (int x = 0; x < 15; x++) {
          short tile = board_[x][y];
          std::cout << "\033[9" << tile << "m" << tile << "\033[0m ";
        }
        std::cout << "|" << std::endl;
      }
      std::cout << "*********************************" << std::endl;
    }

    bool is_game_over() const {
      for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
          short tile = board_[x][y];
          if (tile == 0) {
            break;
          }
          if (y + 1 < 15 && board_[x][y+1] == tile) {
            return false;
          }
          if (x + 1 < 15 && board_[x+1][y] == tile) {
            return false;
          }
        }
      }
      return true;
    }

    bool is_board_empty() const {
      for (auto it = board_.cbegin(); it != board_.cend(); ++it) {
        const std::vector<short>& col = *it;
        bool all_zero = std::all_of(col.begin(), col.end(), 
          [](short i) { return i == 0; });
        if (!all_zero) {
          return false;
        }
      }
      return true;
    }

    std::set<position_type> aggregate(const position_type& cur, move_to_adj_map& adj) {
      std::set<position_type> connected;
      std::vector<position_type>& adjacents = adj[cur]; 

      for (auto it = adjacents.begin(); it != adjacents.end(); ++it) {
        connected.insert(*it);
        std::set<position_type> tmp = aggregate(*it, adj);
        connected.insert(tmp.begin(), tmp.end());
      }
      return connected;
    }

    std::vector<position_type> get_possible_moves() {
      moves_and_connected_.clear();
      move_to_adj_map adj = {};
      for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
          short tile = board_[x][y];
          if (tile == 0) {
            break;
          }
          position_type cur({x, y});

          if (y + 1 < 15 && board_[x][y+1] == tile) {
            if (!adj.count(cur)) {
              adj[cur] = {};
            }
            adj[cur].push_back(std::make_pair(x, y+1));
          }
          if (x + 1 < 15 && board_[x+1][y] == tile) {
            if (!adj.count(cur)) {
              adj[cur] = {};
            }
            adj[cur].push_back(std::make_pair(x+1, y));
          }
        }
      }
      std::vector<position_type> moves;
      std::set<position_type> covered;

      for (auto it = adj.begin(); it != adj.end(); ++it) {
        position_type pos = it->first;
        if (!covered.count(pos)) {
          moves.push_back(pos);
          std::set<position_type> connected = aggregate(pos, adj);
          covered.insert(pos);
          covered.insert(connected.begin(), connected.end());
          std::copy(connected.begin(), connected.end(), 
            std::back_inserter(moves_and_connected_[pos]));
        }
      }

      return moves;
    }

    void collapse() {
      for (int x = 0; x < 15; x++) {
        std::vector<short>& col = board_[x];
        for (auto it = col.begin(); it != col.end();) {
          if (*it == 0) {
            it = col.erase(it);
          } else {
            ++it;
          }
        }
        int num_to_fill = 15 - col.size();
        col.insert(col.end(), num_to_fill, 0);
      }

      for (auto it = board_.begin(); it != board_.end();) {
        std::vector<short>& col = *it;
        bool all_zero = std::all_of(col.begin(), col.end(), 
          [](short i) { return i == 0; });
        if (all_zero) {
          it = board_.erase(it);
        } else {
          ++it;
        }
      }
      
      int num_to_fill = 15 - board_.size();
      std::vector<short> to_fill(15, 0);
      board_.insert(board_.end(), num_to_fill, to_fill);
    }

    void step(position_type pos) {
      assert(moves_and_connected_.count(pos));
      const std::vector<position_type>& adj_tiles = moves_and_connected_[pos];

      board_[pos.first][pos.second] = 0;
      for (auto it = adj_tiles.begin(); it != adj_tiles.end(); ++it) {
        const position_type& tile = *it;
        board_[tile.first][tile.second] = 0;
      }

      int num_removed = adj_tiles.size();
      int reward = std::pow(num_removed - 2, 2);
      collapse();
      bool game_over = is_game_over();

      if (game_over) {
        if (is_board_empty()) {
          reward += 1000;
        } else {
          reward -= 1000;
        }
      }

      get_possible_moves();
      total_reward_ += reward;
    }
};
