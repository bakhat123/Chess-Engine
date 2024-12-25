#pragma once

#include "board.hpp"
#include "movegen.hpp"
#include "types.hpp"

struct TTEntry {  // transposition table entry
  uint64_t hash;
  int age;
  int depth = 0;
  int score;
  EvalType eval_type;
  // Move best_move;
};
typedef TTEntry* TT_t;

class Search {
 public:
  Board board;
  int wtime = 30000, btime = 30000, winc = 0, binc = 0;
  int mtime = 1000;  // move time
  int max_depth = 100;
  SearchType search_type = Time_per_game;
  atomic<bool> searching{false};
  vector<uint64_t> repetitions;  // for checking draw by repetition
  TT_t TT;
  int nodes_searched = 0;
  int ply = 0;
  bool debug_mode = false;

  string debug = "";

  Search();
  pair<Move, int> search();
  void set_clock(int _wtime, int _btime, int _winc, int _binc);
  template <bool debug>
  int eval();
  bool is_repetition();

 protected:
  int negamax(int depth);
  int alphabeta(int depth, int alpha, int beta);
  int quiesce(int depth, int alpha, int beta);
  vector<pair<int, Move>> iterative_search();
};
