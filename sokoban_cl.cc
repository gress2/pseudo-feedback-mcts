#include <iostream>
#include <string>
#include "sokoban_env.hpp"

using direction = sokoban_env::direction;

int main() {
  sokoban_env env("skbn_cfgs/1.cfg");
  env.render();

  std::string input;
  while (!env.is_game_over()) {
    auto possible_moves = env.get_possible_moves();
    std::cout << "possible moves: " << std::endl;
    for (auto dir : possible_moves) {
      std::cout << sokoban_env::get_dir_str(dir) << std::endl;
    }
    std::cout << "enter move: " << std::endl;
    std::getline(std::cin, input); 

    if (input == "u") {
      env.step(direction::up);
    } else if (input == "r") {
      env.step(direction::right);
    } else if (input == "d") {
      env.step(direction::down);
    } else if (input == "l") {
      env.step(direction::left);
    } else if (input == "U") {
      env.step(direction::UP);
    } else if (input == "R") {
      env.step(direction::RIGHT);
    } else if (input == "D") {
      env.step(direction::DOWN);
    } else if (input == "L") {
      env.step(direction::LEFT);
    }
    env.render();
  }
  std::cout << "game over! reward: " << env.get_total_reward() << std::endl;
  
}
