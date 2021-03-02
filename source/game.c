/* game.c */

/* TODO:
   check return statements */

#include <stdlib.h>
#include <stdint.h>
#include <curses.h>
#include <math.h>	/* floorf */
#include <ctype.h>	/* toupper */
#include <time.h>	/* timespec, usleep */

#include "gamefunctions.h"
#include "board.h"
#include "savegame.h"

#include <unistd.h>	/* usleep */
/* use a define statement because usleep isn't portable to windows */
#define Sleep(ms) usleep((ms * 1000))

/* timespec utility functions */
void subtractTimespec (struct timespec * dest, struct timespec * src);	/* adds src to dest */
void addTimespec (struct timespec * dest, struct timespec * src);		/* subtracts src from dest */
double timespecToDouble (struct timespec spec);							/* converts a timespec interval to a float value */

int game (int xDim, int yDim, int qtyMines, Savegame * saveptr) {
	/*** INITIALIZATION ***/
	
	int buf = -1;	/* general purpose buffer */
	
	/* variables for navigation */
	int x = 0, y = 0;	/* absolute array coordinates */
	int h, k;			/* relative array coordinates */
	int cx, cy;			/* cursor coordinates */
	int hudOffset;		/* x-offset of where to print the HUD */
	
	/* variables storing info about the game state */
	bool isFlagMode;		/* flag mode is enabled */
	bool firstClick;		/* player has made the first click */
	bool isAlive = true;	/* player is alive */
	bool exitGame = false;	/* the game is set to exit */
	int flagsPlaced;		/* number of flags placed */

	Savegame save;	/* savegame object */
	Board board;	/* struct storing the state of the game board */
	MEVENT m_event;	/* mouse event */

	/* structs for timekeeping */
	struct timespec timeOffset;	/* running counter to adjust time calculation */
	struct timespec timeMenu;	/* time spent in the menu */
	struct timespec timeBuffer;	/* buffer used in time calculations */

	/* stores what action will be performed by the game */
	uint8_t action;
	
	/* savegame handling */
	if (saveptr != NULL) {
		/* if a valid Savegame pointer was passed, initialize
		   game based on the contents of that structure */
		save = *saveptr;
		/* start with game status variables */
		xDim = save.width;
		yDim = save.height;
		qtyMines = save. qtyMines;
		isFlagMode = ((save.gameBools & MASK_FLAG_MODE) != 0);
		firstClick = ((save.gameBools & MASK_FIRST_CLICK) != 0);
		flagsPlaced = save.flagsPlaced;
		cy = save.cy;
		cx = save.cx;
		clock_gettime (CLOCK_MONOTONIC, &timeOffset);		/* set offset to current time */
		subtractTimespec (&timeOffset, &save.timeOffset);	/* subtract the game duration */
		
		/* then initialize the board */
		board.width = xDim;
		board.height = yDim;
		board.mineCount = qtyMines;
		initBoardArray (&board);
		getGameData (&board, save);
		/* finally, remember to free the memory block used in *saveptr */
		free (saveptr->gameData);
	} else {
		/* otherwise, use the values passed in the first 3 params: */
		isFlagMode = false;
		flagsPlaced = 0;
		firstClick = false;
		/* cursor will be initialized at top left of game board */
		cy = 3, cx = 5;
		timeOffset.tv_sec = 0;
		timeOffset.tv_nsec = 0;
		/* initialize the boards */
		board.width = xDim;
		board.height = yDim;
		board.mineCount = qtyMines;
		initBoardArray (&board);
		initializeMines (&board);
	}

	/* set the hudOffset */
	hudOffset = 2 * board.width + 10;
	if (hudOffset < 18) hudOffset = 18;

	/*** BEGIN GAMEPLAY ***/

	noecho ();
	clear ();
	
	while (isAlive) {
		if (!firstClick)
			clock_gettime (CLOCK_MONOTONIC, &timeOffset);
		
		/* calculate duration of the game */
		clock_gettime (CLOCK_MONOTONIC, &timeBuffer);
		subtractTimespec (&timeBuffer, &timeOffset);	/* duration is now stored in timeBuffer */
		
		printFrame (board);
		printCtrlsyx (0, hudOffset);
		if (allClear (board)) {
			/* only if player has won */
			overlayMines (&board);
			printBoardCustom (board, false, (chtype)'F' | COLOR_PAIR (4), (chtype)'P' | COLOR_PAIR(3));
			mvprintw (7, hudOffset, "[ %02d/%02d ][ %3.3f ]" , flagsPlaced, qtyMines, timespecToDouble (timeBuffer));
			mvprintw (8, hudOffset, "[ You won!        ]");
			refresh ();
			break;
		} else {
			/* player hasn't won yet */
			mvprintw (7, hudOffset, "[ %02d/%02d ][ %03d ]", flagsPlaced, qtyMines, (int) floorf (timespecToDouble (timeBuffer)));
			printBoard (board);
			mvaddstr (8, hudOffset,
				isFlagMode
				? "[ Flag mode    ]"
				: "[ Normal mode  ]"
			);
			move (cy, cx);
		}

		/* set cursor to very visible */
		curs_set (2);

		/* get input */
		nodelay (stdscr, true);
		buf = getch ();
		nodelay (stdscr, false);
		Sleep (16); /* sleep 1/60th of a second */

		/* switch to process keystrokes */
		action = ACTION_NONE;
		switch (buf) {
		case 'q':
		case 27: /* key code for Esc */
			/* open menu */
			action = ACTION_ESCAPE;
			break;
		case KEY_MOUSE:
			getmouse (&m_event);
			cx = m_event.x;
			cy = m_event.y;

			if (isdigit (board.array[x][y])) {
				/* check whether player clicked a number */
				action = ACTION_AUTO;
			} else if (m_event.bstate & BUTTON1_CLICKED) {
				action = isFlagMode
					? ACTION_FLAG
					: ACTION_OPEN;
			} else if (m_event.bstate & BUTTON3_CLICKED) {
				action = isFlagMode
					? ACTION_OPEN
					: ACTION_FLAG;
			}
			break;
		case 32: /* spacebar */
		case 'm':
			/* toggle flag mode */
			isFlagMode = !isFlagMode;
			break;
		case 10: /* Return */
		case 'z':
		case '/':
			/* primary select button */
			if (isdigit (board.array[x][y])) {
				/* check whether player clicked a number */
				action = ACTION_AUTO;
			} else {
				action = isFlagMode
					? ACTION_FLAG
					: ACTION_OPEN;
			}
			break;
		case 'x':
		case '\'':
			/* secondary select button */
			if (isdigit (board.array[x][y])) {
				/* check whether player clicked a number */
				action = ACTION_AUTO;
			} else {
				action = isFlagMode
					? ACTION_OPEN
					: ACTION_FLAG;
			}
			break;
		case 'r':
			freeBoardArray (&board);
			return GAME_RESTART;
		case KEY_UP:
		case 'w':
			cy--;
			break;
		case KEY_DOWN:
		case 's':
			cy++;
			break;
		case KEY_LEFT:
		case 'a':
			cx -= 2;
			break;
		case KEY_RIGHT:
		case 'd':
			cx += 2;
			break;
		}

		/* Round the cursor position down to nearest grid coordinate.
		   This is done in case the player uses the mouse and clicks 
		   on an off-character */
		if (cx % 2 == 0)
			cx--;
		
		/* if the cursor is out of bounds, place it back in the valid range */
		if (cy < 3)
			cy = 3;
		if (cy > 2 + yDim)
			cy = 2 + yDim;
		if (cx < 5)
			cx = 5;
		if (cx > 3 + 2 * xDim)
			cx = 3 + 2 * xDim;
		
		/* finally, move the cursor */
		move (cy, cx);
		refresh ();

		/* translate cursor coordinates to array coordinates */
		x = (cx - 2) / 2;
		y = cy - 2;
		
		/* switch to do board operations or open menu */
		switch (action) {
		case ACTION_OPEN:
			/* make sure that the player does not die on the first move */
			if (!firstClick) {
				buf = 0;
				while (numMines (board, x, y) > 0 || (board.array[x][y] & MASK_MINE)) {
					/* Re-randomize the mines until the current square has 0 neighbors.
					   This also guarantees that the first square chosen is not a mine. */
					initializeMines (&board);
					buf++;
					if (buf > 100) {
						/* After 100 tries, simply give up and instead remove the mine at
						   the current coordinate, to avoid locking up the game. This is
						   done because in some circumstances, it is not possible for the
						   first square to have 0 neighbors, such as if there are too many
						   mines in too small a field */
						board.mineCount--;
						initializeMines (&board);
						board.array[x][y] &= ~MASK_MINE;
						break;
					}
				}
			}
			
			/* if player selects a MINE square to uncover */
			if ((board.array[x][y] & MASK_MINE) && ((board.array[x][y] & MASK_CHAR) != 'P')) {
				clear ();
				clearok (stdscr, 0);
				overlayMines (&board);
				/* clear character and assign new value */
				board.array[x][y] &= MASK_MINE; board.array[x][y] |= '#';
				printBoard (board);
				printFrame (board);
				printCtrlsyx (0, hudOffset);
				mvaddstr (8, hudOffset, "You died! Game over.\n");
				refresh ();
				isAlive = false;
				exitGame = true;
			} else {
				openSquares (&board, x, y);
				firstClick = true;
			}
			break;
		case ACTION_FLAG:
			/* flag the current square */
			if ((board.array[x][y] & MASK_CHAR) == '+') {
				board.array[x][y] &= ~MASK_CHAR;	/* clear char */
				board.array[x][y] |= 'P';		/* assign char */
				flagsPlaced++;
			} else if ((board.array[x][y] & MASK_CHAR) == 'P') {
				board.array[x][y] &= ~MASK_CHAR;	/* clear char */
				board.array[x][y] |= '+';		/* assign char */
				flagsPlaced--;
			}
			break;
		case ACTION_AUTO:
			/* user selected a square holding a number */
			buf = 0;
			/* count number of adjacent flags */
			for (k = -1; k <= 1; k++) {
				for (h = -1; h <= 1; h++) {
					if ((board.array[x + h][y + k] & MASK_CHAR) == 'P')
						buf++;
				}
			}
			/* if number of adjacent flags == number displayed on square */
			if (buf == ((board.array[x][y] & MASK_CHAR) - '0')) {
				buf = 0;
				for (k = -1; k <= 1; k++) {
					for (h = -1; h <= 1; h++) {
						/* for each adjacent square */
						if ((board.array[x + h][y + k] & MASK_CHAR) == '+') {
							if (!(board.array[x + h][y + k] & MASK_MINE)) {
								/* if the square is not a mine */
								openSquares (&board, x + h, y + k);
							} else {
								/* if the square is a mine */
								buf = 1;
								board.array[x + h][y + k] &= ~MASK_CHAR;
								board.array[x + h][y + k] |= '#';
							}
						}
					}
				}
				if (buf == 1) {
					overlayMines (&board);
					move (1, 0);
					printBoard (board);
					printCtrlsyx (0, hudOffset);
					mvaddstr (8, hudOffset, "You died! Game over.\n");
					refresh ();
					isAlive = false;
					exitGame = true;
				}
			}
			break;
		case ACTION_ESCAPE:
			/* open the pause menu */
			clock_gettime (CLOCK_MONOTONIC, &timeMenu);
			
			printBlank (board);
			buf = menu (5, "Paused",
				"Return to game",
				"New game",
				"Save game",
				"Exit game",
				"View tutorial");

			clear ();
			printFrame (board);
			printBlank (board);
			printCtrlsyx (0, hudOffset);

			/* increment the time offset by the amount of time spent in menu */
			clock_gettime (CLOCK_MONOTONIC, &timeBuffer);
			subtractTimespec (&timeBuffer, &timeMenu);
			addTimespec (&timeOffset, &timeBuffer);
			
			/* Use the input from menu to decide what to do. Since we will
			   be prompting the user for input again and it might take time
			   for them to decide, we will start the menu counter again,
			   and then proceed to use the input. */
			clock_gettime (CLOCK_MONOTONIC, &timeMenu);
			switch (buf) {
			case -1:
			case MENU_NO_INPUT:
				break;
			case MENU_RESTART:
				/* ask user if they really want to restart */
				buf = mvmenu (7, hudOffset, 2, "Really restart?", "Yes", "No");
				clear ();
				if (buf == 1) break;

				freeBoardArray (&board);
				return GAME_RESTART;
			case MENU_EXIT_GAME:
				/* prompt user to save before exiting */
				buf = mvmenu (7, hudOffset, 3, "Save before exiting?", "Yes", "No", "Cancel");
				
				clock_gettime (CLOCK_MONOTONIC, &timeBuffer);
				subtractTimespec (&timeBuffer, &timeMenu);
				addTimespec (&timeOffset, &timeBuffer);
				
				if (buf == 1) {
					/* Exit without saving */
					exitGame = true;
					break;
				} else if (buf == 2 || buf == -1) {
					/* C = cancel, so don't actually exit */
					clear ();
					break;
				} else {
					/* buf == 0 is implied, so fall through to the save game case */
					exitGame = true;
					/* no break statement */
				}
			case MENU_SAVE_GAME:
				/* save the game */
				save.size = xDim * yDim;
				save.width = xDim;
				save.height = yDim;
				save.qtyMines = qtyMines;
				save.flagsPlaced = flagsPlaced;
				save.gameBools = 0;
				if (isFlagMode)
					save.gameBools |= MASK_FLAG_MODE;
				if (firstClick)
					save.gameBools |= MASK_FIRST_CLICK;
				save.cy = cy;
				save.cx = cx;
				clock_gettime (CLOCK_MONOTONIC, &timeBuffer);
				subtractTimespec (&timeBuffer, &timeOffset);
				save.timeOffset = timeBuffer;
				setGameData (board, &save);

				buf = writeSaveFile ("savefile", save);
				free (save.gameData);
				break;
			case MENU_TUTORIAL:
				tutorial ();
				clear ();
				break;
			}
			break;
		}
		if (exitGame) break;
		refresh ();
	}
	/* once player either has won or lost */
	move (19, 0);

	freeBoardArray (&board);
	/* if player exited through menu */
	if (action == ACTION_ESCAPE) return GAME_EXIT;
	/* GAME_FAILURE and GAME_SUCCESS are set to 0 and 1 respectively, hence why
	   returning the state of isAlive works. */
	else return isAlive;
}

void subtractTimespec (struct timespec * dest, struct timespec * src) {
	dest->tv_sec -= src->tv_sec;
	if (dest->tv_nsec - src->tv_nsec < 0) {
		/* borrow */
		dest->tv_sec--;
		dest->tv_nsec = 1000000000 + dest->tv_nsec - src->tv_nsec;
	}
	else {
		dest->tv_nsec -= src->tv_nsec;
	}
	return;
}

void addTimespec (struct timespec * dest, struct timespec * src) {
	dest->tv_sec += src->tv_sec;
	if (dest->tv_nsec + src->tv_nsec > 999999999) {
		/* carry */
		dest->tv_sec++;
		dest->tv_nsec = src->tv_nsec - dest->tv_nsec;
	}
	else {
		dest->tv_nsec += src->tv_nsec;
	}
	return;
}

double timespecToDouble (struct timespec spec) {
	double result = 0.0;
	result += spec.tv_sec;
	result += spec.tv_nsec / 1.0e9;
	return result;
}