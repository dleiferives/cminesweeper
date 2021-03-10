# cminesweeper

A curses-based version of the classic game of Minesweeper, built for the Linux console.

***

Cminesweeper is a lightweight implementation of the classic Minesweeper game,
written in C and utilizing the ncurses API to work completely in a terminal. It
is simple, responsive, and SSH-compatible without the need for X-forwarding. 

## Building

Linux:

```sh
make all
```

### Dependencies

Cminesweeper is built using the curses API. As such, you'll need to make sure to 
install it before you try to build cminesweeper. Depending on your Linux type,
the ncurses development package may have a different name.

Ubuntu:

```sh
apt install ncurses-devel
```

openSUSE:

```sh
zypper in ncurses-dev
```

## Usage

To start a game of Minesweeper, run the executable through a terminal. You'll
have to `cd` into the directory where you compiled cminesweeper, because it
doesn't install itself into a central directory like /usr/bin yet.

```sh
cd ~/src/cminesweeper
./cminesweeper
```

If you have a previous game already saved, you'll be prompted to choose between
starting a new game or loading your previous game. If you start a new game or if
you have no game saved, you will be prompted to choose a difficulty level.

## Controls

### Menus

Menus should be pretty intuitive, but just in case you're confused, here's how to
navigate them:

- Use **WASD** or the **Arrow Keys** to move the cursor
- Use **Enter/Return** to choose that option
- Alternatively, type the number next to an option to instantly choose that
option
- Press **Q** or **Escape** to close the menu

### Gameplay

When it comes to controlling the game itself, you have several options:

- Use **WASD** or the **Arrow Keys** to move the cursor around the field
- Use **Z**, **Enter/Return**, or **/** (slash) as the Primary Select Button 
(equivalent to a left-click)
- Use **X** or **'** (apostrophe) as the Secondary Select Button (equivalent to
a right-click)
- Press the **Spacebar** or **M** to toggle between Normal mode and Flagging mode
- Press **Q** or **Escape** to open the pause menu
- Press **R** to start a new game without being prompted first
- Press **E** or **Ctrl+S** (on some systems) to save your game

