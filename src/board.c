/* 
 * board.c
 *
 * Defines functions for managing and using the Board struct
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h> /* memset */

#include "util.h"
#include "board.h"

int initBoardArray(Board * board) {
    board->array = (unsigned char **) malloc((board->width + 2) * sizeof(unsigned char *));
	for (int i = 0; i < board->width + 2; i++) {
		board->array[i] = (unsigned char *) malloc(board->height + 2);
        memset(board->array[i], '+', board->height + 2);
    }
    return 0;
}

int freeBoardArray(Board * board) {
    for (int i = 0; i < board->width + 2; i++)
        free(board->array[i]);
    free(board->array);
    return 0;
}

int printBoardCustom(Board board, bool hide, chtype mineAttr) {
	int chars = 0;
	int x, y;

	/* for every element in the array */
	for (y = 1; y <= board.height; y++) {
		mvaddch(y, 0, '|');
		for (x = 1; x <= board.width; x++) {
			if (hide) {
				/* to print hidden board */
				addch('[' | COLOR_PAIR(0));
				addch(']' | COLOR_PAIR(0));
			} else {
				if (isdigit(board.array[x][y] & MASK_CHAR)) {
					/* if character is a number, then print space and number */
					addch(' ' | COLOR_PAIR(5));
					addch((board.array[x][y] & MASK_CHAR) | COLOR_PAIR(5));
					chars++;
				} else {
					switch (board.array[x][y] & MASK_CHAR) {
					case '+':
						addch('[' | COLOR_PAIR(0));
						addch(']' | COLOR_PAIR(0));
						break;
					case 'X':
						if (mineAttr == 0) {
							/* if no custom attributes were provided */
							addch('>' | COLOR_PAIR(3) | A_BOLD);
							addch('<' | COLOR_PAIR(3) | A_BOLD);
						} else {
							addch('|' | mineAttr);
							addch('>' | mineAttr);
						}
						break;
					case '#':
						addch('@' | COLOR_PAIR(3) | A_BOLD);
						addch('@' | COLOR_PAIR(3) | A_BOLD);
						break;
					case 'P':
						addch('|' | COLOR_PAIR(3) | A_BOLD);
						addch('>' | COLOR_PAIR(3) | A_BOLD);
						break;
					case 'F':
						addch('|' | COLOR_PAIR(4) | A_BOLD);
						addch('>' | COLOR_PAIR(4) | A_BOLD);
						break;
					default:
						addstr("  ");
					}
				}
			}
			chars += 2;
		}
		if (board.width < 7) {
			addch(' ');
			for(x = 0; x < (7 - board.width); x++) printw("  ");
		}
		addch('|' | COLOR_PAIR(1));
	}
	
	refresh();
	return chars;
}

int printBoard(Board board) {
	return printBoardCustom(board, false, (chtype) 0);
}

int printFrame(Board board) {
	int x;

	mvaddstr(0, 0, "+= Minesweeper ");
	for (x = 8; x < board.width; x++) addstr("==");
	if (board.width > 7) addch('=');
	addstr("=+");

	mvaddstr(board.height + 1, 0, "+==============");
	for (x = 8; x < board.width; x++) addstr("==");
	if (board.width > 7) addch('=');
	addstr("=+");
	return 0;
}

int initializeMines(Board * board) {
	int mineCount = 0;
	int x, y;

	for (y = 1; y < board->height + 1; y++) {
		for (x = 1; x < board->width + 1; x++) {
			/* unset mine bit */
			board->array[x][y] &= ~MASK_MINE;
		}
	}

	while (mineCount < board->mineCount) {
		x = rand() % (board->width) + 1;
		y = rand() % (board->height) + 1;
		if (!(board->array[x][y] & MASK_MINE)) {
			board->array[x][y] |= MASK_MINE;
			mineCount++;
		}
		if (mineCount > (board->width * board->height) - 2) break;
	}

	return mineCount;
}

int overlayMines(Board * board) {
	int x, y;
	for (y = 1; y < board->height + 2; y++) {
		for (x = 1; x < board->width + 2; x++) {
			if (board->array[x][y] & MASK_MINE) {
				if ((board->array[x][y] & MASK_CHAR) == 'P') {
					board->array[x][y] &= ~MASK_CHAR;	/* clear char */
					board->array[x][y] |= 'F';			/* assign char */
				} else if ((board->array[x][y] & MASK_CHAR) != '#') {
					board->array[x][y] &= ~MASK_CHAR;	/* clear char */
					board->array[x][y] |= 'X';			/* assign char */
				}
			}
		}
	}
	return 0;
}

int numMines(Board board, int x, int y) {
	int numOfMines = 0;
	int h, k;

	/* return 0 if the coordinate being read is outside the printable board region */
	if (x < 1 || board.width < x || y < 1 || board.height < y)
		return 0;


	for (k = -1; k <= 1; k++) {
		for (h = -1; h <= 1; h++) {
			if (board.array[x + h][y + k] & MASK_MINE) numOfMines++;
		}
	}

	return numOfMines;
}

int openSquares(Board * board, int x, int y) {
	/* used for relative navigation of the board array */
	int h, k;
	int neighbors = 0;
	
	/* return if either index is outside the printable board boundaries */
	if (x < 1 || board->width < x || y < 1 || board->height < y)
		return -1;

	/* return if this coordinate is flagged */
	if ((board->array[x][y] & MASK_CHAR) == 'P')
		return 0;

	/* now that all is well, count the number of neighbors.
	   note that this function assumes that there is not a mine at (x, y), 
	   since the game function is responsible for handling that first */
	neighbors = numMines(*board, x, y);
	if (neighbors > 0) {
		board->array[x][y] &= ~MASK_CHAR;		/* clear char */
		board->array[x][y] |= '0' + neighbors;	/* assign char */
		return 0;
	} else {
		/* if this coordinate has already been marked as opened by the openSquares 
		   function, then return, to avoid infinite recursion. Otherwise, mark it. */
		if ((board->array[x][y] & MASK_CHAR) == ' ') {
			return 0;
		} else {
			board->array[x][y] &= ~MASK_CHAR;	/* clear char */
			board->array[x][y] |= ' ';			/* assign char */
		}
		/* at this point, we know there are no mines nearby, so we will recursively
		   keep opening squares until all the necessary squares are open. */
		for (k = -1; k <= 1; k++) {
			for (h = -1; h <= 1; h++) {
				openSquares(board, x + h, y + k);
			}
		}
	}

	

	return 0;
}

bool allClear(Board board) {
	int x, y;
	char buf;

	for (y = 1; y <= board.height; y++) {
		for (x = 1; x <= board.width; x++) {
			buf = board.array[x][y];
			if (!(buf & MASK_MINE) && ((buf & MASK_CHAR) == '+' || (buf & MASK_CHAR) == 'P')) {
				/* return false if a square has no mine but is still covered */
				return false;
			}
		}
	}

	return true;
}

int printBlank(Board board) {
	return printBoardCustom(board, true, (chtype) 0);
}
