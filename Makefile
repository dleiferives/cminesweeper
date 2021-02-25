all: main.c
	gcc -o cminesweeper main.c board.c gamefunctions.c -lncurses -lm

debug: main.c
	gcc -o cminesweeper -ggdb3 -Wall main.c board.c gamefunctions.c -lncurses -lm