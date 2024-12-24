#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "bitboard.h"

// Forward declarations for move generation functions
Bitboard generate_white_pawn_moves(Bitboard whitePawns, Bitboard emptySquares);
Bitboard generate_black_pawn_moves(Bitboard blackPawns, Bitboard emptySquares);
Bitboard generate_knight_moves(Bitboard knight, Bitboard allPieces);
Bitboard generate_sliding_moves(Bitboard piece, Bitboard allPieces, bool isRook, bool isBishop);
Bitboard generate_king_moves(Bitboard king, Bitboard allPieces);
Bitboard generate_rook_moves(Bitboard rook, Bitboard allPieces);
Bitboard generate_bishop_moves(Bitboard bishop, Bitboard allPieces);
Bitboard generate_queen_moves(Bitboard queen, Bitboard allPieces);

#endif // MOVE_GENERATION_H
