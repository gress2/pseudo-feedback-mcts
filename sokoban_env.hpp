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
    static std::set<position_type> reachable_positions_; 
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

      if (reachable_positions_.empty()) {
        get_reachable();
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

    std::size_t hash() const {
      std::stringstream ss;
      for (int i = 0; i < board_.size(); i++) {
        for (int j = 0; j < board_[i].size(); j++) {
          ss << board_[i][j];
        }
      }

      std::size_t h = std::hash<std::string>{}(ss.str());
      return h;
    }

    static std::string get_dir_str(direction dir) {
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
      return get_reward();
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

    bool is_in_bounds(position_type pos) {
      return pos.first >= 0 && pos.first < board_.size()
        && pos.second >= 0 && pos.second < board_[0].size();
    }

    bool is_not_wall(position_type pos) {
      return board_[pos.first][pos.second] != '#';
    }

    void bfs_explore(position_type pos, std::set<position_type>& visited) {
      if (visited.count(pos)) {
        return;
      }

      visited.insert(pos);

      std::vector<direction> dirs = { 
        direction::up, direction::right, 
        direction::down, direction::left 
      };

      for (auto& dir : dirs) {
        auto one_step = get_shifted_position(pos, dir);
        auto two_step = get_shifted_position(one_step, dir);
        if (is_in_bounds(one_step) && is_in_bounds(two_step) &&
            is_not_wall(one_step) && is_not_wall(two_step)) {
          bfs_explore(one_step, visited);
        }
      }
    }

    void get_reachable() {
      for (auto it = goal_positions_.begin(); it != goal_positions_.end(); ++it) {
        auto goal_pos = *it;
        std::set<position_type> visited;
        bfs_explore(goal_pos, visited);
        reachable_positions_.insert(visited.begin(), visited.end());
      }
      for (auto& pos : reachable_positions_) {
        std::cout << pair_to_str(pos) << std::endl;
      }
    }


    bool would_not_lock(position_type considered, position_type hypothetical) {
      auto up = get_shifted_position(considered, direction::up);
      auto right = get_shifted_position(considered, direction::right);
      auto down = get_shifted_position(considered, direction::down);
      auto left = get_shifted_position(considered, direction::left);

      bool up_clear = is_in_bounds(up) && up != hypothetical && is_goal_or_free(board_[up.first][up.second]);
      bool right_clear = is_in_bounds(right) && right  != hypothetical && is_goal_or_free(board_[right.first][right.second]);
      bool down_clear = is_in_bounds(down) && down != hypothetical && is_goal_or_free(board_[down.first][down.second]);
      bool left_clear = is_in_bounds(left) && left != hypothetical && is_goal_or_free(board_[left.first][left.second]);

      return ((up_clear && down_clear) || (left_clear && right_clear));
    }

    bool will_freeze_deadlock(position_type src, position_type dest) {
      if (board_[dest.first][dest.second] == '.') {
        return false;
      }

      auto up = get_shifted_position(dest, direction::up);
      auto right = get_shifted_position(dest, direction::right);
      auto down = get_shifted_position(dest, direction::down);
      auto left = get_shifted_position(dest, direction::left);

      bool up_clear = up == src || is_goal_or_free(board_[up.first][up.second]) || would_not_lock(up, dest);
      bool right_clear = right == src || is_goal_or_free(board_[right.first][right.second]) || would_not_lock(right, dest);
      bool down_clear = down == src || is_goal_or_free(board_[down.first][down.second]) || would_not_lock(down, dest);
      bool left_clear = left == src || is_goal_or_free(board_[left.first][left.second]) || would_not_lock(left, dest);

      return !((up_clear && down_clear) || (left_clear && right_clear));
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
            auto one_step = get_shifted_position(human_pos_, dir);
            auto two_step = get_shifted_position(one_step, dir);

            if (reachable_positions_.count(two_step) && 
                !will_freeze_deadlock(one_step, two_step)) {
              moves.push_back(uppercase(dir));
            }
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
          is_any_box_stuck() || num_moves_ > 40;
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
          if (is_in_bounds(v)) {
            int cost = is_not_wall(v) ? 1 : 9999;
            dist[v] = std::min(dist[v], dist[u] + cost);
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
      return -min_cost + 100 * get_num_correct_boxes();
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
