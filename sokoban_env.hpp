#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class sokoban_env {
  public:
    enum direction { up, right, down, left }; 
    using board_type = std::vector<std::vector<char>>;
    using position_type = std::pair<short, short>;
  private:
    board_type board_;
    position_type human_pos_;
    std::vector<position_type> box_positions_;
    std::vector<position_type> goal_positions_;
    int total_reward_ = 0;
    int num_moves_ = 0;
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

    sokoban_env(sokoban_env& other) 
      : board_(other.board_), 
        total_reward_(other.total_reward_),
        human_pos_(other.human_pos_)
    {
    }

    int get_total_reward() {
      return total_reward_;
    }

    position_type get_shifted_position(position_type pos, direction dir) {
      if (dir == direction::up) {
        return std::make_pair(pos.first - 1, pos.second);
      } else if (dir == direction::right) {
        return std::make_pair(pos.first, pos.second + 1);
      } else if (dir == direction::down) {
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

    std::vector<direction> get_possible_moves() {
      std::vector<direction> moves;

      std::vector<direction> dirs = {
        direction::up,
        direction::right,
        direction::down,
        direction::left
      };

      for (auto dir : dirs) {
        if (is_valid_direction(dir)) {
          moves.push_back(dir);
        }
      }

      return moves;
    }

    bool is_box_stuck(position_type pos) {
      std::vector<position_type> sides = {
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

    void step(direction dir) {
      num_moves_++;
      board_[human_pos_.first][human_pos_.second] = ' ';
      int reward = -1;

      position_type pos = get_shifted_position(human_pos_, dir);
      std::cout << "human_pos_: (" << human_pos_.first << "," << human_pos_.second << ")" << std::endl;


      std::cout << "pos: (" << pos.first << "," << pos.second << ")" << std::endl;

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
      if (is_game_over()) {
        int num_correct_boxes = get_num_correct_boxes();  
        reward += 50 * num_correct_boxes;
      } else {
        get_possible_moves();
      }
      total_reward_ += reward;
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
};
