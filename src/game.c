#include <stdio.h>
#include <ctype.h>
#include "game.h"

// Print functions
static void print_welcome_message(const Board *b) {
    printf("Welcome to Battleship! Board size: %d x %d\n", b->rows, b->cols);
}

static void print_enter_position(void) {
    printf("Please enter position:\n");
}

static void print_error_row_or_col(void) {
    printf("Error in row or column - out of bound\n");
}

static void print_error_position_already_bombed(void) {
    printf("This position was already bombed - try again!\n");
}

static void print_winning_message(int n_submarines, int n_moves) {
    printf("Congratulations, Admiral!\nYou've successfully revealed "
           "all %d submarines in %d moves!\n", n_submarines, n_moves);
}

// Print the board as the opponent sees it
static void print_board(const Board *b) {
    int width = b->rows > 10 ? 2 : 1;  // row labels go up to rows - 1

    printf("%*s ", width, "");
    for (int j = 0; j < b->cols; j++) {
        printf(" %c", 'A' + j);
    }
    printf("\n");

    for (int i = 0; i < b->rows; i++) {
        printf("%*d ", width, i);
        for (int j = 0; j < b->cols; j++) {
            printf("|%c", board_view(b, i, j));
        }
        printf("|\n");
    }
}

int read_move(int *row, char *col) {
    int res = scanf("%d %c", row, col);
    if (res == EOF) return -1;
    if (res != 2) {
        // Discard the rest of the line so the bad input isn't re-read
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        if (ch == EOF) return -1;
        return 0;
    }
    *col = (char)toupper((unsigned char)*col);
    return 1;
}

// Returns false if input ended before a valid move was made
static bool process_turn(Player *p) {
    bool shouldPrintBoard = true;
    // Local flag for controlling board printing

    while (1) {
        if (shouldPrintBoard) print_board(p->board);
        // Print the board only when allowed
        print_enter_position();

        // Get user input
        int row;
        char colChar;
        int inputStatus = read_move(&row, &colChar);
        if (inputStatus == -1) return false;
        if (inputStatus == 0) {
            shouldPrintBoard = false;
            // Don't reprint the board for invalid input
            continue;
        }

        // Convert column character to index
        int col = colChar - 'A';

        ShotResult result = board_fire(p->board, row, col);
        if (result == SHOT_INVALID) {
            print_error_row_or_col();
            shouldPrintBoard = false;
            // Don't reprint the board for invalid position
            continue;
        }
        if (result == SHOT_REPEAT) {
            print_error_position_already_bombed();
            shouldPrintBoard = false;
            continue;
        }

        p->moves++;
        return true; // Exit the loop after a successful turn
    }
}

// Gameplay loop
void game_play(Game *g) {
    Player *p = &g->players[g->current];
    print_welcome_message(p->board);

    int totalShips = board_count_ships(p->board);
    if (totalShips < 0) {
        fprintf(stderr, "Out of memory\n");
        return;
    }

    while (!board_all_sunk(p->board)) {
        if (!process_turn(p)) {
            return; // End of input - quit without a winner
        }
    }

    board_reveal_all(p->board);
    print_board(p->board);
    print_winning_message(totalShips, p->moves);
}
