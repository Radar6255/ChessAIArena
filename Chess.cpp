#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <regex>
#include <utility>
#include <vector>
#include <unordered_set>
#include "Chess.h"

bool isPieceWhite(short piece) {
    bool isWhite = true;
    switch (piece) {
        case BLACK_PAWN:
        case BLACK_ROOK:
        case BLACK_KING:
        case BLACK_QUEEN:
        case BLACK_BISHOP:
        case BLACK_KNIGHT:
            isWhite = false;
    }

    return isWhite;
}

std::pair<short, short> convertToXY(std::string coords) {
    return std::make_pair(coords[1] - '1', coords[0] - 'a');
}

bool Chess::performMove(std::string move, bool isWhite) {
    // First we need to figure out what the move is
    std::smatch sm;
    std::regex re("[KQRBN]?([a-h][1-8])([a-h][1-8])");
    std::regex_match(move, sm, re);

    // The first element is the start square
    std::string start = sm[1].str();
    std::pair<short, short> startXY = convertToXY(start);

    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> moves = getPieceMoves(startXY.first, startXY.second);

    // The second element is the destination
    std::string dest = sm[2].str();
    std::pair<short, short> destXY = convertToXY(dest);

    bool isValid = moves.count(destXY);
    if (!isValid) {
        return false;
    }

    // Now we need to perform the move
    // Probably should have a lock here so no one can read the board in a bad state
    board[destXY.first][destXY.second] = board[startXY.first][startXY.second];
    board[startXY.first][startXY.second] = EMPTY;

    return true;
}

std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> Chess::getPieceMoves(short row, short col) {
    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> out;

    // First thing we need to do is find which piece it is
    short piece = board[row][col];

    bool isWhite = isPieceWhite(piece);
    short forwardDir = isWhite ? 1 : -1;

    switch (piece) {
        case WHITE_PAWN:
        case BLACK_PAWN:
            // Seeing if the pawn can move forward one
            if (row + forwardDir >= 0 && row + forwardDir < 8) {
                if (board[row + forwardDir][col] == EMPTY) {
                    out.insert({row + forwardDir, col});

                    // If the pawn is in the starting row see if it can move two spaces forward
                    if ((isWhite && row == 1) || (!isWhite && row == 6)) {
                        if (board[row + 2 * forwardDir][col] == EMPTY)
                            out.insert({row + 2 * forwardDir, col});
                    }
                }

                // Now we need to check diagonals
                short diagRight = board[row + forwardDir][col + 1];
                if (diagRight != EMPTY && isWhite != isPieceWhite(diagRight)) {
                    out.insert({row + forwardDir, col + 1});
                }

                short diagLeft = board[row + forwardDir][col - 1];
                if (diagLeft != EMPTY && isWhite != isPieceWhite(diagLeft)) {
                    out.insert({row + forwardDir, col - 1});
                }
            }
            // TODO Handle promoting here potentially
            break;
        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            for (int t = 0; t < 2; t++) {
                short x = 1 + t;
                short y = 2 - t;
                for (int i = 0; i < 4; i++) {
                    // Alternating x every iteration
                    x = -x;

                    if (i == 2) {
                        y = -y;
                    }

                    if (row + x < 8 && row + x >= 0 && col + y < 8 && col + y >= 0) {
                        short tmp = board[row + x][col + y];
                        if (tmp == EMPTY || isPieceWhite(tmp) != isWhite) {
                            out.insert({row + x, col + y});
                        }
                    }
                }
            }
            break;
        case WHITE_QUEEN:
        case BLACK_QUEEN:
        case WHITE_ROOK:
        case BLACK_ROOK:
            // Starting by checking row movement going up
            for (int i = row + 1; i < 8; i++) {
                short curPiece = board[i][col];
                if (curPiece == EMPTY) {
                    out.insert({i, col});
                } else if (isPieceWhite(curPiece) != isWhite) {
                    out.insert({i, col});
                    break;
                } else {
                    break;
                }
            }

            // Checking row movement going down
            for (int i = row - 1; i >= 0; i--) {
                short curPiece = board[i][col];
                if (curPiece == EMPTY) {
                    out.insert({i, col});
                } else if (isPieceWhite(curPiece) != isWhite) {
                    out.insert({i, col});
                    break;
                } else {
                    break;
                }
            }

            // Checking column movement going right
            for (int i = col + 1; i < 8; i++) {
                short curPiece = board[row][i];
                if (curPiece == EMPTY) {
                    out.insert({row, i});
                } else if (isPieceWhite(curPiece) != isWhite) {
                    out.insert({row, i});
                    break;
                } else {
                    break;
                }
            }

            // Checking column movement going left
            for (int i = col - 1; i >= 0; i--) {
                short curPiece = board[row][i];
                if (curPiece == EMPTY) {
                    out.insert({row, i});
                } else if (isPieceWhite(curPiece) != isWhite) {
                    out.insert({row, i});
                    break;
                } else {
                    break;
                }
            }

            // This is so we can have the queeen do both rook and bishop moves
            if (piece != WHITE_QUEEN && piece != BLACK_QUEEN) {
                break;
            }
        case WHITE_BISHOP:
        case BLACK_BISHOP: {
            // Looping in the right up direction first
            short curX = row + 1;
            short curY = col + 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                if (board[curX][curY] == EMPTY) {
                    out.insert({curX, curY});
                } else if (isWhite != isPieceWhite(board[curX][curY])) {
                    out.insert({curX, curY});
                    break;
                } else {
                    break;
                }
                curX++;
                curY++;
            }

            curX = row - 1;
            curY = col - 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                if (board[curX][curY] == EMPTY) {
                    out.insert({curX, curY});
                } else if (isWhite != isPieceWhite(board[curX][curY])) {
                    out.insert({curX, curY});
                    break;
                } else {
                    break;
                }
                curX--;
                curY--;
            }

            curX = row - 1;
            curY = col + 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                if (board[curX][curY] == EMPTY) {
                    out.insert({curX, curY});
                } else if (isWhite != isPieceWhite(board[curX][curY])) {
                    out.insert({curX, curY});
                    break;
                } else {
                    break;
                }
                curX--;
                curY++;
            }

            curX = row + 1;
            curY = col - 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                if (board[curX][curY] == EMPTY) {
                    out.insert({curX, curY});
                } else if (isWhite != isPieceWhite(board[curX][curY])) {
                    out.insert({curX, curY});
                    break;
                } else {
                    break;
                }
                curX++;
                curY--;
            }
            break;
        }
        case WHITE_KING:
        case BLACK_KING:
            for (int tx = 0; tx < 2; tx++) {
                for (int ty = 0; ty < 2; ty++) {
                    if (tx == 1 && ty == 1) {
                        continue;
                    }

                    short curPiece = board[row + tx][col + ty];
                    if (curPiece == EMPTY || isWhite != isPieceWhite(curPiece)) {
                        out.insert({row + tx, col + ty});
                    }
                }
            }
            break;
    }

    return out;
}
