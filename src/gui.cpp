// TODO:
// - properly integrate with engine!!
// - drag and drop pieces
// - show what computer is thinking
// - show evaluation bar
// - show move history

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <gtk/gtk.h>

#include "game.hpp"

Game g;

thread ai_thread;
atomic<bool> thinking = false;
int ai_think_time = 5000;

enum MoveType { Human, Computer, PreMove };
enum PlayerType { HumanPlayer, RandomMover, AIPlayer };

bool board_flipped = false;
int sel_sq = -1, prom_sq = -1;
vector<Move> moves;
map<int, Move> promotions;
set<int> valid_sqs, valid_capture_sqs;

bool disable_engine = false;
bool game_running = false;
Move premove;

GtkWidget *squares[64], *chess_board_grid, *statusbar, *move_label;
GtkEntryBuffer *fen_entry_buffer;
GtkWidget *white_human, *white_randommover, *white_ai, *black_human,
    *black_randommover, *black_ai, *stop_think, *start_game, *eval_bar,
    *ai_time_scale;

void computer_move();
void move_manager();
void make_move(Move m, MoveType type);

int flipped(int sq) { return board_flipped ? 63 - sq : sq; }

void show_statusbar_msg(string s) {
  gtk_statusbar_pop(GTK_STATUSBAR(statusbar), 0);
  gtk_statusbar_push(GTK_STATUSBAR(statusbar), 0, s.c_str());
}

void update_piece(int sq, Piece piece) {
  sq = flipped(sq);
  gtk_widget_set_name(squares[sq], "");
  gtk_widget_set_name(squares[sq],
                      string("piece_").append(1, piece2char(piece)).c_str());
}

void generate_move_hints() {
  Board temp = g.board;
  if (thinking) temp.change_turn();
  // TODO: generate all possible moves even non-existent pawn captures
  moves = !thinking ? generate_legal_moves(temp) : generate_pseudo_moves(temp);
  valid_sqs.clear();
  valid_capture_sqs.clear();
  for (auto &move : moves)
    if (move.from == sel_sq)
      (temp.empty(move.to) ? valid_sqs : valid_capture_sqs).insert(move.to);

  // deselect square if no valid move possible
  if (valid_sqs.size() == 0 && valid_capture_sqs.size() == 0) sel_sq = -1;
}

void update_board() {
  if (~sel_sq)
    if (g.result == Undecided) generate_move_hints();
  auto &board = g.board.board;

  int last_from = -1, last_to = -1;
  if (g.ply > 0) {
    Move last = g.movelist[g.ply - 1];
    last_from = last.from;
    last_to = last.to;

    g.board.unmake_move(last);
    show_statusbar_msg("Moved " + to_san(g.board, last));
    g.board.make_move(last);
  } else
    show_statusbar_msg("");
  for (int i = 0; i < 64; i++) {
    int j = flipped(i);
    if (!promotions.count(i)) update_piece(i, board[i]);
    for (auto &c : {"selected_sq", "valid_sq", "valid_capture_sq", "check_sq",
                    "premove_sq"})
      gtk_widget_remove_css_class(squares[j], c);
    if ((premove.from == i) ^ (premove.to == i)) {
      gtk_widget_add_css_class(squares[j], "premove_sq");
      continue;
    }
    if (i == sel_sq || i == last_from || i == last_to)
      gtk_widget_add_css_class(squares[j], "selected_sq");
    if (valid_sqs.count(i))
      gtk_widget_add_css_class(squares[j], "valid_sq");
    else if (valid_capture_sqs.count(i))
      gtk_widget_add_css_class(squares[j], "valid_capture_sq");
  }
  int K_pos = g.board.turn == White ? g.board.Kpos : g.board.kpos;
  if (~K_pos && is_in_check(g.board, g.board.turn)) {
    gtk_widget_add_css_class(squares[flipped(K_pos)], "check_sq");
  }
}

void update_gui() {
  update_board();

  string fen = g.board.to_fen();
  gtk_entry_buffer_set_text(fen_entry_buffer, fen.c_str(), fen.length());
  gtk_label_set_text(
      GTK_LABEL(move_label),
      ("Move: " + to_string(g.ply / 2 + 1) + " " + get_result_str(g.result))
          .c_str());

  if (g.result != Undecided) {
    string result = get_result_str(g.result);
    if (g.draw_type) result += " " + get_draw_type_str(g.draw_type);
    show_statusbar_msg(result);
  }
}

void print_material_count() {
  for (int i = 0; i < 13; i++) {
    if (i == 6) continue;
    cout << piece2char(Piece(i - 6)) << ":" << g.material_count[i] << " ";
  }
  cout << endl;
}

// button events
void first_click() { g.seek(0), update_gui(); }
void prev_click() { g.prev(), update_gui(); }
void next_click() { g.next(), update_gui(); }
void last_click() { g.seek(g.end), update_gui(); }
void flip_click() {
  board_flipped = !board_flipped;
  update_gui();
}
void lichess_click() {
  string fen = g.board.to_fen();
  replace(fen.begin(), fen.end(), ' ', '_');
  cout << "https://lichess.org/analysis/" << fen << endl;
  gtk_show_uri(NULL, ("https://lichess.org/analysis/" + fen).c_str(),
               GDK_CURRENT_TIME);
}
void newgame_click() {
  g.new_game();
  update_gui();
}
void copypgn_click() {
  gdk_clipboard_set_text(gdk_display_get_clipboard(gdk_display_get_default()),
                         g.to_pgn().c_str());
}
void stop_think_click() {
  // TODO: make elegant
  disable_engine = !disable_engine;
  gtk_button_set_label(GTK_BUTTON(stop_think),
                       disable_engine ? "Enable AI" : "Disable AI");
  if (disable_engine) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(eval_bar), 0);
    thinking = false;
    if (ai_thread.joinable()) ai_thread.join();
  } else
    computer_move();
}
void start_game_click() {
  game_running = !game_running;
  gtk_button_set_label(GTK_BUTTON(start_game),
                       game_running ? "Pause Game" : "Start Game");
  if (game_running) {
    g_timeout_add(
        500,
        [](gpointer data) {
          move_manager();
          return game_running ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
        },
        NULL);
  }
}
void fen_apply_click() {
  if (g.load_fen(gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(fen_entry_buffer))))
    show_statusbar_msg("FEN loaded.");
  else
    show_statusbar_msg("Bad FEN.");
  update_gui();
}

void ai_level_change() {
  ai_think_time = gtk_range_get_value(GTK_RANGE(ai_time_scale));
  gtk_widget_set_tooltip_text(ai_time_scale,
                              (to_string(ai_think_time) + " ms").c_str());
}

void computer_move() {
  if (gtk_check_button_get_active(GTK_CHECK_BUTTON(
          g.board.turn == White ? white_randommover : black_randommover))) {
    make_move(g.random_move(), Computer);
  } else if (gtk_check_button_get_active(GTK_CHECK_BUTTON(
                 g.board.turn == White ? white_ai : black_ai))) {
    if (thinking) return;
    if (disable_engine) {
      show_statusbar_msg("Please enable AI.");
      return;
    }
    show_statusbar_msg("Thinking...");
    // start a timer to complete the progress bar after `ai_think_time` ms
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(eval_bar), 0);
    g_timeout_add(
        200,
        [](gpointer data) {
          double frac =
              gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(eval_bar));
          frac += 1.0 / ai_think_time * 200;
          gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(eval_bar), frac);
          return frac >= 1 ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE;
        },
        NULL);
    thinking = true;
    if (ai_thread.joinable()) ai_thread.join();
    ai_thread = thread([&]() {
      auto [move, score] = g.ai_move(ai_think_time);
      thinking = false;
      make_move(move, Computer);
    });
  }
}

bool is_legal(Move m) {
  for (auto &move : generate_legal_moves(g.board))
    if (move.equals(m)) return true;
  return false;
}

void make_move(Move m, MoveType type) {
  if (!is_legal(m)) return;
  if (!g.make_move(m)) return;

  if (type == PreMove) {
    premove = Move();
  }
  if (type == Human) {
    valid_sqs.clear();
    valid_capture_sqs.clear();
  }

  update_gui();
}

void pre_move() {
  if (premove.from == premove.to) return;
  make_move(premove, PreMove);
}

void move_manager() {
  if (g.result != Undecided) {
    start_game_click();
    return;
  }
  computer_move();
  pre_move();
}

void move_intent(int sq) {
  vector<Move> valids;
  for (auto &move : moves)
    if (move.equals(sel_sq, sq)) valids.push_back(move);
  valid_sqs.clear();
  valid_capture_sqs.clear();
  if (valids.size() == 1)  // move is not a promotion
    if (thinking)
      premove = valids.front();
    else
      make_move(valids.front(), Human);
  else if (valids.size() > 1) {  // move is a promotion
    prom_sq = valids.front().to;
    Direction d = prom_sq / 8 == 0 ? S : N;
    for (size_t i = 0; i < valids.size(); i++) {
      int temp_sq = prom_sq + d * i;
      promotions.insert({temp_sq, valids[i]});
      gtk_widget_add_css_class(squares[flipped(temp_sq)], "promotion");
      update_piece(temp_sq, valids[i].promotion);
    }
    gtk_widget_add_css_class(chess_board_grid, "promotion");
    return;
  } else if (sel_sq != sq)
    sel_sq = sq;
  else  // illegal move
    sel_sq = -1;
}

void square_click(GtkWidget *widget, gpointer data) {
  if (g.result != Undecided) return;
  premove.from = premove.to;  // reset premove
  int sq = GPOINTER_TO_INT(data);
  sq = flipped(sq);
  if (~prom_sq) {
    for (auto &x : promotions) {
      if (x.first == sq)
        make_move(x.second, Human);  // clicked on promotion piece
      gtk_widget_remove_css_class(squares[flipped(x.first)], "promotion");
    }
    gtk_widget_remove_css_class(chess_board_grid, "promotion");
    promotions.clear();
    prom_sq = -1;
  } else if (~sel_sq)
    move_intent(sq);
  else
    sel_sq = sq;

  update_gui();
}

GtkWidget *chess_board() {
  chess_board_grid = gtk_grid_new();
  for (int i = 0; i < 64; i++) {
    squares[i] = gtk_button_new();
    g_signal_connect(squares[i], "clicked", G_CALLBACK(square_click),
                     GINT_TO_POINTER(i));
    gtk_button_set_has_frame(GTK_BUTTON(squares[i]), false);
    gtk_widget_add_css_class(squares[i], "piece");
    gtk_grid_attach(GTK_GRID(chess_board_grid), squares[i], i % 8, i / 8, 1, 1);
  }
  gtk_grid_set_column_homogeneous(GTK_GRID(chess_board_grid), true);
  gtk_grid_set_row_homogeneous(GTK_GRID(chess_board_grid), true);
  gtk_widget_set_size_request(chess_board_grid, 600, 600);

  // auto-resize, but keep aspect ratio
  GtkWidget *frame = gtk_aspect_frame_new(0.0, 0.0, 1.0, false);
  gtk_aspect_frame_set_child(GTK_ASPECT_FRAME(frame), chess_board_grid);
  gtk_widget_set_hexpand(frame, true);
  gtk_widget_set_vexpand(frame, true);
  gtk_widget_set_margin_start(chess_board_grid, 50);
  gtk_widget_set_margin_end(chess_board_grid, 50);
  gtk_widget_set_margin_top(chess_board_grid, 50);
  gtk_widget_set_margin_bottom(chess_board_grid, 50);

  gtk_widget_set_name(chess_board_grid, "board");

  return frame;
}

pair<GtkWidget *, GtkWidget *> create_player_chooser() {
  white_human = gtk_check_button_new_with_label("Human");
  white_randommover = gtk_check_button_new_with_label("Random mover");
  white_ai = gtk_check_button_new_with_label("AI");
  gtk_check_button_set_group(GTK_CHECK_BUTTON(white_randommover),
                             GTK_CHECK_BUTTON(white_human));
  gtk_check_button_set_group(GTK_CHECK_BUTTON(white_ai),
                             GTK_CHECK_BUTTON(white_human));
  GtkWidget *white_frame = gtk_frame_new("White"),
            *white_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_append(GTK_BOX(white_box), white_human);
  gtk_box_append(GTK_BOX(white_box), white_randommover);
  gtk_box_append(GTK_BOX(white_box), white_ai);
  gtk_frame_set_child(GTK_FRAME(white_frame), white_box);
  gtk_check_button_set_active(GTK_CHECK_BUTTON(white_human), true);

  black_human = gtk_check_button_new_with_label("Human");
  black_randommover = gtk_check_button_new_with_label("Random mover");
  black_ai = gtk_check_button_new_with_label("AI");
  gtk_check_button_set_group(GTK_CHECK_BUTTON(black_randommover),
                             GTK_CHECK_BUTTON(black_human));
  gtk_check_button_set_group(GTK_CHECK_BUTTON(black_ai),
                             GTK_CHECK_BUTTON(black_human));
  GtkWidget *black_frame = gtk_frame_new("Black"),
            *black_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_append(GTK_BOX(black_box), black_human);
  gtk_box_append(GTK_BOX(black_box), black_randommover);
  gtk_box_append(GTK_BOX(black_box), black_ai);
  gtk_frame_set_child(GTK_FRAME(black_frame), black_box);
  gtk_check_button_set_active(GTK_CHECK_BUTTON(black_ai), true);

  return {white_frame, black_frame};
}

GtkWidget *navigation_buttons() {
  GtkWidget *grid = gtk_grid_new();
  move_label = gtk_label_new("Move 1");
  GtkWidget *first = gtk_button_new_from_icon_name("go-first");
  GtkWidget *prev = gtk_button_new_from_icon_name("go-previous");
  GtkWidget *next = gtk_button_new_from_icon_name("go-next");
  GtkWidget *last = gtk_button_new_from_icon_name("go-last");
  GtkWidget *flip = gtk_button_new_with_label("Flip Board");
  GtkWidget *lichess = gtk_button_new_with_label("Open in LiChess");
  GtkWidget *copypgn = gtk_button_new_with_label("Copy PGN");
  GtkWidget *newgame = gtk_button_new_with_label("New Game");
  stop_think = gtk_button_new_with_label("Disable AI");
  start_game = gtk_button_new_with_label("Start Game");
  eval_bar = gtk_progress_bar_new();
  ai_time_scale =
      gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1000, 10000, 100);
  gtk_range_set_value(GTK_RANGE(ai_time_scale), ai_think_time);
  GtkWidget *ai_time_scale_label = gtk_label_new("AI thinking time (ms)");

  gtk_grid_attach(GTK_GRID(grid), move_label, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), first, 1, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), prev, 2, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), next, 3, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), last, 4, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), flip, 1, 3, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), lichess, 1, 4, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), newgame, 1, 5, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), copypgn, 1, 6, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), stop_think, 1, 7, 4, 1);

  auto [white_frame, black_frame] = create_player_chooser();
  gtk_grid_attach(GTK_GRID(grid), black_frame, 1, 8, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), white_frame, 1, 9, 4, 1);

  gtk_grid_attach(GTK_GRID(grid), eval_bar, 1, 10, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), ai_time_scale_label, 1, 11, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), ai_time_scale, 1, 12, 4, 1);
  gtk_grid_attach(GTK_GRID(grid), start_game, 1, 13, 4, 1);

  g_signal_connect(first, "clicked", G_CALLBACK(first_click), NULL);
  g_signal_connect(prev, "clicked", G_CALLBACK(prev_click), NULL);
  g_signal_connect(next, "clicked", G_CALLBACK(next_click), NULL);
  g_signal_connect(last, "clicked", G_CALLBACK(last_click), NULL);
  g_signal_connect(flip, "clicked", G_CALLBACK(flip_click), NULL);
  g_signal_connect(lichess, "clicked", G_CALLBACK(lichess_click), NULL);
  g_signal_connect(newgame, "clicked", G_CALLBACK(newgame_click), NULL);
  g_signal_connect(copypgn, "clicked", G_CALLBACK(copypgn_click), NULL);
  g_signal_connect(stop_think, "clicked", G_CALLBACK(stop_think_click), NULL);
  g_signal_connect(ai_time_scale, "value-changed", G_CALLBACK(ai_level_change),
                   NULL);
  g_signal_connect(start_game, "clicked", G_CALLBACK(start_game_click), NULL);

  gtk_widget_set_margin_start(grid, 5);

  return grid;
}

GtkWidget *create_grid() {
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), chess_board(), 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), navigation_buttons(), 2, 1, 1, 1);
  GtkWidget *fen_entry = gtk_entry_new();
  fen_entry_buffer = gtk_entry_buffer_new("", 0);
  gtk_entry_set_buffer(GTK_ENTRY(fen_entry), fen_entry_buffer);
  // gtk_entry_set_placeholder_text(GTK_ENTRY(fen_entry), "FEN");
  GtkWidget *fen_expander = gtk_expander_new("FEN");
  GtkWidget *fen_apply = gtk_button_new_from_icon_name("dialog-ok");
  GtkWidget *fen_grid = gtk_grid_new();
  gtk_grid_attach(GTK_GRID(fen_grid), fen_entry, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(fen_grid), fen_apply, 2, 1, 1, 1);
  gtk_expander_set_child(GTK_EXPANDER(fen_expander), fen_grid);
  gtk_grid_attach(GTK_GRID(grid), fen_expander, 1, 2, 2, 1);
  g_signal_connect(fen_apply, "clicked", G_CALLBACK(fen_apply_click), NULL);
  statusbar = gtk_statusbar_new();
  gtk_grid_attach(GTK_GRID(grid), statusbar, 1, 3, 2, 1);
  return grid;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Chess w16");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
  gtk_window_set_icon_name(GTK_WINDOW(window), "chess");
  GtkWidget *grid = create_grid();
  gtk_window_set_child(GTK_WINDOW(window), grid);

  GtkCssProvider *cssProvider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(cssProvider, "interface/style.css");
  gtk_style_context_add_provider_for_display(gtk_widget_get_display(window),
                                             GTK_STYLE_PROVIDER(cssProvider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_window_present(GTK_WINDOW(window));
  update_gui();
}

int main(int argc, char **argv) {
  zobrist_init();
  GtkApplication *app =
      gtk_application_new("org.w16.chess", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}