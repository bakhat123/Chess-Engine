import chess
import chess.engine
import chess.pgn
import argparse
import time
import tqdm
import os

parser = argparse.ArgumentParser(description='Chess engine tester')
parser.add_argument('--engine1', type=str, default='stockfish', help='Path to engine')
parser.add_argument('--engine2', type=str, default='stockfish', help='Path to engine')
args = parser.parse_args()

# create dir 'games' if not exists
# (in current dir)
os.chdir(os.path.dirname(os.path.abspath(__file__)))
if not os.path.exists('games'):
	os.makedirs('games')

engine1_name=os.path.basename(args.engine1)
engine2_name=os.path.basename(args.engine2)

pgn=open(f"games/{engine1_name} vs {engine2_name} at {time.strftime('%Y-%m-%d %H:%M:%S')}.pgn", "a+")

def match(round,white,black):
	board = chess.Board()
	wtime=0.01
	btime=0.01
	turn = white

	for i in range(100):
		time_limit=wtime if turn == white else btime
		result = turn.play(board, chess.engine.Limit(depth=2))
		if result.move == None:
			break
		board.push(result.move)
		if board.is_game_over():
			break
		turn = black if turn == white else white
		
	# else:
		# print("Game too long: aborted.")
	
	# print(board.result())
	# print(board)


	# save game
	game=chess.pgn.Game.from_board(board)
	game.headers["Event"]="Engine match"
	game.headers["Site"]="Local"
	game.headers["Round"]=round
	# game.headers["Date"]=time.strftime('%Y/%m/%d %H:%M:%S')
	game.headers["White"]=white.id["name"]
	game.headers["Black"]=black.id["name"]
	game.headers["Result"]=board.result()

	print(game, file=pgn, end="\n\n")

	return board.result()


def runall(e1_path=args.engine1, e2_path=args.engine2):
	engine1=chess.engine.SimpleEngine.popen_uci(e1_path)
	engine2=chess.engine.SimpleEngine.popen_uci(e2_path)

	print(engine1.id["name"], "vs", engine2.id["name"])

	scores={
		engine1: [],
		engine2: [],
	}

	white, black = engine1, engine2
	for i in tqdm.trange(10):
		result= match(i+1, white, black)
		if result == "1-0":
			scores[white].append((1,'1'))
			scores[black].append((0,'0'))
		elif result == "0-1":
			scores[white].append((0,'0'))
			scores[black].append((1,'1'))
		elif result == "1/2-1/2":
			scores[white].append((0.5,'½'))
			scores[black].append((0.5,'½'))
		else:
			scores[white].append((0,'*'))
			scores[black].append((0,'*'))
	
	for engine in [engine1, engine2]:
		print(f"{engine.id['name']}:", " ".join(text for score,text in scores[engine]), "=", sum(score for score,text in scores[engine]))

	engine1.quit()
	engine2.quit()


runall(args.engine1, args.engine2)
runall(args.engine2, args.engine1)
