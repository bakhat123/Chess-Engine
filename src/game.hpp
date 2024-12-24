#pragma once

#include "board.hpp"
#include "search.hpp"

class Game {
 public:
  Board board;
  vector<Move> movelist;
  int ply = 0, end = 0;
  Status result = Undecided;
  DrawType draw_type = None;
  int material_count[13] = {0};
  vector<uint64_t> repetitions;

  Game();
  bool make_move(string m);
  bool make_move(Move m);
  void prev();
  void next();
  void print_movelist();
  string to_pgn();
  void seek(int n);
  Move random_move();
  pair<Move, int> ai_move(int time);
  Status get_result();
  void new_game();
  bool load_fen(string fen);
  void update_material_count();
};

string get_result_str(Status result);
string get_draw_type_str(DrawType draw_type);