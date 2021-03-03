/* savegame.c */

/* TODO:
   add integrity checking to the savefile I/O functions */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>	/* NAME_MAX */
#include <string.h>	/* strcpy, strcat, memset */

#include "savegame.h"
#include "board.h"

/* these are defined as macros in case we need to redefine them for
   non-unix-like platforms */
#define HOME_ENV_NAME	"HOME"
#define PATH_MAXSIZE 	NAME_MAX

int getGameData (Board * board, Savegame save) {
	int outputIndex = 0;
	int x, y;

	/* iterate through elements on both boards */
	for (y = 1; y <= board->height; y++) {
		for (x = 1; x <= board->width; x++) {
			/* XOR byte with a constant and write to board */
			board->array[x][y] = save.gameData[outputIndex++] ^ 0x55;
		}
	}
	return outputIndex;
}

int setGameData (Board board, Savegame * save) {
	int outputIndex = 0;
	int x, y;
	save->gameData = malloc (save->size);

	/* iterate through elements on both boards */
	for (y = 1; y <= board.height; y++) {
		for (x = 1; x <= board.width; x++) {
			/* XOR array with a constant and assign to block */
			save->gameData[outputIndex++] = board.array[x][y] ^ 0x55;
		}
	}
	return outputIndex;
}

int writeSaveFile (const char * filename, Savegame save) {
	/* the full name of the save file */
	char longname[PATH_MAXSIZE];
	memset (longname, 0, PATH_MAXSIZE);
	strcpy (longname, getenv (HOME_ENV_NAME));
	strcat (longname, "/.cminesweeper/");
	strcat (longname, filename);

	FILE * savefile = fopen (longname, "wb");
	if (savefile != NULL) {
		/* first write the raw struct data. Note that we subtract the value of
		   sizeof (uint8_t) from the size of the data block, because the last
		   member in the Savegame struct is actually a pointer. */
		fwrite (&save, sizeof (save) - sizeof (uint8_t *), 1, savefile);
		/* then write the data pointed to by gameData */
		fwrite (save.gameData, save.size, 1, savefile);
		fclose (savefile);
	} else {
		/* error opening file */
		return -1;
	}
	return 0;
}

int loadSaveFile (const char * filename, Savegame * saveptr) {
	/* the full name of the save file */
	char longname[PATH_MAXSIZE];
	memset (longname, 0, PATH_MAXSIZE);
	strcpy (longname, getenv (HOME_ENV_NAME));
	strcat (longname, "/.cminesweeper/");
	strcat (longname, filename);

	FILE * savefile = fopen (longname, "rb");
	if (savefile != NULL) {
		/* first read the raw struct data. Note that we subtract the value of
		   sizeof (uint8_t) from the size of the data block, because the last
		   member in the Savegame struct is actually a pointer. */
		fread (saveptr, sizeof (Savegame) - sizeof (uint8_t *), 1, savefile);
		/* allocate the necessary memory */
		saveptr->gameData = (unsigned char *) malloc (saveptr->size);
		/* then read the data pointed to by gameData */
		fread (saveptr->gameData, saveptr->size, 1, savefile);
		fclose (savefile);
	} else {
		/* error opening file */
		return -1;
	}
	return 0;
}