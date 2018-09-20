#include "same_game_env.hpp"

int main() {
  same_game_env env;
  env.render();

  auto moves = env.get_possible_moves();
  for (auto& move : moves) {
    std::cout << "(" << move.first << ", " << move.second << ")" << std::endl;
  }

}
