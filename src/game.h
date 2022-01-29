/*
 * game.h
 *
 * Contains the declaration of the game function.
 */
#include "savegame.h"

#ifndef GAME_H
#define GAME_H

/* returns 0 on game loss, 1 on success, 2 on manual exit, 3 on restart.
   *state is expected to be a fully initialized Savegame object. *state will be
   modified during normal operation if the save file is overwritten. */
int game(Savegame *state);

#endif /* GAME_H */