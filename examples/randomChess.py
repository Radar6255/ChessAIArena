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
    print(resp.text)
    return resp.json()

gameId = ws.recv()[0]
gameId = 0
if not isWhite:
    print(ws.recv())

while True:
    makeMove(getAllMoves()[random.randint(0, len(getAllMoves())-1)])

ws.close()
