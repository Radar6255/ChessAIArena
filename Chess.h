#ifndef CHESS_H
#define CHESS_H

#include <atomic>
#include <crow/websocket.h>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <crow.h>
#include <vector>

enum Piece {
    WHITE_PAWN = 0,
    WHITE_KNIGHT = 1,
    WHITE_BISHOP = 2,
    WHITE_ROOK = 3,
    WHITE_QUEEN = 4,
    WHITE_KING = 5,
    BLACK_PAWN = 6,
    BLACK_KNIGHT = 7,
    BLACK_BISHOP = 8,
    BLACK_ROOK = 9,
    BLACK_QUEEN = 10,
    BLACK_KING = 11,
    EMPTY
};

struct pair_hash {
    template<class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2>& p) const {
            // TODO See if I can make a better hash for this
            return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second); 
        }
};

struct pair_equal {
    template<class T1, class T2>
        bool operator()(const std::pair<T1, T2>& p1, const std::pair<T1, T2>& p2) const {
            return p1.first == p2.first && p1.second == p2.second;
        }
};
std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> getPieceMoves(unsigned char board[8][8], short row, short col);

class Chess {
public:
    Chess(crow::websocket::connection* whiteConn, crow::websocket::connection* blackConn, bool whiteFirst, size_t id);
    bool performMove(std::string move, bool isWhite);

    std::stringstream getBoardState();
    std::stringstream getPieceLocations(bool isWhite);
private:
    std::atomic<bool> isWhiteTurn;
    std::mutex performMoveMutex;

    crow::websocket::connection* connections[2];
    size_t id;

    unsigned char board[8][8] = {
        { WHITE_ROOK,   WHITE_KNIGHT,   WHITE_BISHOP,   WHITE_QUEEN,    WHITE_KING,     WHITE_BISHOP,   WHITE_KNIGHT,   WHITE_ROOK },
        { WHITE_PAWN,   WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN },
        { EMPTY,        EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY },
        { EMPTY,        EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY },
        { EMPTY,        EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY },
        { EMPTY,        EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY },
        { BLACK_PAWN,   BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN },
        { BLACK_ROOK,   BLACK_KNIGHT,   BLACK_BISHOP,   BLACK_QUEEN,    BLACK_KING,     BLACK_BISHOP,   BLACK_KNIGHT,   BLACK_ROOK },
    };
    unsigned char idBoard[8][8] = {
        { 1,    2,  3,      4,  5,  6,  7,  8 },
        { 9,    10, 11,     12, 13, 14, 15, 16 },
        { 0,    0,  0,      0,  0,  0,  0,  0 },
        { 0,    0,  0,      0,  0,  0,  0,  0 },
        { 0,    0,  0,      0,  0,  0,  0,  0 },
        { 0,    0,  0,      0,  0,  0,  0,  0 },
        { 17,   18, 19,     20, 21, 22, 23, 24 },
        { 25,   26, 27,     28, 29, 30, 31, 32 },
    };

    std::vector<unsigned char> potentialBoard[8][8];

    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> whitePieceLocations;
    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> blackPieceLocations;

    bool movePossible(std::string pieceName, std::string dest);
    void populatePotentailMoves(short row, short col);
    bool moveCreatesCheck(unsigned char board[8][8], short row, short col, bool isWhite);
    bool isCheck(unsigned char board[8][8], bool isWhite);

    /* std::string stringFromXY(std::pair<short, short> coords); */
};
#endif
