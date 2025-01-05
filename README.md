# chess_engine
A UCI chess engine written in C++. GUI is developed using GTK4.


# about
## features
- Board representation
	- Array of 64 pieces
- Search
	- Minimax with α-β pruning
	- Quiescence search
	- Iterative deepening
	- Mate distance pruning
	- Null move pruning
	- Late move reduction
	- Check extension
	- Repetition draw detection using transposition table
- Move ordering
	- Capture, en passant, promotion moves are ordered first
- Evaluation
	- Piece-square tables
	- Material
	- Mop-up evaluation
- Perft
- Time management
	- Time per move
	- Time per game
	- Infinite
- UCI protocol
- GUI
	- Premoves
	- Flip board
	- Promotion
	- Open in LiChess
	- Move navigation
	- FEN board setup

## supported UCI options
None as of now.

## why did I make this?
I wanted to learn more about chess engines and how they work. I also wanted to learn more about C++.

## how did I make this?
I read a lot of articles and watched a few videos on chess programming. I used [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) as a reference time and again. I also spent a lot of time reading the source code of [Stockfish](https://github.com/official-stockfish/Stockfish).

# how to run
Linux:
```bash
make
./main # for UCI engine
./gui  # for GUI chess application
```
