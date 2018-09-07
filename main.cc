#include "mcts.hpp"
#include "same_game_env.hpp"

int main() {
  same_game_env env;
  env.render();
  MCTS<same_game_env> mcts(&env);
}