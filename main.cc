#include "mcts.hpp"
#include "same_game_env.hpp"

int main() {
  same_game_env env;
  MCTS<same_game_env> mcts(env);
  int score = mcts.search(4000);
  std::cout << "score: " << score << std::endl;
}
