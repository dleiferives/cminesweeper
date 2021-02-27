/* gamefunctions.h
   contains declarations for gameplay utility functions */

#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include <curses.h>
#include "board.h"
#include "savegame.h"

/* display a graphical representation of board, displaying mines as mineChar 
   and displaying flags as flagChar, unless hide is set to true */
int printBoardCustom (Board board, bool hide, chtype mineChar, chtype flagChar);

/* printBoard with default arguments for hide, mineChar, and flagChar */
int printBoard (Board board);

/* randomize locations of mines on the board */
int initializeMines (Board * mines);

/* overlay the locations of mines onto the game board */
int overlayMines (Board mines, Board * board);

/* returns number of mines adjacent to (x, y) */
int numMines (Board board, int x, int y);

/* recursively uncovers squares on board starting at (x, y) */
int openSquares (Board mines, Board * board, int x, int y);

/* returns true if the minefield has been cleared */
bool allClear (Board mines, Board board);

/* returns 0 on game loss, 1 on success, 2 on manual exit, 3 on restart.
   If save is not NULL, then xDim, yDim and qtyMines will be used to 
   initialize the game. */
int game (int xDim, int yDim, int qtyMines, Savegame * saveptr);

/* returns the 0-indexed option chosen by the user */
int menu (int optc, const char * title, ...);

/* play the game tutorial */
int tutorial ();

/* print user controls with the top left corner at (y, x) */
int printCtrlsyx (int y, int x);

/* printCtrls using default location at (3, 29) */
int printCtrls ();

/* print a blank game board of dimensions defined in board */
int printBlank (Board board);

/* prints the top and bottom of the board frame for convenience */
int printFrame (Board board);

/* macros for game return codes */
#define GAME_FAILURE	0
#define GAME_SUCCESS	1
#define GAME_EXIT		2
#define GAME_RESTART	3

/* macros for menu return codes */
#define MENU_NO_INPUT	0
#define MENU_RESTART	1
#define MENU_SAVE_GAME	2
#define MENU_EXIT_GAME	3
#define MENU_TUTORIAL	4

/* macros for game actions */
#define ACTION_NONE		0
#define ACTION_BOARD_OP	1
#define ACTION_CHG_MODE	2
#define ACTION_ESCAPE	3
#define ACTION_INC_TIME	4

#endif /* GAMEFUNCTIONS_H */