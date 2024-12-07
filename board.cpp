#include "board.h"
#include <iostream>

// Constructor: Initializes the board
Board::Board() {
    initialize();
}


void Board::initialize() {
    // Initialize white pawns
    whitePawnA = set_bit(0ULL, A2);    whitePawnB = set_bit(0ULL, B2);   whitePawnC = set_bit(0ULL, C2);
    whitePawnD = set_bit(0ULL, D2);    whitePawnE = set_bit(0ULL, E2);   whitePawnF = set_bit(0ULL, F2);
    whitePawnG = set_bit(0ULL, G2);    whitePawnH = set_bit(0ULL, H2);
    whitePawns = whitePawnA | whitePawnB | whitePawnC | whitePawnD | 
                 whitePawnE | whitePawnF | whitePawnG | whitePawnH;

    // Initialize black pawns
    blackPawnA = set_bit(0ULL, A7);    blackPawnB = set_bit(0ULL, B7);   blackPawnC = set_bit(0ULL, C7);
    blackPawnD = set_bit(0ULL, D7);    blackPawnE = set_bit(0ULL, E7);   blackPawnF = set_bit(0ULL, F7);
    blackPawnG = set_bit(0ULL, G7);    blackPawnH = set_bit(0ULL, H7);
    blackPawns = blackPawnA | blackPawnB | blackPawnC | blackPawnD | 
                 blackPawnE | blackPawnF | blackPawnG | blackPawnH;

    // Initialize knights
    whiteKnight1 = set_bit(0ULL, B1);  whiteKnight2 = set_bit(0ULL, G1);
    whiteKnights = whiteKnight1 | whiteKnight2;
    blackKnight1 = set_bit(0ULL, B8);  blackKnight2 = set_bit(0ULL, G8);
    blackKnights = blackKnight1 | blackKnight2;

    // Initialize bishops
    whiteBishop1 = set_bit(0ULL, C1);  whiteBishop2 = set_bit(0ULL, F1);
    whiteBishops = whiteBishop1 | whiteBishop2;
    blackBishop1 = set_bit(0ULL, C8);  blackBishop2 = set_bit(0ULL, F8);
    blackBishops = blackBishop1 | blackBishop2;

    // Initialize rooks
    whiteRook1 = set_bit(0ULL, A1);    whiteRook2 = set_bit(0ULL, H1);
    whiteRooks = whiteRook1 | whiteRook2;
    blackRook1 = set_bit(0ULL, A8);    blackRook2 = set_bit(0ULL, H8);
    blackRooks = blackRook1 | blackRook2;

    // Initialize queens
    whiteQueen = set_bit(0ULL, D1);    blackQueen = set_bit(0ULL, D8);

    // Initialize kings
    whiteKing = set_bit(0ULL, E1);     blackKing = set_bit(0ULL, E8);





    // Combine all white pieces
    whitePieces = whitePawnA | whitePawnB | whitePawnC | whitePawnD |   whitePawnE | whitePawnF | whitePawnG | whitePawnH |
                  whiteKnight1 | whiteKnight2 |   whiteBishop1 | whiteBishop2 | whiteRook1 | whiteRook2 | 
                  whiteQueen | whiteKing;

    // Combine all black pieces
    blackPieces = blackPawnA | blackPawnB | blackPawnC | blackPawnD |  blackPawnE | blackPawnF | blackPawnG | blackPawnH |
                  blackKnight1 | blackKnight2 |  blackBishop1 | blackBishop2 | blackRook1 | blackRook2 |
                  blackQueen | blackKing;

    allPieces = whitePieces | blackPieces;
    emptySquares = ~allPieces & 0xFFFFFFFFFFFFFFFFULL;
}


void Board::make_move(Bitboard from, Bitboard to, Bitboard &piece) {
  // First, validate the move
    if (!is_valid_move(from, to, piece)) {
        std::cerr << "Error: Invalid move from " << from << " to " << to << std::endl;
        return;
    }

    // Check if the move is a capture
    if ((1ULL << to) & (blackPieces | whitePieces)) {
        capture_piece(to);  // Capture the piece
    }

    // Move the piece to the new location
    piece = clear_bit(piece, from);  // Clear the piece from the 'from' position
    piece = set_bit(piece, to);      // Set the piece at the 'to' position


    // Update all pieces and empty squares
    update_all();
}


void Board::capture_piece(Bitboard to) {
    // Store pointers to the bitboards for white and black pieces
    Bitboard* whitePiecesArr[16] = { &whitePawnA, &whitePawnB, &whitePawnC, &whitePawnD,
                                     &whitePawnE, &whitePawnF, &whitePawnG, &whitePawnH,
                                     &whiteKnight1, &whiteKnight2, &whiteBishop1, &whiteBishop2,
                                     &whiteRook1, &whiteRook2, &whiteQueen, &whiteKing };

    Bitboard* blackPiecesArr[16] = { &blackPawnA, &blackPawnB, &blackPawnC, &blackPawnD,
                                     &blackPawnE, &blackPawnF, &blackPawnG, &blackPawnH,
                                     &blackKnight1, &blackKnight2, &blackBishop1, &blackBishop2,
                                     &blackRook1, &blackRook2, &blackQueen, &blackKing };

    // Check if the captured piece is white
    if ((1ULL << to) & whitePieces) {
        // Iterate over the white pieces and clear the captured piece
        for (int i = 0; i < 16; ++i) {
            if ((1ULL << to) & *whitePiecesArr[i]) {
                *whitePiecesArr[i] = clear_bit(*whitePiecesArr[i], to); // Clear the captured piece
                break;
            }
        }
    }
    // Check if the captured piece is black
    else if ((1ULL << to) & blackPieces) {
        // Iterate over the black pieces and clear the captured piece
        for (int i = 0; i < 16; ++i) {
            if ((1ULL << to) & *blackPiecesArr[i]) {
                *blackPiecesArr[i] = clear_bit(*blackPiecesArr[i], to); // Clear the captured piece
                break;
            }
        }
    }

    // Update the allPieces and emptySquares bitboards after a capture
    update_all();
}



void Board::update_all() {
    // Initialize white and black pieces as 0
    whitePieces = 0ULL;
    blackPieces = 0ULL;

    // Create arrays of pointers to each piece (white and black)
    Bitboard* whitePiecesArr[16] = { &whitePawnA, &whitePawnB, &whitePawnC, &whitePawnD, 
                                     &whitePawnE, &whitePawnF, &whitePawnG, &whitePawnH, 
                                     &whiteKnight1, &whiteKnight2, &whiteBishop1, &whiteBishop2, 
                                     &whiteRook1, &whiteRook2, &whiteQueen, &whiteKing };

    Bitboard* blackPiecesArr[16] = { &blackPawnA, &blackPawnB, &blackPawnC, &blackPawnD, 
                                     &blackPawnE, &blackPawnF, &blackPawnG, &blackPawnH, 
                                     &blackKnight1, &blackKnight2, &blackBishop1, &blackBishop2, 
                                     &blackRook1, &blackRook2, &blackQueen, &blackKing };

    // Add each white piece to whitePieces if it's still in the game (not 0ULL)
    for (int i = 0; i < 16; ++i) {
        if (*whitePiecesArr[i] != 0ULL) {
            whitePieces |= *whitePiecesArr[i];
        }
    }

    // Add each black piece to blackPieces if it's still in the game (not 0ULL)
    for (int i = 0; i < 16; ++i) {
        if (*blackPiecesArr[i] != 0ULL) {
            blackPieces |= *blackPiecesArr[i];
        }
    }

    // Update allPieces and emptySquares
    allPieces = whitePieces | blackPieces;
    emptySquares = ~allPieces & 0xFFFFFFFFFFFFFFFFULL; 
}



bool Board::is_valid_move(Bitboard from, Bitboard to, Bitboard &piece) const {
   
    if (to < 0 || to >= 64) {
        std::cout << "Error: Invalid move. 'To' position is not a valid destination." << std::endl;
        return false;
    }

    // Ensure 'from' is part of the piece's current position
    if ((piece & (1ULL << from)) == 0) {
        std::cerr << "Error: 'From' position does not match the piece's current position." << std::endl;
        print_bitboard(1ULL << from);
        std::cerr << "Piece positions:" << std::endl;
        print_bitboard(piece);
        return false;
    }

    // Generate possible moves for the given piece
    Bitboard possibleMoves = generate_moves_for_piece(from, piece);

    // Check if 'to' position is within the possible moves
    if ((possibleMoves & (1ULL << to)) == 0) {
        std::cerr << "Error: Invalid move. 'To' position is not a valid destination." << std::endl;
        std::cout << "Possible moves:" << std::endl;
        print_bitboard(possibleMoves);
        std::cout << "Attempted move to: " << to << std::endl;
        print_bitboard(1ULL << to);
        return false;
    }

    // Move is valid
    return true;
}


Bitboard Board::generate_moves_for_piece(Bitboard position, Bitboard &piece) const {
    Bitboard moves = 0ULL;

    // Ensure that a captured piece does not generate any moves
    if (piece == 0ULL) {
        return moves;  // No moves for a captured piece
    }

    // Generate moves based on the type of the piece
    if (piece & (whitePawnA | whitePawnB | whitePawnC | whitePawnD |
                 whitePawnE | whitePawnF | whitePawnG | whitePawnH)) {
        // White pawn moves and attacks
        moves = generate_white_pawn_moves(piece, emptySquares) |
                generate_pawn_attacks(piece, true, blackPieces);
    } else if (piece & (blackPawnA | blackPawnB | blackPawnC | blackPawnD |
                        blackPawnE | blackPawnF | blackPawnG | blackPawnH)) {
        // Black pawn moves and attacks
        moves = generate_black_pawn_moves(piece, emptySquares) |
                generate_pawn_attacks(piece, false, whitePieces);
    } else if (piece & (whiteKnight1 | whiteKnight2)) {
        // Knight moves and attacks
        moves = generate_knight_moves(piece, allPieces) |
                generate_knight_attacks(piece, blackPieces);
    } else if (piece & (blackKnight1 | blackKnight2)) {
        // Knight moves and attacks
        moves = generate_knight_moves(piece, allPieces) |
                generate_knight_attacks(piece, whitePieces);
    } else if (piece & (whiteBishop1 | whiteBishop2)) {
        // Bishop sliding moves and attacks
        moves = generate_bishop_moves(piece, allPieces) |
                generate_bishop_attacks(piece, whitePieces, blackPieces);
    } else if (piece & (blackBishop1 | blackBishop2)) {
        // Bishop sliding moves and attacks
        moves = generate_bishop_moves(piece, allPieces) |
                generate_bishop_attacks(piece, blackPieces, whitePieces);
    } else if (piece & (whiteRook1 | whiteRook2)) {
        // Rook sliding moves and attacks
        moves = generate_rook_moves(piece, allPieces) |
                generate_rook_attacks(piece, whitePieces, blackPieces);
    } else if (piece & (blackRook1 | blackRook2)) {
        // Rook sliding moves and attacks
        moves = generate_rook_moves(piece, allPieces) |
                generate_rook_attacks(piece, blackPieces, whitePieces);
    } else if (piece & (whiteQueen)) {
        // Queen sliding moves and attacks
        moves = generate_queen_moves(piece, allPieces) |
                generate_queen_attacks(piece, whitePieces, blackPieces);
    } else if (piece & (blackQueen)) {
        // Queen sliding moves and attacks
        moves = generate_queen_moves(piece, allPieces) |
                generate_queen_attacks(piece, blackPieces, whitePieces);
    } else if (piece & (whiteKing)) {
        // King moves and attacks
        moves = generate_king_moves(piece, allPieces) |
                generate_king_attacks(piece, blackPieces);
    } else if (piece & (blackKing)) {
        // King moves and attacks
        moves = generate_king_moves(piece, allPieces) |
                generate_king_attacks(piece, whitePieces);
    }

    return moves;
}


void Board::remove_piece(Bitboard square, Bitboard &piece) {
    piece = clear_bit(piece, square);
    update_all();
}


bool Board::is_square_empty(Bitboard square) const {
    return (emptySquares & (1ULL << square)) != 0;
}


void Board::print_piece(Bitboard piece) const {
    print_bitboard(piece);
}

// Generate all white pieces attack moves
Bitboard Board::generate_White_pieces() const {
    return (
        generate_king_attacks(whiteKing, blackPieces) |        // King attacks
        generate_queen_attacks(whiteQueen, whitePieces, blackPieces) |    // Queen attacks
        generate_bishop_attacks(whiteBishop1, whitePieces, blackPieces) |   // Bishop 1 attacks
        generate_bishop_attacks(whiteBishop2, whitePieces, blackPieces) |   // Bishop 2 attacks
        generate_knight_attacks(whiteKnight1, blackPieces) |   // Knight 1 attacks
        generate_knight_attacks(whiteKnight2, blackPieces) |   // Knight 2 attacks
        generate_rook_attacks(whiteRook1, whitePieces, blackPieces) |       // Rook 1 attacks
        generate_rook_attacks(whiteRook2, whitePieces, blackPieces) |       // Rook 2 attacks
        generate_pawn_attacks(whitePawnA, true, blackPieces) | // White Pawn A attacks
        generate_pawn_attacks(whitePawnB, true, blackPieces) | // White Pawn B attacks
        generate_pawn_attacks(whitePawnC, true, blackPieces) | // White Pawn C attacks
        generate_pawn_attacks(whitePawnD, true, blackPieces) | // White Pawn D attacks
        generate_pawn_attacks(whitePawnE, true, blackPieces) | // White Pawn E attacks
        generate_pawn_attacks(whitePawnF, true, blackPieces) | // White Pawn F attacks
        generate_pawn_attacks(whitePawnG, true, blackPieces) | // White Pawn G attacks
        generate_pawn_attacks(whitePawnH, true, blackPieces)   // White Pawn H attacks
    );
}

// Generate all black pieces' attack moves
Bitboard Board::generate_Black_pieces() const {
    return (
        generate_king_attacks(blackKing, whitePieces) |        // King attacks
        generate_queen_attacks(blackQueen, blackPieces, whitePieces) |    // Queen attacks
        generate_bishop_attacks(blackBishop1, blackPieces, whitePieces) |   // Bishop 1 attacks
        generate_bishop_attacks(blackBishop2, blackPieces, whitePieces) |   // Bishop 2 attacks
        generate_knight_attacks(blackKnight1, whitePieces) |   // Knight 1 attacks
        generate_knight_attacks(blackKnight2, whitePieces) |   // Knight 2 attacks
        generate_rook_attacks(blackRook1, blackPieces, whitePieces) |       // Rook 1 attacks
        generate_rook_attacks(blackRook2, blackPieces, whitePieces) |       // Rook 2 attacks
        generate_pawn_attacks(blackPawnA, false, whitePieces) | // Black Pawn A attacks
        generate_pawn_attacks(blackPawnB, false, whitePieces) | // Black Pawn B attacks
        generate_pawn_attacks(blackPawnC, false, whitePieces) | // Black Pawn C attacks
        generate_pawn_attacks(blackPawnD, false, whitePieces) | // Black Pawn D attacks
        generate_pawn_attacks(blackPawnE, false, whitePieces) | // Black Pawn E attacks
        generate_pawn_attacks(blackPawnF, false, whitePieces) | // Black Pawn F attacks
        generate_pawn_attacks(blackPawnG, false, whitePieces) | // Black Pawn G attacks
        generate_pawn_attacks(blackPawnH, false, whitePieces)   // Black Pawn H attacks
    );
}

void Board::copy(const Board &other) {
    // Copy individual bitboards
    whitePawnA = other.whitePawnA; whitePawnB = other.whitePawnB; whitePawnC = other.whitePawnC;
    whitePawnD = other.whitePawnD; whitePawnE = other.whitePawnE; whitePawnF = other.whitePawnF;
    whitePawnG = other.whitePawnG; whitePawnH = other.whitePawnH;

    blackPawnA = other.blackPawnA; blackPawnB = other.blackPawnB; blackPawnC = other.blackPawnC;
    blackPawnD = other.blackPawnD; blackPawnE = other.blackPawnE; blackPawnF = other.blackPawnF;
    blackPawnG = other.blackPawnG; blackPawnH = other.blackPawnH;

    whiteKnight1 = other.whiteKnight1; whiteKnight2 = other.whiteKnight2;
    blackKnight1 = other.blackKnight1; blackKnight2 = other.blackKnight2;

    whiteBishop1 = other.whiteBishop1; whiteBishop2 = other.whiteBishop2;
    blackBishop1 = other.blackBishop1; blackBishop2 = other.blackBishop2;

    whiteRook1 = other.whiteRook1; whiteRook2 = other.whiteRook2;
    blackRook1 = other.blackRook1; blackRook2 = other.blackRook2;

    whiteQueen = other.whiteQueen; blackQueen = other.blackQueen;
    whiteKing = other.whiteKing; blackKing = other.blackKing;

    // Copy combined bitboards
    whitePieces = other.whitePieces;
    blackPieces = other.blackPieces;
    allPieces = other.allPieces;
    emptySquares = other.emptySquares;
}

const char* pieceSymbols[] = {
    "♙", // White Pawn
    "♘", // White Knight
    "♗", // White Bishop
    "♖", // White Rook
    "♕", // White Queen
    "♔", // White King
    "♟", // Black Pawn
    "♞", // Black Knight
    "♝", // Black Bishop
    "♜", // Black Rook
    "♛", // Black Queen
    "♚"  // Black King
};

void Board::print_board() const {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int square = row * 8 + col;
            // Print the piece on the current square
            if (is_bit_set(whitePawns, square)) std::cout << "♙ "; // White Pawn
            else if (is_bit_set(blackPawns, square)) std::cout << "♟ "; // Black Pawn
            else if (is_bit_set(whiteRooks, square)) std::cout << "♖ "; // White Rook
            else if (is_bit_set(blackRooks, square)) std::cout << "♜ "; // Black Rook
            else if (is_bit_set(whiteKnights, square)) std::cout << "♘ "; // White Knight
            else if (is_bit_set(blackKnights, square)) std::cout << "♞ "; // Black Knight
            else if (is_bit_set(whiteBishops, square)) std::cout << "♗ "; // White Bishop
            else if (is_bit_set(blackBishops, square)) std::cout << "♝ "; // Black Bishop
            else if (is_bit_set(whiteQueen, square)) std::cout << "♕ "; // White Queen
            else if (is_bit_set(blackQueen, square)) std::cout << "♛ "; // Black Queen
            else if (is_bit_set(whiteKing, square)) std::cout << "♔ "; // White King
            else if (is_bit_set(blackKing, square)) std::cout << "♚ "; // Black King
            else std::cout << ". "; // Empty square
        }
        std::cout << "\n";
    }
}
