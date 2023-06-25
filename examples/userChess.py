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
    print("Success: "+moveSuccess)
    if moveSuccess != "false":
        print("Response: "+ws.recv())

gameId = 0

def getAllMoves():
    resp = requests.get(f"http://localhost:8000/game/{gameId}/moves/{'white' if isWhite else 'black'}/list")
    return resp.json()

def printBoard():
    resp = requests.get(f"http://localhost:8000/game/{gameId}")

    respJson = resp.json()

    print("  | A| B| C| D| E| F| G| H|")
    for row in range(8):
        print(str(8 - row) + " ", end="")
        for col in respJson[7 - row]:
            if col == "":
                print("|  ", end="")
            else:
                print(f"|{col}", end="")
        print("| " + str(8 - row))
    print("  | A| B| C| D| E| F| G| H|")

gameStartStr = ws.recv()

gameId = int(gameStartStr.split(":")[0])
isWhite = gameStartStr.split(":")[1].strip() == "white"
# gameId = 0
if not isWhite:
    print(ws.recv())

printBoard()
while True:
    moves = getAllMoves()
    makeMove(input("Move: ").strip())
    printBoard()

ws.close()
