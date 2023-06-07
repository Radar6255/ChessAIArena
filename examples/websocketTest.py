import websocket
import sys

if len(sys.argv) < 2:
    print("Usage: websocketTest.py <isWhite>")
    sys.exit(1)

isWhite = False
if sys.argv[1] == "white" or sys.argv[1] == "true":
    isWhite = True

websocket.enableTrace(True)
ws = websocket.WebSocket()
ws.connect("ws://localhost:8000/ws")

def makeMove(move):
    ws.send(move)
    print(move+": "+ws.recv())

if isWhite:
    print(ws.recv())
    # print("b1a3")
    makeMove("b1a3")
    # ws.send("b1a3")
    # print(ws.recv())
    print(ws.recv())
    # print("c2c4")
    makeMove("c2c4")
    # ws.send("c2c4")
    # print(ws.recv())
    print(ws.recv())
    # print("d2d3")
    makeMove("d2d3")
    # ws.send("d2d3")
    # print(ws.recv())
    print(ws.recv())
    makeMove("e2e4")
    # ws.send("e2e4")
    # print(ws.recv())
    print(ws.recv())
    ws.send("e1e2")
    # print(ws.recv())
    # ws.send("d1b3")
    # print(ws.recv())
else:
    print(ws.recv())
    print(ws.recv())
    makeMove("c7c5")
    # print("c7c5")
    # ws.send("c7c5")
    # print(ws.recv())
    print(ws.recv())
    makeMove("d8a5")
    # print("d8a5")
    # ws.send("d8a5")
    # print(ws.recv())
    print(ws.recv())
    makeMove("a5a6")
    # ws.send("a5a6")
    # print(ws.recv())
    print(ws.recv())



ws.close()
