#include "sokoban_env.hpp"

sokoban_env::dist_table sokoban_env::dist_;
std::set<sokoban_env::position_type> sokoban_env::reachable_positions_;
