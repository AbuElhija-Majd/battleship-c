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

typedef struct {
    int rows, cols;
    char *cells;     // ground truth: SHIP or EMPTY
    char *revealed;  // what the opponent sees: HIDDEN, HIT, MISS
} Board;

typedef enum { SHOT_MISS, SHOT_HIT, SHOT_SUNK, SHOT_INVALID, SHOT_REPEAT } ShotResult;

// Allocate a rows x cols board with every cell EMPTY and HIDDEN.
// Returns NULL on invalid size or allocation failure.
Board *board_create(int rows, int cols);

// Free both arrays, then the struct. Safe to call with NULL.
void board_destroy(Board *b);

// Randomly place ships of the given lengths, horizontally or vertically.
// Ships never overlap or touch side-to-side, so each ship stays a separate
// connected group. Returns 1 on success, 0 if the fleet could not be placed
// (board too small); the board is then partially filled.
int board_place_ships(Board *b, const int *lengths, int count, unsigned seed);

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
