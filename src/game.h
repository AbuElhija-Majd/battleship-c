#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "board.h"

typedef struct { char name[32]; Board *board; int moves; } Player;
typedef struct { Player players[2]; int current; } Game;

// Returns 1 = valid, 0 = malformed (retry), -1 = end of input (quit)
int read_move(int *row, char *col);

// Create both players' boards and place their fleets (seeds seed and
// seed + 1). Prints an error and returns false on failure; nothing is
// left allocated in that case.
bool game_init(Game *g, int rows, int cols, unsigned seed);

// Run turns until one player sinks every ship or input ends
void game_play(Game *g);

// Free both boards. Safe to call more than once.
void game_destroy(Game *g);

#endif
