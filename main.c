/* main.c */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <curses.h>
#include <stdint.h>

#include "gamefunctions.h"
#include "splash.h"
#include "board.h"

int main (int argc, char* argv[]) {
	/* if the user provided command line arguments */
	bool gotInput = false;
	/* status returned by game () */
	int exitCode = GAME_SUCCESS;
	int input = 0;
	/* dimensions of the game board */
	int xDim = 9, yDim = 9;
	int qtyMines = 10;
	/* x-offset of where to print the HUD */
	int hudOffset;
	/* dimensions of the terminal */
	int termWidth, termHeight;

	srand (time (0));
	initscr ();
	keypad (stdscr, true);
	noecho ();
	
	clear ();
	mmask_t old;
	mousemask (ALL_MOUSE_EVENTS, &old);
	addstr (SPLASH);
	curs_set (0);
	wgetch (stdscr);
	curs_set (1);
	clear ();
	refresh ();

	if (argc >= 5 && strcmp (argv[argc - 4], "custom") == 0) {
		xDim = atoi (argv[argc - 3]);
		yDim = atoi (argv[argc - 2]);
		qtyMines = atoi (argv[argc - 1]);
		gotInput = true;
	}
	
	/* get terminal dimensions */
	getmaxyx(stdscr, termHeight, termWidth);

	/* set upper and lower limits for acceptable dimensions based on terminal dimensions */
	/* minimum dimension is always 1 */
	if (xDim < 1)
		xDim = 1;
	if (yDim < 1)
		yDim = 1;
	/* maxima will be set based on the terminal dimensions */
	if (2 * xDim + 49 > termWidth)
		xDim = (termWidth - 49) / 2;
	if (yDim + 6 > termHeight)
		yDim = termHeight - 6;
	if (qtyMines < 1)
		qtyMines = 1;
	if (qtyMines > xDim * yDim - 1)
		qtyMines = xDim * yDim - 1;
	
	/* set the offset now that the dimensions have been adjusted */
	hudOffset = 2 * xDim + 10;
	if (hudOffset < 18) hudOffset = 18;

	start_color ();

	if (!gotInput) {
		printw ("Choose difficulty:\n");
		printw ("1) Beginner    : 9x9, 10 mines\n");
		printw ("2) Intermediate: 16x16, 40 mines\n");
		printw ("3) Advanced    : 30x24, 99 mines\n>");
		while (!(('1' <= input && input <= '3') || input == 27 || input == 'Q'))
			input = toupper (getch ());

		switch (input) {
		case '1':
			xDim = 9;
			yDim = 9;
			qtyMines = 10;
			break;
		case '2':
			xDim = 16;
			yDim = 16;
			qtyMines = 40;
			break;
		case '3':
			xDim = 30;
			yDim = 24;
			qtyMines = 99;
			break;
		// case '4':
		// 	break;
		case 'Q':
		case 27:
			exitCode = -1;
			break;
		default:
			xDim = 9;
			yDim = 9;
			qtyMines = 10;
			break;
		}
	}

	/* do while exitCode is one of the 4 valid values */
	while (0 <= exitCode && exitCode <= 4) {
		clear ();
		exitCode = game (xDim, yDim, qtyMines);
		if (exitCode == GAME_FAILURE || exitCode == GAME_SUCCESS) {
			/* if player won or lost */
			echo ();
			mvprintw (10, hudOffset, "Play again? (Y/N)");
			mvprintw (11, hudOffset, ">");
			input = toupper (wgetch (stdscr));
			
			if (input == 'N') break;
		}
		if (exitCode == GAME_EXIT) {
			/* if player exited manually */
			echo ();
			break;
			mvprintw (10, hudOffset, "Game closed. Exit now? (Y/N)");
			mvprintw (11, hudOffset, ">");
			input = toupper (wgetch (stdscr));
			if (input == 'Y' || input == 10) break;
		}
		/* the last option is for exitCode to equal GAME_RESTART, but we don't
		   check against that, since the while loop would just continue to start
		   the next game anyways. */
	}
	
	endwin ();
	return 0;
}

int game (int xDim, int yDim, int qtyMines) {
	/* used for array reading/writing */
	int x = 0, y = 0, h, k;
	/* cursor coordinates: initialized at top left of game board */
	int cx = 5, cy = 3;
	/* x-offset of where to print the HUD */
	int hudOffset = 2 * xDim + 10;
	if (hudOffset < 18) hudOffset = 18;
	/* mouse event */
	MEVENT m_event;
	/* bools storing info about the game state */
	bool isFlagMode = false, gotInput = false;
	bool isAlive = true, exitGame = false;
	bool firstClick = false;
	bool setBreak = false;
	/* game duration */
	double gameDur = 0;

	/* struct storing locations of mines */
	Board mines;
	mines.width = xDim;
	mines.height = yDim;
	mines.mineCount = qtyMines;
	initBoardArray (&mines);

	/* struct storing the state of the game board */
	Board vMem;
	vMem.width = xDim;
	vMem.height = yDim;
	vMem.mineCount = qtyMines;
	initBoardArray (&vMem);

	/* stores whether the user used the primary or secondary button */
	uint8_t op = 0;
	/* stores what action will be performed by the game */
	uint8_t action = ACTION_NONE;
	/* variables for timekeeping */
	long secsLastLoop = 0;
	long timeOffset = 0;
	long menuTime;

	int buf = -1;
	int flagsPlaced = 0;
	
	/* initialize colors */
	init_pair (1, COLOR_WHITE, COLOR_BLACK);	/* default pair */
	init_pair (2, COLOR_BLACK, COLOR_WHITE);	/* inverted default */
	init_pair (3, COLOR_RED,   COLOR_BLACK);	/* for exploded mines and wrong flags */
	init_pair (4, COLOR_GREEN, COLOR_BLACK);	/* for correct flags and unexploded mines */
	init_pair (5, COLOR_CYAN,  COLOR_BLACK);	/* for numbers */

	/* initialize array of elements to be displayed */
	for (y = 0; y < vMem.height + 2; y++) {
		for (x = 0; x < vMem.width + 2; x++) {
			vMem.array[x][y] = '+';
		}
	}

	/* now begin gameplay */
	noecho ();
	clear ();
	initializeMines (&mines);
	secsLastLoop = clock ();

	while (isAlive) {
		printFrame (vMem);
		
		if (allClear (mines, vMem)) {
			/* only if player has won */
			overlayMines (mines, &vMem);
			printBoardCustom (vMem, false, (chtype)'F' | COLOR_PAIR (4), (chtype)'P' | COLOR_PAIR(3));
			printCtrlsyx (0, hudOffset);
			// addstr ("+==============");
			// for (x = 4; x < xDim; x++) addstr ("==");
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
		if (isFlagMode) printw ("Flag   | ");
		else printw ("Normal | ");
		if (!firstClick) gameDur = 0;
		printw ("Time: %03d", (int)floorf (gameDur));
		
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
				if (clock () - secsLastLoop > CLOCKS_PER_SEC) {
					action = ACTION_INC_TIME;
					gameDur = ((float)(clock () - timeOffset)) / CLOCKS_PER_SEC;
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
				if (mines.array[x][y] == 'X')	mines.array[x][y] = '.';
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
			printBoard (vMem);
			printFrame (vMem);
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
				/* ask user if they really want to exit */
				mvprintw (10, hudOffset, "Really exit? (Y/N)");
				mvprintw (11, hudOffset, ">");
				buf = 0;
				while (!(buf == 'Y' || buf == 'N'))
					buf = toupper (getch ());
				if (buf == 'Y')
					exitGame = true;

				clear ();
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
