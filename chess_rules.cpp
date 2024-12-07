// chess_rules.cpp
#include "chess_rules.h"
#include <iostream>

bool is_check(const Board& board, bool isWhite) {
    Bitboard king = isWhite ? board.whiteKing : board.blackKing;
    if(isWhite){
        return ((board.generate_Black_pieces() & king) == king);
    }
    else 
        return( (board.generate_White_pieces() & king )== king);


}

bool causes_check( Board& board, int from, int to, bool isWhite,Bitboard &piece) {
    // Simulate the move and check if the king is in check
    Board tempBoard; // Create a copy of the board
    tempBoard.copy(board); // Copy the present State
    board.make_move(from, to , piece);  
    bool isTrue = is_check(board, isWhite);  // Check if the move results in a check
    board.copy(tempBoard); // Copy the original state back
    return isTrue;
}

bool is_checkmate(const Board& board, bool isWhite) {
    // If the king is in check and there are no valid moves to escape, it's checkmate
    if (!is_check(board, isWhite)) {
        return false;
    }

    // Generate all possible moves for the player
    // For simplicity, you could iterate over all pieces and try to find a legal move
    // that would remove the check. If no such move exists, it is checkmate.
    // Implement logic to try all moves here.
    return false;
}

bool is_stalemate(const Board& board, bool isWhite) {
    // If the king is not in check and there are no valid moves left, it's stalemate
    if (is_check(board, isWhite)) {
        return false; // The player is in check, so it cannot be stalemate
    }

    // Check for no moves left
    // You need to check if there are no legal moves left for the player
    // Implement the logic to check for no moves here.
    return false;
}

// bool can_castle(const Board& board, bool isWhite, bool kingside) {
//     Bitboard king = isWhite ? board.whiteKing : board.blackKing;
//     Bitboard rook = isWhite ? (kingside ? board.whiteRook2 : board.whiteRook1)
//                             : (kingside ? board.blackRook2 : board.blackRook1);

//     // Make sure the king and rook are in their starting positions
//     if (kingside) {
//         if ((king != (isWhite ? board.whiteKing : board.blackKing)) ||
//             (rook != (isWhite ? board.whiteRook2 : board.blackRook2))) {
//             return false;
//         }
//     } else {
//         if ((king != (isWhite ? board.whiteKing : board.blackKing)) ||
//             (rook != (isWhite ? board.whiteRook1 : board.blackRook1))) {
//             return false;
//         }
//     }

//     // Ensure the path between the king and rook is empty
//     Bitboard path = (kingside ? 0x0E00000000000000ULL : 0x0000000000000007ULL);
//     if (board.emptySquares & path) {
//         return true;
//     }
//     return false;
// }


bool en_passant(Board& board, int from, int to, bool isWhite) {
    // Determine the row and file of the move
    int fromRank = from / 8;  // Row of the 'from' square
    int fromFile = from % 8;  // File of the 'from' square
    int toRank = to / 8;      // Row of the 'to' square
    int toFile = to % 8;      // File of the 'to' square

    // Access the relevant bitboards
    Bitboard whitePawns = board.whitePawns;
    Bitboard blackPawns = board.blackPawns;

    // Case 1: White pawn en passant (opponent's pawn on 5th rank, moved two squares)
    if (isWhite && fromRank == 4 && toRank == 5 && abs(fromFile - toFile) == 1) {
        // Check if the opponent's black pawn has just moved two squares forward
        int capturedPawnPosition = to - 8; // Captured pawn is one rank down (black)

        // Ensure the opponent's black pawn is there (en passant target)
        if (is_bit_set(blackPawns, capturedPawnPosition)) {
            // Perform the en passant capture
            blackPawns = clear_bit(blackPawns, capturedPawnPosition);
            whitePawns = set_bit(whitePawns, to);

            // Check if the move causes check on the white king
            if (causes_check(board, from, to, isWhite, whitePawns)) {
                // Revert the move if it causes check
                blackPawns = set_bit(blackPawns, capturedPawnPosition);
                whitePawns = clear_bit(whitePawns, to);
                return false;
            }

            return true;  // Successful en passant capture
        }
    }

    // Case 2: Black pawn en passant (opponent's pawn on 4th rank, moved two squares)
    if (!isWhite && fromRank == 3 && toRank == 2 && abs(fromFile - toFile) == 1) {
        // Check if the opponent's white pawn has just moved two squares forward
        int capturedPawnPosition = to + 8; // Captured pawn is one rank up (white)

        // Ensure the opponent's white pawn is there (en passant target)
        if (is_bit_set(whitePawns, capturedPawnPosition)) {
            // Perform the en passant capture
            whitePawns = clear_bit(whitePawns, capturedPawnPosition);
            blackPawns = set_bit(blackPawns, to);

            // Check if the move causes check on the black king
            if (causes_check(board, from, to, isWhite, blackPawns)) {
                // Revert the move if it causes check
                whitePawns = set_bit(whitePawns, capturedPawnPosition);
                blackPawns = clear_bit(blackPawns, to);
                return false;
            }

            return true;  // Successful en passant capture
        }
    }

    // If en passant conditions aren't met, return false
    return false;
}
