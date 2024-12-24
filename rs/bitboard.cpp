#include "bitboard.h"
#include <iostream>

using namespace std;

// Function to print the bitboard as an 8x8 chessboard
void print_bitboard(Bitboard bb) {
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            cout << (IS_BIT_SET(bb, square) ? "1 " : "0 ");
        }
        cout << endl;
    }
    cout << endl;
}

// Function to set a bit on the bitboard (represent a piece on the square)
Bitboard set_bit(Bitboard bb, int square) {
    return (bb | (1ULL << square));
}

// Function to clear a bit on the bitboard (remove a piece from the square)
Bitboard clear_bit(Bitboard bb, int square) {
    return (bb & ~(1ULL << square));
}

// Function to check if a bit is set on the bitboard (check if a piece exists on the square)
bool is_bit_set(Bitboard bb, int square) {
    return (bb & (1ULL << square)) != 0;
}
