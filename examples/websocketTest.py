import websocket
import sys

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

if isWhite:
    print(ws.recv())

    makeMove("b1a3")
    print(ws.recv())

    makeMove("c2c4")
    print(ws.recv())

    makeMove("d2d3")
    makeMove("e2e4")
    print(ws.recv())

    makeMove("e1e2")
    print(ws.recv())
else:
    print(ws.recv())
    print(ws.recv())
    makeMove("c7c5")
    print(ws.recv())

    makeMove("d8a5")
    print(ws.recv())

    makeMove("a5a6")
    print(ws.recv())



ws.close()
