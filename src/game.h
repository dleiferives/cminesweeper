/*
 * game.h
 *
 * Contains the declaration of the game function.
 */
#include "savegame.h"

#ifndef GAME_H
#define GAME_H

/* returns 0 on game loss, 1 on success, 2 on manual exit, 3 on restart.
   If saveptr is not NULL, then xDim, yDim and qtyMines will be used to 
   initialize the game. */
int game(int xDim, int yDim, int qtyMines, Savegame *saveptr);

#endif /* GAME_H */