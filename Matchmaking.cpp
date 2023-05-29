#include "Chess.h"
#include "ChessPlayer.h"
#include "Matchmaking.h"
#include <crow/websocket.h>
#include <iostream>
#include <mutex>
#include <queue>
#include <memory>

Matchmaking::Matchmaking() {
    std::cout << "Matchmaking started" << std::endl;
}

void Matchmaking::matchmake(crow::websocket::connection& conn) {
    if (inQueue.size() > 0) {
        inQueueMutex.lock();
        if (inQueue.size() > 0) {
            crow::websocket::connection* opponent = inQueue.front();
            inQueue.pop();
            inQueueMutex.unlock();

            // TODO Flip coin to determine who goes first
            bool playerFirst = false;
            std::shared_ptr<Chess> t = std::make_shared<Chess>(&conn, opponent, playerFirst);

            ChessPlayer *playerData = new ChessPlayer(t, playerFirst);
            ChessPlayer *opponentData = new ChessPlayer(t, !playerFirst);


            opponent->userdata(opponentData);
            conn.userdata(playerData);

            opponent->send_text(playerFirst ? "black" : "white");
            conn.send_text(playerFirst ? "white" : "black");
            return;
        }
        inQueueMutex.unlock();
    } else {
        inQueueMutex.lock();
        inQueue.push(&conn);
        inQueueMutex.unlock();
    }
}
