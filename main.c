/* main.c */

/* TODO:
   write a function to draw a menu based on a number of options,
   which returns the index of the option chosen */

#include <stdlib.h>	
#include <stdio.h>	
#include <curses.h>	
#include <stdint.h>	
#include <string.h>	/* strcmp */
#include <ctype.h>	/* toupper */
#include <math.h>	/* floorf */
#include <time.h>	/* time */

#include "gamefunctions.h"
#include "splash.h"

int main (int argc, char* argv[]) {
	/* if the user provided valid command line arguments */
	bool gotArgs = false;
	/* status returned by game () */
	int exitCode = GAME_SUCCESS;
	/* vars for menu navigation */
	int input = 0;
	int option;
	bool gotInput = false;
	/* dimensions of the game board */
	int xDim = 9, yDim = 9;
	int qtyMines = 10;
	/* x-offset of where to print the HUD */
	int hudOffset;
	/* dimensions of the terminal */
	int termWidth, termHeight;

	/* savegame struct to load game */
	Savegame savegame;
	Savegame * saveptr = &savegame;
	/* loadSaveFile returns -1 if error opening file */
	bool saveFileExists = (loadSaveFile ("savefile", saveptr) != -1);

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
	curs_set (0);
	clear ();
	refresh ();

	if (argc >= 5 && strcmp (argv[argc - 4], "custom") == 0) {
		xDim = atoi (argv[argc - 3]);
		yDim = atoi (argv[argc - 2]);
		qtyMines = atoi (argv[argc - 1]);
		gotArgs = true;
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

	start_color ();

	/* determine whether to load game or start new game */
	if (!gotArgs) {
		/* if save file does not exist, then automatically start new game */
		if (!saveFileExists) 
			gotInput = true;
		/* always default to first option (new game) */
		option = 0;
		while (!gotInput) {
			/* menu defaults to first option */
			mvprintw (0, 0, "Welcome to Minesweeper!\n\n");
			mvprintw (1, 0, " 1) New game");
			mvprintw (2, 0, " 2) Load game");

			/* draw option pointer */
			mvaddch (option + 1, 3, '>' | A_BLINK);

			refresh();
			
			input = getch ();
			switch (input) {
			case 'q':
			case 27: /* key code for Esc */
				option = -1;
				exitCode = -1;
				gotInput = true;
				break;
			case '1':
				/* new game */
				option = 0;
				gotInput = true;
				break;
			case '2':
				/* load game */
				option = 1;
				gotInput = true;
				break;
			case 10: /* key code for Return */
				/* selects the current option */
				gotInput = true;
				break;
			case 'w':
			case KEY_UP:
				option--;
				if (option < 0) option = 0;
				break;
			case 'a':
			case KEY_LEFT:
				option--;
				if (option < 0) option = 0;
				break;
			case 's':
			case KEY_DOWN:
				option++;
				if (option > 1) option = 1;
				break;
			case 'd':
			case KEY_RIGHT:
				option++;
				if (option > 1) option = 1;
				break;
			}
		}

		if (option == 0) {
			/* user chooses to start new game */
			saveptr = NULL;

			mvprintw (0, 0, "Choose difficulty:\n");
			printw ("1) Beginner    : 9x9, 10 mines\n");
			printw ("2) Intermediate: 16x16, 40 mines\n");
			printw ("3) Advanced    : 30x24, 99 mines\n>");

			input = 0;
			while (!(('1' <= input && input <= '3') || input == 'Q'))
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
		} else {
			/* user chooses to load game */
			loadSaveFile ("savefile", saveptr);
		}
	}

	/* set the offset now that the dimensions have been set */
	hudOffset = 2 * xDim + 10;
	if (hudOffset < 18) hudOffset = 18;

	/*** PLAY THE GAME ***/

	/* do while exitCode is one of the 4 valid values */
	while (0 <= exitCode && exitCode <= 4) {
		clear ();
		exitCode = game (xDim, yDim, qtyMines, saveptr);
		if (exitCode == GAME_FAILURE || exitCode == GAME_SUCCESS) {
			/* if player won or lost */
			mvprintw (10, hudOffset, "Play again? (Y/N)");
			mvprintw (11, hudOffset, ">");
			input = 0;
			while (!(input == 'Y' || input == 'N'))
				input = toupper (getch ());
			if (input == 'N')
				break;
		} else if (exitCode == GAME_EXIT) {
			/* if player exited manually */
			echo ();
			break;
		} else if (exitCode == GAME_RESTART) {
			/* don't allow the user to restart a loaded game */
			saveptr = NULL;
		}
	}
	echo ();
	endwin ();
	return 0;
}
