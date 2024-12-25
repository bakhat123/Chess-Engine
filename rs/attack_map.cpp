#include "attack_map.h"
#include <vector>

// Generate pawn attacks (white or black)
Bitboard generate_pawn_attacks(Bitboard pawns, bool isWhite, Bitboard enemyPieces) {
    Bitboard attacks = 0ULL;

    if (isWhite) {
        attacks |= ((pawns << 7) & enemyPieces) & 0x7F7F7F7F7F7F7F7FULL; // Attacks left
        attacks |= ((pawns << 9) & enemyPieces) & 0xFEFEFEFEFEFEFEFEULL; // Attacks right
    } else {
        attacks |= ((pawns >> 9) & enemyPieces) & 0x7F7F7F7F7F7F7F7FULL; // Attacks left
        attacks |= ((pawns >> 7) & enemyPieces) & 0xFEFEFEFEFEFEFEFEULL; // Attacks right
    }

    return attacks;
}

// Generate knight attacks
Bitboard generate_knight_attacks(Bitboard knight, Bitboard enemyPieces) {
    Bitboard attack = 0ULL;

    // Knight moves in all 8 directions with corresponding edge masks
    attack |= (knight << 17) & 0xFEFEFEFEFEFEFEFEULL; // Up two, right one
    attack |= (knight << 15) & 0x7F7F7F7F7F7F7F7FULL; // Up two, left one
    attack |= (knight >> 17) & 0x7F7F7F7F7F7F7F7FULL; // Down two, left one
    attack |= (knight >> 15) & 0xFEFEFEFEFEFEFEFEULL; // Down two, right one
    attack |= (knight << 10) & 0xFCFCFCFCFCFCFCFCULL; // Right two, up one
    attack |= (knight << 6) & 0x3F3F3F3F3F3F3F3FULL;  // Left two, up one
    attack |= (knight >> 10) & 0x3F3F3F3F3F3F3F3FULL; // Left two, down one
    attack |= (knight >> 6) & 0xFCFCFCFCFCFCFCFCULL;  // Right two, down one

    return attack & enemyPieces;
}

// Generate king attacks
Bitboard generate_king_attacks(Bitboard king, Bitboard enemyPieces) {
    Bitboard attack = 0ULL;

    attack |= (king << 7) & 0x7F7F7F7F7F7F7F7FULL; // Up-left
    attack |= (king << 9) & 0xFEFEFEFEFEFEFEFEULL; // Up-right
    attack |= (king >> 7) & 0xFEFEFEFEFEFEFEFEULL; // Down-right
    attack |= (king >> 9) & 0x7F7F7F7F7F7F7F7FULL; // Down-left
    attack |= (king >> 1) & 0x7F7F7F7F7F7F7F7FULL; // Left
    attack |= (king << 1) & 0xFEFEFEFEFEFEFEFEULL; // Right
    attack |= (king << 8) & 0xFFFFFFFFFFFFFFFFULL; // Up
    attack |= (king >> 8) & 0xFFFFFFFFFFFFFFFFULL; // Down

    return attack & enemyPieces;
}
// Generate sliding piece attacks (rook, bishop, queen)
Bitboard generate_sliding_attacks(Bitboard piece, Bitboard friendlyPieces, Bitboard enemyPieces, bool isRook, bool isBishop) {
    Bitboard attacks = 0ULL;
    
    // Masks to prevent wraparound at file boundaries
    const Bitboard FILE_A_MASK = 0x7F7F7F7F7F7F7F7FULL; // Clears file A
    const Bitboard FILE_H_MASK = 0xFEFEFEFEFEFEFEFEULL; // Clears file H

    // Direction handling
    const int directions[] = { 1, -1, 8, -8, 9, 7, -9, -7 }; // Right, Left, Up, Down, Up-Right, Up-Left, Down-Left, Down-Right

    // Restrict directions based on piece type (rook, bishop, or queen)
    std::vector<int> valid_directions;
    if (isRook) {
        // Rooks move horizontally (left, right) and vertically (up, down)
        valid_directions = { 1, -1, 8, -8 };
    } else if (isBishop) {
        // Bishops move diagonally (up-right, up-left, down-left, down-right)
        valid_directions = { 9, 7, -9, -7 };
    } else if (!isRook && !isBishop) {
        // Queens move like both rooks and bishops
        valid_directions = { 1, -1, 8, -8, 9, 7, -9, -7 };
    }

    // Loop through valid directions
    for (int i = 0; i < valid_directions.size(); i++) {
        Bitboard ray = piece;
        while (true) {
            // Move the ray in the specified direction
            if (valid_directions[i] == 1) { // Right
                ray = (ray << 1) & FILE_H_MASK;
            } else if (valid_directions[i] == -1) { // Left
                ray = (ray >> 1) & FILE_A_MASK;
            } else if (valid_directions[i] == 8) { // Up
                ray <<= 8;
            } else if (valid_directions[i] == -8) { // Down
                ray >>= 8;
            } else if (valid_directions[i] == 9) { // Up-Right
                ray = (ray << 9) & FILE_H_MASK;
            } else if (valid_directions[i] == 7) { // Up-Left
                ray = (ray << 7) & FILE_A_MASK;
            } else if (valid_directions[i] == -9) { // Down-Left
                ray = (ray >> 9) & FILE_A_MASK;
            } else if (valid_directions[i] == -7) { // Down-Right
                ray = (ray >> 7) & FILE_H_MASK;
            }

            if (!ray) break; // If ray has moved off the board, stop

            // If we hit an enemy piece, register the attack
            if (ray & enemyPieces) {
                attacks |= ray; // Add the attack position
                break; // Stop after hitting an enemy piece
            }

            // If we hit a friendly piece, stop the attack (blocking piece)
            if (ray & friendlyPieces) {
                break;
            }

            attacks |= ray; // Continue adding attack positions
        }
    }

    return attacks; // Return the final attack positions
}

// Generate rook attacks
Bitboard generate_rook_attacks(Bitboard rook, Bitboard friendlyPieces, Bitboard enemyPieces) {
    Bitboard attacks = 0ULL;
    attacks |= generate_sliding_attacks(rook, friendlyPieces, enemyPieces, true, false);  
    return attacks & enemyPieces;
}

// Generate bishop attacks
Bitboard generate_bishop_attacks(Bitboard bishop, Bitboard friendlyPieces, Bitboard enemyPieces) {
    Bitboard attacks = 0ULL;
    attacks |= generate_sliding_attacks(bishop, friendlyPieces, enemyPieces, false, true);   
     return attacks & enemyPieces;
}

// Generate queen attacks
Bitboard generate_queen_attacks(Bitboard queen, Bitboard friendlyPieces, Bitboard enemyPieces) {
    Bitboard attacks = 0ULL;
    // Queen combines rook and bishop attacks 
    attacks |= generate_sliding_attacks(queen, friendlyPieces, enemyPieces, false, false);  
   return attacks & enemyPieces;
}
