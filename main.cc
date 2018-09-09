#include "mcts.hpp"
#include "same_game_env.hpp"

int main() {
  same_game_env env;
  MCTS<same_game_env> mcts(env);
  mcts.search(10);
}