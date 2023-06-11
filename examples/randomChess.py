import websocket
import requests
import sys
import random

if len(sys.argv) < 2:
    print("Usage: websocketTest.py <isWhite>")
    sys.exit(1)

isWhite = False
if sys.argv[1] == "white" or sys.argv[1] == "true":
    isWhite = True

# websocket.enableTrace(True)
ws = websocket.WebSocket()
ws.connect("ws://localhost:8000/ws")

def makeMove(move):
    ws.send(move)
    print(move+": "+ws.recv())

gameId = 0

def getAllMoves():
    resp = requests.get(f"http://localhost:8000/game/{gameId}/moves/{'white' if isWhite else 'black'}/list")
    # print(resp.text)
    return resp.json()

gameStartStr = ws.recv()

gameId = int(gameStartStr.split(":")[0])
isWhite = gameStartStr.split(":")[1].strip() == "white"
gameId = 0
if not isWhite:
    print(ws.recv())

while True:
    moves = getAllMoves()
    makeMove(moves[random.randint(0, len(moves) - 1)])

ws.close()
