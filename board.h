#ifndef BOARD_H
#define BOARD_H


#include "bitboard.h"
#include "attack_map.h"
#include "move_generation.h"
#include <iostream>

class Board {
public:
    // Individual white pieces
    Bitboard whitePawnA, whitePawnB, whitePawnC, whitePawnD;
    Bitboard whitePawnE, whitePawnF, whitePawnG, whitePawnH;
    Bitboard whiteKnight1, whiteKnight2;
    Bitboard whiteBishop1, whiteBishop2;
    Bitboard whiteRook1, whiteRook2;
    Bitboard whiteQueen;
    Bitboard whiteKing;

    // Individual black pieces
    Bitboard blackPawnA, blackPawnB, blackPawnC, blackPawnD;
    Bitboard blackPawnE, blackPawnF, blackPawnG, blackPawnH;
    Bitboard blackKnight1, blackKnight2;
    Bitboard blackBishop1, blackBishop2;
    Bitboard blackRook1, blackRook2;
    Bitboard blackQueen;
    Bitboard blackKing;

    // Aggregate bitboards
    Bitboard whitePieces, blackPieces; // All white and black pieces combined
    Bitboard whitePawns, blackPawns;
    Bitboard whiteKnights, blackKnights;
    Bitboard whiteBishops, blackBishops;
    Bitboard whiteRooks , blackRooks;
    Bitboard allPieces;                // All pieces on the board
    Bitboard emptySquares;             // Squares without any pieces


    // Constructor
    Board();

    // Methods for board management
    void initialize(); // Set up the board with initial positions
    void make_move(Bitboard from, Bitboard to, Bitboard &piece); // Move a specific piece
    void remove_piece(Bitboard square, Bitboard &piece);         // Remove a specific piece
    bool is_square_empty(Bitboard square) const;                // Check if a square is empty
    void update_all();                                           // Update aggregate bitboards
    void print_board() const;                                    // Print the entire board  
    void print_piece(Bitboard piece) const;                     // Print a specific piece bitboard
    void capture_piece(Bitboard to);
    void copy(const Board &other);                              // Function to copy the code
    // Validation and move generation
    bool is_valid_move(Bitboard from, Bitboard to, Bitboard &piece) const; // Validate if a move is valid
    Bitboard generate_moves_for_piece(Bitboard position, Bitboard &piece) const; // Generate all possible moves for a given piece
    // Possible moves for white and Black pieces
    Bitboard generate_White_pieces() const;
    Bitboard generate_Black_pieces() const;
};

#endif
