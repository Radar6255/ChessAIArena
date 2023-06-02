#include <crow.h>
#include <crow/app.h>
#include <crow/http_response.h>
#include <crow/websocket.h>
#include <iostream>
#include <sstream>
#include "Chess.h"
#include "ChessPlayer.h"
#include "Matchmaking.h"

int main() {
    Matchmaking matchmaking;
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello, world!";
    });

    CROW_ROUTE(app, "/ws")
        .websocket()
        .onopen([&](crow::websocket::connection& conn) {
                    std::cout << "Connection opened" << std::endl;
                    // TODO store some data in the connection so we can use if for later in determining which color they are and which game this connection is for
                    /* conn.userdata(); */
                    // Here we want to find a match for this user
                    matchmaking.matchmake(conn);
                })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
                    std::cout << "Connection closed: " << reason << std::endl;
                })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
                    std::cout << "Message received: " << data << std::endl;
                    ChessPlayer *playerData = static_cast<ChessPlayer*>(conn.userdata());

                    bool success = playerData->performMove(data);
                    /* bool success = t.performMove(data, true); */
                    conn.send_text(success ? "true" : "false");
                });

    CROW_ROUTE(app, "/game/<int>/moves/<string>")([&matchmaking](int id, std::string isWhite) {
        std::shared_ptr<Chess> game = matchmaking.getGame(id);
        if (!game){
            return crow::response(404);
        }
        bool white = false;
        if (isWhite == "white") {
            white = true;
        }

        std::stringstream board = game->getPieceLocations(white);

        crow::response out(board.str());
        out.set_header("Content-Type", "application/json");
        return out;
    });

    CROW_ROUTE(app, "/game/<int>")([&matchmaking](int id) {
        std::shared_ptr<Chess> game = matchmaking.getGame(id);
        if (!game){
            return crow::response(404);
        }
        std::stringstream board = game->getBoardState();

        crow::response out(board.str());
        out.set_header("Content-Type", "application/json");
        return out;
    });

    app.port(8000).multithreaded().run();
}
