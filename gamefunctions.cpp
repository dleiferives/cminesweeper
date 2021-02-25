/* gamefunctions.cpp 
   contains definitions for game utility functions */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <curses.h>

#include "gamefunctions.h"
#include "board.h"

int printBoardCustom (Board board, bool hide, chtype mineChar, chtype flagChar) {
	int chars = 0;
	int x, y;
	int line = 1;

	mvprintw (line, 0, "|  ");
	for (x = 0; x <= board.width; x++) {
		if (x % 5 == 0) printw ("%02d", x);
		else printw ("  ");
	}
	printw ("   ");
	if (board.width < 4)
		for (x = 0; x < (4 - board.width); x++) printw ("  ");
	addch ('|' | COLOR_PAIR (1));
	
	mvprintw(++line, 0, "|  ");
	for (x = 0; x <= board.width; x++) {
		if (x % 5 == 0) printw ("| ");
		else printw (". ");
	}
	printw ("   ");
	if (board.width < 4)
		for (x = 0; x < (4 - board.width); x++) printw ("  ");
	addch ('|' | COLOR_PAIR (1));


	for (y = 1; y <= board.height; y++) {
		mvprintw (++line, 0, "|%02d-", y);
		for (x = 1; x <= board.width; x++) {
			addch (' ');
			if (hide) addch ('+');
			else {
				if ('0' <= board.array[x][y] && board.array[x][y] <= '9') {
					addch (board.array[x][y] | COLOR_PAIR (5));
					chars++;
				}
				else switch (board.array[x][y]) {
				case '+':
					addch ('+' | COLOR_PAIR (0));
					break;
				case 'X':
					addch (mineChar);
					break;
				case '#':
					addch ('#' | COLOR_PAIR (3));
					break;
				case 'P':
					addch (flagChar);
					break;
				case 'F':
					addch ('F' | COLOR_PAIR (4));
					break;
				default:
					addch (' ');
				}
			}
			chars++;
		}
		addch (' ' | COLOR_PAIR (1));
		printw ("-%02d", y);
		if (board.width < 4)
			for (x = 0; x < (4 - board.width); x++) printw ("  ");
		addch ('|' | COLOR_PAIR (1));
	}
	mvprintw (++line, 0, "|  ");
	for (x = 0; x <= board.width; x++) {
		if (x % 5 == 0) printw ("| ");
		else printw ("' ");
	}
	printw ("   ");
	if (board.width < 4)
		for (x = 0; x < (4 - board.width); x++) printw ("  ");
	addch ('|' | COLOR_PAIR (1));
	
	mvprintw (++line, 0, "|  ");
	for (x = 0; x <= board.width; x++) {
		if (x % 5 == 0) printw ("%02d", x);
		else printw ("  ");
	}
	printw ("   ");
	if (board.width < 4)
		for (x = 0; x < (4 - board.width); x++) printw ("  ");
	printw ("|\n");
	
	refresh ();
	return chars;
}

int printBoard (Board board) {
	return printBoardCustom (board, false, (chtype)'X' | COLOR_PAIR(3), (chtype)'P' | COLOR_PAIR(3));
}

int initializeMines (Board * mines) {
	int mineCount = 0;
	int x, y;

	for (y = 0; y < mines->height + 2; y++) {
		for (x = 0; x < mines->width + 2; x++) {
			mines->array[x][y] = '.';
		}
	}

	while (mineCount < mines->mineCount) {
		x = rand () % (mines->width) + 1;
		y = rand () % (mines->height) + 1;
		if (mines->array[x][y] != 'X') {
			mines->array[x][y] = 'X';
			mineCount++;
		}
		if (mineCount > (mines->width * mines->height) - 2) break;
	}

	return mineCount;
}

int overlayMines (Board mines, Board * board) {
	int x, y;
	for (y = 1; y < mines.height + 2; y++) {
		for (x = 1; x < mines.width + 2; x++) {
			if (mines.array[x][y] == 'X') {
				switch (board->array[x][y]) {
				case 'P':
					board->array[x][y] = 'F';
					break;
				default:
					board->array[x][y] = 'X';
				}
			}
		}
	}
	return 0;
}

int numMines (Board board, int x, int y) {
	int numOfMines = 0;
	int h, k;

	for (k = -1; k <= 1; k++) {
		for (h = -1; h <= 1; h++) {
			/* check whether index is out of bounds before reading */
			if (x + h >= 0 && x + h < board.width + 2
				&& y + k >= 0 && y + k < board.width + 2) {
				if (board.array[x + h][y + k] == 'X') numOfMines++;
			}
		}
	}

	return numOfMines;
}

int openSquares (Board mines, Board * board, int x, int y) {
	/* All squares with 0 adjacent mines must have all adjacent squares cleared */
	int h, k;
	bool freeSquares = true;
	int neighbors = 0;
	while (freeSquares) {
		freeSquares = false;
		if (board->array[x][y] != ' ') break;
		/* do this 12 times: */
		for (int q = 0; q < 12; q++) {
			/* for every tile on board: */
			for (y = 1; y < board->height + 1; y++) {
				for (x = 1; x < board->height + 1; x++) {
					if (board->array[x][y] == ' ') {
						/* only if already uncovered */
						for (k = -1; k <= 1; k++) {
							for (h = -1; h <= 1; h++) {
								if (board->array[x + h][y + k] != 'P') board->array[x + h][y + k] = 48 + numMines (mines, x + h, y + k);
								if (board->array[x + h][y + k] == '0') board->array[x + h][y + k] = ' ';
							}
						}
					}
				}
			}
		    /* for every tile on board */
			for (y = 1; y < board->height + 1; y++) {
				for (x = 1; x < board->height + 1; x++) {
					if (49 <= board->array[x][y] && board->array[x][y] <= 57) {
						/* only if is nonzero num */
						neighbors = 0;
						for (k = -1; k <= 1; k++) {
							for (h = -1; h <= 1; h++) {

								if (48 <= board->array[x + h][y + k] && board->array[x + h][y + k] <= 57) {
									neighbors++;
								}

							}
						}
						if (neighbors < 3) {
							freeSquares = true;
							break;
						}
					}
				}
			}
		}
		freeSquares = false;
	}
	return 0;
}

bool allClear (Board mines, Board board) {
	int x, y;

	for (y = 1; y <= mines.height; y++) {
		for (x = 1; x <= mines.width; x++) {
			if (mines.array[x][y] == '.' && (board.array[x][y] == '+' || board.array[x][y] == 'P')) {
				return false;
			}
		}
	}

	return true;
}

int menu () {
	bool gotInput = false;
	int buf = 0;
	int8_t option = MENU_NO_INPUT;
	/* set cursor to invisible */
	curs_set (0);

	while (!gotInput) {
		/* get arrow key input */
		mvprintw (0, 0, "+= Paused ================+");
		mvprintw (1, 0, "|                         |");
		mvprintw (2, 0, "|  1) Return to game      |");
		mvprintw (3, 0, "|  2) New game            |");
		mvprintw (4, 0, "|  3) Exit game           |");
		mvprintw (5, 0, "|  4) View Tutorial       |");
		mvprintw (6, 0, "|                         |");
		mvprintw (7, 0, "+=========================+");

		/* draw option pointer */
		for (int x = 0; x < 4; x++) {
			if (option == x) mvaddch (2 + x, 5, '>' | A_BLINK);
			else mvaddch (2 + x, 5, ' ');
		}

		refresh ();

		buf = (wgetch (stdscr));
		switch (buf) {
		case 'q':
		case 27: /* key code for Esc */
			option = MENU_NO_INPUT;
			gotInput = true;
			break;
		case '1':
			option = MENU_NO_INPUT;
			gotInput = true;
			break;
		case '2':
			option = MENU_RESTART;
			gotInput = true;
			break;
		case '3':
			option = MENU_EXIT_GAME;
			gotInput = true;
			break;
		case '4':
			option = MENU_TUTORIAL;
			gotInput = true;
			break;
		case 10: /* key code for Return */
			gotInput = true;
			break;
		case 32: /* key code for space */
			gotInput = false;
			break;
		case KEY_UP:
			option--;
			if (option < 0) option = 0;
			break;
		case KEY_LEFT:
			option--;
			if (option < 0) option = 0;
			break;
		case KEY_DOWN:
			option++;
			if (option > 3) option = 3;
			break;
		case KEY_RIGHT:
			option++;
			if (option > 3) option = 3;
			break;
		}
	}

	if (option == MENU_TUTORIAL) {
		tutorial ();
	}
	
	/* set cursor to visible */
	curs_set (1);
	return option;
}

int tutorial () {
	int x, y;
	Board mines;
	mines.width = 9;
	mines.height = 9;
	mines.mineCount = 10;
	initBoardArray (&mines);

	Board vMem;
	vMem.width = 9;
	vMem.height = 9;
	vMem.mineCount = 10;
	initBoardArray (&vMem);

	uint8_t xm[10] = {7, 2, 6, 3, 9, 3, 1, 6, 6, 6};
	uint8_t ym[10] = {1, 3, 3, 4, 4, 5, 6, 7, 8, 9};

	curs_set (1);

	/* initialize boards */
	for (y = 1; y <= 10; y++) {
		for (x = 1; x <= 10; x++) {
			mines.array[x][y] = '.';
			vMem.array[x][y] = '+';
		}
	}

	/* write mine data to mine struct */
	for (x = 0; x < 10; x++) {
		mines.array[xm[x]][ym[x]] = 'X';
	}

	clear ();
	addstr ("+= Tutorial ==============+\n");
	printBoard (vMem);
	addstr ("+=========================+\n");
	mvaddstr (16, 0, "Welcome to minesweeper!\n\n\nPress any key to continue...");
	move (20, 0);
	refresh ();
	getch ();

	mvaddstr (16, 0, "The goal of the game is to uncover all of the empty squares, while leaving the squares with mines untouched.\n\n\nPress any key to continue...\n");
	move (20, 0);
	refresh ();
	getch ();

	mvaddstr (16, 0, "Uncovering a square will make it display a number representing the number of mines in the 8 squares adjacent to it.\n\n\nPress any key to continue...\n");
	move (1, 0);
	vMem.array[2][4] = 48 + numMines (mines, 2, 4);
	if (vMem.array[2][4] == '0') vMem.array[2][4] = ' ';
	openSquares (mines, &vMem, 2, 4);
	printBoard (vMem);
	move (20, 0);
	refresh ();
	getch ();

	mvaddstr (16, 0, "If you uncover a square surrounded by 0 mines, the game will automatically open squares until all of the adjacent blank squares are open.\n\nPress any key to continue...\n");
	move (1, 0);
	vMem.array[4][7] = 48 + numMines (mines, 4, 7);
	if (vMem.array[4][7] == '0') vMem.array[4][7] = ' ';
	openSquares (mines, &vMem, 4, 7);
	printBoard (vMem);
	move (20, 0);
	refresh ();
	getch ();

	mvaddstr (16, 0, "Once you are certain that a square contains a mine, you can flag it to avoid accidentally opening it.\n\n\nPress any key to continue...\n");
	move (1, 0);
	vMem.array[1][6] = 'P';
	vMem.array[6][7] = 'P';
	vMem.array[6][8] = 'P';
	vMem.array[6][9] = 'P';
	printBoard (vMem);
	move (20, 0);
	refresh ();
	getch ();
	
	mvaddstr (16, 0, "The game will end when you either uncover all mine-free squares, or as soon as you uncover a mine.\nGood luck, and SWEEP THOSE MINES!\n\nPress any key to continue...\n");
	for (y = 1; y <= 10; y++) {
		for (x = 1; x <= 10; x++) {
			vMem.array[x][y] = 48 + numMines (mines, x, y);
			if (vMem.array[x][y] == '0') vMem.array[x][y] = ' ';
			if (mines.array[x][y] == 'X') vMem.array[x][y] = 'F';
		}
	}
	move (1, 0);
	printBoard (vMem);
	move (20, 0);
	refresh ();
	getch ();

	mvprintw (19, 0, "\n");
	curs_set (2);

	freeBoardArray (&mines);
	freeBoardArray (&vMem);
	return 0;
}

int printCtrlsyx (int y, int x) {
	int cy, cx;
	getyx (stdscr, cy, cx);
	mvaddstr (y++, x, "+============= Controls =============+");
	mvaddstr (y++, x, "| W A S D  : navigate the field      |");
	mvaddstr (y++, x, "| /  MOUSE1: primary select button   |");
	mvaddstr (y++, x, "| '  MOUSE2: secondary select button |");
	mvaddstr (y++, x, "| M  Space : toggle flagging mode    |");
	mvaddstr (y++, x, "| Q  Esc   : pause game              |");
	mvaddstr (y++, x, "+====================================+");
	move (cy, cx);

	return 0;
}

int printCtrls () {
    return printCtrlsyx (3, 29);
}

int printBlank (Board board) {
	return printBoardCustom (board, true, (chtype)0, (chtype)0);
}
