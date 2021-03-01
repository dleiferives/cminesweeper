/* main.c */

#include <stdlib.h>	
#include <stdio.h>	
#include <curses.h>	
#include <stdint.h>	
#include <string.h>	/* strcmp */
#include <ctype.h>	/* toupper */
#include <math.h>	/* floorf */
#include <time.h>	/* time */

#include "gamefunctions.h"
#include "savegame.h"
#include "splash.h"

int main (int argc, char* argv[]) {
	bool gotArgs = false;	/* if the user provided valid command line arguments */
	int exitCode = GAME_SUCCESS;	/* status returned by game () */
	int option;				/* used for user input */
	int xDim = 9, yDim = 9;	/* dimensions of the game board */
	int qtyMines = 10;		/* number of mines to play with */
	int hudOffset;			/* x-offset of where to print the HUD */
	int termWidth, termHeight;	/* dimensions of the terminal */

	/* savegame struct to load game */
	Savegame savegame;
	Savegame * saveptr = &savegame;

	/* loadSaveFile returns -1 if error opening file */
	bool saveFileExists = (loadSaveFile ("savefile", saveptr) != -1);
	srand (time (NULL));
	initscr ();
	keypad (stdscr, true);
	noecho ();
	
	clear ();
	mmask_t old;
	mousemask (ALL_MOUSE_EVENTS, &old);
	addstr (SPLASH);
	curs_set (0);
	/* press any key to continue */
	getch ();
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

	/* set upper and lower limits for acceptable dimensions */
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

	/* initialize colors */
	start_color ();
	init_pair (1, COLOR_WHITE, COLOR_BLACK);	/* default pair */
	init_pair (2, COLOR_BLACK, COLOR_WHITE);	/* inverted default */
	init_pair (3, COLOR_RED,   COLOR_BLACK);	/* for exploded mines and wrong flags */
	init_pair (4, COLOR_GREEN, COLOR_BLACK);	/* for correct flags and unexploded mines */
	init_pair (5, COLOR_CYAN,  COLOR_BLACK);	/* for numbers */

	/* determine whether to load game or start new game */
	if (!gotArgs) {
		/* if save file does not exist, then automatically start new game */
		if (!saveFileExists) {
			option = 0;
		} else {
			/* otherwise, prompt user */
			option = menu (2, "Welcome to Minesweeper!", "New game", "Load game");
			if (option == -1)
				exitCode = -1;
		}

		if (option == 0) {
			/* user chooses to start new game */
			saveptr = NULL;

			option = menu (3, "Choose difficulty",
				"Beginner    : 9x9, 10 mines",
				"Intermediate: 16x16, 40 mines",
				"Advanced    : 30x24, 99 mines");

			switch (option) {
			case -1:
				exitCode = -1;
				break;
			case 0:
				xDim = 9;
				yDim = 9;
				qtyMines = 10;
				break;
			case 1:
				xDim = 16;
				yDim = 16;
				qtyMines = 40;
				break;
			case 2:
				xDim = 30;
				yDim = 24;
				qtyMines = 99;
				break;
			}
		} else {
			/* user chooses to load game */
			loadSaveFile ("savefile", saveptr);
			/* update xDim to calculate hudOffset later */
			xDim = saveptr->width;
		}
	} else {
		/* command line arguments were supplied, so don't use a save file */
		saveptr = NULL;
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
			option = 0;
			while (!(option == 'Y' || option == 'N'))
				option = toupper (getch ());
			if (option == 'N')
				break;
		} else if (exitCode == GAME_EXIT) {
			/* if player exited manually */
			echo ();
			break;
		}
		/* game restart condition requires no special action */
		
		/* user can only load game on the first run */
		saveptr = NULL;
	}
	echo ();
	endwin ();
	return 0;
}
