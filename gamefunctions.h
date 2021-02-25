/* gamefunctions.h
   contains declarations for gameplay utility functions */

#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include <curses.h>
#include "board.h"

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

/* uncovers squares on board starting at (x, y) */
int openSquares (Board mines, Board * board, int x, int y);

/* returns true if the minefield has been cleared */
bool allClear (Board mines, Board board);

/* returns 0 on game loss, 1 on success, 2 on manual exit, 3 on restart */
int game (int xDim, int yDim, int qtyMines);

/* returns 0 if no input, 1 to restart game, 2 to exit game, 3 to play tutorial */
int menu ();

/* play the game tutorial */
int tutorial ();

/* print user controls with the top left corner at (y, x) */
int printCtrlsyx (int y, int x);

/* printCtrls with default location at (3, 29) */
int printCtrls ();

/* print a blank game board of dimensions defined in board */
int printBlank (Board board);

/* macros for game return codes */
#define GAME_FAILURE    0
#define GAME_SUCCESS    1
#define GAME_EXIT       2
#define GAME_RESTART    3

/* macros for menu return codes */
#define MENU_NO_INPUT   0
#define MENU_RESTART    1
#define MENU_EXIT_GAME  2
#define MENU_TUTORIAL   3

/* macros for game actions */
#define ACTION_NONE 	0
#define ACTION_BOARD_OP	1
#define ACTION_CHG_MODE	2
#define ACTION_ESCAPE	3
#define ACTION_INC_TIME	4

#endif /* GAMEFUNCTIONS_H */