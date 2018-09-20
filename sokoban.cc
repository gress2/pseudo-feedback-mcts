#include "mcts.hpp"
#include "sokoban_env.hpp"

int main() {
  sokoban_env env("skbn_cfgs/1.cfg");
  MCTS<sokoban_env> mcts(env);
  auto seq = mcts.search_aio(1e6);
  for (auto& pos : seq) {
    std::cout << "Move made: " << env.get_dir_str(pos) << std::endl;
    env.step(pos);
    env.render();
  }

  std::cout << "Score: " << env.get_total_reward() << std::endl;
}
