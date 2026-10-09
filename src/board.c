#include <stdlib.h>
#include "board.h"

// Predefined Boards
static const char MATRIX_1[BOARD_ROWS][BOARD_COLS] = {
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', 'S', ' ', ' ', ' ', 'S', ' '},
        {' ', ' ', 'S', ' ', ' ', ' ', 'S', ' '},
        {' ', ' ', 'S', ' ', ' ', ' ', 'S', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', 'S', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', 'S', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', 'S', ' ', ' ', ' ', ' '}
};
static const char MATRIX_2[BOARD_ROWS][BOARD_COLS] = {
        {'S', ' ', ' ', ' ', ' ', ' ', ' ', '~'},
        {'S', ' ', 'S', ' ', ' ', ' ', ' ', ' '},
        {'S', ' ', 'S', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', 'S', ' ', ' ', ' ', ' ', 'S'},
        {' ', ' ', ' ', ' ', ' ', ' ', '~', 'S'},
        {' ', ' ', ' ', ' ', ' ', ' ', '~', 'S'},
        {' ', 'S', 'S', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
};
static const char MATRIX_3[BOARD_ROWS][BOARD_COLS] = {
        {' ', 'S', 'S', 'S', ' ', ' ', ' ', '~'},
        {' ', ' ', ' ', ' ', '~', 'S', 'S', 'S'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', 'S', 'S', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', 'S', 'S', 'S', 'S', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', ' ', ' ', ' ', ' ', '~'}
};
static const char MATRIX_4[BOARD_ROWS][BOARD_COLS] = {
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
};
static const char MATRIX_5[BOARD_ROWS][BOARD_COLS] = {
        {'S', ' ', 'S', ' ', ' ', ' ', '~', 'S'},
        {' ', ' ', ' ', ' ', ' ', 'S', ' ', '~'},
        {' ', ' ', 'S', ' ', ' ', ' ', ' ', '~'},
        {' ', ' ', ' ', ' ', ' ', ' ', 'S', '~'},
        {' ', ' ', ' ', 'S', ' ', ' ', ' ', '~'},
        {' ', 'S', ' ', ' ', ' ', ' ', 'S', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', 'S', ' ', ' ', '~', 'S'}
};

static const char (*const PRESETS[NUM_OF_BOARDS])[BOARD_COLS] = {
        MATRIX_1, MATRIX_2, MATRIX_3, MATRIX_4, MATRIX_5
};

// Index of cell (r, c) in the one-dimensional arrays
static int idx(const Board *b, int r, int c) {
    return r * b->cols + c;
}

bool board_load_preset(Board *b, int boardNumber) {
    if (boardNumber < 1 || boardNumber > NUM_OF_BOARDS) return false;
    if (b->rows != BOARD_ROWS || b->cols != BOARD_COLS) return false;

    const char (*source)[BOARD_COLS] = PRESETS[boardNumber - 1];
    for (int i = 0; i < b->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            b->cells[idx(b, i, j)] = source[i][j] == 'S' ? SHIP : EMPTY;
            b->revealed[idx(b, i, j)] = HIDDEN;
        }
    }
    return true;
}

bool board_in_bounds(const Board *b, int row, int col) {
    return row >= 0 && row < b->rows && col >= 0 && col < b->cols;
}

char board_view(const Board *b, int row, int col) {
    return b->revealed[idx(b, row, col)];
}

// Helper function to perform flood-fill
static void markConnectedCells(const Board *b, bool *visited,
                               int row, int col) {
    // Check boundaries and if the cell is a ship and not visited
    if (!board_in_bounds(b, row, col) || visited[idx(b, row, col)]
        || b->cells[idx(b, row, col)] != SHIP) {
        return;
    }

    // Mark the current cell as visited
    visited[idx(b, row, col)] = true;

    // Recursively visit all 4 adjacent cells
    markConnectedCells(b, visited, row + 1, col); // Down
    markConnectedCells(b, visited, row - 1, col); // Up
    markConnectedCells(b, visited, row, col + 1); // Right
    markConnectedCells(b, visited, row, col - 1); // Left
}

int board_count_ships(const Board *b) {
    bool *visited = calloc((size_t)(b->rows * b->cols), sizeof(bool));
    if (visited == NULL) return -1;

    int count = 0;
    // Loop through every cell in the board
    for (int i = 0; i < b->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            // If we find an unvisited ship cell, it's a new ship
            if (b->cells[idx(b, i, j)] == SHIP && !visited[idx(b, i, j)]) {
                count++;
                markConnectedCells(b, visited, i, j);
                // Mark all connected parts as visited
            }
        }
    }
    free(visited);
    return count;
}

// Helper function to reveal all connected ship cells
static void revealConnectedShips(Board *b, int row, int col) {
    // Check boundaries and if the cell is a ship
    // and hidden on the display board
    if (!board_in_bounds(b, row, col) ||
        b->revealed[idx(b, row, col)] == HIT ||
        b->cells[idx(b, row, col)] != SHIP) {
        return;
    }

    // Reveal the current cell on the display board
    b->revealed[idx(b, row, col)] = HIT;

    // Recursively reveal all 4 adjacent cells
    revealConnectedShips(b, row + 1, col); // Down
    revealConnectedShips(b, row - 1, col); // Up
    revealConnectedShips(b, row, col + 1); // Right
    revealConnectedShips(b, row, col - 1); // Left
}

ShotResult board_fire(Board *b, int row, int col) {
    if (!board_in_bounds(b, row, col)) return SHOT_INVALID;
    if (b->revealed[idx(b, row, col)] != HIDDEN) return SHOT_REPEAT;

    if (b->cells[idx(b, row, col)] == SHIP) {
        // Use flood-fill to reveal the entire ship
        revealConnectedShips(b, row, col);
        return SHOT_SUNK;
    }
    b->revealed[idx(b, row, col)] = MISS;
    return SHOT_MISS;
}

bool board_all_sunk(const Board *b) {
    for (int i = 0; i < b->rows * b->cols; i++) {
        if (b->cells[i] == SHIP && b->revealed[i] != HIT) return false;
    }
    return true;
}

// Reveal all hidden cells at the end
void board_reveal_all(Board *b) {
    for (int i = 0; i < b->rows * b->cols; i++) {
        if (b->revealed[i] == HIDDEN) {
            b->revealed[i] = MISS;
        }
    }
}
