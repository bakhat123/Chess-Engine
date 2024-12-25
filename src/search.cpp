#include "search.hpp"

#include "knowledge.hpp"

const int MateScore = 1e6;
const int TT_miss = 404000;
int TT_size = 1 << 16;

void init_TT(TT_t& TT, int size) {
  // get next power of 2
  int n = 1;
  while (n < size) n <<= 1;
  TT_size = n;

  // allocate memory, half the size if fails
  do TT = new (nothrow) TTEntry[TT_size];
  while (!TT && (TT_size >>= 1));

  for (int i = 0; i < TT_size; i++) {
    TT[i].age = -1;
    TT[i].depth = 0;
  }
}

int normalize_score(int score, int depth) {
  if (score > MateScore) return score - depth;
  if (score < -MateScore) return score + depth;
  return score;
}

inline int TT_probe(TT_t TT, u_int64_t hash, int depth, int alpha, int beta) {
  TT_t entry = &TT[hash % TT_size];
  if (entry->hash != hash) return TT_miss;  // verify hash
  if (entry->age > 0 && entry->depth >= depth) {
    int score = normalize_score(entry->score, depth);
    if (entry->eval_type == Exact) return score;
    if (entry->eval_type == LowerBound && score >= beta) return beta;
    if (entry->eval_type == UpperBound && score <= alpha) return alpha;
  }
  return TT_miss;
}

inline void TT_store(TT_t TT, u_int64_t hash, int depth, int score,
                     EvalType eval_type) {
  TT_t entry = &TT[hash % TT_size];
  if (entry->depth > depth) return;  // don't overwrite deeper scores
  entry->age = 1;
  entry->hash = hash;
  entry->depth = depth;
  entry->score = normalize_score(score, depth);
  entry->eval_type = eval_type;
}

Search::Search() {
  // init_TT(TT, TT_size);
}

void Search::set_clock(int _wtime, int _btime, int _winc, int _binc) {
  wtime = _wtime;
  btime = _btime;
  winc = _winc;
  binc = _binc;
}

pair<Move, int> Search::search() {
  searching = true;
  auto movelist = iterative_search();
  searching = false;

  auto bestmove = movelist.front().second;
  auto bestscore = movelist.front().first;

  // choose random move out of same-scoring moves
  vector<pair<int, Move>> bestmoves;
  for (auto& score_move : movelist)
    if (score_move.first == bestscore) bestmoves.emplace_back(score_move);

  if (bestmoves.size() > 1) {
    random_device rd;
    uniform_int_distribution<int> dist(0, bestmoves.size() - 1);
    auto& best = bestmoves[dist(rd)];
    bestmove = best.second;
    bestscore = best.first;
  }

  cout << "info bestmove: " << bestscore << " = " << to_san(board, bestmove)
       << " out of " << movelist.size() << " legal, " << bestmoves.size()
       << " best" << endl;
  cout << "bestmove " << bestmove.to_uci() << endl;
  return {bestmove, bestscore};
}

// mate 2: 2K5/8/2k5/1r6/8/8/8/8 b - - 0 1
// mate -2: 2k5/8/2K5/8/1R6/8/8/8 b - - 0 1
// mate 1: 2k5/8/2K5/5R2/8/8/8/8 w - - 0 1
// mate -1: K7/8/1k6/5r2/8/8/8/8 w - - 0 1
int get_mate_score(int score) {
  if (score > MateScore / 2)
    return (MateScore - score) / 2 + 1;
  else if (score < -MateScore / 2)
    return (-score - MateScore) / 2 - 1;
  return 0;
}

void print_score(int score) {
  int mate = get_mate_score(score);
  if (mate == 0)
    cout << " score cp " << score;
  else
    cout << " score mate " << mate;
}

void print_info(string infostring, int depth, int score, int nodes_searched,
                int time_taken, string move) {
  cout << infostring << " depth " << depth;
  print_score(score);
  cout << " nodes " << nodes_searched << " time " << time_taken << " pv "
       << move << endl;
}

// TODO: verify thoroughly
vector<pair<int, Move>> Search::iterative_search() {
  vector<pair<int, Move>> bestmoves;
  int time_taken = 0;
  int max_search_time = (board.turn == White) ? (wtime + winc) : (btime + binc);
  if (search_type == Fixed_depth) {
    max_search_time = INT_MAX;
    cout << "info using maxdepth: " << max_depth << endl;
  } else if (search_type == Time_per_move) {
    max_search_time = mtime;
    cout << "info using movetime: " << max_search_time << endl;
  } else if (search_type == Time_per_game) {
    double percentage = 1;  // 0.88;
    max_search_time *= min((percentage + board.moves / 116.4) / 50, percentage);
    cout << "info using time: " << max_search_time << endl;
  } else {
    max_search_time = INT_MAX;
    cout << "info using infinite: " << max_search_time << endl;
  }

  const auto start_time = chrono::high_resolution_clock::now();

  // convert moves to score-move pairs
  auto legals = generate_legal_moves(board);
  vector<pair<int, Move>> legalmoves;
  legalmoves.reserve(legals.size());
  for (auto& move : legals) legalmoves.emplace_back(0, move);

  // conservative time management
  max_search_time *= 0.9;

  // iterative deepening
  int depth = 1;
  for (; searching && time_taken * 2 < max_search_time && depth <= max_depth;
       depth++) {
    bestmoves.clear();

    auto& curr_best = legalmoves.front();
    auto& curr_worst = legalmoves.back();

    // if (search_type != Mate && search_type != Infinite) {
    // no need to seach deeper if there's only one legal move
    if (legalmoves.size() == 1) {
      // limit search time
      max_search_time = min(max_search_time, 500);
      cout << "info only one legal move" << endl;
      // bestmoves.emplace_back(curr_best);
      // break;
    }

    // mate found so prune non-mating moves (TODO: verify)
    if (get_mate_score(curr_best.first) > 0) {
      for (auto& move : legalmoves)
        if (get_mate_score(move.first) > 0) bestmoves.emplace_back(move);
      cout << "info mate found" << endl;
      break;
    }
    // }

    // worst move is losing so prune losing moves
    if (get_mate_score(curr_worst.first) < 0) {
      // there is atleast one non-losing move
      if (get_mate_score(curr_best.first) == 0) {
        // prune losing moves
        for (auto& move : legalmoves)
          if (get_mate_score(move.first) == 0) bestmoves.emplace_back(move);
        if (bestmoves.size() != 0) legalmoves = bestmoves;
        // break;
        cout << "info pruned losing moves" << endl;
      } else {
        // best and worst move is losing, so no point in searching deeper
        bestmoves = legalmoves;
        // if (search_type != Mate && search_type != Infinite)
        cout << "info all moves are losing" << endl;
        break;
      }
    }

    for (auto& score_move : legalmoves) {
      nodes_searched = 0;
      ply = -1;  // somehow this fixes the reported mate score
      ply++;
      board.make_move(score_move.second);
      int score = 0;
      // score = -negamax(depth);
      score = -alphabeta(depth, -MateScore, +MateScore);
      board.unmake_move(score_move.second);
      ply--;
      time_taken = chrono::duration_cast<chrono::milliseconds>(
                       chrono::high_resolution_clock::now() - start_time)
                       .count();
      // break if time is up, but ensure we have atleast one move
      if (!searching || time_taken >= max_search_time) {
        if (bestmoves.size() == 0) {
          bestmoves = legalmoves;
        }
        searching = false;
        cout << "info time is up" << endl;
        break;
      }
      score_move.first = score;
      if (debug_mode)
        print_info("info string", depth, score, nodes_searched, time_taken,
                   score_move.second.to_uci());
      bestmoves.emplace_back(score, score_move.second);
    }

    if (!searching) break;
    // move-ordering
    stable_sort(legalmoves.begin(), legalmoves.end(),
                [](auto& a, auto& b) { return a.first > b.first; });

    time_taken = chrono::duration_cast<chrono::milliseconds>(
                     chrono::high_resolution_clock::now() - start_time)
                     .count();
    print_info("info", depth, legalmoves.front().first, nodes_searched,
               time_taken, legalmoves.front().second.to_uci());

    // TODO: fix this
    if (search_type == Mate)
      debug = to_string(get_mate_score(legalmoves.front().first));
  }

  cout << "info total time: " << time_taken << endl;

  // TODO: choose random move out of same-scoring moves

  // move-ordering
  stable_sort(bestmoves.begin(), bestmoves.end(),
              [](auto& a, auto& b) { return a.first > b.first; });

  return bestmoves;
}

template int Search::eval<true>();   // prints eval
template int Search::eval<false>();  // doesn't print eval

template <bool debug>
inline int Search::eval() {
  int material_score = 0;
  int pst_score = 0;
  int mobility_score = 0;
  int endgame_score = 0;
  int material_count[7] = {0};

  // calculate material and piece-square table score
  for (int i = 0; i < 64; i++) {
    const Piece piece = board.board[i];
    material_count[abs(piece)]++;
    material_score += piece_val[piece + 6];
    if (piece)
      pst_score +=
          pst[abs(piece) - 1][piece > 0 ? i : 63 - i] * (piece > 0 ? 1 : -1);
  }

  // calculate phase
  // https://www.chessprogramming.org/Tapered_Eval
  const int PawnPhase = 0;
  const int KnightPhase = 1;
  const int BishopPhase = 1;
  const int RookPhase = 2;
  const int QueenPhase = 4;
  const int TotalPhase = PawnPhase * 16 + KnightPhase * 4 + BishopPhase * 4 +
                         RookPhase * 4 + QueenPhase * 2;

  int phase = TotalPhase;
  phase -= material_count[wP] * PawnPhase;
  phase -= material_count[wN] * KnightPhase;
  phase -= material_count[wB] * BishopPhase;
  phase -= material_count[wR] * RookPhase;
  phase -= material_count[wQ] * QueenPhase;
  phase = (phase * 256 + TotalPhase / 2) / TotalPhase;

  // calculate endgame score
  endgame_score += pst_k_end[board.Kpos] - pst_k_end[63 - board.kpos];
  // https://www.chessprogramming.org/Mop-up_Evaluation
  const int cmd =
      (board.turn == Black ? pst_cmd[board.Kpos] : pst_cmd[63 - board.kpos]);
  const int file1 = board.Kpos % 8, rank1 = board.Kpos / 8;
  const int file2 = board.kpos % 8, rank2 = board.kpos / 8;
  const int md = abs(rank2 - rank1) + abs(file2 - file1);
  endgame_score += (5 * cmd + 2 * (14 - md));

  // int rel_mobility = board.generate_legal_moves().size();
  // board.change_turn();
  // int opp_mobility = board.generate_legal_moves().size();
  // board.change_turn();
  // mobility_score = (rel_mobility - opp_mobility) * board.turn;
  const int score = material_score + pst_score + mobility_score;
  const int eval = (score * (256 - phase) + endgame_score * phase) / 256;

  if (debug) {
    board.print();
    cout << "material: " << material_score << endl;
    cout << "position: " << pst_score << endl;
    cout << "opening score: " << score << endl;
    cout << "endgame score: " << endgame_score << endl;
    cout << "phase: " << phase << endl;
    cout << "adjusted opening score: " << score * (256 - phase) / 256 << endl;
    cout << "adjusted endgame score: " << endgame_score * phase / 256 << endl;

    cout << "is in check: " << is_in_check(board, board.turn) << endl;
    cout << "is repetition: " << is_repetition() << endl;
  }

  return eval;
}

int Search::negamax(int depth) {
  if (depth == 0) return eval<false>() * board.turn;
  int bestscore = -MateScore;
  auto legals = generate_legal_moves(board);
  for (auto& move : legals) {
    ply++;
    repetitions.push_back(board.zobrist_hash());
    board.make_move(move);
    int score = -negamax(depth - 1);
    board.unmake_move(move);
    repetitions.pop_back();
    ply--;
    if (score > bestscore) {
      bestscore = score;
    }
  }
  if (legals.size() == 0)
    return is_in_check(board, board.turn) ? -MateScore + ply : 0;

  return bestscore;
}

int Search::alphabeta(int depth, int alpha, int beta) {
  if (ply && is_repetition()) return 0;

  // TODO: probe TT
  // const int TT_score = TT_probe(TT, board.zobrist_hash(), depth, alpha,
  // beta); if (TT_score != TT_miss) return TT_score;

  if (depth == 0) return quiesce(0, alpha, beta);

  bool in_check = is_in_check(board, board.turn);

  nodes_searched++;
  // check extension
  if (in_check) depth++;

  int score = 0;

  auto legals = generate_legal_moves(board);

  // EvalType eval_type = UpperBound;

  for (auto& move : legals) {
    ply++;
    repetitions.push_back(board.zobrist_hash());
    board.make_move(move);
    // TODO: late move reduction
    // full window search if not LMR
    score = -alphabeta(depth - 1, -beta, -alpha);
    board.unmake_move(move);
    repetitions.pop_back();
    ply--;
    if (score > alpha) {
      // TODO: PV update
      // eval_type = Exact;
      alpha = score;
      if (alpha >= beta) {  // fail-high beta-cutoff
        // // TODO: store TT
        // TT_store(TT, board.zobrist_hash(), depth, beta, LowerBound);
        return beta;
      }
    }
  }

  // checkmate or stalemate
  if (legals.size() == 0) return in_check ? -MateScore + ply : 0;

  // // TODO: store TT
  // TT_store(TT, board.zobrist_hash(), depth, alpha, eval_type);
  return alpha;  // fail-low alpha-cutoff
}

int Search::quiesce(int depth, int alpha, int beta) {
  int stand_pat = eval<false>() * board.turn;

  if (depth > max_depth) return stand_pat;  // max depth reached

  if (stand_pat >= beta) {  // fail-high beta-cutoff
    return beta;
  }

  if (stand_pat > alpha) {  // fail-low alpha-cutoff
    alpha = stand_pat;
  }
  nodes_searched++;

  auto legals = generate_legal_moves(board);

  for (auto& move : legals) {
    if (board[move.to] == Empty) continue;  // ignore non-capturing moves
    ply++;
    repetitions.push_back(board.zobrist_hash());
    board.make_move(move);
    int score = -quiesce(depth + 1, -beta, -alpha);
    board.unmake_move(move);
    repetitions.pop_back();
    ply--;
    if (score > alpha) {
      alpha = score;
      if (alpha >= beta) {  // fail-high beta-cutoff
        return beta;
      }
    }
  }

  return alpha;  // fail-low alpha-cutoff
}

bool Search::is_repetition() {
  const auto hash = board.zobrist_hash();
  int n = repetitions.size();
  for (int i = 0; i < n - 1; i++) {
    if (repetitions[i] == hash) return true;
  }
  return false;
}