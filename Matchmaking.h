#ifndef MATCHMAKING_H
#define MATCHMAKING_H

#include "Chess.h"
#include <crow/websocket.h>
#include <cstddef>
#include <queue>
#include <unordered_map>

class Matchmaking {
private:
    std::queue<crow::websocket::connection*> inQueue;
    std::unordered_map<size_t, std::shared_ptr<Chess>> games;
    std::mutex inQueueMutex;
    std::mutex gameWriteMutex;

    size_t nextGameId = 0;
public:
    Matchmaking();
    void matchmake(crow::websocket::connection& conn);
    std::shared_ptr<Chess> getGame(size_t id);
};
#endif
