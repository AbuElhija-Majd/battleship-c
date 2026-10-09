#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

static const int FLEET[] = {4, 3, 3, 2};
#define FLEET_SIZE (int)(sizeof(FLEET) / sizeof(FLEET[0]))
#define FLEET_CELLS 12

static int failures = 0;

// Record a failure with its location and keep running the other checks
#define CHECK(cond)                                                      \
    do {                                                                 \
        if (!(cond)) {                                                   \
            fprintf(stderr, "%s:%d: %s: check failed: %s\n",             \
                    __FILE__, __LINE__, __func__, #cond);                \
            failures++;                                                  \
        }                                                                \
    } while (0)

static Board *create_or_die(int rows, int cols) {
    Board *b = board_create(rows, cols);
    if (b == NULL) {
        fprintf(stderr, "board_create(%d, %d) failed\n", rows, cols);
        exit(1);
    }
    return b;
}

// Write a ship directly into cells so a test controls the layout exactly
static void put_ship(Board *b, int row, int col, int length, int horizontal) {
    for (int k = 0; k < length; k++) {
        int r = horizontal ? row : row + k;
        int c = horizontal ? col + k : col;
        b->cells[r * b->cols + c] = SHIP;
    }
}

static int count_cells(const char *grid, int n, char value) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (grid[i] == value) count++;
    }
    return count;
}

static int compare_ints(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

// Total SHIP cells == sum of lengths, every cell is SHIP or EMPTY, and the
// fleet forms separate ships of exactly the requested lengths
static void test_placement_counts(void) {
    static const int sizes[][2] = { {5, 5}, {8, 8}, {6, 10}, {26, 5}, {26, 26} };
    int expected[FLEET_SIZE];
    memcpy(expected, FLEET, sizeof(FLEET));
    qsort(expected, FLEET_SIZE, sizeof(int), compare_ints);

    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); s++) {
        for (unsigned seed = 0; seed < 50; seed++) {
            int rows = sizes[s][0], cols = sizes[s][1], n = rows * cols;
            Board *b = create_or_die(rows, cols);

            CHECK(board_place_ships(b, FLEET, FLEET_SIZE, seed) == 1);
            CHECK(count_cells(b->cells, n, SHIP) == FLEET_CELLS);
            CHECK(count_cells(b->cells, n, SHIP) +
                  count_cells(b->cells, n, EMPTY) == n);
            CHECK(count_cells(b->revealed, n, HIDDEN) == n);
            CHECK(board_count_ships(b) == FLEET_SIZE);

            // Reveal each ship once and compare its size with the fleet
            int lengths[FLEET_SIZE + 1], found = 0;
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    int got = board_reveal_ship(b, r, c);
                    if (got > 0 && found <= FLEET_SIZE) lengths[found++] = got;
                }
            }
            CHECK(found == FLEET_SIZE);
            if (found == FLEET_SIZE) {
                qsort(lengths, FLEET_SIZE, sizeof(int), compare_ints);
                CHECK(memcmp(lengths, expected, sizeof(expected)) == 0);
            }
            board_destroy(b);
        }
    }
}

// The same seed always gives the same layout
static void test_placement_deterministic(void) {
    Board *a = create_or_die(8, 8);
    Board *b = create_or_die(8, 8);
    CHECK(board_place_ships(a, FLEET, FLEET_SIZE, 42) == 1);
    CHECK(board_place_ships(b, FLEET, FLEET_SIZE, 42) == 1);
    CHECK(memcmp(a->cells, b->cells, 64) == 0);
    board_destroy(a);
    board_destroy(b);
}

// A fleet that cannot fit is reported; invalid sizes are refused
static void test_placement_failure(void) {
    Board *b = create_or_die(3, 3);
    CHECK(board_place_ships(b, FLEET, FLEET_SIZE, 1) == 0);
    board_destroy(b);

    CHECK(board_create(0, 5) == NULL);
    CHECK(board_create(5, -1) == NULL);
    board_destroy(NULL);  // must be a no-op
}

// Empty cell -> SHOT_MISS; ship cell -> SHOT_SUNK revealing the whole ship
// and nothing else. (SHOT_HIT only occurs when memory runs out.)
static void test_fire_miss_and_hit(void) {
    Board *b = create_or_die(5, 5);
    put_ship(b, 1, 1, 3, 1);  // row 1, columns B-D
    put_ship(b, 3, 4, 2, 0);  // column E, rows 3-4

    CHECK(board_fire(b, 0, 0) == SHOT_MISS);
    CHECK(board_view(b, 0, 0) == MISS);

    CHECK(board_fire(b, 1, 2) == SHOT_SUNK);
    CHECK(board_view(b, 1, 1) == HIT);
    CHECK(board_view(b, 1, 2) == HIT);
    CHECK(board_view(b, 1, 3) == HIT);
    CHECK(board_view(b, 1, 0) == HIDDEN);
    CHECK(board_view(b, 1, 4) == HIDDEN);
    CHECK(board_view(b, 3, 4) == HIDDEN);  // the other ship stays hidden
    CHECK(count_cells(b->revealed, 25, HIT) == 3);

    board_destroy(b);
}

// BFS reveal returns the number of newly revealed cells
static void test_reveal_ship(void) {
    Board *b = create_or_die(6, 6);
    put_ship(b, 2, 0, 4, 1);

    CHECK(board_reveal_ship(b, 0, 0) == 0);  // not a ship
    CHECK(board_reveal_ship(b, 2, 3) == 4);  // from the end of the ship
    CHECK(board_reveal_ship(b, 2, 1) == 0);  // already revealed
    CHECK(board_reveal_ship(b, -1, 2) == 0); // out of bounds
    board_destroy(b);
}

// A second shot on the same cell -> SHOT_REPEAT, whether it was a hit,
// a miss, or part of a ship revealed by an earlier hit
static void test_repeat_shot(void) {
    Board *b = create_or_die(5, 5);
    put_ship(b, 0, 0, 3, 1);

    CHECK(board_fire(b, 4, 4) == SHOT_MISS);
    CHECK(board_fire(b, 4, 4) == SHOT_REPEAT);
    CHECK(board_view(b, 4, 4) == MISS);

    CHECK(board_fire(b, 0, 0) == SHOT_SUNK);
    CHECK(board_fire(b, 0, 0) == SHOT_REPEAT);
    CHECK(board_fire(b, 0, 2) == SHOT_REPEAT);  // revealed with the ship

    board_destroy(b);
}

// Shots outside the board -> SHOT_INVALID and nothing changes
static void test_out_of_bounds(void) {
    Board *b = create_or_die(5, 7);
    put_ship(b, 0, 0, 2, 1);

    CHECK(board_fire(b, -1, 0) == SHOT_INVALID);
    CHECK(board_fire(b, 0, -1) == SHOT_INVALID);
    CHECK(board_fire(b, 5, 0) == SHOT_INVALID);
    CHECK(board_fire(b, 0, 7) == SHOT_INVALID);
    CHECK(board_fire(b, 100, 100) == SHOT_INVALID);
    CHECK(count_cells(b->revealed, 35, HIDDEN) == 35);

    board_destroy(b);
}

// board_all_sunk stays false until every ship cell is revealed
static void test_win_condition(void) {
    Board *b = create_or_die(5, 5);
    put_ship(b, 0, 0, 2, 1);
    put_ship(b, 4, 2, 3, 1);

    CHECK(!board_all_sunk(b));
    CHECK(board_fire(b, 2, 2) == SHOT_MISS);
    CHECK(!board_all_sunk(b));
    CHECK(board_fire(b, 0, 1) == SHOT_SUNK);
    CHECK(!board_all_sunk(b));
    CHECK(board_fire(b, 4, 4) == SHOT_SUNK);
    CHECK(board_all_sunk(b));

    // A full random game: firing at every cell always ends in a win
    Board *r = create_or_die(10, 10);
    CHECK(board_place_ships(r, FLEET, FLEET_SIZE, 7) == 1);
    for (int i = 0; i < 100 && !board_all_sunk(r); i++) {
        board_fire(r, i / 10, i % 10);
    }
    CHECK(board_all_sunk(r));

    board_destroy(b);
    board_destroy(r);
}

int main(void) {
    test_placement_counts();
    test_placement_deterministic();
    test_placement_failure();
    test_fire_miss_and_hit();
    test_reveal_ship();
    test_repeat_shot();
    test_out_of_bounds();
    test_win_condition();

    if (failures > 0) {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    printf("All tests passed\n");
    return 0;
}
