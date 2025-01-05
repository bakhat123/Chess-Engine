import chess
import chess.engine
import chess.pgn
import argparse

parser = argparse.ArgumentParser(description='UCI compatibility tester')
parser.add_argument('path', type=str, default='stockfish', help='Path to engine')
args = parser.parse_args()

def check_uci(path):
	try:
		engine=chess.engine.SimpleEngine.popen_uci(path)
		# check if engine is UCI compatible
		fen="k7/8/K7/2Q5/8/8/8/8 b - - 0 1"
		result=engine.analyse(chess.Board(fen), chess.engine.Limit())
		engine.quit()
		# check best move
		if result and result["pv"][0] != chess.Move.from_uci("a8b8"):
			print(f"Error: {path}: best move is not a8b8.")
			exit(1)
	except (chess.engine.EngineTerminatedError, chess.engine.EngineError):
		print(f"Error: {path} is not UCI compatible.")
		exit(1)
	

check_uci(args.path)