#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "board.h"

typedef struct { char name[32]; Board *board; int moves; } Player;
typedef struct { Player players[2]; int current; } Game;

// Returns 1 = valid, 0 = malformed (retry), -1 = end of input (quit)
int read_move(int *row, char *col);

// Ask the user for a preset board number and load it into b.
// Returns false if input ended before a valid number was entered.
bool game_select_board(Board *b);

// Play until the current player's board is cleared or input ends
void game_play(Game *g);

#endif
