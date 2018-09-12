#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
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
    using move_to_adj_map = std::map<position_type, std::vector<position_type>>;
  private:
    int num_colors_ = 5;
    int total_reward_ = 0;
    board_type board_;
    move_to_adj_map moves_and_connected_ = {};
    int width = 7;
  public:
    same_game_env(int random_seed = 32) {
      std::srand(random_seed);
      for (int x = 0; x < width; x++) {
        board_.push_back({});
        for (int y = 0; y < width; y++) {
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
      for (int y = width - 1; y >= 0; y--) {
        std::cout << "| ";
        for (int x = 0; x < width; x++) {
          short tile = board_[x][y];
          std::cout << "\033[9" << tile << "m" << tile << "\033[0m ";
        }
        std::cout << "|" << std::endl;
      }
      std::cout << "*********************************" << std::endl;
    }

    bool is_game_over() const {
      for (int x = 0; x < width; x++) {
        for (int y = 0; y < width; y++) {
          short tile = board_[x][y];
          if (tile == 0) {
            break;
          }
          if (y + 1 < width && board_[x][y+1] == tile) {
            return false;
          }
          if (x + 1 < width && board_[x+1][y] == tile) {
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

    std::set<position_type> find_adj(position_type pos, std::set<position_type>& adjacent) {
      adjacent.insert(pos);
      int x = pos.first;
      int y = pos.second;

      auto cur = board_[x][y];

      position_type up(x, y+1);
      position_type right(x+1, y);
      position_type down(x, y-1);
      position_type left(x-1, y);

      if (!adjacent.count(up) && up.second < width && board_[up.first][up.second] == cur) {
        find_adj(up, adjacent);
      } 

      if (!adjacent.count(right) && right.first < width && board_[right.first][right.second] == cur) {
        find_adj(right, adjacent);
      }

      if (!adjacent.count(down) && down.second >= 0 && board_[down.first][down.second] == cur) {
        find_adj(down, adjacent);
      }

      if (!adjacent.count(left) && left.first >= 0 && board_[left.first][left.second] == cur) {
        find_adj(left, adjacent);
      } 

      return adjacent;
    }


    std::vector<position_type> get_possible_moves() {
      moves_and_connected_.clear();
      move_to_adj_map adj = {};
      std::set<position_type> covered;
      std::vector<position_type> moves;
      for (int x = 0; x < width; x++) {
        for (int y = 0; y < width; y++) {
          short tile = board_[x][y];
          position_type cur(x, y);
          if (covered.count(cur)) {
            continue;
          } 

          std::set<position_type> adjacent;
          find_adj(cur, adjacent);

          if (adjacent.size() > 1) {
            if (!moves_and_connected_.count(cur)) {
              moves_and_connected_[cur] = {};
            }
            moves.push_back(cur);
            covered.insert(adjacent.begin(), adjacent.end());
            adjacent.erase(adjacent.find(std::make_pair(x,y)));
            std::vector<position_type> connected;
            std::copy(connected.begin(), connected.end(), 
              std::back_inserter(moves_and_connected_[cur]));
          }
        }
      }

      return moves;
    }

    void collapse() {
      for (int x = 0; x < width; x++) {
        std::vector<short>& col = board_[x];
        for (auto it = col.begin(); it != col.end();) {
          if (*it == 0) {
            it = col.erase(it);
          } else {
            ++it;
          }
        }
        int num_to_fill = width - col.size();
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
      
      int num_to_fill = width - board_.size();
      std::vector<short> to_fill(width, 0);
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
