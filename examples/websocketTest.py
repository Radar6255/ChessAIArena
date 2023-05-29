import websocket

ws = websocket.WebSocket()
ws.connect("ws://localhost:8000/ws")

print(ws.recv())
ws.send("b1a3")
print(ws.recv())
print(ws.recv())
ws.send("c2c4")
print(ws.recv())
print(ws.recv())
ws.send("d1b3")
print(ws.recv())
print(ws.recv())
# ws.send("d1b3")
# print(ws.recv())


ws.close()
