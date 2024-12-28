#include "movegen.hpp"

template <Direction dir>
constexpr bool is_safe(int idx) {
  const int rank = idx / 8, file = idx % 8;

  // // always true cases
  // if (idx >= 8 * 2 && idx <= 8 * 5 && file >= 2 && file <= 5) return true;

  // northwards
  if (dir == N || dir == NE || dir == NW || dir == ENE || dir == WNW)
    if (rank < 1) return false;
  // northwards (2 squares)
  if (dir == NN || dir == NNE || dir == NNW)
    if (rank < 2) return false;
  // southwards
  if (dir == S || dir == SE || dir == SW || dir == ESE || dir == WSW)
    if (rank > 6) return false;
  // southwards (2 squares)
  if (dir == SS || dir == SSE || dir == SSW)
    if (rank > 5) return false;
  // eastwards
  if (dir == E || dir == NE || dir == SE || dir == NNE || dir == SSE)
    if (file > 6) return false;
  // eastwards (2 squares)
  if (dir == ENE || dir == ESE)
    if (file > 5) return false;
  // westwards
  if (dir == W || dir == NW || dir == SW || dir == NNW || dir == SSW)
    if (file < 1) return false;
  // westwards (2 squares)
  if (dir == WNW || dir == WSW)
    if (file < 2) return false;

  return true;
}

template <Direction dir>
void only_capture(Position& pos, vector<Move>& movelist, int sq) {
  if (is_safe<dir>(sq) && hostile(pos[sq], pos[sq + dir]))
    movelist.emplace_back(sq, sq + dir);
}
template <Direction dir>
bool only_move(Position& pos, vector<Move>& movelist, int sq) {
  if (is_safe<dir>(sq) && pos[sq + dir] == Empty) {
    movelist.emplace_back(sq, sq + dir);
    return true;
  }
  return false;
}

template <Direction dir>
void slide(Position& pos, vector<Move>& movelist, int sq) {
  if (!is_safe<dir>(sq)) return;
  for (int dest = sq + dir; !friendly(pos[sq], pos[dest]); dest += dir) {
    movelist.emplace_back(sq, dest);
    if (pos[dest] != Empty) break;
    if (!is_safe<dir>(dest)) break;
  }
}

template <Direction dir, Piece piece>
bool is_occupied(Position& pos, int sq) {
  return is_safe<dir>(sq) && pos[sq + dir] == piece;
}

template <Direction dir, Piece p1, Piece p2, Piece King>
bool diagonal_threats(Position& pos, int sq) {
  if (!is_safe<dir>(sq)) return false;
  if (pos[sq + dir] == King) return true;
  for (int dest = sq + dir; !friendly(pos[sq], pos[dest]); dest += dir) {
    if (pos[dest] == p1 || pos[dest] == p2) return true;
    if (pos[dest] != Empty) break;  // different from slide
    if (!is_safe<dir>(dest)) break;
  }
  return false;
}

template <Direction dir>
int slide_find_end(Position& pos, int sq) {
  if (!is_safe<dir>(sq)) return -1;
  for (int dest = sq + dir;; dest += dir) {
    if (pos[dest] != Empty) return dest;
    if (!is_safe<dir>(dest)) return -1;
  }
  return -1;
}

template <Player turn>
bool is_sq_attacked_by_BRQ(Position& pos, int sq) {
  // opp_<piece> is an opponent piece
  constexpr Piece opp_B = Piece(-turn * wB);
  constexpr Piece opp_R = Piece(-turn * wR);
  constexpr Piece opp_Q = Piece(-turn * wQ);

#define check(p1, p2, dir)                                                   \
  {                                                                          \
    const int blocker = slide_find_end<dir>(pos, sq);                        \
    if (~blocker && (pos[blocker] == p1 || pos[blocker] == p2)) return true; \
  }
  // bishop and queen attacks
  check(opp_B, opp_Q, NW);
  check(opp_B, opp_Q, NE);
  check(opp_B, opp_Q, SW);
  check(opp_B, opp_Q, SE);
  // rook and queen attacks
  check(opp_R, opp_Q, N);
  check(opp_R, opp_Q, S);
  check(opp_R, opp_Q, E);
  check(opp_R, opp_Q, W);
#undef check

  return false;
}

template <Player turn>
bool is_sq_attacked_by_K(Position& pos, int sq) {
  // check for king attacks
#define check(dir) \
  if (is_occupied<dir, Piece(-turn* wK)>(pos, sq)) return true;
  check(NE) check(NW) check(SE) check(SW);
  check(N) check(S) check(E) check(W);
#undef check
  return false;
}

template <Player turn>
bool is_sq_attacked_by_N(Position& pos, int sq) {
  // check for knight attacks
#define check(dir) \
  if (is_occupied<dir, Piece(-turn* wN)>(pos, sq)) return true;
  check(NNW) check(NNE);
  check(WNW) check(WSW);
  check(ENE) check(ESE);
  check(SSW) check(SSE);
#undef check
  return false;
}

template <Player turn>
bool is_sq_attacked_by_P(Position& pos, int sq) {
// check for pawn attacks (relative to turn)
#define check(dir) \
  if (is_occupied<Direction(turn* dir), Piece(-turn* wP)>(pos, sq)) return true;
  check(NW) check(NE);
#undef check
  return false;
}

template <Player turn>
bool is_sq_attacked_by_KBRQ(Position& pos, int sq) {
  return is_sq_attacked_by_K<turn>(pos, sq) ||
         is_sq_attacked_by_BRQ<turn>(pos, sq);
}

template <Player turn>
bool is_sq_attacked_by_KBRQ2(Position& pos, int sq) {
  // TODO: deprecate if not used
  // opp_<piece> is an opponent piece
  constexpr Piece opp_B = Piece(-turn * wB);
  constexpr Piece opp_R = Piece(-turn * wR);
  constexpr Piece opp_Q = Piece(-turn * wQ);
  constexpr Piece opp_K = Piece(-turn * wK);

  // check for king, bishop, rook queen threats
  if (diagonal_threats<NW, opp_B, opp_Q, opp_K>(pos, sq)) return true;
  if (diagonal_threats<NE, opp_B, opp_Q, opp_K>(pos, sq)) return true;
  if (diagonal_threats<SW, opp_B, opp_Q, opp_K>(pos, sq)) return true;
  if (diagonal_threats<SE, opp_B, opp_Q, opp_K>(pos, sq)) return true;
  if (diagonal_threats<N, opp_R, opp_Q, opp_K>(pos, sq)) return true;
  if (diagonal_threats<S, opp_R, opp_Q, opp_K>(pos, sq)) return true;
  if (diagonal_threats<E, opp_R, opp_Q, opp_K>(pos, sq)) return true;
  if (diagonal_threats<W, opp_R, opp_Q, opp_K>(pos, sq)) return true;
  return false;
}

template <Player turn>
bool is_in_threat(Position& pos, int sq) {
  // generate and check reverse threats from sq
  return is_sq_attacked_by_P<turn>(pos, sq) ||
         is_sq_attacked_by_N<turn>(pos, sq) ||
         is_sq_attacked_by_KBRQ<turn>(pos, sq);
}

template <Direction dir>
void jump(Position& pos, vector<Move>& movelist, int sq) {
  if (is_safe<dir>(sq) && !friendly(pos[sq], pos[sq + dir]))
    movelist.emplace_back(sq, sq + dir);
}

template <Player turn>
void generate_king_moves(Board& board, vector<Move>& pseudo) {
  const int K_pos = turn == White ? board.Kpos : board.kpos;
  jump<NE>(board.board, pseudo, K_pos);
  jump<NW>(board.board, pseudo, K_pos);
  jump<SE>(board.board, pseudo, K_pos);
  jump<SW>(board.board, pseudo, K_pos);
  jump<N>(board.board, pseudo, K_pos);
  jump<S>(board.board, pseudo, K_pos);
  jump<E>(board.board, pseudo, K_pos);
  jump<W>(board.board, pseudo, K_pos);
}

template <Player turn>
void generate_promotions_and_ep(Board& board, vector<Move>& pseudo) {
  constexpr int prom_start_sq = turn == White ? 8 : 48;
  constexpr Direction rel_N = turn == White ? N : S;
  constexpr Direction rel_NW = turn == White ? NW : SW;
  constexpr Direction rel_NE = turn == White ? NE : SE;
  constexpr Direction rel_SW = turn == White ? SW : NW;
  constexpr Direction rel_SE = turn == White ? SE : NE;
  constexpr Piece rel_P = turn == White ? wP : bP;

  for (int sq = prom_start_sq; sq < prom_start_sq + 8; sq++) {
    if (board[sq] != rel_P) continue;
    if (board.empty(sq + rel_N))
      for (auto& piece : {wQ, wR, wB, wN})
        pseudo.emplace_back(sq, sq + rel_N, Piece(piece * turn));
    if (isnt_A(sq) && hostile(board[sq], board[sq + rel_NW]))
      for (auto& piece : {wQ, wR, wB, wN})
        pseudo.emplace_back(sq, sq + rel_NW, Piece(piece * turn));
    if (isnt_H(sq) && hostile(board[sq], board[sq + rel_NE]))
      for (auto& piece : {wQ, wR, wB, wN})
        pseudo.emplace_back(sq, sq + rel_NE, Piece(piece * turn));
  }

  const auto& ep_sq = board.enpassant_sq_idx;
  // only one enpassant square can be occupied at a time, so `else` is safe
  if (~ep_sq) {
    // capture enpassant NW
    if (is_occupied<rel_SE, rel_P>(board.board, ep_sq))
      pseudo.emplace_back(ep_sq + rel_SE, ep_sq, Empty, Empty, true);
    // capture enpassant NE
    if (is_occupied<rel_SW, rel_P>(board.board, ep_sq))
      pseudo.emplace_back(ep_sq + rel_SW, ep_sq, Empty, Empty, true);
  }
}

template <Player turn>
void generate_pawn_moves(Position& pos, vector<Move>& pseudo, int sq) {
  const int rank = sq / 8;
  const int rel_rank = turn == White ? rank : 7 - rank;

  // capture NW and NE
  only_capture<Direction(NW * turn)>(pos, pseudo, sq);
  only_capture<Direction(NE * turn)>(pos, pseudo, sq);

  // push
  if (only_move<Direction(N * turn)>(pos, pseudo, sq))
    // double push only if push succeeds
    if (rel_rank == 6) only_move<Direction(NN * turn)>(pos, pseudo, sq);
}

template <Player turn>
void generate_castling_moves(Board& board, vector<Move>& pseudo) {
  auto castling_rights = board.castling_rights;
#define empty board.empty

  if constexpr (turn == White) {
    // kingside
    if (castling_rights[0] && empty(61) && empty(62))
      pseudo.emplace_back(60, 60 + E + E, Empty, Empty, false, true);
    // queenside
    if (castling_rights[1] && empty(57) && empty(58) && empty(59))
      pseudo.emplace_back(60, 60 + W + W, Empty, Empty, false, true);
  } else {
    // kingside
    if (castling_rights[2] && empty(5) && empty(6))
      pseudo.emplace_back(4, 4 + E + E, Empty, Empty, false, true);
    // queenside
    if (castling_rights[3] && empty(1) && empty(2) && empty(3))
      pseudo.emplace_back(4, 4 + W + W, Empty, Empty, false, true);
  }

#undef empty
}

template <Player turn>
vector<Move> generate_pseudo_moves(Board& board) {
  // cout << "gen pseudo @" << zobrist_hash() << endl;
  vector<Move> pseudo;
  pseudo.reserve(40);  // average number of pseudo moves per position

  // we know where the king is
  generate_king_moves<turn>(board, pseudo);
  generate_castling_moves<turn>(board, pseudo);
  generate_promotions_and_ep<turn>(board, pseudo);

  constexpr Piece rel_P = Piece(wP * turn);
  constexpr Piece rel_N = Piece(wN * turn);
  constexpr Piece rel_B = Piece(wB * turn);
  constexpr Piece rel_R = Piece(wR * turn);
  constexpr Piece rel_Q = Piece(wQ * turn);

  auto& pos = board.board;

  for (int sq = 0; sq < 64; sq++) {
    const int rank = sq / 8;
    const int rel_rank = turn == White ? rank : 7 - rank;
    const Piece p = pos[sq];
    if (p == rel_P && rel_rank != 1)  // rank 2 is handled by promotions
      generate_pawn_moves<turn>(pos, pseudo, sq);
    else if (p == rel_N) {
      jump<NNW>(pos, pseudo, sq);
      jump<NNE>(pos, pseudo, sq);
      jump<WNW>(pos, pseudo, sq);
      jump<WSW>(pos, pseudo, sq);
      jump<ENE>(pos, pseudo, sq);
      jump<ESE>(pos, pseudo, sq);
      jump<SSW>(pos, pseudo, sq);
      jump<SSE>(pos, pseudo, sq);
    }
    if (p == rel_B || p == rel_Q) {
      slide<NW>(pos, pseudo, sq);
      slide<NE>(pos, pseudo, sq);
      slide<SW>(pos, pseudo, sq);
      slide<SE>(pos, pseudo, sq);
    }
    if (p == rel_R || p == rel_Q) {
      slide<N>(pos, pseudo, sq);
      slide<S>(pos, pseudo, sq);
      slide<E>(pos, pseudo, sq);
      slide<W>(pos, pseudo, sq);
    }
  }
  return pseudo;
}

vector<Move> generate_pseudo_moves(Board& board) {
  if (board.turn == White)
    return generate_pseudo_moves<White>(board);
  else
    return generate_pseudo_moves<Black>(board);
}

// check if a pseudo-legal move is legal
template <Player turn>
bool is_legal(Board& board, Move& move) {
  bool legal = true;
  if (move.castling) {
// check if any squares that king moves to are threatened

// check unsafe square
#define US(sq) is_in_threat<turn>(board.board, sq)
    if (  // white kingside castling
        (move.equals(60, 62) && (US(60) || US(61) || US(62))) ||
        // black kingside castling
        (move.equals(4, 6) && (US(4) || US(5) || US(6))) ||
        // white queenside castling
        (move.equals(60, 58) && (US(60) || US(59) || US(58))) ||
        // black queenside castling
        (move.equals(4, 2) && (US(4) || US(3) || US(2)))) {
      legal = false;
    }
#undef US

  } else {
    // check if king is threatened
    board.make_move(move);
    legal = !is_in_check<turn>(board);
    board.unmake_move(move);
  }
  return legal;
}

template <Direction dir, Piece p1, Piece p2, Piece King>
void slide_capture_threat(Position& pos, vector<Move>& movelist, int sq) {
  if (!is_safe<dir>(sq)) return;
  for (int dest = sq + dir; !friendly(pos[sq], pos[dest]); dest += dir) {
    if (pos[dest] == p1 || pos[dest] == p2) {
      slide<dir>(pos, movelist, sq);
      break;
    }
    if (pos[dest] != Empty) break;
    if (!is_safe<dir>(dest)) break;
  }
}

template <Player turn>
Direction get_absolute_pin_attacker_dir(Board& board, int sq) {
  constexpr Piece rel_K = Piece(turn * wK);
  constexpr Piece opp_B = Piece(-turn * wB);
  constexpr Piece opp_R = Piece(-turn * wR);
  constexpr Piece opp_Q = Piece(-turn * wQ);

  auto& pos = board.board;

  // vertical
  const int north = slide_find_end<N>(pos, sq);
  const int south = slide_find_end<S>(pos, sq);
  if (~north && ~south) {
    if (pos[north] == rel_K && (pos[south] == opp_R || pos[south] == opp_Q))
      return S;
    if (pos[south] == rel_K && (pos[north] == opp_R || pos[north] == opp_Q))
      return N;
  }
  // horizontal
  const int east = slide_find_end<E>(pos, sq);
  const int west = slide_find_end<W>(pos, sq);
  if (~east && ~west) {
    if (pos[east] == rel_K && (pos[west] == opp_R || pos[west] == opp_Q))
      return W;
    if (pos[west] == rel_K && (pos[east] == opp_R || pos[east] == opp_Q))
      return E;
  }
  // diagonal
  const int northwest = slide_find_end<NW>(pos, sq);
  const int southeast = slide_find_end<SE>(pos, sq);
  if (~northwest && ~southeast) {
    if (pos[northwest] == rel_K &&
        (pos[southeast] == opp_B || pos[southeast] == opp_Q))
      return SE;
    if (pos[southeast] == rel_K &&
        (pos[northwest] == opp_B || pos[northwest] == opp_Q))
      return NW;
  }
  const int northeast = slide_find_end<NE>(pos, sq);
  const int southwest = slide_find_end<SW>(pos, sq);
  if (~northeast && ~southwest) {
    if (pos[northeast] == rel_K &&
        (pos[southwest] == opp_B || pos[southwest] == opp_Q))
      return SW;
    if (pos[southwest] == rel_K &&
        (pos[northeast] == opp_B || pos[northeast] == opp_Q))
      return NE;
  }

  return EmptyDirection;
}

template <Player turn>
bool is_check(Board& board, Move& move) {
  // the move has already been made
  // we are not the player who made the move
  auto& pos = board.board;
  const Piece p = pos[move.to];
  int sq = move.to;

  const int K_pos = turn == White ? board.Kpos : board.kpos;
  constexpr Piece opp_P = Piece(-turn * wP);
  constexpr Piece opp_N = Piece(-turn * wN);
  constexpr Piece opp_B = Piece(-turn * wB);
  constexpr Piece opp_R = Piece(-turn * wR);
  constexpr Piece opp_Q = Piece(-turn * wQ);

  // direct check
  if (p == opp_P) {
    // check for pawn attacks (relative to turn)
    const Direction opp_NW = turn == Black ? NW : SW;
    const Direction opp_NE = turn == Black ? NE : SE;
#define check_pawn(dir) \
  if (is_safe<dir>(sq) && sq + dir == K_pos) return true;
    check_pawn(opp_NW) check_pawn(opp_NE);
#undef check_pawn
  }

  if (p == opp_N) {
    // check for knight attacks
#define check_knight(dir) \
  if (is_safe<dir>(sq) && sq + dir == K_pos) return true;
    check_knight(NNW) check_knight(NNE);
    check_knight(WNW) check_knight(WSW);
    check_knight(ENE) check_knight(ESE);
    check_knight(SSW) check_knight(SSE);
#undef check_knight
  }

#define check_slider(p1, p2, dir) \
  if (slide_find_end<dir>(pos, sq) == K_pos) return true;
  if (p == opp_B || p == opp_Q) {
    // check for bishop and queen attacks
    check_slider(opp_B, opp_Q, NW);
    check_slider(opp_B, opp_Q, NE);
    check_slider(opp_B, opp_Q, SW);
    check_slider(opp_B, opp_Q, SE);
  }
  if (p == opp_R || p == opp_Q) {
    // check for rook and queen attacks
    check_slider(opp_R, opp_Q, N);
    check_slider(opp_R, opp_Q, S);
    check_slider(opp_R, opp_Q, E);
    check_slider(opp_R, opp_Q, W);
  }
#undef check_slider

  // discovered check
  if (is_sq_attacked_by_BRQ<turn>(pos, K_pos)) return true;

  return false;
}

template <Player turn>
void generate_king_moves_safe_xray(Board& board, vector<Move>& movelist) {
  // generate king moves, except castling
  const int K_pos = turn == White ? board.Kpos : board.kpos;
  const Piece rel_K = Piece(turn * wK);
  // remove king
  board.board[K_pos] = Empty;
#define jump(dir)                                                          \
  if (is_safe<dir>(K_pos) && !friendly(rel_K, board.board[K_pos + dir]) && \
      !is_in_threat<turn>(board.board, K_pos + dir))                       \
    movelist.emplace_back(K_pos, K_pos + dir);
  jump(NE) jump(NW) jump(SE) jump(SW) jump(N) jump(S) jump(E) jump(W)
#undef jump
      // add king back
      board.board[K_pos] = rel_K;
}

template <Player turn>
void generate_promotion_moves_safe(Position& pos, vector<Move>& movelist,
                                   int sq) {
  const Direction rel_North = turn == White ? N : S;
  const Direction rel_NW = turn == White ? NW : SW;
  const Direction rel_NE = turn == White ? NE : SE;

#define promote(dir)                   \
  for (auto& piece : {wQ, wR, wB, wN}) \
    movelist.emplace_back(sq, sq + dir, Piece(piece * turn));
  if (pos[sq + rel_North] == Empty) promote(rel_North);
  if (isnt_A(sq) && hostile(pos[sq], pos[sq + rel_NW])) promote(rel_NW);
  if (isnt_H(sq) && hostile(pos[sq], pos[sq + rel_NE])) promote(rel_NE);
#undef promote
}

template <Player turn>
void generate_en_passant_moves_safe(Board& board, vector<Move>& movelist) {
  constexpr Piece opp_P = Piece(-turn * wP);
  constexpr Piece rel_P = Piece(wP * turn);
  constexpr Direction rel_SW = turn == White ? SW : NW;
  constexpr Direction rel_SE = turn == White ? SE : NE;
  constexpr Direction rel_S = turn == White ? S : N;

  // en-passant
  const auto& ep_sq = board.enpassant_sq_idx;
  // only one enpassant square can be occupied at a time, so `else` is
  // safe
  if (~ep_sq) {
    // remove enpassant pawn
    board.board[ep_sq + rel_S] = Empty;

    // capture enpassant NW
    if (is_occupied<rel_SE, rel_P>(board.board, ep_sq))
      if (get_absolute_pin_attacker_dir<turn>(board, ep_sq + rel_SE) ==
          EmptyDirection)
        movelist.emplace_back(ep_sq + rel_SE, ep_sq, Empty, Empty, true);
    // capture enpassant NE
    if (is_occupied<rel_SW, rel_P>(board.board, ep_sq))
      if (get_absolute_pin_attacker_dir<turn>(board, ep_sq + rel_SW) ==
          EmptyDirection)
        movelist.emplace_back(ep_sq + rel_SW, ep_sq, Empty, Empty, true);
    // add enpassant pawn back
    board.board[ep_sq + rel_S] = opp_P;
  }
}

template <Player turn>
void generate_castling_moves_safe(Board& board, vector<Move>& movelist) {
  auto castling_rights = board.castling_rights;
  // check if any squares that king moves to are threatened
// check unsafe square
#define US(sq) is_in_threat<turn>(board.board, sq)
#define empty board.empty

  if constexpr (turn == White) {
    // kingside
    if (castling_rights[0] && empty(61) && empty(62) &&
        !(US(60) || US(61) || US(62)))
      movelist.emplace_back(60, 60 + E + E, Empty, Empty, false, true);
    // queenside
    if (castling_rights[1] && empty(57) && empty(58) && empty(59) &&
        !(US(60) || US(59) || US(58)))
      movelist.emplace_back(60, 60 + W + W, Empty, Empty, false, true);
  } else {
    // kingside
    if (castling_rights[2] && empty(5) && empty(6) &&
        !(US(4) || US(5) || US(6)))
      movelist.emplace_back(4, 4 + E + E, Empty, Empty, false, true);
    // queenside
    if (castling_rights[3] && empty(1) && empty(2) && empty(3) &&
        !(US(4) || US(3) || US(2)))
      movelist.emplace_back(4, 4 + W + W, Empty, Empty, false, true);
  }

#undef empty
#undef US
}

template <Player turn, MoveGenType type>
vector<Move> generate_legal_moves2(Board& board) {
  vector<Move> movelist;
  movelist.reserve(30);  // average number of legal moves per position

  constexpr Piece rel_P = Piece(wP * turn);
  constexpr Piece rel_N = Piece(wN * turn);
  constexpr Piece rel_B = Piece(wB * turn);
  constexpr Piece rel_R = Piece(wR * turn);
  constexpr Piece rel_Q = Piece(wQ * turn);

  constexpr Piece opp_P = Piece(-turn * wP);
  constexpr Piece opp_N = Piece(-turn * wN);
  constexpr Piece opp_B = Piece(-turn * wB);
  constexpr Piece opp_R = Piece(-turn * wR);
  constexpr Piece opp_Q = Piece(-turn * wQ);

  constexpr Direction rel_NW = turn == White ? NW : SW;
  constexpr Direction rel_NE = turn == White ? NE : SE;
  constexpr Direction rel_North = turn == White ? N : S;
  constexpr Direction rel_NN = turn == White ? NN : SS;

  auto& pos = board.board;

  const int K_pos = turn == White ? board.Kpos : board.kpos;

  int attackers_n = 0;
  int intermediate_sq[7] = {-1, -1, -1, -1, -1, -1, -1};
#define is_intermediate_sq(sq)                                 \
  ((sq) == intermediate_sq[0] || (sq) == intermediate_sq[1] || \
   (sq) == intermediate_sq[2] || (sq) == intermediate_sq[3] || \
   (sq) == intermediate_sq[4] || (sq) == intermediate_sq[5] || \
   (sq) == intermediate_sq[6])

#define check_slider(p1, p2, dir)                               \
  if (const int blocker = slide_find_end<dir>(pos, K_pos);      \
      ~blocker && (pos[blocker] == p1 || pos[blocker] == p2)) { \
    attackers_n++;                                              \
    int temp = K_pos + dir;                                     \
    for (int i = 0; i < 7; i++) {                               \
      intermediate_sq[i] = temp;                                \
      if (temp == blocker) break;                               \
      temp += dir;                                              \
    }                                                           \
  }
  // rook and queen attacks
  check_slider(opp_R, opp_Q, N);
  check_slider(opp_R, opp_Q, S);
  check_slider(opp_R, opp_Q, E);
  check_slider(opp_R, opp_Q, W);
  // bishop and queen attacks
  check_slider(opp_B, opp_Q, NW);
  check_slider(opp_B, opp_Q, NE);
  check_slider(opp_B, opp_Q, SW);
  check_slider(opp_B, opp_Q, SE);
#undef check_slider
  // knight attacks
  // check for knight attacks
#define check_knight(dir)                    \
  if (is_occupied<dir, opp_N>(pos, K_pos)) { \
    attackers_n++;                           \
    intermediate_sq[0] = K_pos + dir;        \
  }
  check_knight(NNW) check_knight(NNE);
  check_knight(WNW) check_knight(WSW);
  check_knight(ENE) check_knight(ESE);
  check_knight(SSW) check_knight(SSE);
#undef check_knight
  // pawn attacks
  // check for pawn attacks (relative to turn)
#define check_pawn(dir)                      \
  if (is_occupied<dir, opp_P>(pos, K_pos)) { \
    attackers_n++;                           \
    intermediate_sq[0] = K_pos + dir;        \
  }
  check_pawn(rel_NW) check_pawn(rel_NE);
#undef check_pawn

  generate_castling_moves_safe<turn>(board, movelist);
  generate_en_passant_moves_safe<turn>(board, movelist);
  generate_king_moves_safe_xray<turn>(board, movelist);

  for (int sq = 0; sq < 64; sq++) {
    const Piece p = pos[sq];
    if (p == Empty) continue;

    const int attacker_dir = get_absolute_pin_attacker_dir<turn>(board, sq);
    const int rank = sq / 8;
    const int rel_rank = turn == White ? rank : 7 - rank;

    if (type == Evasions) {
      if (attacker_dir == EmptyDirection) {  // no pin
        if (attackers_n == 1) {              // single check
          // capture checking piece with unpinned piece
          // interpose unpinned piece in between king and attacker
          if (p == rel_P) {
            if (rel_rank != 1) {  // non-promotion moves
              // capture NW and NE
              if (is_intermediate_sq(sq + rel_NW))
                only_capture<rel_NW>(pos, movelist, sq);
              if (is_intermediate_sq(sq + rel_NE))
                only_capture<rel_NE>(pos, movelist, sq);

              // push
              if (is_intermediate_sq(sq + rel_North))
                only_move<rel_North>(pos, movelist, sq);
              // double push
              if (rel_rank == 6)
                if (is_safe<rel_North>(sq) && pos[sq + rel_North] == Empty)
                  if (is_intermediate_sq(sq + rel_NN))
                    only_move<rel_NN>(pos, movelist, sq);

            } else {  // promotion moves
              if (is_intermediate_sq(sq + rel_NW))
                if (isnt_A(sq) && hostile(pos[sq], pos[sq + rel_NW]))
                  for (auto& piece : {wQ, wR, wB, wN})
                    movelist.emplace_back(sq, sq + rel_NW, Piece(piece * turn));
              if (is_intermediate_sq(sq + rel_NE))
                if (isnt_H(sq) && hostile(pos[sq], pos[sq + rel_NE]))
                  for (auto& piece : {wQ, wR, wB, wN})
                    movelist.emplace_back(sq, sq + rel_NE, Piece(piece * turn));
            }
          }
          if (p == rel_N) {
            if (is_intermediate_sq(sq + NNW)) jump<NNW>(pos, movelist, sq);
            if (is_intermediate_sq(sq + NNE)) jump<NNE>(pos, movelist, sq);
            if (is_intermediate_sq(sq + WNW)) jump<WNW>(pos, movelist, sq);
            if (is_intermediate_sq(sq + WSW)) jump<WSW>(pos, movelist, sq);
            if (is_intermediate_sq(sq + ENE)) jump<ENE>(pos, movelist, sq);
            if (is_intermediate_sq(sq + ESE)) jump<ESE>(pos, movelist, sq);
            if (is_intermediate_sq(sq + SSW)) jump<SSW>(pos, movelist, sq);
            if (is_intermediate_sq(sq + SSE)) jump<SSE>(pos, movelist, sq);
          }
#define slide2(dir)                                                         \
  if (is_safe<dir>(sq)) {                                                   \
    for (int dest = sq + dir; !friendly(pos[sq], pos[dest]); dest += dir) { \
      if (is_intermediate_sq(dest)) {                                       \
        movelist.emplace_back(sq, dest);                                    \
      }                                                                     \
      if (pos[dest] != Empty) break;                                        \
      if (!is_safe<dir>(dest)) break;                                       \
    }                                                                       \
  }
          if (p == rel_B || p == rel_Q) {
            slide2(NW);
            slide2(NE);
            slide2(SW);
            slide2(SE);
          }
          if (p == rel_R || p == rel_Q) {
            slide2(N);
            slide2(S);
            slide2(E);
            slide2(W);
          }
        }
      }
    } else {
      if (attacker_dir == EmptyDirection) {  // no pin
        if (p == rel_P) {
          if (rel_rank != 1) {  // non-promotion moves
            generate_pawn_moves<turn>(pos, movelist, sq);
          } else {  // promotion moves
            generate_promotion_moves_safe<turn>(pos, movelist, sq);
          }
        }
        if (p == rel_N) {
          jump<NNW>(pos, movelist, sq);
          jump<NNE>(pos, movelist, sq);
          jump<WNW>(pos, movelist, sq);
          jump<WSW>(pos, movelist, sq);
          jump<ENE>(pos, movelist, sq);
          jump<ESE>(pos, movelist, sq);
          jump<SSW>(pos, movelist, sq);
          jump<SSE>(pos, movelist, sq);
        }
        if (p == rel_B || p == rel_Q) {
          slide<NW>(pos, movelist, sq);
          slide<NE>(pos, movelist, sq);
          slide<SW>(pos, movelist, sq);
          slide<SE>(pos, movelist, sq);
        }
        if (p == rel_R || p == rel_Q) {
          slide<N>(pos, movelist, sq);
          slide<S>(pos, movelist, sq);
          slide<E>(pos, movelist, sq);
          slide<W>(pos, movelist, sq);
        }
      } else {  // pin
        if (p == rel_P) {
          if (rel_rank != 1) {  // non-promotion moves
            if (attacker_dir == N || attacker_dir == S) {
              // generate_pawn_moves<turn>(pos, movelist, sq);
              // push
              if (only_move<rel_North>(pos, movelist, sq))
                // double push only if push succeeds
                if (rel_rank == 6) only_move<rel_NN>(pos, movelist, sq);
            } else if (attacker_dir == rel_NW) {
              only_capture<rel_NW>(pos, movelist, sq);
            } else if (attacker_dir == rel_NE) {
              only_capture<rel_NE>(pos, movelist, sq);
            }
          } else {  // promotion moves
            if (attacker_dir == N || attacker_dir == S) {
              generate_promotion_moves_safe<turn>(pos, movelist, sq);
            } else if (attacker_dir == rel_NW) {
              if (isnt_A(sq) && hostile(pos[sq], pos[sq + rel_NW]))
                for (auto& piece : {wQ, wR, wB, wN})
                  movelist.emplace_back(sq, sq + rel_NW, Piece(piece * turn));
            } else if (attacker_dir == rel_NE) {
              if (isnt_H(sq) && hostile(pos[sq], pos[sq + rel_NE]))
                for (auto& piece : {wQ, wR, wB, wN})
                  movelist.emplace_back(sq, sq + rel_NE, Piece(piece * turn));
            }
          }
        }
// slide_capture_threat<attacker_dir, rel_B, rel_Q, rel_K>(pos, movelist, sq);
#define slide_capture(dir)                         \
  if (attacker_dir == dir) {                       \
    slide<dir>(pos, movelist, sq);                 \
    slide<Direction(dir * -1)>(pos, movelist, sq); \
  }
        if (p == rel_B || p == rel_Q) {
          slide_capture(NW) slide_capture(NE) slide_capture(SW)
              slide_capture(SE)
        }
        if (p == rel_R || p == rel_Q) {
          slide_capture(N) slide_capture(S) slide_capture(E) slide_capture(W)
        }
#undef slide_capture
      }
    }
  }
#undef is_intermediate_sq
  return movelist;
}

template <MoveGenType type>
vector<Move> generate_legal_moves2(Board& board) {
  if (board.turn == White)
    return generate_legal_moves2<White, type>(board);
  else
    return generate_legal_moves2<Black, type>(board);
}

template <Player turn>
vector<Move> generate_legal_moves(Board& board) {
  if (is_in_check<turn>(board)) {
    return generate_legal_moves2<turn, Evasions>(board);
  } else {
    return generate_legal_moves2<turn, NonEvasions>(board);
  }

  // cout << "gen legal @" << zobrist_hash() << endl;
  auto movelist = generate_pseudo_moves<turn>(board);
  vector<Move> better_moves, others;
  better_moves.reserve(movelist.size());
  others.reserve(movelist.size());

  // split movelist into better_moves and others
  // better_moves are captures, promotions, enpassant, castling
  // finally combine both

  for (auto& move : movelist) {
    if (is_legal<turn>(board, move))
      (board[move.to] != Empty || move.promotion != Empty || move.enpassant ||
               move.castling
           ? better_moves
           : others)
          .push_back(move);
  }

  better_moves.insert(better_moves.end(), others.begin(), others.end());
  return better_moves;
}

vector<Move> generate_legal_moves(Board& board) {
  if (board.turn == White)
    return generate_legal_moves<White>(board);
  else
    return generate_legal_moves<Black>(board);
}

int perft(Board& board, int depth, bool last_move_check) {
  if (depth <= 0) return 1;

  auto legal = last_move_check ? generate_legal_moves2<Evasions>(board)
                               : generate_legal_moves2<NonEvasions>(board);

  if (depth == 1) return legal.size();

  int nodes = 0;

  for (auto& move : legal) {
    board.make_move(move);
    // bool check = board.turn == White ? is_in_check<White>(board)
    //                                  : is_in_check<Black>(board);
    bool check = board.turn == White ? is_check<White>(board, move)
                                     : is_check<Black>(board, move);
    nodes += perft(board, depth - 1, check);
    board.unmake_move(move);
  }
  return nodes;
}

int divide(Board& board, int depth) {
  int sum = 0;
  auto t1 = chrono::high_resolution_clock::now();
  auto legal = generate_legal_moves(board);
  // Board before = board;
  for (auto& move : legal) {
    board.make_move(move);
    // Board after = board;
    // bool check = board.turn == White ? is_in_check<White>(board)
    //                                  : is_in_check<Black>(board);
    bool check = board.turn == White ? is_check<White>(board, move)
                                     : is_check<Black>(board, move);
    int nodes = perft(board, depth - 1, check);
    board.unmake_move(move);
    // if (before.to_fen() != board.to_fen()) {
    //   cerr << "! "
    //        << "undo failed" << endl;
    //   cerr << "move: ";
    //   move.print();
    //   cerr << "before:       " << before.to_fen() << endl;
    //   cerr << "after move:   " << after.to_fen() << endl;
    //   cerr << "after unmove: " << board.to_fen() << endl;
    //   break;
    // }
    sum += nodes;
    cout << move.to_uci() << ": " << nodes << endl;
  }
  auto t2 = chrono::high_resolution_clock::now();
  auto diff = chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count();

  cout << "Moves: " << legal.size() << endl;
  cout << "Nodes: " << sum << endl;
  cout << "Nodes/sec: " << int(diff ? sum * 1e9 / diff : -1) << endl;
  return sum;
}

bool make_move_if_legal(Board& board, const string& move) {
  auto m = get_move_if_legal(board, move);
  if (m.equals(0, 0)) return false;
  board.make_move(m);
  return true;
}

Move get_move_if_legal(Board& board, const string& move) {
  int l = move.length();
  if (l == 4 || l == 5) {
    const int from = sq2idx(move[0], move[1]);
    const int to = sq2idx(move[2], move[3]);
    Piece promotion = Empty;
    if (l == 5)
      promotion =
          char2piece(board.turn == White ? toupper(move[4]) : tolower(move[4]));
    for (auto& m : generate_legal_moves(board))
      if (m.equals(from, to) && m.promotion == promotion) {
        return m;
      }
  }
  return Move();
}

// NOTE: to be called before making the move
string to_san(Board& board, Move move) {
  string san;
  // castling
  if (move.castling) {
    if (move.equals(4, 6) || move.equals(60, 62))
      san = "O-O";
    else if (move.equals(4, 2) || move.equals(60, 58))
      san = "O-O-O";
  } else {
    char piece = toupper(piece2char(board[move.from]));
    string origin = idx2sq(move.from);
    string target = idx2sq(move.to);

    // piece identification
    if (piece != 'P') san = piece;

    // disambiguation
    bool same_file = false, same_rank = false, same_piece = false;
    for (auto& move2 : generate_pseudo_moves(board)) {
      if (move2.from != move.from)                    // don't compare to self
        if (move2.to == move.to)                      // same destination
          if (board[move2.from] == board[move.from])  // same kind of piece
          {
            same_piece = true;
            if (move2.from / 8 == move.from / 8)  // same rank
              same_file = true;
            if (move2.from % 8 == move.from % 8)  // same file
              same_rank = true;
          }
    }
    if (same_file && piece != 'P') san += origin[0];  // add file, if not pawn
    if (same_rank) san += origin[1];                  // add rank
    if (!same_file && !same_rank && same_piece)       // add file, e.g. knights
      san += origin[0];

    // capture
    if (!board.empty(move.to) || move.enpassant) {
      if (piece == 'P') san += origin[0];  // add file if piece is pawn
      san += 'x';
    }

    // target
    san += target;

    // promotion
    if (move.promotion != Empty) {
      san += '=';
      san += toupper(piece2char(move.promotion));
    }
  }
  board.make_move(move);
  if (generate_legal_moves(board).size() == 0)  // mate indication
    san += '#';
  else if (is_in_check(board, board.turn))  // check indication
    san += '+';
  board.unmake_move(move);
  return san;
}
