import chess
import chess.engine
import chess.pgn
import argparse

from tqdm import tqdm

mates_epd = "mates.epd"

parser = argparse.ArgumentParser(description='Mate tester')
parser.add_argument('path', type=str, default='stockfish', help='Path to engine')
parser.add_argument('line0', type=int, default='0', help='Line to begin at')
args = parser.parse_args()

def play_till_end(fen, mate_in, engine, stockfish):
	moves_made = 0
	moves=[]
	mated=False
	board = chess.Board(fen)
	while moves_made < mate_in:
		result = engine.play(board, chess.engine.Limit(depth=mate_in+1-moves_made))
		board.push(result.move)
		moves.append(result.move)
		moves_made += 1
		if board.is_game_over():
			mated=True
			break
		result = stockfish.play(board, chess.engine.Limit(depth=mate_in+1-moves_made))
		board.push(result.move)
		moves.append(result.move)
		if board.is_game_over():
			mated=True
			break
	return mated, moves

def test_mate(path,line0=0):
	engine=chess.engine.SimpleEngine.popen_uci(path)
	stockfish = chess.engine.SimpleEngine.popen_uci("stockfish")
	
	lines=open(mates_epd).readlines()

	solved=line0
	for i in tqdm(range(len(lines))):
		if i < solved:
			continue
		line = lines[i]
		fen, mate_in = line.strip().split("; M")
		mate_in = int(mate_in)
		mated0, moves0 = play_till_end(fen, mate_in, stockfish, stockfish)
		mated1, moves1 = play_till_end(fen, mate_in, engine, stockfish)

		if mated0 and not mated1:
			print(f"# {solved+1} Failed")
			print("Mate in", mate_in)
			print(fen)
			print("Expected:", [str(x) for x in moves0])
			print("Got:", [str(x) for x in moves1])
			break
		solved += 1

	
	engine.quit()
	stockfish.quit()
	

test_mate(args.path, args.line0)