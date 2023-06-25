#ifndef CHESSOBSERVER_H
#define CHESSOBSERVER_H

#include "Chess.h"
#include <memory>

class ChessObserver {
private:
    std::shared_ptr<Chess> chess;
public:
    ChessObserver(std::shared_ptr<Chess> chess) {
        this->chess = chess;
    }
};
#endif
