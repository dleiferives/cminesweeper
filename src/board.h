/* 
 * board.h
 * 
 * Contains declarations of the Board struct, and functions to manage, manipulate,
 * and print the struct.
 */

#include <curses.h>
#include <stdbool.h>

#ifndef BOARD_H
#define BOARD_H

typedef struct {
    int width;
    int height;
    long mineCount;
    unsigned char ** array;
} Board;

/* allocate memory for array member based on value of dimension members */
int initBoardArray(Board * board);

/* free the memory allocated for the array member */
int freeBoardArray(Board * board);

/* Prints a graphical representation of board, displaying mines as mineChar.
   If hide is true, all squared will be printed as "[]" */
int printBoardCustom(Board board, bool hide, chtype mineAttr);

/* printBoard with default arguments for hide and mineChar */
int printBoard(Board board);

/* randomize locations of mines on the board */
int initializeMines(Board * board);

/* overlay the locations of mines onto the game board */
int overlayMines(Board * board);

/* returns number of mines adjacent to (x, y) */
int numMines(Board board, int x, int y);

/* recursively uncovers squares on board starting at (x, y) */
int openSquares(Board * board, int x, int y);

/* returns true if the minefield has been cleared */
bool allClear(Board board);

/* print a blank game board of dimensions defined in board */
int printBlank(Board board);

/* prints the top and bottom of the board frame for convenience */
int printFrame(Board board);

#endif /* BOARD_H */