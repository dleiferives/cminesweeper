#ifndef BOARDFUNCTIONS_H
#define BOARDFUNCTIONS_H

#include <curses.h>

// function protypes
int printBoard (char[32][32], int = 9, int = 9, bool = 0, chtype = (chtype)'X' | COLOR_PAIR(3), chtype = (chtype)'P' | COLOR_PAIR(3));
int initializeMines (char[32][32], int = 9, int = 9, int = 10);
int overlayMines (char[32][32], char[32][32]);
int numMines (char[32][32], int, int);
int openSquares (char[32][32], char[32][32], int, int, int = 9, int = 9);
bool allClear (char[32][32], char[32][32], int = 9, int = 9);
int game (int = 9, int = 9, int = 10); // returns 0 on game failure, 1 on success, 2 on manual exit, 3 on restart
int menu (); // returns 0 on no input, 1 on restart, 2 on exit, 3 on debug
int tutorial ();
int printCtrls (int = 3, int = 29);

#endif /* BOARDFUNCTIONS_H */