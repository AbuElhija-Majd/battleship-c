#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "game.h"

static const int FLEET[] = {4, 3, 3, 2};
#define FLEET_SIZE (int)(sizeof(FLEET) / sizeof(FLEET[0]))

static void clear_screen(void) {
    printf("\033[2J\033[H");
}

// Print a board as the opponent sees it
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

// Discard the rest of the current input line. Returns false on EOF.
static bool discard_line(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    return ch != EOF;
}

bool game_init(Game *g, int rows, int cols, unsigned seed) {
    memset(g, 0, sizeof(*g));

    for (int p = 0; p < 2; p++) {
        Player *player = &g->players[p];
        snprintf(player->name, sizeof(player->name), "Player %d", p + 1);

        player->board = board_create(rows, cols);
        if (player->board == NULL) {
            fprintf(stderr, "Out of memory\n");
            game_destroy(g);
            return false;
        }
        if (!board_place_ships(player->board, FLEET, FLEET_SIZE,
                               seed + (unsigned)p)) {
            fprintf(stderr, "Board %dx%d is too small for the fleet\n",
                    rows, cols);
            game_destroy(g);
            return false;
        }
    }
    return true;
}

void game_destroy(Game *g) {
    for (int p = 0; p < 2; p++) {
        board_destroy(g->players[p].board);
        g->players[p].board = NULL;
    }
}

// Read moves until one is a real shot at target. Returns the result,
// or SHOT_INVALID if input ended.
static ShotResult take_shot(Board *target, bool *eof) {
    *eof = false;
    while (1) {
        printf("Please enter position (e.g. 3 B):\n");

        int row;
        char colChar;
        int inputStatus = read_move(&row, &colChar);
        if (inputStatus == -1) {
            *eof = true;
            return SHOT_INVALID;
        }
        if (inputStatus == 0) {
            printf("Invalid input - enter a row number and a column letter\n");
            continue;
        }

        // Ignore anything typed after the move on the same line
        if (!discard_line()) *eof = true;

        ShotResult result = board_fire(target, row, colChar - 'A');
        if (result == SHOT_INVALID) {
            printf("Error in row or column - out of bound\n");
        } else if (result == SHOT_REPEAT) {
            printf("This position was already bombed - try again!\n");
        } else {
            return result;  // a real shot, even if input ends right after
        }
        if (*eof) return SHOT_INVALID;
    }
}

static void print_result(ShotResult result) {
    switch (result) {
        case SHOT_MISS: printf("MISS.\n"); break;
        case SHOT_HIT:  printf("HIT!\n"); break;
        case SHOT_SUNK: printf("HIT! Ship sunk!\n"); break;
        default: break;
    }
}

void game_play(Game *g) {
    const Board *any = g->players[0].board;

    while (1) {
        Player *shooter = &g->players[g->current];
        Player *opponent = &g->players[1 - g->current];

        clear_screen();
        printf("Battleship %dx%d - %s's turn (move %d)\n\n",
               any->rows, any->cols, shooter->name, shooter->moves + 1);
        printf("%s's waters:\n", opponent->name);
        print_board(opponent->board);

        bool eof;
        ShotResult result = take_shot(opponent->board, &eof);
        if (result == SHOT_INVALID) break;  // input ended without a shot

        shooter->moves++;
        bool won = board_all_sunk(opponent->board);
        if (won) board_reveal_all(opponent->board);

        printf("\n");
        print_board(opponent->board);
        print_result(result);

        if (won) {
            printf("\n%s wins! All of %s's ships are sunk.\n",
                   shooter->name, opponent->name);
            printf("Moves: %s %d, %s %d\n",
                   g->players[0].name, g->players[0].moves,
                   g->players[1].name, g->players[1].moves);
            return;
        }
        if (eof) break;

        printf("Press Enter to pass the turn to %s...\n", opponent->name);
        if (!discard_line()) break;
        g->current = 1 - g->current;
    }

    printf("\nInput ended - game over without a winner.\n");
}
