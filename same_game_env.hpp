#pragma once

#include <array>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <vector>

class same_game_env {
  public:
    using board_type = std::array<std::array<short, 15>, 15>;
    using position_type = std::pair<short, short>;
  private:
    const int num_colors_ = 5;
    int total_reward_ = 0;
    board_type board_;
  public:
    same_game_env(int random_seed = 32) {
      std::srand(random_seed);
      for (int x = 0; x < 15; x++) {
        for (int y = 0; y < 15; y++) {
          board_[x][y] = (std::rand() % 5) + 1;
        }
      }
    }
    
    same_game_env(same_game_env& other) 
      : board_(other.board_), total_reward_(other.total_reward_) 
    {}
      
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

    
};