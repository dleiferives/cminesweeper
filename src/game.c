/*
 * game.c
 *
 * Defines the function that plays the game, called by main.
 */

/* TODO:
   - add smiley face */ 

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <curses.h>
#include <math.h>	/* floorf */
#include <ctype.h>	/* toupper */
#include <time.h>	/* timespec, usleep */

#include "util.h"
#include "board.h"
#include "savegame.h"
#include "menu.h"

#include <unistd.h>	/* usleep */
/* use a define statement because usleep isn't portable to windows */
#define Sleep(ms) usleep((ms * 1000))

/* timespec utility functions */
void subtractTimespec(struct timespec *dest, struct timespec *src);	/* adds src to dest */
void addTimespec(struct timespec *dest, struct timespec *src);		/* subtracts src from dest */
double timespecToDouble(struct timespec spec);							/* converts a timespec interval to a float value */

/* game() will always work beginning from a saved state. When the game is saved,
   it is saved in *state. game() expects that *state be fully initialized when
   it is called. */
int game(Savegame *state) {
	/*** DECLARATIONS ***/
	
	int buf;	/* general purpose buffer */
	
	/* variables for navigation */
	int x = 1, y = 1;	/* absolute array coordinates */
	int h, k;			/* relative array coordinates */
	int cx, cy;			/* cursor coordinates */
	int hudOffset;		/* x-offset of where to print the HUD */
	
	/* variables storing info about the game state */
	bool isFlagMode;		/* flag mode is enabled */
	bool firstClick;		/* the first click of the game has been made */
	bool isAlive = true;	/* the player is alive */
	bool exitGame = false;	/* the game is set to exit */
	int flagsPlaced;		/* number of flags placed */
	int xDim, yDim;			/* dimensions of the board */
	int qtyMines;			/* number of mines in play */

	Board board;	/* struct storing the state of the game board */
	MEVENT m_event;	/* mouse event */

	/* structs for timekeeping */
	struct timespec timeOffset;	/* running counter to adjust time calculation */
	struct timespec timeMenu;	/* time spent in the menu */
	struct timespec timeBuffer;	/* buffer used in time calculations */

	/* stores what action will be performed by the game */
	uint8_t action;
	
	/*** INITIALIZATION ***/

	if (state == NULL)
		return GAME_FAILURE;

	/* start with game status variables */
	xDim = state->width;
	yDim = state->height;
	qtyMines = state-> qtyMines;
	clock_gettime(CLOCK_MONOTONIC, &timeOffset);		/* set offset to current time */
	subtractTimespec(&timeOffset, &state->timeOffset);	/* subtract the game duration */
	board.width = xDim;
	board.height = yDim;
	board.mineCount = qtyMines;
	initBoardArray(&board);
	if (state->gameData == NULL) {
		/* defaults for new games */
		isFlagMode = false;
		firstClick = false;
		flagsPlaced = 0;
		cy = 1;
		cx = 1;
		initializeMines(&board);
	} else {
		/* only do this if gameData was initialized from a previous save file */
		isFlagMode = ((state->gameBools & MASK_FLAG_MODE) != 0);
		firstClick = ((state->gameBools & MASK_FIRST_CLICK) != 0);
		flagsPlaced = state->flagsPlaced;
		cy = state->cy;
		cx = state->cx;
		getGameData(&board, *state);
		free(state->gameData);
		state->gameData = NULL;
	}

	/* set the hudOffset */
	hudOffset = 2 * board.width + 3;
	if (hudOffset < 18) hudOffset = 18;

	/*** BEGIN GAMEPLAY ***/

	noecho();
	curs_set(0);	/* cursor invisible */
	clear();
	
	while (isAlive) {
		if (!firstClick)
			clock_gettime(CLOCK_MONOTONIC, &timeOffset);
		
		/* calculate duration of the game */
		clock_gettime(CLOCK_MONOTONIC, &timeBuffer);
		subtractTimespec(&timeBuffer, &timeOffset);	/* duration is now stored in timeBuffer */
		
		printFrame(board);
		printCtrlsyx(0, hudOffset);
		if (allClear(board)) {
			/* Break if player has won; note that isAlive is still set to true */
			break;
		} else {
			/* player hasn't won yet */
			mvprintw(7, hudOffset, "[ %02d/%02d ][ %03d ]", flagsPlaced, qtyMines, (int) floorf(timespecToDouble(timeBuffer)));
			printBoard(board);
			mvaddstr(8, hudOffset,
				isFlagMode
				? "[ Flag mode    ]"
				: "[ Normal mode  ]"
			);
			move(cy, cx);
		}

		/* draw virtual cursor, colored based on the character under it */
		buf = board.array[x][y] & MASK_CHAR;
		if (isdigit(buf)) {
			/* color for numbers */
			chgat(2, A_REVERSE, 5, NULL);
		} else if (buf == 'P') {
			/* color for flags */
			chgat(2, A_REVERSE, 3, NULL);
		} else {
			/* default color */
			chgat(2, A_REVERSE, 1, NULL);
		}
		refresh();

		/* get input */
		nodelay(stdscr, true);
		buf = getch();
		nodelay(stdscr, false);
		Sleep(16); /* sleep 1/60th of a second */

		/* TODO:
		   Reorder switch cases in an order closer to descending probability */
		   
		/* switch to process keystrokes */
		action = ACTION_NONE;
		switch (buf) {
		case 'q':
		case 27: /* key code for Esc */
			/* open menu */
			action = ACTION_ESCAPE;
			break;
		case 'S' & 0x1f: /* Ctrl+S */
		case 'E':
			/* quicksave */
			action = ACTION_SAVE;
			break;
		case KEY_MOUSE:
			getmouse(&m_event);
			cx = m_event.x;
			cy = m_event.y;

			if (m_event.bstate & BUTTON1_CLICKED) {
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
			action = isFlagMode
				? ACTION_FLAG
				: ACTION_OPEN;
			break;
		case 'x':
		case '\'':
			/* secondary select button */
			action = isFlagMode
				? ACTION_OPEN
				: ACTION_FLAG;
			break;
		case 'r':
			freeBoardArray(&board);
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
		/* note capital letters:
		   player can hold shift to navigate faster */
		case 'W':
			cy -= 2;
			break;
		case 'S':
			cy += 2;
			break;
		case 'A':
			cx -= 4;
			break;
		case 'D':
			cx += 4;
			break;
		}

		/* Round the cursor position down to nearest grid coordinate.
		   This is done in case the player uses the mouse and clicks 
		   on an off-character */
		if (cx % 2 == 0)
			cx--;
		
		/* if the cursor is out of bounds, place it back in the valid range */
		if (cy < 1)
			cy = 1;
		if (cy > + yDim)
			cy = + yDim;
		if (cx < 1)
			cx = 1;
		if (cx > 2 * xDim - 1)
			cx = 2 * xDim - 1;

		/* translate cursor coordinates to array coordinates */
		x = cx / 2 + 1;
		y = cy;

		/* check whether player clicked a number */
		if (isdigit(board.array[x][y]) && (action == ACTION_OPEN || action == ACTION_FLAG))
			action = ACTION_AUTO;
		
		/* switch to do board operations or open menu */
		switch (action) {
		case ACTION_OPEN:
			/* make sure that the player does not die on the first move */
			if (!firstClick) {
				buf = 0;
				while (numMines(board, x, y) > 0 || (board.array[x][y] & MASK_MINE)) {
					/* Re-randomize the mines until the current square has 0 neighbors.
					   This also guarantees that the first square chosen is not a mine. */
					initializeMines(&board);
					buf++;
					if (buf > 100) {
						/* After 100 tries, simply give up and instead remove the mine at
						   the current coordinate, to avoid locking up the game. This is
						   done because in some circumstances, it is not possible for the
						   first square to have 0 neighbors, such as if there are too many
						   mines in too small a field */
						board.mineCount--;
						initializeMines(&board);
						board.array[x][y] &= ~MASK_MINE;
						break;
					}
				}
			}
			
			/* if player selects a MINE square to uncover */
			if ((board.array[x][y] & MASK_MINE) && ((board.array[x][y] & MASK_CHAR) != 'P')) {
				/* clear character and assign new value */
				board.array[x][y] &= MASK_MINE; board.array[x][y] |= '#';
				isAlive = false;
			} else {
				openSquares(&board, x, y);
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
				for (k = -1; k <= 1; k++) {
					for (h = -1; h <= 1; h++) {
						/* for each adjacent square */
						if ((board.array[x + h][y + k] & MASK_CHAR) == '+') {
							if (!(board.array[x + h][y + k] & MASK_MINE)) {
								/* if the square is not a mine */
								openSquares(&board, x + h, y + k);
							} else {
								/* if the square is a mine */
								isAlive = false;
								board.array[x + h][y + k] &= ~MASK_CHAR;
								board.array[x + h][y + k] |= '#';
							}
						}
					}
				}
			} else {
				beep();
			}
			break;
		case ACTION_ESCAPE:
			/* open the pause menu */
			clock_gettime(CLOCK_MONOTONIC, &timeMenu);
			
			printBlank(board);
			buf = menu(5, "Paused",
				"Return to game ",
				"Restart",
				"Save game",
				"Main menu",
				"View tutorial");

			clear();
			printFrame(board);
			printBlank(board);
			printCtrlsyx(0, hudOffset);

			/* increment the time offset by the amount of time spent in menu */
			clock_gettime(CLOCK_MONOTONIC, &timeBuffer);
			subtractTimespec(&timeBuffer, &timeMenu);
			addTimespec(&timeOffset, &timeBuffer);
			
			/* Use the input from menu to decide what to do. Since we will
			   be prompting the user for input again and it might take time
			   for them to decide, we will start the menu counter again,
			   and then proceed to use the input. */
			clock_gettime(CLOCK_MONOTONIC, &timeMenu);
			switch (buf) {
			case -1:
			case 0:
				/* return to game */
				break;
			case 1:
				/* restart */
				/* ask user if they really want to restart */
				buf = mvmenu(7, hudOffset, 2, "Really restart?", "Yes", "No");
				clear();
				if (buf == 1) break;

				freeBoardArray(&board);
				return GAME_RESTART;
			case 3:
				/* main menu */
				/* prompt user to save before exiting */
				buf = mvmenu(7, hudOffset, 3, "Save before exiting?", "Yes", "No", "Cancel");
				
				clock_gettime(CLOCK_MONOTONIC, &timeBuffer);
				subtractTimespec(&timeBuffer, &timeMenu);
				addTimespec(&timeOffset, &timeBuffer);
				
				if (buf == 1) {
					/* Exit without saving */
					exitGame = true;
					break;
				} else if (buf == 2 || buf == -1) {
					/* cancel, so don't actually exit */
					clear();
					break;
				} else {
					/* buf == 0 is implied, so fall through to the save game case */
					action = ACTION_SAVE;
					exitGame = true;
				}
				break;
			case 2:
				/* save game */
				action = ACTION_SAVE;
				break;
			case 4:
				/* tutorial */
				tutorial();
				curs_set(0);
				clear();
				break;
			}
			if (action != ACTION_SAVE)
				break;
		case ACTION_SAVE:
			/* save the game */
			state->size = xDim * yDim;
			state->width = xDim;
			state->height = yDim;
			state->qtyMines = qtyMines;
			state->flagsPlaced = flagsPlaced;
			state->gameBools = 0;
			if (isFlagMode)
				state->gameBools |= MASK_FLAG_MODE;
			if (firstClick)
				state->gameBools |= MASK_FIRST_CLICK;
			state->cy = cy;
			state->cx = cx;
			clock_gettime(CLOCK_MONOTONIC, &timeBuffer);
			subtractTimespec(&timeBuffer, &timeOffset);
			state->timeOffset = timeBuffer;
			setGameData(board, state);

			buf = writeSaveFile("savefile", *state);
			if (buf == -1) {
				/* save error */
				mvmenu(7, hudOffset, 1, "Error saving game!", "I understand");
				clear();
			}
			free(state->gameData);
			break;
		}
		if (exitGame) break;
		refresh();
	}
	
	if (isAlive) {
		overlayMines(&board);
		printBoardCustom(board, false, COLOR_PAIR(4) | A_BOLD);
		mvprintw(7, hudOffset, "[ %02d/%02d ][ %3.3f ]" , flagsPlaced, qtyMines, timespecToDouble(timeBuffer));
		mvprintw(8, hudOffset, "[ You won!        ]");
		refresh();
	} else {
		clear();
		overlayMines(&board);
		printBoard(board);
		printFrame(board);
		printCtrlsyx(0, hudOffset);
		mvaddstr(8, hudOffset, "You died! Game over.\n");
		refresh();

		/* if this game was loaded from a save file, delete that save file */
		if (state != NULL) {
			/* this is done so that the player only ever has one chance to play a
			   particular game; i.e., once you die, you can't try again. */
			//removeSaveFile("savefile");
			/* TODO:
			   Find a way to delete the save file at the end of a game, ONLY if
			   the current game was from a save file. */
		}
	}

	freeBoardArray(&board);
	/* if player exited through menu */
	if (exitGame) return GAME_EXIT;
	/* GAME_FAILURE and GAME_SUCCESS are set to 0 and 1 respectively, hence why
	   returning the state of isAlive works. */
	return isAlive;
}

void subtractTimespec(struct timespec *dest, struct timespec *src) {
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

void addTimespec(struct timespec *dest, struct timespec *src) {
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

double timespecToDouble(struct timespec spec) {
	double result = 0.0;
	result += spec.tv_sec;
	result += spec.tv_nsec / 1.0e9;
	return result;
}