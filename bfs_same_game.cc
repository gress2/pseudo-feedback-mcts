#include "same_game_env.hpp"
#include "bfs.hpp"

int main() {
  same_game_env env;
  bfs<same_game_env> search(env);
}
