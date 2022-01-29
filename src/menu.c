/*
 * menu.c
 *
 * Contains definitions of menu functions
 */

#include <curses.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "menu.h"

int menu(int optc, const char *title, ...) {
	int choice;
	va_list options;
	va_start(options, title);
	choice = vmenu(0, 0, optc, title, options);
	va_end(options);
	return choice;
}

int mvmenu(int y, int x, int optc, const char *title, ...) {
	int choice;
	va_list options;
	va_start(options, title);
	choice = vmenu(y, x, optc, title, options);
	va_end(options);
	return choice;
}

int vmenu(int y, int x, int optc, const char *title, va_list options) {
	int i, k; /* counting variables */
	size_t maxLength;
	/* string array to hold option names */
	const char **optionNames = malloc(optc * sizeof(char *));
	/* array to cache string lengths to avoid calling strlen multiple times */
	size_t *optionLengths = malloc(optc * sizeof(size_t));
	size_t titleLength;

	/* variables for navigating the menu */
	bool gotInput = false; /* the user has made a choice */
	int buf = 0;
	int option = 0;
	
	/* populate the string array using the varargs */
	for (i = 0; i < optc; i++)
		optionNames[i] = va_arg(options, char *);
	va_end(options);

	/* calculate the width of the box necessary to fit all options */
	/* title margin is 3 characters left of options and has 1 padding */
	titleLength = strlen(title) - 2;
	maxLength = titleLength;

	for (i = 0; i < optc; i++) {
		optionLengths[i] = strlen(optionNames[i]);
		if (optionLengths[i] > maxLength) {
			maxLength = optionLengths[i];
		}
	}

	curs_set(0); /* cursor invisible */

	do {
		/* print the menu */
		/* start with top of border */
		mvprintw(y, x, "+= %s ", title);
		for (k = 0; k < maxLength - titleLength; k++)
			addch('=');
		addstr("=+");

		/* blank space */
		mvaddstr(y + 1, x, "|     ");
		for (k = 0; k < maxLength; k++)
			addch(' ');
		addstr(" |");

		/* print every option */
		for (i = 0; i < optc; i++) {
			mvprintw(y + i + 2, x, "| %2d) %s", i + 1, optionNames[i]);
			for (k = 0; k < maxLength - optionLengths[i]; k++)
				addch(' ');
			addstr(" |");
		}

		/* another blank space */
		mvaddstr(y + i + 2, x, "|     ");
		for (k = 0; k < maxLength; k++)
			addch(' ');
		addstr(" |");

		/* end with bottom of border */
		mvaddstr(y + i + 3, x, "+=====");
		for (k = 0; k < maxLength; k++)
			addch('=');
		addstr("=+");

		/* draw option pointer */
		mvaddch(y + option + 2, x + 5, '>' | A_BLINK);

		refresh();

		/* now get input */
		buf = toupper(getch());

		if (buf == 'Q' || buf == 27) {
			/* quit or Esc */
			option = -1;
			gotInput = true;
		} else if ('1' <= buf && buf <= '9') {
			/* numbers corresponding to options */
			if (buf - '0' <= optc) {
				/* the user selected an option that DOES exist */
				option = buf - '0' - 1;
				gotInput = true;
			}
			/* otherwise, do nothing */
		} else if (buf == 'W' || buf == KEY_UP) {
			option--;
			if (option < 0) option = 0;
		} else if (buf == 'A' || buf == KEY_LEFT) {
			option--;
			if (option < 0) option = 0;
		} else if (buf == 'S' || buf == KEY_DOWN) {
			option++;
			if (option > optc - 1) option = optc - 1;
		} else if (buf == 'D' || buf == KEY_RIGHT) {
			option++;
			if (option > optc - 1) option = optc - 1;
		} else if (buf == 10) {
			/* return or enter */
			gotInput = true;
		}
	} while (!gotInput); 
	
	free(optionNames);
	free(optionLengths);
	/* un-blink the option cursor */
	mvchgat(y + option + 2, x + 5, 1, A_NORMAL, 1, NULL);
	return option;
}
