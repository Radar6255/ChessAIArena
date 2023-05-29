#ifndef CHESSPLAYER_H
#define CHESSPLAYER_H

#include "Chess.h"
#include <memory>

class ChessPlayer {
private:
    bool isWhite;
    std::shared_ptr<Chess> chess;
public:
    ChessPlayer(std::shared_ptr<Chess> chess, bool isWhite) {
        this->isWhite = isWhite;
        this->chess = chess;
    }

    bool performMove(std::string move) {
        return this->chess->performMove(move, this->isWhite);
    }
};
#endif
