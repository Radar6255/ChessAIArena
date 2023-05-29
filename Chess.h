#ifndef CHESS_H
#define CHESS_H

#include <iostream>
#include <memory>
#include <string>
#include <regex>
#include <unordered_set>

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

class Chess {
public:
    bool performMove(std::string move, bool isWhite);
private:
    short board[8][8] = {
        { WHITE_ROOK,   WHITE_KNIGHT,   WHITE_BISHOP,   WHITE_QUEEN,    WHITE_KING,     WHITE_BISHOP,   WHITE_KNIGHT,   WHITE_ROOK },
        { WHITE_PAWN,   WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN,     WHITE_PAWN },
        { EMPTY,        EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY },
        { EMPTY,        EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY },
        { EMPTY,        EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY },
        { EMPTY,        EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY,          EMPTY },
        { BLACK_PAWN,   BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN,     BLACK_PAWN },
        { BLACK_ROOK,   BLACK_KNIGHT,   BLACK_BISHOP,   BLACK_QUEEN,    BLACK_KING,     BLACK_BISHOP,   BLACK_KNIGHT,   BLACK_ROOK },
    };
    bool movePossible(std::string pieceName, std::string dest);
    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> getPieceMoves(short row, short col);
};
#endif
