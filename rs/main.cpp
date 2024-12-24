#include "bitboard.h"
#include "move_generation.h"
#include "attack_map.h"
#include "board.h"
#include "chess_rules.h"

#include <iostream>
#include <string>
#include <sstream>
#include <cctype>

int string_to_square(const std::string &square) {
    // Convert a chess square like "A2" to an integer index (0-63)
    char column = std::toupper(square[0]);
    int row = square[1] - '1';
    int col = column - 'A';

    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        return row * 8 + col;
    }
    return -1; // Invalid square
}

void print_move_prompt(int player) {
    std::cout << (player == 1 ? "White's turn: " : "Black's turn: ");
}

int main() {
    // Initialize the chessboard
    Board board;

    // Print initial board state
    std::cout << "Initial Board State:" << std::endl;
    board.print_board();

    // Loop to handle moves until the game ends (e.g., checkmate, stalemate)
    int turn = 1; // 1 for White's turn, 2 for Black's turn
    while (true) {
        print_move_prompt(turn);

        std::string piece, from_square, to_square;
        
        // Ask for the piece type (e.g., "whitePawn" or "blackPawn")
        std::cout << "Enter the piece (e.g., whitePawn, blackPawn): ";
        std::cin >> piece;

        // Ask for the 'from' square
        std::cout << "Enter the 'from' square (e.g., D2): ";
        std::cin >> from_square;

        // Convert 'from' square to board index
        int from = string_to_square(from_square);
        if (from == -1) {
            std::cout << "Invalid square!" << std::endl;
            continue;
        }

        // Ask for the 'to' square
        std::cout << "Enter the 'to' square (e.g., D4): ";
        std::cin >> to_square;

        // Convert 'to' square to board index
        int to = string_to_square(to_square);
        if (to == -1) {
            std::cout << "Invalid square!" << std::endl;
            continue;
        }

        // Determine the piece bitboard
        Bitboard &pieceBitboard = (piece == "whitePawn") ? board.whitePawns :
                                  (piece == "blackPawn") ? board.blackPawns :
                                  (piece == "whiteRook") ? board.whiteRooks :
                                  (piece == "blackRook") ? board.blackRooks :
                                  (piece == "whiteKnight") ? board.whiteKnights :
                                  (piece == "blackKnight") ? board.blackKnights :
                                  (piece == "whiteBishop") ? board.whiteBishops :
                                  (piece == "blackBishop") ? board.blackBishops :
                                  (piece == "whiteQueen") ? board.whiteQueen :
                                  (piece == "blackQueen") ? board.blackQueen :
                                  (piece == "whiteKing") ? board.whiteKing :
                                  (piece == "blackKing") ? board.blackKing : board.whitePawns;

        // Make the move on the board
        board.make_move(from, to, pieceBitboard);

        // Print the updated board state
        std::cout << "Board after move:" << std::endl;
        board.print_board();

        // Alternate turns
        turn = (turn == 1) ? 2 : 1;
    }

    return 0;
}
