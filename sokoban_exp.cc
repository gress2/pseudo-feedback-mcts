#include "sokoban_env.hpp"
#include "search.hpp"

int main() {
  sokoban_env env("./skbn_cfgs/1.cfg");
  dfs(env, 10000, 1000);
}
