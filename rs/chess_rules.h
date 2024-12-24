// chess_rules.h
#ifndef CHESS_RULES_H
#define CHESS_RULES_H

#include "chess_rules.h"
#include "bitboard.h"
#include "move_generation.h"
#include "attack_map.h"
#include "board.h"

// Check if the king is in check
bool is_check(const Board& board, bool isWhite);

// Check if a move causes a check
bool causes_check( Board& board, int from, int to, bool isWhite,Bitboard &piece);

// Handle en passant capture
bool en_passant(Board& board, int from, int to, bool isWhite);

// Check if the player is in checkmate
bool is_checkmate(const Board& board, bool isWhite);

// Check if the player is in stalemate
bool is_stalemate(const Board& board, bool isWhite);

// Perform castling (check if valid)
bool can_castle(const Board& board, bool isWhite, bool kingside);



#endif
