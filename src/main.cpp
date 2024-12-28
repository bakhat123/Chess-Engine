#include "board.hpp"
#include "search.hpp"

thread ai_thread;

void parse_and_make_moves(istringstream& iss, Board& board,
                          vector<uint64_t>& repetitions) {
  string token;
  while (iss >> token) {
    if (make_move_if_legal(board, token))
      repetitions.push_back(board.zobrist_hash());
    else
      break;
  }
}

void uci_loop() {
  Search ai;
  auto& board = ai.board;
  auto& repetitions = ai.repetitions;
  string line, token;

  while (getline(cin, line)) {
    istringstream iss(line);
    iss >> token;
    if (token[0] == '#') continue;  // ignore comments

    if (token == "uci") {
      cout << "id name Chess by W 16" << endl;
      cout << "uciok" << endl;
    } else if (token == "ucinewgame") {
    } else if (token == "position") {
      iss >> token;
      repetitions.clear();
      if (token == "fen") {
        string fen;
        for (int i = 0; i < 6; i++) {
          if (iss >> token && token != "moves")
            fen += token + " ";
          else
            break;
        }
        // cout << "--\n{" << fen << "}\n--\n";
        board.load_fen(fen);
      } else if (token == "startpos") {
        repetitions.clear();
        board.load_startpos();
      }
      iss >> token;
      if (token == "moves") parse_and_make_moves(iss, board, repetitions);
    } else if (token == "go") {
      // example: go wtime 56329 btime 86370 winc 1000 binc 1000
      repetitions.clear();
      ai.search_type = Time_per_game;
      ai.set_clock(30000, 30000, 0, 0);
      ai.max_depth = 100;
      while (iss >> token) {
        if (token == "searchmoves") {
        } else if (token == "ponder") {
          ai.search_type = Ponder;
        } else if (token == "wtime") {
          iss >> ai.wtime;
        } else if (token == "btime") {
          iss >> ai.btime;
        } else if (token == "winc") {
          iss >> ai.winc;
        } else if (token == "binc") {
          iss >> ai.binc;
        } else if (token == "movestogo") {
        } else if (token == "depth") {
          iss >> ai.max_depth;
          ai.search_type = Fixed_depth;
        } else if (token == "nodes") {
        } else if (token == "mate") {
          ai.search_type = Mate;
        } else if (token == "movetime") {
          iss >> ai.mtime;
          ai.search_type = Time_per_move;
        } else if (token == "infinite") {
          ai.search_type = Infinite;
        } else if (token == "startpos") {
          board.load_startpos();
          iss >> token;
          if (token == "moves") parse_and_make_moves(iss, board, repetitions);
        } else if (token == "perft") {
          iss >> token;
          if (!ai.searching) {
            if (ai_thread.joinable()) ai_thread.join();
            ai_thread = thread([&]() { divide(board, stoi(token)); });
          }
        }
      }
      if (!ai.searching) {
        if (ai_thread.joinable()) ai_thread.join();
        ai_thread = thread([&]() { ai.search(); });
      }
    } else if (token == "stop") {
      ai.searching = false;
      if (ai_thread.joinable()) ai_thread.join();
    } else if (token == "ponderhit") {
    } else if (token == "debug") {
      iss >> token;
      ai.debug_mode = token == "on";
      cout << "debug mode: " << (ai.debug_mode ? "on" : "off") << endl;
    } else if (token == "isready") {
      cout << "readyok" << endl;
    } else if (token == "setoption") {
    } else if (token == "register") {
    } else if (token == "d") {
      board.print();
    } else if (token == "quit") {
      break;

      // additional commands
    } else if (token == "pseudo") {
      for (auto& move : generate_pseudo_moves(board))
        cout << to_san(board, move) << endl;
    } else if (token == "legal") {
      for (auto& move : generate_legal_moves(board))
        cout << to_san(board, move) << endl;
    } else if (token == "lichess") {
      string fen = board.to_fen();
      replace(fen.begin(), fen.end(), ' ', '_');
      cout << "https://lichess.org/analysis/" << fen << endl;
    } else if (token == "perft" || token == "divide") {
      iss >> token;
      if (!ai.searching) {
        if (ai_thread.joinable()) ai_thread.join();
        ai_thread = thread([&]() { divide(board, stoi(token)); });
      }
    } else if (token == "moves") {
      while (iss >> token) {
        if (make_move_if_legal(board, token))
          board.print();
        else
          break;
      }
    } else if (token == "eval") {
      ai.eval<true>();
    } else if (token == "isincheck") {
      cout << is_in_check(board, board.turn) << endl;
    } else if (token == "turn") {
      board.change_turn();
    } else {
      cout << "Invalid command: " << line << endl;
    }
  }
  ai.searching = false;
  if (ai_thread.joinable()) ai_thread.join();
}

int main() {
  zobrist_init();
  uci_loop();
}