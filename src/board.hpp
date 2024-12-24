#pragma once

#include "types.hpp"

// https://www.chessprogramming.org/Zobrist_Hashing

void zobrist_init();

class Move {
 public:
  int from = 0, to = 0;
  int enpassant_sq_idx = -1, fifty = 0, moves = 0;
  Piece promotion = Empty, captured = Empty;
  bool castling_rights[4] = {};
  bool enpassant = false, castling = false;
  Move(int _from = 0, int _to = 0, Piece _promotion = Empty,
       Piece _captured = Empty, bool _enpassant = false, bool _castling = false)
      : from(_from),
        to(_to),
        promotion(_promotion),
        captured(_captured),
        enpassant(_enpassant),
        castling(_castling) {}
  void print();
  string to_uci();
  inline bool equals(int _from, int _to) { return from == _from && to == _to; }
  inline bool equals(Move& move) {
    return equals(move.from, move.to) && promotion == move.promotion;
  }
};

class Board {
 public:
  Position board;
  int enpassant_sq_idx = -1, fifty = 0, moves = 1;
  int Kpos = -1, kpos = -1;
  bool castling_rights[4] = {};
  Player turn = White;
  // CheckType check = CheckNotChecked;
  uint64_t hash = 0;

  Board() { load_startpos(); }
  constexpr Piece operator[](int i) { return board[i]; }
  inline void change_turn() { turn = Player(-turn); }
  inline bool empty(int idx) { return board[idx] == Empty; }

  inline int piece_color(int sq_idx);
  void print(string sq = "", bool flipped = false);
  void make_move(Move& move);
  void unmake_move(Move& move);
  bool load_fen(string fen);
  string to_fen();
  void load_startpos();

  uint64_t zobrist_hash();
};

int sq2idx(char file, char rank);
string idx2sq(int idx);

inline bool friendly(Piece a, Piece b) { return a * b > 0; }
inline bool hostile(Piece a, Piece b) { return a * b < 0; }
inline bool in_board(int idx) { return idx >= 0 && idx < 64; }

inline bool isnt_H(int idx) { return idx % 8 != 7; }
inline bool isnt_A(int idx) { return idx % 8 != 0; }
inline bool isnt_8(int idx) { return idx / 8 != 0; }
inline bool isnt_1(int idx) { return idx / 8 != 7; }

Piece char2piece(char p);
char piece2char(Piece p);

inline int Board::piece_color(int sq_idx) {
  if (board[sq_idx] == Empty) return 0;
  return board[sq_idx] > 0 ? White : Black;
}

inline int sq_color(int sq_idx) {
  return (sq_idx % 2 && (sq_idx / 8) % 2) ||
         (sq_idx % 2 == 0 && (sq_idx / 8) % 2 == 0);
}

