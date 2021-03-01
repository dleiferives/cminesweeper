/* gamefunctions.c
   contains definitions for game utility functions */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <curses.h>
#include <ctype.h>	/* toupper */
#include <string.h>	/* strlen */

#include "gamefunctions.h"
#include "board.h"

int printBoardCustom (Board board, bool hide, chtype mineChar, chtype flagChar) {
	int chars = 0;
	int x, y;
	int line = 1;

	/* numbers for the top of the board */
	mvprintw (line, 0, "|  ");
	for (x = 0; x <= board.width; x++) {
		if (x % 5 == 0) printw ("%02d", x);
		else printw ("  ");
	}
	printw ("   ");
	if (board.width < 4)
		for (x = 0; x < (4 - board.width); x++) printw ("  ");
	addch ('|' | COLOR_PAIR (1));
	
	/* tick marks under the numbers */
	mvprintw(++line, 0, "|  ");
	for (x = 0; x <= board.width; x++) {
		if (x % 5 == 0) printw ("| ");
		else printw (". ");
	}
	printw ("   ");
	if (board.width < 4)
		for (x = 0; x < (4 - board.width); x++) printw ("  ");
	addch ('|' | COLOR_PAIR (1));

	/* the actual minefield */
	for (y = 1; y <= board.height; y++) {
		mvprintw (++line, 0, "|%02d-", y);
		for (x = 1; x <= board.width; x++) {
			addch (' ');
			if (hide)
				addch ('+');
			else {
				if ('0' <= (board.array[x][y] & MASK_CHAR) && (board.array[x][y] & MASK_CHAR) <= '9') {
					addch ((board.array[x][y] & MASK_CHAR) | COLOR_PAIR (5));
					chars++;
				}
				else switch (board.array[x][y] & MASK_CHAR) {
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
	addch ('|');
	move(++line, 0);
	
	refresh ();
	return chars;
}

int printBoard (Board board) {
	return printBoardCustom (board, false, (chtype)'X' | COLOR_PAIR(3), (chtype)'P' | COLOR_PAIR(3));
}

int printFrame (Board board) {
	int x;

	mvaddstr (0, 0, "+= Minesweeper ");
	for (x = 4; x < board.width; x++) addstr ("==");
	addstr ("=+");

	mvaddstr (board.height + 5, 0, "+==============");
	for (x = 4; x < board.width; x++) addstr ("==");
	addstr ("=+");
	return 0;
}

int initializeMines (Board * board) {
	int mineCount = 0;
	int x, y;

	for (y = 1; y < board->height + 1; y++) {
		for (x = 1; x < board->width + 1; x++) {
			/* unset mine bit */
			board->array[x][y] &= ~MASK_MINE;
		}
	}

	while (mineCount < board->mineCount) {
		x = rand () % (board->width) + 1;
		y = rand () % (board->height) + 1;
		if (!(board->array[x][y] & MASK_MINE)) {
			board->array[x][y] |= MASK_MINE;
			mineCount++;
		}
		if (mineCount > (board->width * board->height) - 2) break;
	}

	return mineCount;
}

int overlayMines (Board * board) {
	int x, y;
	for (y = 1; y < board->height + 2; y++) {
		for (x = 1; x < board->width + 2; x++) {
			if (board->array[x][y] & MASK_MINE) {
				if ((board->array[x][y] & MASK_CHAR) == 'P') {
					board->array[x][y] &= ~MASK_CHAR;	/* clear char */
					board->array[x][y] |= 'F';			/* assign char */
				} else {
					board->array[x][y] &= ~MASK_CHAR;	/* clear char */
					board->array[x][y] |= 'X';			/* assign char */
				}
			}
		}
	}
	return 0;
}

int numMines (Board board, int x, int y) {
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

int openSquares (Board * board, int x, int y) {
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
	neighbors = numMines (*board, x, y);
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
				openSquares (board, x + h, y + k);
			}
		}
	}

	

	return 0;
}

bool allClear (Board board) {
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

int menu (int optc, const char * title, ...) {
	int i, x; /* counting variables */
	size_t maxLength;
	/* string array to hold option names */
	const char ** optionNames = malloc (optc * sizeof (char *));
	/* array to cache string lengths to avoid calling strlen multiple times */
	size_t * optionLengths = malloc (optc * sizeof (size_t));
	size_t titleLength;

	/* variables for navigating the menu */
	bool gotInput = false; /* the user has made a choice */
	int buf = 0;
	int option = 0;

	va_list options;
	va_start (options, title);
	
	/* populate the string array using the varargs */
	for (i = 0; i < optc; i++)
		optionNames[i] = va_arg (options, char *);
	va_end (options);

	/* calculate the width of the box necessary to fit all options */
	/* title margin is 3 characters left of options and has 1 padding */
	titleLength = strlen (title) - 2;
	maxLength = titleLength;

	for (i = 0; i < optc; i++) {
		optionLengths[i] = strlen (optionNames[i]);
		if (optionLengths[i] > maxLength) {
			maxLength = optionLengths[i];
		}
	}

	curs_set (0); /* cursor invisible */

	while (!gotInput) {
		/* print the menu */
		/* start with top of border */
		mvprintw (0, 0, "+= %s ", title);
		for (x = 0; x < maxLength - titleLength; x++)
			addch ('=');
		addstr ("==+");

		/* blank space */
		mvaddstr (1, 0, "|     ");
		for (x = 0; x < maxLength; x++)
			addch (' ');
		addstr ("  |");

		/* print every option */
		for (i = 0; i < optc; i++) {
			mvprintw (i + 2, 0, "| %2d) %s", i + 1, optionNames[i]);
			for (x = 0; x < maxLength - optionLengths[i]; x++)
				addch (' ');
			addstr ("  |");
		}

		/* another blank space */
		mvaddstr (i + 2, 0, "|     ");
		for (x = 0; x < maxLength; x++)
			addch (' ');
		addstr ("  |");

		/* end with bottom of border */
		mvaddstr (i + 3, 0, "+=====");
		for (x = 0; x < maxLength; x++)
			addch ('=');
		addstr ("==+");

		/* draw option pointer */
		mvaddch (option + 2, 5, '>' | A_BLINK);

		refresh ();

		/* now get input */
		buf = toupper (getch ());

		if (buf == 'Q' || buf == 27) {
			/* quit or Esc */
			option = -1;
			gotInput = true;
		} else if ('1' <= buf && buf <= '9') {
			/* numbers corresponding to options */
			if (buf - '0' <= optc) {
				/* the user selected an option that DOES exist */
				option = buf - '0' - 1;
				gotInput = true;
			}
			/* otherwise, do nothing */
		} else if (buf == 'W' || buf == KEY_UP) {
			option--;
			if (option < 0) option = 0;
		} else if (buf == 'A' || buf == KEY_LEFT) {
			option--;
			if (option < 0) option = 0;
		} else if (buf == 'S' || buf == KEY_DOWN) {
			option++;
			if (option > optc - 1) option = optc - 1;
		} else if (buf == 'D' || buf == KEY_RIGHT) {
			option++;
			if (option > optc - 1) option = optc - 1;
		} else if (buf == 10) {
			/* return or enter */
			gotInput = true;
		}
	}
	
	curs_set (1); /* cursor visible */
	free (optionNames);
	free (optionLengths);
	return option;
}

int tutorial () {
	int x, y;
	Board vMem;
	vMem.width = 9;
	vMem.height = 9;
	vMem.mineCount = 10;
	initBoardArray (&vMem);

	uint8_t xm[10] = {7, 2, 6, 3, 9, 3, 1, 6, 6, 6};
	uint8_t ym[10] = {1, 3, 3, 4, 4, 5, 6, 7, 8, 9};

	curs_set (1);

	/* write mine data to board struct */
	for (x = 0; x < 10; x++) {
		vMem.array[xm[x]][ym[x]] |= MASK_MINE;
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
	vMem.array[2][4] = '0' + numMines (vMem, 2, 4);
	if (vMem.array[2][4] == '0') vMem.array[2][4] = ' ';
	openSquares (&vMem, 2, 4);
	printBoard (vMem);
	move (20, 0);
	refresh ();
	getch ();

	mvaddstr (16, 0, "If you uncover a square surrounded by 0 mines, the game will automatically open squares until all of the adjacent blank squares are open.\n\nPress any key to continue...\n");
	move (1, 0);
	vMem.array[4][7] = '0' + numMines (vMem, 4, 7);
	if (vMem.array[4][7] == '0') vMem.array[4][7] = ' ';
	openSquares (&vMem, 4, 7);
	printBoard (vMem);
	move (20, 0);
	refresh ();
	getch ();

	mvaddstr (16, 0, "Once you are certain that a square contains a mine, you can flag it to avoid accidentally opening it.\n\n\nPress any key to continue...\n");
	move (1, 0);
	vMem.array[1][6] = 'P' | MASK_MINE;
	vMem.array[6][7] = 'P' | MASK_MINE;
	vMem.array[6][8] = 'P' | MASK_MINE;
	vMem.array[6][9] = 'P' | MASK_MINE;
	printBoard (vMem);
	move (20, 0);
	refresh ();
	getch ();
	
	mvaddstr (16, 0, "The game will end when you either uncover all mine-free squares, or as soon as you uncover a mine.\nGood luck, and SWEEP THOSE MINES!\n\nPress any key to continue...\n");
	for (y = 1; y <= 10; y++) {
		for (x = 1; x <= 10; x++) {
			vMem.array[x][y] &= ~MASK_CHAR;
			vMem.array[x][y] |= '0' + numMines (vMem, x, y);
			if ((vMem.array[x][y] & MASK_CHAR) == '0') {
				vMem.array[x][y] &= ~MASK_CHAR;
				vMem.array[x][y] |= ' ';
			}
			if (vMem.array[x][y] == 'X') {
				vMem.array[x][y] &= ~MASK_CHAR;
				vMem.array[x][y] |= 'F';
			}
		}
	}
	move (1, 0);
	printBoard (vMem);
	move (20, 0);
	refresh ();
	getch ();

	mvprintw (19, 0, "\n");
	curs_set (2);

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
