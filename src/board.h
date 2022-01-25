/* board.h
   contains definition for game board data structure and prototypes
   for game board utility functions */

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

#endif /* BOARD_H */