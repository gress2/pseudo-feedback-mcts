main: main.o
	g++ -o main main.o

main.o: main.cc mcts.hpp same_game_env.hpp
	g++ -std=c++17 -g -Wfatal-errors -c main.cc

clean:
	rm *.o
