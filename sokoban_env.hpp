#pragma once
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string> 
#include <utility>
#include <vector>

class sokoban_env {
  public:
    enum direction { null, up, right, down, left, UP, RIGHT, DOWN, LEFT }; 
    using board_type = std::vector<std::vector<char>>;
    using position_type = std::pair<short, short>;
    using move_type = direction;
    using dist_table = std::map<std::pair<position_type, position_type>, int>;
  private: 
    board_type board_;
    position_type human_pos_;
    std::vector<position_type> box_positions_;
    std::vector<position_type> goal_positions_;
    int num_moves_ = 0;
    std::vector<move_type> seq_;
    bool is_game_over_ = false;
    static dist_table dist_;
  public:
    sokoban_env(std::string board_file_str) {
      std::ifstream board_file(board_file_str);
      std::string line;

      board_type tmp;

      while (std::getline(board_file, line)) {
        board_.push_back(std::vector<char>(line.begin(), line.end()));
      }

      for (short i = 0; i < board_.size(); i++) {
        for (short j = 0; j < board_[i].size(); j++) {
          char tile = board_[i][j];
          if (tile == '@') {
            human_pos_ = std::make_pair(i, j);
          } else if (tile == '$') {
            box_positions_.push_back(std::make_pair(i, j));
          } else if (tile == '.') {
            goal_positions_.push_back(std::make_pair(i, j));
          }
        }
      }
    }

    sokoban_env(const sokoban_env& other) 
      : board_(other.board_), 
        human_pos_(other.human_pos_),
        box_positions_(other.box_positions_),
        goal_positions_(other.goal_positions_),
        num_moves_(other.num_moves_),
        seq_(other.seq_),
        is_game_over_(other.is_game_over_)
    {
    }

    std::string get_dir_str(direction dir) const {
      if (dir == direction::null) {
        return "null";
      } else if (dir == direction::up) {
        return "up";
      } else if (dir == direction::right) {
        return "right";
      } else if (dir == direction::down) {
        return "down";
      } else if (dir == direction::left) {
        return "left";
      } else if (dir == direction::UP) {
        return "UP";
      } else if (dir == direction::RIGHT) {
        return "RIGHT";
      } else if (dir == direction::DOWN) {
        return "DOWN";
      } else {
        return "LEFT";
      } 
    } 

    int get_total_reward() {
      return get_reward();;
    }

    position_type get_shifted_position(position_type pos, direction dir) {
      if (dir == direction::up || dir == direction::UP) {
        return std::make_pair(pos.first - 1, pos.second);
      } else if (dir == direction::right || dir == direction::RIGHT) {
        return std::make_pair(pos.first, pos.second + 1);
      } else if (dir == direction::down || dir == direction::DOWN) {
        return std::make_pair(pos.first + 1, pos.second);
      } else {
        return std::make_pair(pos.first, pos.second - 1);
      }
    }

    bool is_goal_or_free(char tile) {
      return tile == ' ' || tile == '.';
    }

    bool is_valid_direction(direction dir) {
      position_type pos = get_shifted_position(human_pos_, dir);
      char tile = board_[pos.first][pos.second];

      if (is_goal_or_free(tile)) {
        return true;
      }

      if (tile == '$') {
        position_type shifted = get_shifted_position(pos, dir);
        char shifted_tile = board_[shifted.first][shifted.second];
        return is_goal_or_free(shifted_tile);
      }

      return false;
    }

    bool is_block_push(direction dir) {
      position_type pos = get_shifted_position(human_pos_, dir);
      return board_[pos.first][pos.second] == '$';
    }

    direction uppercase(direction dir) {
      if (dir == direction::up) {
        return direction::UP;
      } else if (dir == direction::right) {
        return direction::RIGHT;
      } else if (dir == direction::down) {
        return direction::DOWN;
      } else {
        return direction::LEFT;
      }
    }

    std::vector<direction> get_possible_moves() {
      if (is_game_over_) {
        return {};
      }

      std::vector<direction> moves;

      std::vector<direction> dirs = {
        direction::up,
        direction::right,
        direction::down,
        direction::left
      };

      for (auto dir : dirs) {
        if (is_valid_direction(dir)) {
          if (is_block_push(dir)) {
            moves.push_back(uppercase(dir));
          } else {
            moves.push_back(dir);
          }
        }
      }

      return moves;
    }

    bool is_box_stuck(position_type pos) {
      if (std::find(goal_positions_.begin(), goal_positions_.end(), pos) != goal_positions_.end()) {
        return false;
      }

      std::vector<position_type> sides = {
        get_shifted_position(pos, direction::left),
        get_shifted_position(pos, direction::up),
        get_shifted_position(pos, direction::right),
        get_shifted_position(pos, direction::down),
        get_shifted_position(pos, direction::left)
      }; 

      bool prev_blocked = false;
      for (auto it = sides.begin(); it != sides.end(); ++it) {
        char tile = board_[it->first][it->second];
        if (tile == '#') {
          if (prev_blocked) {
            return true;
          } else {
            prev_blocked = true;
          }
        } else {
          prev_blocked = false;
        }
      }
      return false;
    }

    bool is_any_box_stuck() {
      for (auto it = box_positions_.begin(); it != box_positions_.end(); ++it) {
        if (is_box_stuck(*it)) {
          return true;
        }     
      }
      return false;
    }

    bool is_game_over() {
      return get_num_correct_boxes() == box_positions_.size() || 
          is_any_box_stuck() || num_moves_ > 75;
    }

    int get_num_correct_boxes() {
      int num_correct = 0;
      for (auto it = box_positions_.begin(); it != box_positions_.end(); ++it) {
        if (std::find(goal_positions_.begin(), goal_positions_.end(), *it) != 
            goal_positions_.end()) {
          num_correct++;
        } 
      }
      return num_correct;
    }

    std::vector<move_type> get_seq() const {
      return seq_;
    }

    void print_seq() const {
      std::cout << "seq: ";
      for (auto it = seq_.begin(); it != seq_.end(); ++it) {
        std::cout << get_dir_str(*it) << " "; 
      }
      std::cout << std::endl;
    }

    std::string pair_to_str(position_type pos) {
      std::stringstream ss;
      ss << "(" << pos.first << "," << pos.second << ")";
      return ss.str(); 
    }

    struct MinElemQueue {
      private:
        std::map<position_type, int>& dm_;
      public:
        MinElemQueue(std::map<position_type, int>& dist) 
          : dm_(dist) 
        {}

        bool operator()(const position_type& l, const position_type& r) const { 
          return dm_[l] < dm_[r]; 
        }
    };


    int shortest_distance_path(position_type src, position_type dest) {
      std::pair<position_type, position_type> src_dest_pair(src, dest);
      if (dist_.count(src_dest_pair)) {
        return dist_[src_dest_pair];
      }

      std::map<position_type, int> dist;
      std::deque<position_type> Q;

      for (int y = 0; y < board_.size(); y++) {
        for (int x = 0; x < board_[0].size(); x++) {
          position_type tmp(y, x);
          dist[tmp] = 9999;
          Q.push_back(tmp);
        }
      }

      dist[src] = 0;

      MinElemQueue meq(dist);
      while (!Q.empty()) {
        auto min_elem_it = std::min_element(Q.begin(), Q.end(), meq);
        position_type u = *min_elem_it;

        std::vector<position_type> neighbors = {
          get_shifted_position(u, direction::up),
          get_shifted_position(u, direction::right),
          get_shifted_position(u, direction::down),
          get_shifted_position(u, direction::left)
        };

        for (auto& v : neighbors) {
          int y = v.first;
          int x = v.second;

          if (y >= 0 && y < board_.size() && x >= 0 && x < board_[0].size()) {
            char tile = board_[y][x];
            int cost = 9999;
            if (tile != '#') {
              cost = 1;
            }
              if (dist[v] > dist[u] + cost) {
                dist[v] = dist[u] + cost;
              } 
            } 
          }
          Q.erase(min_elem_it);
        }
      int dist_to_dest = dist[dest];
      dist_[src_dest_pair] = dist_to_dest;
      return dist[dest];
    }

    int absolute_distance(position_type src, position_type dest) {
      return std::abs(src.first - dest.first) + std::abs(src.first - dest.first);
    }

    std::vector<int> generate_seq_vec(int n) {
      std::vector<int> tmp;
      for (int i = 0; i < n; i++) {
        tmp.push_back(i);
      }
      return tmp;
    }

    std::vector<std::vector<std::pair<int, int>>> generate_matches(int n) {
      std::vector<int> box_inds;
      for (int i = 0; i < n; i++) {
        box_inds.push_back(i);
      }
      std::vector<int> goal_inds(box_inds);
      std::vector<std::vector<std::pair<int, int>>> matches;

      do {
        std::vector<std::pair<int, int>> match;
        for (int i = 0; i < n; i++) {
          match.push_back(std::make_pair(goal_inds[i], box_inds[i]));
        }
        matches.push_back(match);
      } while (std::next_permutation(goal_inds.begin(), goal_inds.end()));
      return matches;
    }

    int get_reward() {
      for (auto i = box_positions_.begin(); i != box_positions_.end(); ++i) {
        for (auto j = goal_positions_.begin(); j != goal_positions_.end(); ++j) {
          shortest_distance_path(*i, *j);
        } 
      } 

      int min_cost = 9999;

      int n = box_positions_.size();
      auto matchings = generate_matches(n);
      for (auto& matching : matchings) {
        int cost = 0;
        for (auto& match : matching) {
          cost += shortest_distance_path(box_positions_[match.first],
              goal_positions_[match.second]);
        }
        min_cost = std::min(cost, min_cost);
      }
      return -min_cost;
    }

    void step(direction dir) {
      num_moves_++;
      board_[human_pos_.first][human_pos_.second] = ' ';
      seq_.push_back(dir);

      position_type pos = get_shifted_position(human_pos_, dir);

      char tile = board_[pos.first][pos.second];

      if (!is_goal_or_free(tile)) {
        // its a box
        position_type new_box_pos = get_shifted_position(pos, dir);

        auto box_pos = std::find(box_positions_.begin(), box_positions_.end(), pos);
        box_pos->first = new_box_pos.first;
        box_pos->second = new_box_pos.second;

        board_[new_box_pos.first][new_box_pos.second] = '$';
      }

      board_[pos.first][pos.second] = '@';
      human_pos_.first = pos.first;
      human_pos_.second = pos.second;

      for (auto it = goal_positions_.begin(); it != goal_positions_.end(); ++it) {
        auto& board_pos = board_[it->first][it->second];
        if (board_pos == ' ') {
          board_pos = '.';
        }
      }

      if (is_game_over()) {
        is_game_over_ = true;
      }
    }

    void render() {
      std::cout << "  ";
      for (int k = 0; k < board_.size(); k++) {
        std::cout << k; 
      }
      std::cout << std::endl << std::endl;

      for (int i = 0; i < board_.size(); i++) {
        std::cout << i << " ";
        for (int j = 0; j < board_[i].size(); j++) {
          char tile = board_[i][j];
          if (tile == '#') {
            std::cout << "\033[91m#\033[0m";
          } else if (tile == '.') {
            std::cout << "\033[92m.\033[0m";
          } else if (tile == '$') {
            std::cout << "\033[93m$\033[0m";
          } else if (tile == '@') {
            std::cout << "\033[94m@\033[0m";
          } else {
            std::cout << " ";
          }
        }
        std::cout << std::endl;
      }
    }

    static move_type root_state() {
      return direction::null;
    }

    class rollout_move_getter {
      private:
        sokoban_env* parent_;
      public:
        rollout_move_getter(sokoban_env* parent)
          : parent_(parent)
        {}

        std::vector<move_type> get() {
          return parent_->get_possible_moves();
        }
    };

    rollout_move_getter get_rmg() {
      return rollout_move_getter(this);
    }
};
