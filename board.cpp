#include <stdlib.h>
#include <stdio.h>
#include "board.h"

/* utility functions for board memory management */

int initBoardArray (Board * board) {
    board->array = (char **) malloc ((board->width + 2) * sizeof (char *));
	for (int i = 0; i < board->width + 2; i++) {
		board->array[i] = (char *) malloc (board->height + 2);
    }
    return 0;
}

int freeBoardArray (Board * board) {
    for (int i = 0; i < board->width + 2; i++)
        free (board->array[i]);
    free (board->array);
    return 0;
}