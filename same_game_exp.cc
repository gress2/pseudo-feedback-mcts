#include "same_game_env.hpp"
#include "search.hpp"

int main() {
  same_game_env env;

  dfs(env, 10000, 1000);
}
