/* game.c */

#include <stdlib.h>
#include <stdint.h>
#include <curses.h>
#include <math.h>	/* floorf */
#include <ctype.h>	/* toupper */
#include <time.h>	/* clock */

#include "gamefunctions.h"
#include "board.h"
#include "savegame.h"

int game (int xDim, int yDim, int qtyMines, Savegame * saveptr) {
	/*** INITIALIZATION ***/

	/* used for array reading/writing */
	int x = 0, y = 0, h, k;
	/* cursor coordinates */
	int cx, cy;
	/* x-offset of where to print the HUD */
	int hudOffset = 2 * xDim + 10;
	if (hudOffset < 18) hudOffset = 18;
	/* mouse event */
	MEVENT m_event;
	/* bools storing info about the game state */
	bool isFlagMode, firstClick;
	bool isAlive = true;
	bool exitGame = false;
	bool gotInput = false;
	bool setBreak = false;
	/* game duration */
	double gameDur;
	/* number of flags placed */
	int flagsPlaced;
	/* savegame object */
	Savegame save;

	/* struct storing locations of mines */
	Board mines;
	/* struct storing the state of the game board */
	Board vMem;

	/* stores whether the user used the primary or secondary button */
	uint8_t op = 0;
	/* stores what action will be performed by the game */
	uint8_t action = ACTION_NONE;
	/* variables for timekeeping */
	clock_t secsLastLoop = 0;
	clock_t timeOffset;
	clock_t menuTime;

	int buf = -1;
	
	/* initialize colors */
	init_pair (1, COLOR_WHITE, COLOR_BLACK);	/* default pair */
	init_pair (2, COLOR_BLACK, COLOR_WHITE);	/* inverted default */
	init_pair (3, COLOR_RED,   COLOR_BLACK);	/* for exploded mines and wrong flags */
	init_pair (4, COLOR_GREEN, COLOR_BLACK);	/* for correct flags and unexploded mines */
	init_pair (5, COLOR_CYAN,  COLOR_BLACK);	/* for numbers */

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
		timeOffset = -save.timeOffset;
		/* then initialize the boards */
		mines.width = vMem.width = xDim;
		mines.height = vMem.height = yDim;
		mines.mineCount = vMem.mineCount = qtyMines;
		initBoardArray (&mines);
		initBoardArray (&vMem);
		getGameData (&mines, &vMem, save);
		/* finally, remember to free the memory block used in *saveptr */
		free (saveptr->gameData);
	} else {
		/* otherwise, use the values passed in the first 3 params: */
		isFlagMode = false;
		flagsPlaced = 0;
		firstClick = false;
		/* cursor will be initialized at top left of game board */
		cy = 3, cx = 5;
		timeOffset = 0;
		/* initialize the boards */
		mines.width = vMem.width = xDim;
		mines.height = vMem.height = yDim;
		mines.mineCount = vMem.mineCount = qtyMines;
		initBoardArray (&mines);
		initBoardArray (&vMem);

		initializeMines (&mines);
	}

	/*** BEGIN GAMEPLAY ***/

	noecho ();
	clear ();
	secsLastLoop = clock ();
	gameDur = (clock () - timeOffset) / (float) CLOCKS_PER_SEC;

	while (isAlive) {
		printFrame (vMem);
		
		if (allClear (mines, vMem)) {
			/* only if player has won */
			overlayMines (mines, &vMem);
			printBoardCustom (vMem, false, (chtype)'F' | COLOR_PAIR (4), (chtype)'P' | COLOR_PAIR(3));
			printCtrlsyx (0, hudOffset);
			mvaddstr (8, hudOffset, "You won!\n");
			mvprintw (9, hudOffset, "Time: %.3f\n", gameDur);
			refresh ();
			break;
		}
		else {
			printBoard (vMem);
			printCtrlsyx (0, hudOffset);
			printFrame (vMem);
		}

		mvprintw (8, hudOffset, "Flags placed: %02d/%02d", flagsPlaced, qtyMines);
		mvprintw (9, hudOffset, "Mode: ");
		if (isFlagMode)
			printw ("Flag   | ");
		else
			printw ("Normal | ");
		if (!firstClick)
			gameDur = 0;
		printw ("Time: %03d", (int) floorf (gameDur));
		
		if (exitGame) {
			freeBoardArray (&mines);
			freeBoardArray (&vMem);
			return GAME_EXIT;
		}

		action = ACTION_NONE;
		setBreak = false;
		gotInput = false;
		op = 0;

		move (cy, cx);
		noecho ();
		if (!firstClick) {
			timeOffset = clock ();
			secsLastLoop = clock ();
		}

		/* set cursor to very visible */
		curs_set (2);
		nodelay (stdscr, true);

		/* This loop is responsible for taking user input.
		   The loop will be interrupted every time that one second passes to refresh the screen. */
		while (!gotInput) {
			/* arrow key input */
			buf = -1;
			while (buf < 0) {
				/* do until valid input is received */
				buf = wgetch (stdscr);
				/* if one second has passed since the last clock refresh: */
				if (clock () - secsLastLoop > CLOCKS_PER_SEC) {
					action = ACTION_INC_TIME;
					gameDur = (clock () - timeOffset) / (float) CLOCKS_PER_SEC;
					break;
				}
			}

			if (action == ACTION_INC_TIME) break;

			switch (buf) {
			case 'q':
			case 27: /* key code for Esc */
				action = ACTION_ESCAPE;
				gotInput = true;
				break;
			case KEY_MOUSE:
				getmouse (&m_event);
				cx = m_event.x;
				cy = m_event.y;
				if (m_event.bstate & BUTTON1_CLICKED) {
					action = ACTION_BOARD_OP;
					op = 1;
				}
				if (m_event.bstate & BUTTON3_CLICKED) {
					action = ACTION_BOARD_OP;
					op = 2;
				}
				gotInput = true;
				break;
			case 'm':
				action = ACTION_CHG_MODE;
				gotInput = true;
				break;
			case 'z':
			case '/':
				op = 1;
				action = ACTION_BOARD_OP;
				gotInput = true;
				break;
			case 'x':
			case '\'':
				op = 2;
				action = ACTION_BOARD_OP;
				gotInput = true;
				break;
			case 'r':
				return GAME_RESTART;
			case 10: /* key code for Return */
				getyx (stdscr, cy, cx);
				op = 1;
				action = ACTION_BOARD_OP;
				gotInput = true;
				break;
			case 32: /* key code for space */
				action = ACTION_CHG_MODE;
				gotInput = true;
				break;
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

			/* round the cursor position to nearest grid coordinate */
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
		}

		nodelay (stdscr, false);
		/* set cursor to mildly visible */
		curs_set (1);

		/* translate cursor coordinates to array coordinates */
		x = (cx - 2) / 2;
		y = cy - 2;

		/* make sure that the player does not die on the first move */
		buf = 0;
		if (!firstClick) {
			while (numMines (mines, x, y) > 0 || mines.array[x][y] == 'X') {
				initializeMines (&mines);
				buf++;
				if (buf > 100) break;
			}
			if (buf > 100) {
				mines.mineCount++;
				initializeMines (&mines);
				if (mines.array[x][y] == 'X') mines.array[x][y] = '+';
			}
		}

		if (gotInput && action == ACTION_BOARD_OP) firstClick = true;
		
		/* This switch is responsible for handling the user input gathered in the beginning. */
		switch (action) {
		case ACTION_BOARD_OP:
			if (0 <= x && x <= xDim + 1 && 0 <= y && y <= yDim + 1) {
				/* if a selection is made inside game board */
				if (mines.array[x][y] == 'X' && vMem.array[x][y] != 'P') {
					/* if user selects a MINE square to uncover */
					if ((!isFlagMode && op == 1) || (isFlagMode && op == 2)) {
						clear ();
						clearok (stdscr, 0);
						overlayMines (mines, &vMem);
						vMem.array[x][y] = '#';
						printBoard (vMem);
						printFrame (vMem);
						printCtrlsyx (0, hudOffset);
						mvaddstr (8, hudOffset, "You died! Game over.\n");
						refresh ();
						isAlive = false;
						setBreak = true;
						break;
					}
				}

				if ('1' <= vMem.array[x][y] && vMem.array[x][y] <= '9') {
					buf = 0;
					for (k = -1; k <= 1; k++) {
						for (h = -1; h <= 1; h++) {
							if (vMem.array[x + h][y + k] == 'P') buf++;
						}
					}
					if (buf == vMem.array[x][y] - 48) {
						buf = 0;
						for (k = -1; k <= 1; k++) {
							for (h = -1; h <= 1; h++) {
								/* for each adjacent square */
								if (vMem.array[x + h][y + k] == '+') {
									if (mines.array[x + h][y + k] != 'X') {
										/* if the square is not a mine */
										openSquares (mines, &vMem, x + h, y + k);
									} else {
										/* if the square is a mine */
										buf = 1;
										vMem.array[x + h][y + k] = '#';
									}
								}
							}
						}
						if (buf == 1) {
							overlayMines (mines, &vMem);
							move (1, 0);
							printBoard (vMem);
							printCtrlsyx (0, hudOffset);
							mvaddstr (8, hudOffset, "You died! Game over.\n");
							refresh ();
							isAlive = false;
							setBreak = true;
							break;
						}
					}
				}

				else {
					if (!isFlagMode) {
						/* game set to normal mode */
						switch (op) {
						case 1:
							openSquares (mines, &vMem, x, y);
							break;
						case 2:
							if (vMem.array[x][y] == '+') {
								vMem.array[x][y] = 'P';
								flagsPlaced++;
							} else if (vMem.array[x][y] == 'P') {
								vMem.array[x][y] = '+';
								flagsPlaced--;
							}
							break;
						}
					} else {
						/* game is set to flag mode */
						switch (op) {
						case 1:
							if (vMem.array[x][y] == '+') {
								vMem.array[x][y] = 'P';
								flagsPlaced++;
							} else if (vMem.array[x][y] == 'P') {
								vMem.array[x][y] = '+';
								flagsPlaced--;
							}
							break;
						case 2:
							openSquares (mines, &vMem, x, y);
							break;
						}
					}
				}
			}
			break;
		case ACTION_CHG_MODE:
			/* toggle flag mode */
			isFlagMode = !isFlagMode;
			break;
		case ACTION_ESCAPE:
			/* open the pause menu */
			menuTime = clock ();
			printBlank (vMem);
			
			buf = menu ();
			clear ();
			// printBoard (vMem);
			printFrame (vMem);
			printBlank (vMem);
			printCtrlsyx (0, hudOffset);

			timeOffset += clock () - menuTime;
			switch (buf) {
			case MENU_NO_INPUT:
				action = ACTION_ESCAPE;
				break;
			case MENU_RESTART:
				/* ask user if they really want to restart */
				mvprintw (10, hudOffset, "Really restart? (Y/N)");
				mvprintw (11, hudOffset, ">");
				buf = 0;
				while (!(buf == 'Y' || buf == 'N'))
					buf = toupper (getch ());

				clear ();
				if (buf == 'N') break;

				freeBoardArray (&mines);
				freeBoardArray (&vMem);
				return GAME_RESTART;
			case MENU_EXIT_GAME:
				/* prompt user to save before exiting */
				mvprintw (10, hudOffset, "Save before exiting? (Y/N/C)");
				mvprintw (11, hudOffset, ">");
				buf = 0;
				while (!(buf == 'Y' || buf == 'N' || buf == 'C'))
					buf = toupper (getch ());
				if (buf == 'N') {
					/* Exit without saving */
					exitGame = true;
					break;
				} else if (buf == 'C') {
					/* C = cancel, so don't actually exit */
					clear ();
					break;
				} else {
					/* buf == 'Y' is implied, so fall through to the save game case */
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
				save.timeOffset = clock () - timeOffset;
				setGameData (mines, vMem, &save);
				buf = writeSaveFile ("savefile", save);
				free (save.gameData);
				break;
			case MENU_TUTORIAL:
				tutorial ();
				clear ();
				break;
			}
			break;
		case ACTION_INC_TIME:
			secsLastLoop = clock ();
		}
		if (setBreak) break;
		refresh ();
	}
	/* once player either has won or lost */
	move (19, 0);

	freeBoardArray (&mines);
	freeBoardArray (&vMem);
	if (action == ACTION_ESCAPE) return GAME_EXIT;
	/* GAME_FAILURE and GAME_SUCCESS are set to 0 and 1 respectively, hence why
	   returning the state of isAlive works. */
	else return isAlive;
}
