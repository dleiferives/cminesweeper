/* definition for game board data structure */
#ifndef BOARD_H
#define BOARD_H

#include <stdlib.h>

typedef struct {
    int width;
    int height;
    long mineCount;
    char ** array;
} Board;

int initBoardArray (Board * board);

int freeBoardArray (Board * board);

#endif /* BOARD_H */