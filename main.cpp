#include <crow.h>
#include <crow/app.h>
#include <crow/websocket.h>
#include <iostream>

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello, world!";
    });

    CROW_ROUTE(app, "/ws")
        .websocket()
        .onopen([&](crow::websocket::connection& conn) {
                    std::cout << "Connection opened" << std::endl;
                })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
                    std::cout << "Connection closed: " << reason << std::endl;
                })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
                    std::cout << "Message received: " << data << std::endl;
                });

    app.port(8000).run();
}
