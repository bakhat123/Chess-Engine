"""
Converts UCI moves to PGN format

input can be taken from a file or from the command line
optionally, prints the board
"""

import argparse

parser=argparse.ArgumentParser()
parser.add_argument("--file",type=str,help="file containing UCI moves")
parser.add_argument("--print",action="store_true",help="print the board")
args=parser.parse_args()

import chess
import chess.pgn

board=chess.Board()

if args.file:
    with open(args.file) as f:
        moves=f.read().split()
else:
    moves=input().split()

# make the moves
for i in moves:
    board.push_uci(i)

if args.print:
    print(board)

# convert to PGN
game=chess.pgn.Game.from_board(board)
print(game)
