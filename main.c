#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define ROWS 8
#define COLS 8
#define NUM_OF_BOARDS 5
#define SUBMARINE 'S'
#define EMPTY '~'
#define HIDDEN ' '

// Function Declarations
void print_welcome_message();
void print_wrong_board_number();
void print_enter_position();
void print_error_row_or_col();
void print_error_position_already_bombed();
void print_winning_message(int n_submarines, int n_moves);
void printMatrix(char matrix[ROWS][COLS]);
void initializeBoard(const char source[ROWS][COLS],
                     char destination[ROWS][COLS]);
int countShips(char board[ROWS][COLS]);
void revealShip(char gameBoard[ROWS][COLS],char displayBoard[ROWS][COLS]
                , int startRow, int startCol);
int getBoardNumber();
void initializeGameBoards(int boardNumber,char gameBoard[ROWS][COLS]
                          , char displayBoard[ROWS][COLS]);
void playGame(char gameBoard[ROWS][COLS], char displayBoard[ROWS][COLS]
              , int totalShips);
bool getUserInput(int *row, char *colChar);
bool validatePosition(int row, int col, char displayBoard[ROWS][COLS]);
void updateGameState(char gameBoard[ROWS][COLS], char displayBoard[ROWS][COLS]
                     , int row, int col,
                     int *moves, int *revealedSubmarines);
void processTurn(char gameBoard[ROWS][COLS], char displayBoard[ROWS][COLS]
                 , int *moves, int *revealedSubmarines);
void revealHiddenCells(char displayBoard[ROWS][COLS]);
bool isValidPosition(int row, int col);

// Predefined Boards
const char MATRIX_1[ROWS][COLS] = {
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', 'S', ' ', ' ', ' ', 'S', ' '},
        {' ', ' ', 'S', ' ', ' ', ' ', 'S', ' '},
        {' ', ' ', 'S', ' ', ' ', ' ', 'S', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', 'S', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', 'S', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', 'S', ' ', ' ', ' ', ' '}
};
const char MATRIX_2[ROWS][COLS] = {
        {'S', ' ', ' ', ' ', ' ', ' ', ' ', '~'},
        {'S', ' ', 'S', ' ', ' ', ' ', ' ', ' '},
        {'S', ' ', 'S', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', 'S', ' ', ' ', ' ', ' ', 'S'},
        {' ', ' ', ' ', ' ', ' ', ' ', '~', 'S'},
        {' ', ' ', ' ', ' ', ' ', ' ', '~', 'S'},
        {' ', 'S', 'S', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
};
const char MATRIX_3[ROWS][COLS] = {
        {' ', 'S', 'S', 'S', ' ', ' ', ' ', '~'},
        {' ', ' ', ' ', ' ', '~', 'S', 'S', 'S'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', 'S', 'S', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', 'S', 'S', 'S', 'S', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', ' ', ' ', ' ', ' ', '~'}
};
const char MATRIX_4[ROWS][COLS] = {
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
};
const char MATRIX_5[ROWS][COLS] = {
        {'S', ' ', 'S', ' ', ' ', ' ', '~', 'S'},
        {' ', ' ', ' ', ' ', ' ', 'S', ' ', '~'},
        {' ', ' ', 'S', ' ', ' ', ' ', ' ', '~'},
        {' ', ' ', ' ', ' ', ' ', ' ', 'S', '~'},
        {' ', ' ', ' ', 'S', ' ', ' ', ' ', '~'},
        {' ', 'S', ' ', ' ', ' ', ' ', 'S', ' '},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'S', ' ', ' ', 'S', ' ', ' ', '~', 'S'}
};

// Print functions
void print_welcome_message() {
    printf("Welcome to Battleship!"
           " Please enter board number:\n");
}

void print_wrong_board_number() {
    printf("Error in board number, try again\n");
}

void print_enter_position() {
    printf("Please enter position:\n");
}

void print_error_row_or_col() {
    printf("Error in row or column - out of bound\n");
}

void print_error_position_already_bombed() {
    printf("This position was already bombed - try again!\n");
}

void print_winning_message(int n_submarines, int n_moves) {
    printf("Congratulations, Admiral!\nYou've successfully revealed "
           "all %d submarines in %d moves!\n", n_submarines, n_moves);
}

// Print a ROWSxCOLS matrix
void printMatrix(char matrix[ROWS][COLS]) {
    printf("  ");
    for (int j = 0; j < COLS; j++) {
        printf(" %c", 'A' + j);
    }
    printf("\n");

    for (int i = 0; i < ROWS; i++) {
        printf("%d ", i);
        for (int j = 0; j < COLS; j++) {
            printf("|%c", matrix[i][j]);
        }
        printf("|\n");
    }
}

// Initialize a board
void initializeBoard(const char source[ROWS][COLS],
                     char destination[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            destination[i][j] = source[i][j];
        }
    }
}

// Helper function to perform flood-fill
void markConnectedCells(char board[ROWS][COLS], bool visited[ROWS][COLS],
                        int row, int col) {
    // Check boundaries and if the cell is a submarine and not visited
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS || visited[row][col]
    || board[row][col] != SUBMARINE) {
        return;
    }

    // Mark the current cell as visited
    visited[row][col] = true;

    // Recursively visit all 4 adjacent cells
    markConnectedCells(board, visited, row + 1, col); // Down
    markConnectedCells(board, visited, row - 1, col); // Up
    markConnectedCells(board, visited, row, col + 1); // Right
    markConnectedCells(board, visited, row, col - 1); // Left
}

int countShips(char board[ROWS][COLS]) {
    int count = 0;
    bool visited[ROWS][COLS] = {false};

    // Loop through every cell in the board
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // If we find an unvisited submarine cell, it's a new submarine
            if (board[i][j] == SUBMARINE && !visited[i][j]) {
                count++;
                markConnectedCells(board, visited, i, j);
                // Mark all connected parts as visited
            }
        }
    }
    return count;
}

// Helper function to reveal all connected ships cells
void revealConnectedShips(char gameBoard[ROWS][COLS],
                          char displayBoard[ROWS][COLS], int row, int col) {
    // Check boundaries and if the cell is a ships
    // and hidden on the display board
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS ||
        displayBoard[row][col] == SUBMARINE ||
        gameBoard[row][col] != SUBMARINE) {
        return;
    }

    // Reveal the current cell on the display board
    displayBoard[row][col] = SUBMARINE;

    // Recursively reveal all 4 adjacent cells
    revealConnectedShips(gameBoard, displayBoard, row + 1, col); // Down
    revealConnectedShips(gameBoard, displayBoard, row - 1, col); // Up
    revealConnectedShips(gameBoard, displayBoard, row, col + 1); // Right
    revealConnectedShips(gameBoard, displayBoard, row, col - 1); // Left
}

void revealShip(char gameBoard[ROWS][COLS], char displayBoard[ROWS][COLS]
                , int startRow, int startCol) {
    // Use flood-fill to reveal the entire ship
    revealConnectedShips(gameBoard, displayBoard, startRow, startCol);
}

// Get board number from user
int getBoardNumber() {
    int boardNumber;
    while (1) {
        if (scanf("%d", &boardNumber) != 1)  return -1;
        if (boardNumber >= 1 && boardNumber <= NUM_OF_BOARDS) {
            return boardNumber;
        } else {
            print_wrong_board_number();
        }
    }
}

// Initialize game boards
void initializeGameBoards(int boardNumber, char gameBoard[ROWS][COLS],
                          char displayBoard[ROWS][COLS]) {
    switch (boardNumber) {
        case 1: initializeBoard(MATRIX_1, gameBoard); break;
        case 2: initializeBoard(MATRIX_2, gameBoard); break;
        case 3: initializeBoard(MATRIX_3, gameBoard); break;
        case 4: initializeBoard(MATRIX_4, gameBoard); break;
        case 5: initializeBoard(MATRIX_5, gameBoard); break;
    }
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            displayBoard[i][j] = HIDDEN;
        }
    }
}

// Gameplay loop
void playGame(char gameBoard[ROWS][COLS], char displayBoard[ROWS][COLS],
              int totalShips) {
    int moves = 0;
    int revealedSubmarines = 0;

    while (revealedSubmarines < totalShips) {
        processTurn(gameBoard, displayBoard, &moves, &revealedSubmarines);
    }

    revealHiddenCells(displayBoard);
    printMatrix(displayBoard);
    print_winning_message(totalShips, moves);
}

bool getUserInput(int *row, char *colChar) {
    if (scanf("%d %c", row, colChar) != 2) {
        return false;
    }
    return true;
}

bool validatePosition(int row, int col, char displayBoard[ROWS][COLS]) {
    if (!isValidPosition(row, col)) {
        print_error_row_or_col();
        return false;
    }

    if (displayBoard[row][col] != HIDDEN) {
        print_error_position_already_bombed();
        return false;
    }

    return true;
}

void updateGameState(char gameBoard[ROWS][COLS], char displayBoard[ROWS][COLS],
                     int row, int col, int *moves, int *revealedSubmarines) {
    (*moves)++; // Increment moves

    if (gameBoard[row][col] == SUBMARINE) {
        revealShip(gameBoard, displayBoard, row, col);
        *revealedSubmarines = countShips(displayBoard);
        // Update revealed count
    }
    else displayBoard[row][col] = EMPTY;
}


void processTurn(char gameBoard[ROWS][COLS], char displayBoard[ROWS][COLS],
                 int *moves, int *revealedSubmarines) {
    bool shouldPrintBoard = true;
    // Local flag for controlling board printing

    while (1) {
        if (shouldPrintBoard) printMatrix(displayBoard);
        // Print the board only when allowed
        print_enter_position();

        // Get user input
        int row;
        char colChar;
        if (!getUserInput(&row, &colChar)) {
            shouldPrintBoard = false;
            // Don't reprint the board for invalid input
            continue;
        }

        // Convert column character to index
        int col = colChar - 'A';

        // Validate the position
        if (!validatePosition(row, col, displayBoard)) {
            shouldPrintBoard = false;
            // Don't reprint the board for invalid position
            continue;
        }

        // Update game state
        updateGameState(gameBoard, displayBoard, row,
                        col, moves, revealedSubmarines);
        break; // Exit the loop after a successful turn
    }
}





// Reveal all hidden cells at the end
void revealHiddenCells(char displayBoard[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (displayBoard[i][j] == HIDDEN) {
                displayBoard[i][j] = EMPTY;
            }
        }
    }
}

bool isValidPosition(int row, int col) {
    return row >= 0 && row < ROWS && col >= 0 && col < COLS;
}

int main(void) {
    print_welcome_message();
    int boardNumber = getBoardNumber();
    if (boardNumber == -1) return 1;

    char gameBoard[ROWS][COLS], displayBoard[ROWS][COLS];
    initializeGameBoards(boardNumber, gameBoard, displayBoard);

    int totalShips = countShips(gameBoard);
    playGame(gameBoard, displayBoard, totalShips);

    return 0;
}
