#include <stdlib.h>
#include "board.h"

#define MAX_PLACEMENT_ATTEMPTS 1000  // per ship
#define MAX_FLEET_RESTARTS 100      // whole fleet, when a ship gets stuck

// Index of cell (r, c) in the one-dimensional arrays
static int idx(const Board *b, int r, int c) {
    return r * b->cols + c;
}

Board *board_create(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return NULL;

    Board *b = malloc(sizeof(Board));
    if (b == NULL) return NULL;

    b->rows = rows;
    b->cols = cols;
    b->cells = malloc((size_t)rows * (size_t)cols);
    if (b->cells == NULL) {
        free(b);
        return NULL;
    }
    b->revealed = malloc((size_t)rows * (size_t)cols);
    if (b->revealed == NULL) {
        free(b->cells);
        free(b);
        return NULL;
    }

    for (int i = 0; i < rows * cols; i++) {
        b->cells[i] = EMPTY;
        b->revealed[i] = HIDDEN;
    }
    return b;
}

void board_destroy(Board *b) {
    if (b == NULL) return;
    free(b->cells);
    free(b->revealed);
    free(b);
}

static bool is_ship(const Board *b, int row, int col) {
    return board_in_bounds(b, row, col) && b->cells[idx(b, row, col)] == SHIP;
}

// A ship fits if every cell is on the board, empty, and not side-by-side
// with an existing ship
static bool can_place(const Board *b, int row, int col, int length,
                      bool horizontal) {
    for (int k = 0; k < length; k++) {
        int r = horizontal ? row : row + k;
        int c = horizontal ? col + k : col;
        if (!board_in_bounds(b, r, c) || is_ship(b, r, c)) return false;
        if (is_ship(b, r + 1, c) || is_ship(b, r - 1, c) ||
            is_ship(b, r, c + 1) || is_ship(b, r, c - 1)) return false;
    }
    return true;
}

// Try to place one ship at random positions; returns false if it never fits
static bool place_ship(Board *b, int length) {
    for (int attempt = 0; attempt < MAX_PLACEMENT_ATTEMPTS; attempt++) {
        bool horizontal = rand() % 2;
        int row = rand() % b->rows;
        int col = rand() % b->cols;
        if (!can_place(b, row, col, length, horizontal)) continue;

        for (int k = 0; k < length; k++) {
            int r = horizontal ? row : row + k;
            int c = horizontal ? col + k : col;
            b->cells[idx(b, r, c)] = SHIP;
        }
        return true;
    }
    return false;
}

int board_place_ships(Board *b, const int *lengths, int count, unsigned seed) {
    for (int s = 0; s < count; s++) {
        if (lengths[s] <= 0) return 0;
    }

    srand(seed);

    // Earlier ships can leave no room for later ones, so start over
    // with an empty board when a ship cannot be placed
    for (int restart = 0; restart < MAX_FLEET_RESTARTS; restart++) {
        for (int i = 0; i < b->rows * b->cols; i++) {
            b->cells[i] = EMPTY;
        }

        int s = 0;
        while (s < count && place_ship(b, lengths[s])) s++;
        if (s == count) return 1;
    }
    return 0;
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
