all: main.c
	gcc -o cminesweeper main.c game.c board.c gamefunctions.c savegame.c -lncurses -lm
	mkdir -p ~/.cminesweeper

debug: main.c
	gcc -o cminesweeper -g -rdynamic -ggdb3 -Wall main.c game.c board.c gamefunctions.c savegame.c -lncurses -lm
	mkdir -p ~/.cminesweeper