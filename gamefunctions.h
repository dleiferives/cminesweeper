/* definitions for gameplay utility functions */

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
int game (int = 9, int = 9, int = 10); // returns 0 on game failure, 1 on success, 2 on manual exit, 3 on restart
int menu (); // returns 0 on no input, 1 on restart, 2 on exit, 3 on debug
int tutorial ();
int printCtrls (int = 3, int = 29);

#endif /* GAMEFUNCTIONS_H */