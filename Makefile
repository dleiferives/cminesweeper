all: main.cpp
	g++ -o cminesweeper main.cpp board.cpp -lncurses