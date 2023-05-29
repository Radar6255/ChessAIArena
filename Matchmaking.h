#include <crow/websocket.h>
#include <queue>
class Matchmaking {
private:
    std::queue<crow::websocket::connection*> inQueue;
    std::mutex inQueueMutex;
public:
    Matchmaking();
    void matchmake(crow::websocket::connection& conn);
};
