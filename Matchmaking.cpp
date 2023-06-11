#include "Chess.h"
#include "ChessPlayer.h"
#include "Matchmaking.h"
#include <crow/websocket.h>
#include <iostream>
#include <mutex>
#include <queue>
#include <memory>
#include <sstream>

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

            gameWriteMutex.lock();
            size_t curGameId = nextGameId;
            std::shared_ptr<Chess> t = std::make_shared<Chess>(&conn, opponent, playerFirst, nextGameId);
            games[nextGameId] = t;
            nextGameId++;
            gameWriteMutex.unlock();

            ChessPlayer *playerData = new ChessPlayer(t, playerFirst);
            ChessPlayer *opponentData = new ChessPlayer(t, !playerFirst);

            conn.userdata(playerData);
            opponent->userdata(opponentData);

            std::stringstream playerString;
            std::stringstream opponentString;

            playerString << curGameId << ":" << (playerFirst ? "white" : "black") << std::endl;
            opponentString << curGameId << ":" << (playerFirst ? "black" : "white") << std::endl;

            opponent->send_text(opponentString.str());
            conn.send_text(playerString.str());
            return;
        }
        inQueueMutex.unlock();
    } else {
        inQueueMutex.lock();
        inQueue.push(&conn);
        inQueueMutex.unlock();
    }
}

std::shared_ptr<Chess> Matchmaking::getGame(size_t id) {
    if (id >= games.size()) {
        return nullptr;
    }

    return games[id];
}
