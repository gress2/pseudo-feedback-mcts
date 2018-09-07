import mcts
import same_game_env

env = same_game_env.Env()
mcts = mcts.MCTS(env)
print(mcts.search(100))
