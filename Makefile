default: same_game sokoban same_game_cl sokoban_cl same_game_exp sokoban_exp

same_game: same_game.o
	g++ -o same_game same_game.o

same_game.o: same_game.cc mcts.hpp same_game_env.hpp
	g++ -std=c++17 -Ofast -Wfatal-errors -c same_game.cc

sokoban: sokoban.o sokoban_env.o
	g++ -o sokoban sokoban.o sokoban_env.o

sokoban.o: sokoban.cc mcts.hpp sokoban_env.hpp
	g++ -std=c++17 -g -Wfatal-errors -c sokoban.cc

same_game_exp: same_game_exp.o
	g++ -o same_game_exp same_game_exp.o

same_game_exp.o: same_game_exp.cc same_game_env.hpp search.hpp
	g++ -std=c++17 -Ofast -Wfatal-errors -c same_game_exp.cc

same_game_cl: same_game_cl.o
	g++ -o same_game_cl same_game_cl.o

same_game_cl.o: same_game_cl.cc same_game_env.hpp
	g++ -std=c++17 -g -Wfatal-errors -c same_game_cl.cc

sokoban_cl: sokoban_cl.o sokoban_env.o
	g++ -o sokoban_cl sokoban_cl.o sokoban_env.o

sokoban_cl.o: sokoban_cl.cc sokoban_env.hpp
	g++ -std=c++17 -g -Wfatal-errors -c sokoban_cl.cc

sokoban_exp: sokoban_exp.o sokoban_env.o
	g++ -o sokoban_exp sokoban_exp.o sokoban_env.o

sokoban_exp.o: sokoban_exp.cc sokoban_env.hpp search.hpp
	g++ -std=c++17 -Ofast -Wfatal-errors -c sokoban_exp.cc

sokoban_env.o: sokoban_env.cc sokoban_env.hpp
	g++ -std=c++17 -g -Wfatal-errors -c sokoban_env.cc

v8: v8.o
	g++ -o v8 v8.o

v8.o: v8.cc v8.hpp
	g++ -std=c++17 -g -Wfatal-errors -c v8.cc
clean:
	rm *.o


