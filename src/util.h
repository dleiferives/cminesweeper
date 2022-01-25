/*
 * util.h
 *
 * contains declarations for gameplay utility functions
 */

#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

#include <curses.h> 

#include "board.h"
#include "savegame.h"

/* play the game tutorial */
int tutorial();

/* print user controls with the top left corner at (y, x) */
int printCtrlsyx(int y, int x);

/* printCtrls using default location at (3, 29) */
int printCtrls();

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