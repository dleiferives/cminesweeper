/* 
 * gamefunctions.c
 * 
 * Contains definitions of  */

/* TODO:
   Remove refresh statement from end of printBoard function */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <curses.h>
#include <ctype.h>	/* toupper, isdigit */
#include <string.h>	/* strlen */

#include "util.h"
#include "board.h"

int tutorial() {
	int x, y;
	Board vMem;
	vMem.width = 9;
	vMem.height = 9;
	vMem.mineCount = 10;
	initBoardArray(&vMem);

	uint8_t xm[10] = {7, 2, 6, 3, 9, 3, 1, 6, 6, 6};
	uint8_t ym[10] = {1, 3, 3, 4, 4, 5, 6, 7, 8, 9};

	curs_set(1);

	/* write mine data to board struct */
	for(x = 0; x < 10; x++) {
		vMem.array[xm[x]][ym[x]] |= MASK_MINE;
	}

	clear();
	addstr("+= Tutorial =======+\n");
	printBoard(vMem);
	addstr("\n+==================+\n");
	mvaddstr(16, 0, "Welcome to minesweeper!\n\n\nPress any key to continue...");
	move(20, 0);
	refresh();
	getch();

	mvaddstr(16, 0, "The goal of the game is to uncover all of the empty squares, while leaving the squares with mines untouched.\n\n\nPress any key to continue...\n");
	move(20, 0);
	refresh();
	getch();

	mvaddstr(16, 0, "Uncovering a square will make it display a number representing the number of mines in the 8 squares adjacent to it.\n\n\nPress any key to continue...\n");
	move(1, 0);
	vMem.array[2][4] = '0' + numMines(vMem, 2, 4);\
	openSquares(&vMem, 2, 4);
	printBoard(vMem);
	move(20, 0);
	refresh();
	getch();

	mvaddstr(16, 0, "If you uncover a square surrounded by 0 mines, the game will automatically open squares until all of the adjacent blank squares are open.\n\nPress any key to continue...\n");
	move(1, 0);
	vMem.array[4][7] = '0' + numMines(vMem, 4, 7);
	openSquares(&vMem, 4, 7);
	printBoard(vMem);
	move(20, 0);
	refresh();
	getch();

	mvaddstr(16, 0, "Once you are certain that a square contains a mine, you can flag it to avoid accidentally opening it.\n\n\nPress any key to continue...\n");
	move(1, 0);
	vMem.array[1][6] = 'P' | MASK_MINE;
	vMem.array[6][7] = 'P' | MASK_MINE;
	vMem.array[6][8] = 'P' | MASK_MINE;
	vMem.array[6][9] = 'P' | MASK_MINE;
	printBoard(vMem);
	move(20, 0);
	refresh();
	getch();
	
	mvaddstr(16, 0, "The game will end when you either uncover all mine-free squares, or as soon as you uncover a mine.\nGood luck, and SWEEP THOSE MINES!\n\nPress any key to continue...\n");
	for (y = 1; y <= 10; y++) {
		for (x = 1; x <= 10; x++) {
			vMem.array[x][y] &= ~MASK_CHAR;
			vMem.array[x][y] |= '0' + numMines(vMem, x, y);
			if ((vMem.array[x][y] & MASK_CHAR) == '0') {
				vMem.array[x][y] &= ~MASK_CHAR;
				vMem.array[x][y] |= ' ';
			}
			if (vMem.array[x][y] & MASK_MINE) {
				vMem.array[x][y] &= ~MASK_CHAR;
				vMem.array[x][y] |= 'F';
			}
		}
	}
	printBoard(vMem);
	refresh();
	getch();

	mvprintw(19, 0, "\n");
	curs_set(2);

	freeBoardArray(&vMem);
	return 0;
}

int printCtrlsyx(int y, int x) {
	int cy, cx;
	getyx(stdscr, cy, cx);
	mvaddstr(y++, x, "+============= Controls =============+");
	mvaddstr(y++, x, "| W A S D  : navigate the field      |");
	mvaddstr(y++, x, "| /  MOUSE1: primary select button   |");
	mvaddstr(y++, x, "| '  MOUSE2: secondary select button |");
	mvaddstr(y++, x, "| M  Space : toggle flagging mode    |");
	mvaddstr(y++, x, "| Q  Esc   : pause game              |");
	mvaddstr(y++, x, "+====================================+");
	move(cy, cx);

	return 0;
}

int printCtrls() {
    return printCtrlsyx(3, 29);
}
