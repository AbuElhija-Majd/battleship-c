# Battleship in C

## Overview

A two-player, pass-and-play Battleship game for the terminal, written in C11.
Each player gets a randomly placed fleet, and players take turns firing at each
other's waters until one side has sunk every ship.

## Features

- **Two players** on one terminal: the screen is cleared between turns and the
  turn passes when the player presses Enter.
- **Configurable board size** from 5×5 up to 26×26 (columns are labeled A–Z).
- **Seeded random placement** of a `{4, 3, 3, 2}` fleet. Ships never overlap or
  touch side-to-side, and the same seed always gives the same boards.
- **BFS ship detection**: a hit reveals the whole ship with a queue-based
  breadth-first search instead of recursion.
- **Input validation**: malformed input, out-of-bounds shots and repeated shots
  are rejected without costing a move; end of input (Ctrl+D) quits cleanly.

## Build & Run

Requires `gcc` and `make` (on Windows with MinGW, use `mingw32-make`).

```
make
./battleship 10 10 42
```

All arguments are optional:

| Argument | Meaning                                  | Default      |
|----------|------------------------------------------|--------------|
| `rows`   | Number of rows, 5–26                     | 8            |
| `cols`   | Number of columns, 5–26                  | 8            |
| `seed`   | Random seed for ship placement (≥ 0)     | current time |

Player 1's fleet is placed with `seed` and Player 2's with `seed + 1`.

Enter a shot as a row number and a column letter, e.g. `3 B` (lowercase works
too). A hit reveals the entire ship. The first player to sink all of the
opponent's ships wins, and the game reports both players' move counts.

## Testing

```
make test       # build and run the unit tests in tests/test_board.c
make valgrind   # run the tests and a scripted game under Valgrind (Linux)
make clean      # remove the built binaries
```

`make test` prints `All tests passed` on success and exits with a non-zero
status if any check fails. `make valgrind` replays `tests/sample_game.txt`,
a complete game that includes malformed input, out-of-bounds and repeated
shots, and fails if Valgrind reports any memory error or leak.

## Example session

`./battleship 8 8 42`, with the player's input shown after each prompt:

```
Battleship 8x8 - Player 1's turn (move 1)

Player 2's waters:
   A B C D E F G H
0 | | | | | | | | |
1 | | | | | | | | |
2 | | | | | | | | |
3 | | | | | | | | |
4 | | | | | | | | |
5 | | | | | | | | |
6 | | | | | | | | |
7 | | | | | | | | |
Please enter position (e.g. 3 B):
2 d

   A B C D E F G H
0 | | | | | | | | |
1 | | | |S| | | | |
2 | | | |S| | | | |
3 | | | |S| | | | |
4 | | | | | | | | |
5 | | | | | | | | |
6 | | | | | | | | |
7 | | | | | | | | |
HIT! Ship sunk!
Press Enter to pass the turn to Player 2...

Battleship 8x8 - Player 2's turn (move 1)

Player 1's waters:
   A B C D E F G H
0 | | | | | | | | |
...
Please enter position (e.g. 3 B):
9 A
Error in row or column - out of bound
Please enter position (e.g. 3 B):
4 b

   A B C D E F G H
0 | | | | | | | | |
1 | | | | | | | | |
2 | | | | | | | | |
3 | | | | | | | | |
4 | |S| | | | | | |
5 | |S| | | | | | |
6 | |S| | | | | | |
7 | | | | | | | | |
HIT! Ship sunk!
Press Enter to pass the turn to Player 1...
```

`S` marks a revealed ship, `~` a miss, and a blank cell has not been fired at.

## Project structure

```
battleship-c/
├── src/
│   ├── board.h  board.c   # board data + game rules, no I/O
│   ├── game.h   game.c    # turn loop, prompts, printing
│   └── main.c             # argument parsing, start the game
├── tests/
│   ├── test_board.c       # unit tests for board.c
│   └── sample_game.txt    # scripted two-player game for make valgrind
├── Makefile               # build, test, valgrind and clean targets
└── README.md              # this file
```

## Design notes

- **One-dimensional grid.** A `Board` stores `rows * cols` cells in two flat
  arrays: `cells` holds the ground truth (ship or empty) and `revealed` holds
  what the opponent sees (hidden, hit or miss). Cell `(r, c)` is at index
  `r * cols + c`, computed in a single `idx()` helper. Both arrays are
  allocated at runtime, so any board size works with the same code.
- **BFS instead of recursion.** Revealing a ship and counting ships use one
  queue-based breadth-first search over neighboring ship cells. The queue is
  sized `rows * cols`, so memory use is bounded and there is no recursion
  depth that a large board could overflow.
- **Game rules separate from the interface.** `board.c` contains no `printf`
  or `scanf`: it only changes and queries state and returns results such as
  `ShotResult`. All prompts, printing and input parsing live in `game.c`,
  which is what makes the rules easy to unit test.
- **Deterministic seeds for testing.** Ship placement takes an explicit seed,
  so a given seed always produces the same layout on the same C library.
  Unit tests use fixed seeds or write ships directly into `cells`, and the
  scripted game in `tests/sample_game.txt` fires at every cell so it finishes
  for any layout, which keeps it valid even where `rand()` differs between
  platforms.
