import websocket
import requests
import random
import time
import sys

# websocket.enableTrace(True)
ws = websocket.WebSocket()
ws.connect("ws://localhost:8000/ws")

def makeMove(move):
    ws.send(move)
    moveSuccess = ws.recv()
    print(moveSuccess)
    if moveSuccess == "false":
        print(move)
        sys.exit(1)
    print(move+": "+ws.recv())

gameId = 0

def getAllMoves():
    resp = requests.get(f"http://localhost:8000/game/{gameId}/moves/{'white' if isWhite else 'black'}/list")
    return resp.json()

def getBoard():
    resp = requests.get(f"http://localhost:8000/game/{gameId}")

    respJson = resp.json()

    return respJson


def printBoard(board):
    for row in range(8):
    # for row in respJson:
        for col in board[7 - row]:
            if col == "":
                print("|  ", end="")
            else:
                print(f"|{col}", end="")
        print()

def moveToCoords(move):
    return (int(move[1]) - 1, ord(move[0]) - ord('a'), int(move[3]) - 1, ord(move[2]) - ord('a'))

def determineMove(board):
    moves = getAllMoves()
    for move in moves:
        (sx, sy, ex, ey) = moveToCoords(move)

        if board[ex][ey] != "":
            return move
    print(moves)
    print(len(moves))
    return moves[random.randint(0, len(moves) - 1)]

gameStartStr = ws.recv()

gameId = int(gameStartStr.split(":")[0])
isWhite = gameStartStr.split(":")[1].strip() == "white"
# gameId = 0
if not isWhite:
    print(ws.recv())

while True:
    board = getBoard()
    makeMove(determineMove(board))
    printBoard(board)
    if len(sys.argv) > 1:
        time.sleep(float(sys.argv[1]))

ws.close()
