/* savegame.h */

#ifndef SAVEGAME_H
#define SAVEGAME_H

#include <stdlib.h>
#include <stdint.h>
#include <time.h>	/* timespec */

#include "board.h"

/* masks for extracting bools from gameBools */
#define MASK_FLAG_MODE		0x01
#define MASK_FIRST_CLICK	0x02

/* struct storing the state of the game */
typedef struct {
	int64_t size;			/* the size of the board data */
	int32_t width, height;	/* dimensions of game board */
	int32_t qtyMines;		/* number of mines in the game */
	int32_t flagsPlaced;	/* flags placed by user */
	uint32_t gameBools;		/* integer storing the state of in-game bools */
	int32_t cy, cx;			/* cursor coordinates */
	struct timespec timeOffset;	/* game duration */
	unsigned char * gameData;	/* string of bytes storing board data */
} Savegame;

/* prototypes for utility functions */

/* decodes game data from the savegame into the mine and board structs */
int getGameData(Board * board, Savegame save);

/* encodes game data from the mine and board structs into the savegame;
   REMEMBER TO FREE saveptr->gameData AFTER CALLING */
int setGameData(Board board, Savegame * saveptr);

/* write savegame save to disk */
int writeSaveFile(const char * filename, Savegame save);

/* read savegame from disk into *saveptr, returning -1 if there is an error or if
   the save file is invalid or has been tampered with;
   REMEMBER TO FREE saveptr->gameData AFTER CALLING */
int loadSaveFile(const char * filename, Savegame * saveptr);

/* removes a savefile from disk */
int removeSaveFile(const char * filename);

#endif /* SAVEGAME_H */