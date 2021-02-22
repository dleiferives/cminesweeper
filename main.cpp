// main.cpp

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

#define ACTION_NONE 	0
#define ACTION_BOARD_OP	1
#define ACTION_CHG_MODE	2
#define ACTION_ESCAPE	3
#define ACTION_INC_TIME	4

int main (int argc, char* argv[]) {
	bool gotInput = 0;	// received command line arguments from user
	int exitCode = 1;	// status returned by game ()
	int input = 0;		
	int xDim = 9, yDim = 9, qtyMines = 10;
	srand (time(0));
	initscr ();
	keypad (stdscr, 1);
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
		gotInput = 1;
	}
	//gotInput = 1;

	if (xDim < 1)
		xDim = 1;
	if (xDim > 30)
		xDim = 30;
	if (yDim < 1)
		yDim = 1;
	if (yDim > 24)
		yDim = 24;
	if (qtyMines < 1)
		qtyMines = 1;
	if (qtyMines > xDim * yDim - 1)
		qtyMines = xDim * yDim - 1;

	start_color ();

	if (!gotInput) {
		printw ("Choose difficulty:\n");
		printw ("1) Beginner    : 9x9, 10 mines\n");
		printw ("2) Intermediate: 16x16, 40 mines\n");
		printw ("3) Advanced    : 30x24, 99 mines\n>");
		while (!(('1' <= input && input <= '3') || input == 27))
			input = getch ();

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
		//case '4':
		//	break;
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

	while (0 <= exitCode && exitCode <= 4) { // the four valid exit codes
		clear();
		exitCode = game (yDim, xDim, qtyMines);
		if (exitCode == 0 || exitCode == 1) { // won or lost
			echo();
			mvprintw (10, 70, "Play again? (Y/N)");
			mvprintw (11, 70, ">");
			input = toupper (wgetch (stdscr));
			
			if (input == 'N') break;
		}
		if (exitCode == 2) { // manual exit
			echo();
			mvprintw (10, 70, "Game closed. Exit now? (Y/N)");
			mvprintw (11, 70, ">");
			input = toupper (wgetch (stdscr));
			if (input == 'Y' || input == 10) break;
		}
	}
	
	endwin ();
	return 0;
}

int game(int yDim, int xDim, int qtyMines) {
	int x = 0, y = 0; // vars for array navigation
	int h, k;
	int cx = 5, cy = 3; // vars for console navigation: initialized at top left of game board
	MEVENT m_event;
	bool isFlagMode = 0, gotInput = 0; // flags
	bool isAlive = 1, exitGame = 0;
	bool firstClick = 0;
	bool setBreak = 0;
	double gameDur = 0;

	Board mines = {.width = xDim, .height = yDim, .mineCount = qtyMines};
	initBoardArray (&mines);
	Board vMem = {.width = xDim, .height = yDim, .mineCount = qtyMines};
	initBoardArray (&vMem);

	/* variable to store whether the user uses the primary or secondary button */
	uint8_t op = 0;
	uint8_t action = ACTION_NONE;
	long secsLastLoop = 0;
	long timeOffset = 0;
	long menuTime;
	int buf = -1;
	int flagsPlaced = 0;
	
	// initialize colors
	init_pair(1, COLOR_WHITE, COLOR_BLACK); // default pair
	init_pair(2, COLOR_BLACK, COLOR_WHITE); // inverted default
	init_pair(3, COLOR_RED,   COLOR_BLACK); // for exploded mines and wrong flags
	init_pair(4, COLOR_GREEN, COLOR_BLACK); // for correct flags and unexploded mines
	init_pair(5, COLOR_CYAN,  COLOR_BLACK); // for numbers

	// initialize array of elements to be displayed
	for (y = 0; y < vMem.height + 2; y++) {
		for (x = 0; x < vMem.width + 2; x++) {
			vMem.array[x][y] = '+';
		}
	}

	//now begin gameplay
	noecho();
	clear();
	initializeMines(&mines);
	secsLastLoop = clock();

	while (isAlive) {
		move(0, 0);
		addstr("+= Minesweeper ");
		for (x = 4; x < xDim; x++) addstr("==");
		addstr("=+\n");
		
		if (allClear(mines, vMem)) { // only if player has won
			overlayMines (mines, &vMem);
			printBoard(vMem, false, (chtype)'F' | COLOR_PAIR(4));
			printCtrls(0, 70);
			addstr("+==============");
			for (x = 4; x < xDim; x++) addstr("==");
			mvaddstr(8, 70, "You won!\n");
			mvprintw (9, 70, "Time: %.3f\n", gameDur);
			refresh();
			break;
		}
		else {
			printBoard(vMem);
			printCtrls(0, 70);
			addstr("+==============");
			for (x = 4; x < xDim; x++) addstr("==");
			addstr("=+\n");
		}

		mvprintw (8, 70, "Flags placed: %02d/%02d", flagsPlaced, qtyMines);
		mvprintw (9, 70, "Mode: ");
		if (isFlagMode) printw ("Flag   | ");
		else printw ("Normal | ");
		if (!firstClick) gameDur = 0;
		//printw ("firstclick=%d | offset = %ld ", firstClick, timeOffset);
		printw ("Time: %03d", (int)floorf(gameDur));
		
		if (exitGame) {
			freeBoardArray (&mines);
			freeBoardArray (&vMem);
			return 2;
		}

		action = ACTION_NONE;
		setBreak = 0;
		gotInput = 0;
		op = 0;

		move (cy, cx);
		noecho ();
		if (!firstClick) {
			timeOffset = clock ();
			secsLastLoop = clock ();
		}

		// This loop is responsible for gathering user input.
		// The loop will be interrupted every time that one second passes to refresh the screen.
		curs_set (2); // cursor very visible
		nodelay (stdscr, 1);

		while (!gotInput) { // arrow key input
			buf = -1;
			while (buf < 0) { // until valid input
				buf = wgetch(stdscr);
				if (clock() - secsLastLoop > CLOCKS_PER_SEC) {
					action = ACTION_INC_TIME;
					gameDur = ((float)(clock() - timeOffset)) / CLOCKS_PER_SEC;
					break;
				}
			}

			if (action == ACTION_INC_TIME) break;

			switch (buf) {
			case 27: // esc
				action = ACTION_ESCAPE;
				gotInput = 1;
				break;
			case KEY_MOUSE:
				getmouse(&m_event);
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
				gotInput = 1;
				break;
			case 'm':
				action = ACTION_CHG_MODE;
				gotInput = 1;
				break;
			case 'z':
			case '/':
				op = 1;
				action = ACTION_BOARD_OP;
				gotInput = 1;
				break;
			case 'x':
			case '\'':
				op = 2;
				action = ACTION_BOARD_OP;
				gotInput = 1;
				break;
			case 'r':
				return 3;
			case 10: // enter
				getyx(stdscr, cy, cx);
				op = 1;
				action = ACTION_BOARD_OP;
				gotInput = 1;
				break;
			case 32: // space
				action = ACTION_CHG_MODE;
				gotInput = 1;
				break;
			case KEY_UP:
			case 'w':
				if (4 <= cy && cy <= yDim + 2) cy--;
				break;
			case KEY_DOWN:
			case 's':
				if (3 <= cy && cy <= yDim + 1) cy++;
				break;
			case KEY_LEFT:
			case 'a':
				if (7 <= cx && cx <= 2 * xDim + 3) cx -= 2;
				break;
			case KEY_RIGHT:
			case 'd':
				if (5 <= cx && cx <= 2 * xDim + 1) cx += 2;
				break;
			}
			move(cy, cx);
			refresh();
		}

		nodelay (stdscr, 0);
		curs_set (1); // cursor mildly visible

		cx = (cx % 2 == 0)
			? cx - 1
			: cx;
		x = (cx - 2) / 2;
		y = cy - 2;

		// this is to make sure that the player does not die on the first move
		buf = 0;
		if (!firstClick) {
			while (numMines (mines, x, y) > 0 || mines.array[x][y] == 'X') {
				initializeMines(&mines);
				buf++;
				if (buf > 100) break;
			}
			if (buf > 100) {
				mines.mineCount++;
				initializeMines(&mines);
				if (mines.array[x][y] == 'X')	mines.array[x][y] = '.';
			}
		}

		if (gotInput && action != 2 && action != 3) firstClick = 1;
		
		// This switch is responsible for handling the user input gathered in the beginning.
		switch (action) {
		case 1: // if board operation
			if (0 <= x && x <= xDim + 1 && 0 <= y && y <= yDim + 1) { // selection is made inside game board 
				if (mines.array[x][y] == 'X' && vMem.array[x][y] != 'P') { // if user selects a MINE square to uncover
					if ((!isFlagMode && op == 1) || (isFlagMode && op == 2)) {
						clear();
						clearok(stdscr, 0);
						overlayMines (mines, &vMem);
						vMem.array[x][y] = '#';
						addstr("+= Minesweeper ");
						for (x = 4; x < xDim; x++) addstr("==");
						addstr("=+\n");
						printBoard(vMem);
						printCtrls(0, 70);
						addstr("+==============");
						for (x = 4; x < xDim; x++) addstr("==");
						addstr("=+\n");
						mvaddstr(8, 70, "You died! Game over.\n");
						refresh();
						isAlive = 0;
						setBreak = 1;
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
							for (h = -1; h <= 1; h++) { // for each adjacent square
								if (vMem.array[x + h][y + k] == '+') {
									if (mines.array[x + h][y + k] != 'X') { // if not mine
										vMem.array[x + h][y + k] = numMines (mines, x + h, y + k) + 48;
										if (vMem.array[x + h][y + k] == '0') vMem.array[x + h][y + k] = ' ';
										openSquares(mines, &vMem, x + h, y + k);
									}
									else { // is mine
										buf = 1;
										vMem.array[x + h][y + k] = '#';
									}
								}
							}
						}
						if (buf == 1) {
							overlayMines (mines, &vMem);
							move(1, 0);
							printBoard(vMem);
							printCtrls(0, 70);
							mvaddstr(8, 70, "You died! Game over.\n");
							refresh();
							isAlive = 0;
							setBreak = 1;
							break;
						}
					}
				}

				else {
					if (!isFlagMode) { // norm mode
						switch (op) {
						case 1:
							if (vMem.array[x][y] == '+' || vMem.array[x][y] == ' ') { // square not flagged
								vMem.array[x][y] = 48 + numMines (mines, x, y);
								if (vMem.array[x][y] == '0') vMem.array[x][y] = ' ';
								openSquares(mines, &vMem, x, y);
							}
							break;
						case 2:
							if (vMem.array[x][y] == '+') {
								vMem.array[x][y] = 'P';
								flagsPlaced++;
							}
							else if (vMem.array[x][y] == 'P') {
								vMem.array[x][y] = '+';
								flagsPlaced--;
							}
							break;
						}
					}
					// flag mode
					else {
						switch (op) {
							//gameDur+=10;
						case 1:
							if (vMem.array[x][y] == '+') {
								vMem.array[x][y] = 'P';
								flagsPlaced++;
							}
							else if (vMem.array[x][y] == 'P') {
								vMem.array[x][y] = '+';
								flagsPlaced--;
							}
							break;
						case 2:
							if (vMem.array[x][y] == '+' || vMem.array[x][y] == ' ') { // square not flagged
								vMem.array[x][y] = 48 + numMines (mines, x, y);
								if (vMem.array[x][y] == '0') vMem.array[x][y] = ' ';
								openSquares(mines, &vMem, x, y);
							}
							break;
						}
					}
				}
			}
			break;
		case 2: // change mode
			isFlagMode = !isFlagMode;
			break;
		case 3: // pause menu
			menuTime = clock();
			printBlank (yDim, xDim);
			buf = menu();
			timeOffset += clock() - menuTime;
			switch (buf) {
			case 0:
				action = ACTION_ESCAPE;
				break;
			case 1:
				freeBoardArray (&mines);
				freeBoardArray (&vMem);
				return 3;
			case 2:
				exitGame = 1;
				break;
			case 4:
				tutorial();
				break;
			}
			break;
		case 4: // update clock
			secsLastLoop = clock();
		}
		if (setBreak) break;
		refresh();
	}
	// once player either has won or lost
	move(18, 0);

	freeBoardArray (&mines);
	freeBoardArray (&vMem);
	if (action == ACTION_ESCAPE) return 2;
	else return isAlive;
}
