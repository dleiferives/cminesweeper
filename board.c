/* board.c
   defines utility functions for board memory management */

#include <stdlib.h>
#include <stdio.h>
#include "board.h"

/* allocate memory for array member based on value of dimension members */
int initBoardArray (Board * board) {
    board->array = (char **) malloc ((board->width + 2) * sizeof (char *));
	for (int i = 0; i < board->width + 2; i++) {
		board->array[i] = (char *) malloc (board->height + 2);
    }
    return 0;
}

/* free the memory allocated for the array member */
int freeBoardArray (Board * board) {
    for (int i = 0; i < board->width + 2; i++)
        free (board->array[i]);
    free (board->array);
    return 0;
}