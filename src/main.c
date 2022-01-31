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

#ifndef CMINESWEEPER_DEBUG
		mainMenuOption = menu(4, "Main menu",
			"New game...",
			"Load game",
			"Clear saved game...",
			"Exit");
#else
		mainMenuOption = menu(5, "Main menu",
			"New game...",
			"Load game",
			"Clear saved game...",
			"Exit",
			"Debug menu...");
#endif	/* CMINESWEEPER_DEBUG */

		switch (mainMenuOption) {
		case 0:
			/* user chooses to start new game */
			/* labels can only precede statements, so we use a compound statement */
			{
				int difficulty;
				difficulty = menu(4, "Choose difficulty",
					"Beginner    : 9x9, 10 mines",
					"Intermediate: 16x16, 40 mines ",
					"Advanced    : 30x24, 99 mines",
					"Custom dimensions...");

				switch (difficulty) {
				case -1:
					continue;
				case 0:
					savegame.width = 9;
					savegame.height = 9;
					savegame.qtyMines = 10;
					break;
				default:
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
				case 3:
					/* custom dimensions */
					{
						int termWidth, termHeight;
						getmaxyx(stdscr, termHeight, termWidth);

						clear();
						do {
							savegame.width = mvpromptInt(0, 0, "Width:         ");
							mvprintw(22, 0, "%d", savegame.width);
						} while (savegame.width < 2 || savegame.width > (termWidth - 41) / 2);

						do {
							savegame.height = mvpromptInt(4, 0, "Height         ");
						} while (savegame.height < 2 || savegame.height > (termHeight - 2));

						do {
							savegame.qtyMines = mvpromptInt(8, 0, "Number of mines");
						} while ( savegame.qtyMines < 0		/* the - 2 below is arbitrary */
								|| savegame.qtyMines > savegame.height * savegame.width - 2);

						int useDimensions;
						useDimensions = mvmenu(0, 20, 2, "Use these dimensions?", "Yes", "No");
						if (useDimensions != 0)
							continue;
					}
				}
				/* gameData should always be set to NULL when a new game is to
				   be initialized */
				savegame.gameData = NULL;
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
		case 2:
			/* clear savefile */
			{
				int clearSaveFile;
				clearSaveFile = menu(2, "Really clear saved game?", "Yes", "No");
				if (clearSaveFile == 0) {
					/* delete it ! */
					int status = removeSaveFile("savefile");
					if (status != 0)
						mvmenu(6, 0, 1, "No save file exists", "I understand");
					else
						mvmenu(6, 0, 1, "Saved game cleared", "I have no regrets");
				}
			}
			continue;

#ifdef CMINESWEEPER_DEBUG
		case 4:
			/* debug menu */
			{
				int the = mvpromptInt(0, 0, "The");
				mvprintw(15, 0, "%d\n", the);
				getch();
				clear();

				the = mvpromptInt(1, 1, NULL);
				mvprintw(15, 0, "%d\n", the);
				getch();
				clear();

				the = mvpromptInt(8, 4, "Some really impractically long string");
				mvprintw(15, 0, "%d\n", the);
				getch();
				clear();
			}
			continue;
#endif	/* CMINESWEEPER_DEBUG */

		case -1:
			mainMenuOption = 3;
		}

		/* calculate HUD offset */
		int hudOffset;
		hudOffset = 2 * savegame.width + 3;
		if (hudOffset < 18) hudOffset = 18;

		/* game time, using whatever Savegame was set up in the last step */
		if (mainMenuOption != 3) {
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
	} while (mainMenuOption != 3);
	
	echo();
	endwin();
	return 0;
}
