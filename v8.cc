#include <fstream>
#include "v8.hpp"

int main() {
  mcts m;
  m.search_aio(1e6);

  std::ofstream gv("v8.gv");
  gv << m.to_gv();

  std::ofstream td("td");

  auto& terminal_depths = m.get_terminal_depths();

  for (auto& elem : terminal_depths) {
    td << elem << std::endl;
  }

}
