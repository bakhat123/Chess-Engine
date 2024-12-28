#include "board.hpp"
#include "types.hpp"

template <Player turn>
bool is_in_threat(Position& pos, int sq);

template <Player turn>
constexpr bool is_in_check(Board& board){
  return is_in_threat<turn>(board.board, turn == White ? board.Kpos : board.kpos);
}

inline bool is_in_check(Board& board, Player turn) {
  if (turn == White)
    return is_in_threat<White>(board.board, board.Kpos);
  else
    return is_in_threat<Black>(board.board, board.kpos);
}

int divide(Board& board, int depth);

template <Player turn>
vector<Move> generate_pseudo_moves(Board& board);
vector<Move> generate_pseudo_moves(Board& board);
template <Player turn>
vector<Move> generate_legal_moves(Board& board);
vector<Move> generate_legal_moves(Board& board);

// not accurate
template <Player turn>
array<bool, 64> get_threats(Board& board) {
  array<bool, 64> threats{false};
  for (auto& move : generate_pseudo_moves<turn>(board)) threats[move.to] = true;
  return threats;
}

bool make_move_if_legal(Board& board, const string& move);
Move get_move_if_legal(Board& board, const string& move);

string to_san(Board& board, Move move);
