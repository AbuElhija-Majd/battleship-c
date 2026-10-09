#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

// Ground truth (cells)
#define SHIP 'S'
#define EMPTY '~'

// What the opponent sees (revealed)
#define HIDDEN ' '
#define HIT 'S'
#define MISS '~'

#define BOARD_ROWS 8
#define BOARD_COLS 8
#define NUM_OF_BOARDS 5

typedef struct {
    int rows, cols;
    char *cells;     // ground truth: SHIP or EMPTY
    char *revealed;  // what the opponent sees: HIDDEN, HIT, MISS
} Board;

typedef enum { SHOT_MISS, SHOT_HIT, SHOT_SUNK, SHOT_INVALID, SHOT_REPEAT } ShotResult;

// Fill the board from preset 1..NUM_OF_BOARDS and hide every cell.
// Returns false if the number is out of range or the board is not 8x8.
bool board_load_preset(Board *b, int boardNumber);

bool board_in_bounds(const Board *b, int row, int col);

// Revealed state of a cell (HIDDEN, HIT or MISS)
char board_view(const Board *b, int row, int col);

// Number of separate ships on the board, or -1 if out of memory
int board_count_ships(const Board *b);

// Fire at (row, col). A hit reveals the whole ship, so it returns SHOT_SUNK.
ShotResult board_fire(Board *b, int row, int col);

// True once every ship cell has been revealed
bool board_all_sunk(const Board *b);

// Mark all remaining hidden cells as MISS (used at the end of the game)
void board_reveal_all(Board *b);

#endif
