/* main.c */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <curses.h>
#include <string.h>	/* strcmp */
#include <time.h>	/* time */

#include "util.h"
#include "savegame.h"
#include "splash.h"
#include "menu.h"
#include "game.h"

/* home of the main menu (TM) */
int main(int argc, char* argv[]) {
	srand(time(NULL));
	initscr();
	keypad(stdscr, true);
	noecho();
	
	clear();
	mmask_t old;
	mousemask(ALL_MOUSE_EVENTS, &old);

	/* display splash screen */
	addstr(SPLASH);
	curs_set(0);
	/* press any key to continue */
	getch();
	clear();
	refresh();
	
	/* This part is exclusive to custom command line dimensions. We will revisit
	   this in a later revision of the game. */
#if 0
	{
		/* NOTE:
		Command line argument support is currently not supported. */
		bool gotArgs = false;	/* if the user provided valid command line arguments */
		int xDim = 9, yDim = 9;	/* dimensions of the game board */
		int qtyMines = 10;		/* number of mines to play with */

		if (argc >= 5 && strcmp(argv[argc - 4], "custom") == 0) {
			xDim = atoi(argv[argc - 3]);
			yDim = atoi(argv[argc - 2]);
			qtyMines = atoi(argv[argc - 1]);
			gotArgs = true;
		}

		int termWidth, termHeight;
		/* get terminal dimensions */
		getmaxyx(stdscr, termHeight, termWidth);

		/* set upper and lower limits for acceptable dimensions */
		/* minimum dimension is always 1 */
		if (xDim < 1)
			xDim = 1;
		if (yDim < 1)
			yDim = 1;
		/* maxima will be set based on the terminal dimensions */
		if (2 * xDim + 41 > termWidth)
			xDim = (termWidth - 41) / 2;
		if (yDim + 2 > termHeight)
			yDim = termHeight - 2;
		if (qtyMines < 1)
			qtyMines = 1;
		if (qtyMines > xDim * yDim - 1)
			qtyMines = xDim * yDim - 1;
	}
#endif 

	/* initialize colors */
	start_color();
	init_pair(1, COLOR_WHITE,	COLOR_BLACK);	/* default pair */
	init_pair(2, COLOR_BLACK,	COLOR_WHITE);	/* inverted default */
	init_pair(3, COLOR_RED,		COLOR_BLACK);	/* for exploded mines and wrong flags */
	init_pair(4, COLOR_GREEN,	COLOR_BLACK);	/* for correct flags and unexploded mines */
	init_pair(5, COLOR_CYAN,	COLOR_BLACK);	/* for numbers */

	/*** PLAY THE GAME ***/

	int mainMenuOption;
	do {
		/* main menu */
		/* this will hold the game state for every game played */
		Savegame savegame;

		clear();
		mainMenuOption = menu(3, "Main menu",
			"New game...",
			"Load game",
			"Exit");
		
		switch (mainMenuOption) {
		case 0:
			/* user chooses to start new game */
			/* labels can only precede statements, so we use a compound statement */
			{
				int difficulty;
				difficulty = menu(3, "Choose difficulty",
					"Beginner    : 9x9, 10 mines",
					"Intermediate: 16x16, 40 mines ",
					"Advanced    : 30x24, 99 mines");

				switch (difficulty) {
				case -1:
					continue;
				case 0:
					savegame.width = 9;
					savegame.height = 9;
					savegame.qtyMines = 10;
					break;
				case 1:
					savegame.width = 16;
					savegame.height = 16;
					savegame.qtyMines = 40;
					break;
				case 2:
					savegame.width = 30;
					savegame.height = 24;
					savegame.qtyMines = 99;
					break;
				}
			}
			break;
		case 1:
			/* load game */
			/* loadSaveFile returns -1 if error opening file */

			if (loadSaveFile("savefile", &savegame) == -1) {
				mvmenu(0, 0, 1, "No save file exists", "I understand");
				continue;
			}
			/* otherwise, loading was successful and we can continue */
			break;
		case -1:
			mainMenuOption = 2;
		}

		/* calculate HUD offset */
		int hudOffset;
		hudOffset = 2 * savegame.width + 3;
		if (hudOffset < 18) hudOffset = 18;

		/* game time, using whatever Savegame was set up in the last step */
		if (mainMenuOption != 2) {
			/* keep playing while player wants to */
			int exitCode;
			do {
				exitCode = game(&savegame);
				if (exitCode == GAME_FAILURE || exitCode == GAME_SUCCESS) {
					int playAgain;
					playAgain = mvmenu(9, hudOffset, 2, "Play again?",
						"Yes",
						"No");

					if (playAgain == -1 || playAgain == 1) {
						/* Don't play again, go to the menu. */
						exitCode = GAME_EXIT;
					}
				}
			} while (exitCode != GAME_EXIT);
		}
	} while (mainMenuOption != 2);
	
	echo();
	endwin();
	return 0;
}
