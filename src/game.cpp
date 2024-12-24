// TODO: cleanup

#include "game.hpp"

Game::Game() { new_game(); }

bool Game::make_move(string m) { return make_move_if_legal(board, m); }

bool Game::make_move(Move m) {
  if (end > 500) return false;
  if (m.from == m.to) return false;

  if (ply != end) {
    int oldply = ply;
    while (ply < end) next();
    while (ply > oldply) prev(), movelist.pop_back();
    end = ply;
  }
  // cout << "making move: " << m.to_uci() << endl;
  repetitions.push_back(board.zobrist_hash());
  board.make_move(m);
  movelist.push_back(m);
  // if (m.captured) material_count[m.captured + 6]--;
  result = get_result();
  ply++, end++;
  return true;
}

void Game::prev() {
  if (ply <= 0) return;
  board.unmake_move(movelist[--ply]);
  // movelist[ply].print();
  // if (board[movelist[ply].to]) material_count[board[movelist[ply].to] + 6]++;
  repetitions.pop_back();
  result = get_result();
}
void Game::next() {
  if (ply >= end) return;
  // if (board[movelist[ply].to]) material_count[board[movelist[ply].to] + 6]--;
  // movelist[ply].print();
  board.make_move(movelist[ply++]);
  repetitions.push_back(board.zobrist_hash());
  result = get_result();
}

void Game::print_movelist() {
  Board temp;
  for (size_t i = 0; i < movelist.size(); i++) {
    cerr << i + 1 << " " << to_san(temp, movelist[i]) << "    ";
    movelist[i].print();
    temp.make_move(movelist[i]);
  }
}

string Game::to_pgn() {
  time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
  string date = ctime(&now);

  string pgn;
  pgn.reserve(1000);
  pgn += "[Event \"?\"]\n";
  pgn += "[Site \"?\"]\n";
  pgn += "[Date \"" + date + "\"]\n";
  pgn += "[Round \"?\"]\n";
  pgn += "[White \"?\"]\n";
  pgn += "[Black \"?\"]\n";
  pgn += "[Result \"" + get_result_str(result) + "\"]\n";

  Board temp;
  for (int i = 0; i < end; i++) {
    if (i % 2 == 0) pgn += to_string(i / 2 + 1) + ". ";
    pgn += to_san(temp, movelist[i]) + " ";
    temp.make_move(movelist[i]);
  }
  return pgn;
}
void Game::seek(int n) {
  while (ply < n && ply < end) next();
  while (ply > n && ply > 0) prev();
}

Move Game::random_move() {
  vector<Move> legal = generate_legal_moves(board);
  Move bestmove;
  if (legal.size() > 0) {
    random_device rd;
    uniform_int_distribution<int> dist(0, legal.size() - 1);
    bestmove = legal[dist(rd)];
  }
  return bestmove;
}

pair<Move, int> Game::ai_move(int time) {
  pair<Move, int> ai_move;
  string input;
  input.reserve(1000);

  for (auto& m : movelist) {
    input += m.to_uci() + " ";
  }

  // write to a file
  ofstream file;
  file.open("tempConnection");
  file << "uci" << endl;
  file << "position startpos moves " << input << endl;
  file << "go movetime " + to_string(time) << endl;
  file.close();

  cout << ">>> position startpos moves " << input << endl;

#pragma GCC diagnostic ignored "-Wunused-result"

  // run the engine
  string engine_path = "./main";
  string cmd = engine_path + " < tempConnection > tempConnection2";
  system(cmd.c_str());

  // read the output
  ifstream file2;
  file2.open("tempConnection2");
  string output;
  while (getline(file2, output)) {
    cout << output << endl;
    if (output.size() > 8) {
      if (output.substr(0, 8) == "bestmove") {
        // cout << "[" << output.substr(9, 4) << "]" << endl;
        string move = output.substr(9, 4);
        ai_move.first = get_move_if_legal(board, move);
        break;
      }
    }
  }
  file2.close();

  // cleanup
  system("rm tempConnection tempConnection2");

  return ai_move;
}

Status Game::get_result() {
  draw_type = None;

  int can_move = generate_legal_moves(board).size();

  if (!can_move) {
    if (is_in_check(board, board.turn))
      return board.turn == White ? BlackWins : WhiteWins;
    else {
      draw_type = Stalemate;
      return Draw;
    }
  }

  // obvious draws
  if (board.fifty >= 100) {
    draw_type = FiftyMoveRule;
    return Draw;
  }

  update_material_count();

  size_t w_size = 0, b_size = 0;
  for (int i = wP; i <= wK; i++) w_size += material_count[i + 6];
  for (int i = bP; i >= bK; i--) b_size += material_count[i + 6];

  // KvK
  if (w_size == 1 && b_size == 1) {
    draw_type = InsufficientMaterial;
    return Draw;
  }
  // KvKB or KvKN
  if (w_size == 1 && b_size == 2)
    if (material_count[bB + 6] == 1 || material_count[bN + 6] == 1) {
      draw_type = InsufficientMaterial;
      return Draw;
    }
  // KBvK or KNvK
  if (b_size == 1 && w_size == 2)
    if (material_count[wB + 6] == 1 || material_count[wN + 6] == 1) {
      draw_type = InsufficientMaterial;
      return Draw;
    }
  // // KBvKB, bishops being the same color
  // if (b_size == 2 && w_size == 2 &&
  //     board.sq_color(board.board.find('B')) ==
  //         board.sq_color(board.board.find('b')))
  //   return Draw;

  // repetition
  auto hash = board.zobrist_hash();
  int count = 0;
  int n = repetitions.size();
  for (int i = 0; i < n - 1; i++) {
    if (repetitions[i] == hash) count++;
    if (count == 3) {
      draw_type = ThreefoldRepetition;
      return Draw;
    }
  }
  return Undecided;
}

void Game::new_game() {
  ply = end = 0;
  movelist.clear();
  board.load_startpos();
  update_material_count();
  result = Undecided;
  repetitions.clear();
}

bool Game::load_fen(string fen) {
  new_game();
  auto temp = board;
  bool valid = temp.load_fen(fen);
  if (valid) {
    board = temp;
    update_material_count();
    result = get_result();
  }
  return valid;
}

void Game::update_material_count() {
  fill_n(material_count, 13, 0);
  for (auto& x : board.board)
    if (x) material_count[x + 6]++;
}

string get_result_str(Status result) {
  if (result == Draw)
    return "½-½";
  else if (result == WhiteWins)
    return "1-0";
  else if (result == BlackWins)
    return "0-1";
  return "*";
}

string get_draw_type_str(DrawType draw_type) {
  if (draw_type == InsufficientMaterial)
    return "Draw by Insufficient material";
  else if (draw_type == FiftyMoveRule)
    return "Draw by Fifty move rule";
  else if (draw_type == ThreefoldRepetition)
    return "Draw by Threefold repetition";
  else if (draw_type == FivefoldRepetition)
    return "Draw by Fivefold repetition";
  else if (draw_type == SeventyFiveMoveRule)
    return "Draw by Seventy five move rule";
  else if (draw_type == Stalemate)
    return "Draw by Stalemate";
  else if (draw_type == DeadPosition)
    return "Draw by Dead position";
  return "";
}