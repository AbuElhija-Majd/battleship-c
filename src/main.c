#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include "board.h"
#include "game.h"

#define DEFAULT_SIZE 8
#define MIN_SIZE 5
#define MAX_SIZE 26  // columns are labeled A-Z

static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s [rows] [cols] [seed]\n"
                    "  rows, cols: %d-%d (default %d)\n"
                    "  seed: non-negative integer (default: current time)\n",
            prog, MIN_SIZE, MAX_SIZE, DEFAULT_SIZE);
}

// Parse a whole string as an integer in [min, max]
static bool parse_long(const char *s, long min, long max, long *out) {
    char *end;
    errno = 0;
    long value = strtol(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE) return false;
    if (value < min || value > max) return false;
    *out = value;
    return true;
}

int main(int argc, char *argv[]) {
    long rows = DEFAULT_SIZE, cols = DEFAULT_SIZE;
    unsigned seed = (unsigned)time(NULL);

    if (argc > 4) {
        print_usage(argv[0]);
        return 1;
    }
    if (argc > 1 && !parse_long(argv[1], MIN_SIZE, MAX_SIZE, &rows)) {
        fprintf(stderr, "Invalid rows: %s\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }
    if (argc > 2 && !parse_long(argv[2], MIN_SIZE, MAX_SIZE, &cols)) {
        fprintf(stderr, "Invalid cols: %s\n", argv[2]);
        print_usage(argv[0]);
        return 1;
    }
    if (argc > 3) {
        long value;
        if (!parse_long(argv[3], 0, 2147483647L, &value)) {
            fprintf(stderr, "Invalid seed: %s\n", argv[3]);
            print_usage(argv[0]);
            return 1;
        }
        seed = (unsigned)value;
    }

    Game game;
    if (!game_init(&game, (int)rows, (int)cols, seed)) return 1;

    game_play(&game);

    game_destroy(&game);
    return 0;
}
