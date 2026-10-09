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

// Breadth-first search over the ship cells connected to start (an index
// into cells). Every cell reached is set to mark in seen; cells already
// equal to mark are treated as visited. queue must hold rows * cols ints.
// Returns the number of cells newly marked.
static int flood_ship(const Board *b, int start, char *seen, char mark,
                      int *queue) {
    static const int dr[] = {1, -1, 0, 0};
    static const int dc[] = {0, 0, 1, -1};

    if (b->cells[start] != SHIP || seen[start] == mark) return 0;

    int head = 0, tail = 0;
    seen[start] = mark;
    queue[tail++] = start;

    while (head < tail) {
        int i = queue[head++];
        int r = i / b->cols, c = i % b->cols;

        for (int k = 0; k < 4; k++) {
            int nr = r + dr[k], nc = c + dc[k];
            if (!board_in_bounds(b, nr, nc)) continue;

            int j = idx(b, nr, nc);
            if (b->cells[j] == SHIP && seen[j] != mark) {
                seen[j] = mark;
                queue[tail++] = j;
            }
        }
    }
    return tail;  // every cell is pushed exactly once
}

static int *queue_create(const Board *b) {
    return malloc((size_t)b->rows * (size_t)b->cols * sizeof(int));
}

int board_count_ships(const Board *b) {
    char *visited = calloc((size_t)b->rows * (size_t)b->cols, 1);
    int *queue = queue_create(b);
    if (visited == NULL || queue == NULL) {
        free(visited);
        free(queue);
        return -1;
    }

    int count = 0;
    for (int i = 0; i < b->rows * b->cols; i++) {
        // An unvisited ship cell starts a new ship
        if (flood_ship(b, i, visited, 1, queue) > 0) count++;
    }

    free(visited);
    free(queue);
    return count;
}

int board_reveal_ship(Board *b, int r, int c) {
    if (!board_in_bounds(b, r, c)) return 0;

    int *queue = queue_create(b);
    if (queue == NULL) return -1;

    int revealed = flood_ship(b, idx(b, r, c), b->revealed, HIT, queue);
    free(queue);
    return revealed;
}

ShotResult board_fire(Board *b, int r, int c) {
    if (!board_in_bounds(b, r, c)) return SHOT_INVALID;

    int i = idx(b, r, c);
    if (b->revealed[i] != HIDDEN) return SHOT_REPEAT;

    if (b->cells[i] != SHIP) {
        b->revealed[i] = MISS;
        return SHOT_MISS;
    }

    // The first hit reveals the whole ship
    if (board_reveal_ship(b, r, c) < 0) {
        // Out of memory: still record the hit, just this one cell
        b->revealed[i] = HIT;
        return SHOT_HIT;
    }
    return SHOT_SUNK;
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
