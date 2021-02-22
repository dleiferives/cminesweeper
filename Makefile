all: main.cpp 
	g++ -o cminesweeper main.cpp board.cpp -lncurses

debug: main.cpp
	g++ -o cminesweeper -ggdb3 -Wall main.cpp board.cpp -lncurses