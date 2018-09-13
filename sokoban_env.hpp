#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class sokoban_env {
  public:
    using board_type = std::vector<std::vector<char>>;
    using position_type = std::pair<short, short>;
  private:
    board_type board_;
    int total_reward_ = 0;
    position_type human_pos_;
    std::vector<position_type> box_positions_;
    std::vector<position_type> goal_positions_;
  public:
    sokoban_env(std::string board_file_str) {
      std::ifstream board_file(board_file_str);
      std::string line;
      while (std::getline(board_file, line)) {
        board_.push_back(std::vector<char>(line.begin(), line.end()));
      }

      for (short i = 0; i < board_.size(); i++) {
        for (short j = 0; j < board_[i].size(); j++) {
          char tile = board_[i][j];
          if (tile == '@') {
            human_pos_ = std::make_pair(i, j);
            return;
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

    void render() {
      for (auto outer = board_.begin(); outer != board_.end(); ++outer) {
        for (auto inner = outer->begin(); inner != outer->end(); ++inner) {
          char tile = *inner;
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
