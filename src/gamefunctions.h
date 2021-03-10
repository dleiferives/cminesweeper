/* gamefunctions.h
   contains declarations for gameplay utility functions */

#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include <curses.h> 
#include <stdarg.h>

#include "board.h"
#include "savegame.h"

/* Prints a graphical representation of board, displaying mines as mineChar.
   If hide is true, all squared will be printed as "[]" */
int printBoardCustom (Board board, bool hide, chtype mineAttr);

/* printBoard with default arguments for hide and mineChar */
int printBoard (Board board);

/* randomize locations of mines on the board */
int initializeMines (Board * board);

/* overlay the locations of mines onto the game board */
int overlayMines (Board * board);

/* returns number of mines adjacent to (x, y) */
int numMines (Board board, int x, int y);

/* recursively uncovers squares on board starting at (x, y) */
int openSquares (Board * board, int x, int y);

/* returns true if the minefield has been cleared */
bool allClear (Board board);

/* returns 0 on game loss, 1 on success, 2 on manual exit, 3 on restart.
   If saveptr is not NULL, then xDim, yDim and qtyMines will be used to 
   initialize the game. */
int game (int xDim, int yDim, int qtyMines, Savegame * saveptr);

/* returns the 0-indexed option chosen by the user */
int menu (int optc, const char * title, ...);

/* moves the cursor and returns the 0-indexed option chosen by the user */
int mvmenu (int y, int x, int optc, const char * title, ...);

/* internal va_list menu function */
int vmenu (int y, int x, int optc, const char * title, va_list options);

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

/* macros for pause menu return codes */
#define MENU_NO_INPUT	0
#define MENU_RESTART	1
#define MENU_SAVE_GAME	2
#define MENU_EXIT_GAME	3
#define MENU_TUTORIAL	4

/* macros for game actions */
#define ACTION_NONE		0	/* no action */
#define ACTION_OPEN		1	/* open the current square */
#define ACTION_FLAG		2	/* place a flag on the current square */
#define ACTION_AUTO		3	/* automatically open adjacent squares */
#define ACTION_ESCAPE	4	/* open the pause menu */
#define ACTION_SAVE		5	

/* masks for accessing mine data */
#define MASK_MINE	0x80
#define MASK_CHAR	0x7F

#endif /* GAMEFUNCTIONS_H */