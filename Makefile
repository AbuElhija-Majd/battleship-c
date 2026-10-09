CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
SRC     = src/board.c src/game.c src/main.c
TEST_SRC= src/board.c tests/test_board.c

all: battleship

battleship: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

test_board: $(TEST_SRC)
	$(CC) $(CFLAGS) -Isrc -o $@ $(TEST_SRC)

test: test_board
	./test_board

valgrind: test_board battleship
	valgrind --leak-check=full --error-exitcode=1 ./test_board
	valgrind --leak-check=full --error-exitcode=1 ./battleship 8 8 42 < tests/sample_game.txt

clean:
	rm -f battleship test_board

.PHONY: all test valgrind clean
