main: main.o
	g++ -o main main.o

main.o: main.cc mcts.hpp same_game_env.hpp
	g++ -std=c++17 -O3 -Wfatal-errors -c main.cc

cl: cl.o
	g++ -o cl cl.o

cl.o: cl.cc same_game_env.hpp
	g++ -std=c++17 -g -Wfatal-errors -c cl.cc

clean:
	rm *.o


