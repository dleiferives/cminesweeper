/* gamefunctions.h
   contains declarations for gameplay utility functions */

#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include <curses.h>
#include "board.h"

int printBoard (Board, bool = false, chtype = (chtype)'X' | COLOR_PAIR(3), chtype = (chtype)'P' | COLOR_PAIR(3));
int initializeMines (Board *);
int overlayMines (Board, Board *);
int numMines (Board, int, int);
int openSquares (Board, Board *, int = 9, int = 9);
bool allClear (Board, Board);
/* returns 0 on game failure, 1 on success, 2 on manual exit, 3 on restart */
int game (int = 9, int = 9, int = 10);
/* returns 0 on no input, 1 on restart, 2 on exit, 3 on tutorial */
int menu ();
int tutorial ();
int printCtrls (int = 3, int = 29);
int printBlank (int, int);

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