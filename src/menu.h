/*
 * menu.h
 *
 * Contains declarations of menu functions
 */
#include <stdarg.h>

#ifndef MENU_H
#define MENU_H

/* returns the 0-indexed option chosen by the user, or -1 if exit */
int menu(int optc, const char *title, ...);

/* moves the cursor and returns the 0-indexed option chosen by the user, or
   -1 if exit */
int mvmenu(int y, int x, int optc, const char *title, ...);

/* internal va_list menu function */
int vmenu(int y, int x, int optc, const char *title, va_list options);

/* prompt the user for an integer */
int mvpromptInt(int y, int x, const char *prompt);

/* prompt the user for an integer */
int promptInt(const char *prompt);

#endif /* MENU_H */
