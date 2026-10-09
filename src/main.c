#include "board.h"
#include "game.h"

int main(void) {
    char cells[BOARD_ROWS * BOARD_COLS];
    char revealed[BOARD_ROWS * BOARD_COLS];
    Board board = { BOARD_ROWS, BOARD_COLS, cells, revealed };

    if (!game_select_board(&board)) return 1;

    Game game = { .players = { { "Player 1", &board, 0 } }, .current = 0 };
    game_play(&game);

    return 0;
}
