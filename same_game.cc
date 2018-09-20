#include "mcts.hpp"
#include "same_game_env.hpp"

int main() {
  same_game_env env;
  MCTS<same_game_env> mcts(env);
  auto seq  = mcts.search_aio(1e6);

  env.render();
  for (auto& pos : seq) {
    std::cout << "Move made: (" << pos.first << ", " << pos.second << ")" << std::endl;
    env.step(pos);
    env.render(); 
  }
  std::cout << "Score: " << env.get_total_reward() << std::endl;

}
