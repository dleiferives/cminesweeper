/*
 * menu.h
 *
 * Contains declarations of menu functions
 */
#include <stdarg.h>

#ifndef MENU_H
#define MENU_H

/* returns the 0-indexed option chosen by the user */
int menu(int optc, const char * title, ...);

/* moves the cursor and returns the 0-indexed option chosen by the user */
int mvmenu(int y, int x, int optc, const char * title, ...);

/* internal va_list menu function */
int vmenu(int y, int x, int optc, const char * title, va_list options);

#endif /* MENU_H */