CC = gcc
srcfiles = source/main.c source/game.c source/board.c source/gamefunctions.c source/savegame.c
output = cminesweeper

all: $(srcfiles)
	$(CC) -o $(output) -Isource $(srcfiles) -lncurses -lm
	mkdir -p $(HOME)/.cminesweeper

debug: $(srcfiles)
	$(CC) -o $(output) -Isource -g -rdynamic -ggdb3 -Wall $(srcfiles) -lncurses -lm
	mkdir -p $(HOME)/.cminesweeper
