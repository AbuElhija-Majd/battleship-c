#ifndef GAME_H
#define GAME_H

#include "board.h"

typedef struct { char name[32]; Board *board; int moves; } Player;
typedef struct { Player players[2]; int current; } Game;

// Returns 1 = valid, 0 = malformed (retry), -1 = end of input (quit)
int read_move(int *row, char *col);

// Play until the current player's board is cleared or input ends
void game_play(Game *g);

#endif
