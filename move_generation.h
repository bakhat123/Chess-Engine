#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include "bitboard.h"

// Forward declarations for move generation functions
Bitboard generate_white_pawn_moves(Bitboard whitePawns, Bitboard emptySquares, Bitboard blackPieces);
Bitboard generate_knight_moves(Bitboard knight, Bitboard allPieces);
Bitboard generate_sliding_moves(Bitboard piece, Bitboard allPieces, int direction);
Bitboard generate_king_moves(Bitboard king, Bitboard allPieces);

#endif // MOVE_GENERATION_H
