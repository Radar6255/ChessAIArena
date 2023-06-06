#include <algorithm>
#include <crow/websocket.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
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

std::string pieceToString(short piece) {
    switch (piece) {
        case BLACK_PAWN:
            return "BP";
        case BLACK_ROOK:
            return "BR";
        case BLACK_KING:
            return "BK";
        case BLACK_QUEEN:
            return "BQ";
        case BLACK_BISHOP:
            return "BB";
        case BLACK_KNIGHT:
            return "BN";
        case WHITE_PAWN:
            return "WP";
        case WHITE_ROOK:
            return "WR";
        case WHITE_KING:
            return "WK";
        case WHITE_QUEEN:
            return "WQ";
        case WHITE_BISHOP:
            return "WB";
        case WHITE_KNIGHT:
            return "WN";
        default:
            return "";
    }
}

std::pair<short, short> convertToXY(std::string coords) {
    return std::make_pair(coords[1] - '1', coords[0] - 'a');
}

Chess::Chess(crow::websocket::connection* whiteConn, crow::websocket::connection* blackConn, bool whiteFirst, size_t id) {
    if (whiteFirst) {
        connections[0] = whiteConn;
        connections[1] = blackConn;
    }
    connections[0] = blackConn;
    connections[1] = whiteConn;

    isWhiteTurn = true;
    this->id = id;

    // Now we need to setup each players piece locations
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 8; y++) {
            whitePieceLocations.insert(std::make_pair(x, y));
            blackPieceLocations.insert(std::make_pair(7 - x, y));
            populatePotentailMoves(x, y);
            populatePotentailMoves(7 - x, y);
        }
    }
}

bool Chess::performMove(std::string move, bool isWhite) {
    if (isWhiteTurn != isWhite) {
        return false;
    }
    // First we need to figure out what the move is
    std::smatch sm;
    std::regex re("[KQRBN]?([a-h][1-8])([a-h][1-8])");
    std::regex_match(move, sm, re);

    // The first element is the start square
    std::string start = sm[1].str();
    std::pair<short, short> startXY = convertToXY(start);

    // Checking to see if a piece is there or isn't their piece
    if (board[startXY.first][startXY.second] == EMPTY || isPieceWhite(board[startXY.first][startXY.second]) != isWhite) {
        return false;
    }

    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> moves = getPieceMoves(board, startXY.first, startXY.second);

    // The second element is the destination
    std::string dest = sm[2].str();
    std::pair<short, short> destXY = convertToXY(dest);

    bool isValid = moves.count(destXY);
    if (!isValid) {
        return false;
    }

    // Here we are going to check for check
    unsigned char tmpBoard[8][8];
    std::memcpy(tmpBoard, board, sizeof(board));
    // Now we need to perform the move
    // Probably should have a lock here so no one can read the board in a bad state
    tmpBoard[destXY.first][destXY.second] = tmpBoard[startXY.first][startXY.second];
    tmpBoard[startXY.first][startXY.second] = EMPTY;


    if (tmpBoard[destXY.first][destXY.second] == WHITE_KING || tmpBoard[destXY.first][destXY.second] == BLACK_KING) {
        if (moveCreatesCheck(tmpBoard, destXY.first, destXY.second, isWhiteTurn)) {
            return false;
        }
    } else {
        if (moveCreatesCheck(tmpBoard, startXY.first, startXY.second, isWhiteTurn)) {
            return false;
        }
    }

    /* if (isCheck(tmpBoard, isWhiteTurn)) { */
    /*     return false; */
    /* } */

    performMoveMutex.lock();
    // This could be the case if the user tried to send two moves
    if (isWhiteTurn != isWhite) {
        performMoveMutex.unlock();
        return false;
    }

    unsigned char movedPiece = idBoard[startXY.first][startXY.second];
    unsigned char takenPiece = idBoard[destXY.first][destXY.second];
    if (!takenPiece) {
        takenPiece = movedPiece;
    }

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            for (auto piece = potentialBoard[x][y].begin(); piece != potentialBoard[x][y].end(); piece++) {
                if (!(*piece)) {
                    break;
                }
                if (*piece == movedPiece || *piece == takenPiece) {
                    potentialBoard[x][y].erase(piece);
                }
            }
        }
    }

    // Now we need to perform the move
    // Probably should have a lock here so no one can read the board in a bad state
    idBoard[destXY.first][destXY.second] = idBoard[startXY.first][startXY.second];
    board[destXY.first][destXY.second] = board[startXY.first][startXY.second];

    idBoard[startXY.first][startXY.second] = 0;
    board[startXY.first][startXY.second] = EMPTY;

    if (isWhite) {
        whitePieceLocations.erase(startXY);
        blackPieceLocations.erase(destXY);
        whitePieceLocations.insert(destXY);
    } else {
        blackPieceLocations.erase(startXY);
        whitePieceLocations.erase(destXY);
        blackPieceLocations.insert(destXY);
    }

    this->populatePotentailMoves(destXY.first, destXY.second);

    isWhiteTurn = !isWhiteTurn;
    performMoveMutex.unlock();
    std::cout << "Performed " << move << std::endl;

    /* std::stringstream moveString; */
    /* moveString << id << ":" << move; */

    /* connections[isWhiteTurn]->send_text(moveString.str()); */
    if (isWhiteTurn) {
        connections[0]->send_text(move);
    } else {
        connections[1]->send_text(move);
    }

    return true;
}

std::string stringFromXY(std::pair<short, short> coords) {
    std::stringstream out;
    out << (char) (coords.second + 'a') << coords.first + 1;
    return out.str();
}

std::stringstream Chess::getPieceLocations(bool isWhite) {
    std::stringstream output;

    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> *pieceLocations = isWhite ? &whitePieceLocations : &blackPieceLocations;

    bool first = true;

    output << "[";
    for (auto it = pieceLocations->begin(); it != pieceLocations->end(); it++) {
        if (first) {
            first = false;
        } else {
            output << ",";
        }

        output << "\"" << stringFromXY(*it) << "\"";
    }
    output << "]";

    return output;
}

std::stringstream Chess::getBoardState() {
    std::stringstream output;

    output << "[";
    for (int x = 0; x < 8; x++) {
        if (x != 0) {
            output << ",";
        }
        output << "[";
        for (int y = 0; y < 8; y++) {
            if (y != 0) {
                output << ",";
            }

            output << "\"" << pieceToString(board[x][y]) << "\"";
        }
        output << "]";
    }
    output << "]";

    return output;
}

void Chess::populatePotentailMoves(short row, short col) {
    // First thing we need to do is find which piece it is
    unsigned char piece = board[row][col];
    unsigned char curPieceId = idBoard[row][col];

    bool isWhite = isPieceWhite(piece);
    short forwardDir = isWhite ? 1 : -1;

    switch (piece) {
        case WHITE_PAWN:
        case BLACK_PAWN:
            // Seeing if the pawn can move forward one
            if (row + forwardDir >= 0 && row + forwardDir < 8) {
                potentialBoard[row + forwardDir][col].push_back(curPieceId);

                // If the pawn is in the starting row see if it can move two spaces forward
                if ((isWhite && row == 1) || (!isWhite && row == 6)) {
                    potentialBoard[row + 2 * forwardDir][col].push_back(curPieceId);
                }

                if (col + 1 < 8) {
                    potentialBoard[row + forwardDir][col + 1].push_back(curPieceId);
                }
                if (col - 1 >= 0) {
                    potentialBoard[row + forwardDir][col - 1].push_back(curPieceId);
                }
            }
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
                        potentialBoard[row + x][col + y].push_back(curPieceId);
                    }
                }
            }
            break;
        case WHITE_QUEEN:
        case BLACK_QUEEN:
        case WHITE_ROOK:
        case BLACK_ROOK:
            // Starting by checking row movement
            for (int i = 0; i < 8; i++) {
                potentialBoard[i][col].push_back(curPieceId);
            }

            // Checking column movement
            for (int i = 0; i < 8; i++) {
                potentialBoard[row][i].push_back(curPieceId);
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
                potentialBoard[curX][curY].push_back(curPieceId);

                curX++;
                curY++;
            }

            curX = row - 1;
            curY = col - 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                potentialBoard[curX][curY].push_back(curPieceId);
                curX--;
                curY--;
            }

            curX = row - 1;
            curY = col + 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                potentialBoard[curX][curY].push_back(curPieceId);
                curX--;
                curY++;
            }

            curX = row + 1;
            curY = col - 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                potentialBoard[curX][curY].push_back(curPieceId);
                curX++;
                curY--;
            }
            break;
        }
        case WHITE_KING:
        case BLACK_KING:
            for (short tx = -1; tx < 1; tx++) {
                for (short ty = -1; ty < 1; ty++) {
                    if (tx == 0 && ty == 0) {
                        continue;
                    }

                    if (row + tx < 8 && row + tx >= 0 && col + ty < 8 && col + ty >= 0) {
                        potentialBoard[row + tx][col + ty].push_back(curPieceId);
                    }
                }
            }
            break;
    }
}

std::pair<short, short> getKingPos(unsigned char board[8][8], bool isWhite) {
    std::pair<short, short> kingPos;

    for (unsigned char x = 0; x < 8; x++){
        for (unsigned char y = 0; y < 8; y++){
            unsigned short piece = board[x][y];
            if ((isWhite && piece == WHITE_KING) || (!isWhite && piece == BLACK_KING)) {
                return std::make_pair(x, y);
            }
        }
    }

    return std::make_pair(-1, -1);
}

bool Chess::moveCreatesCheck(unsigned char board[8][8], short row, short col, bool isWhite) {
    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> pieceLocations = isWhite ? whitePieceLocations : blackPieceLocations;

    std::pair<short, short> kingPos;
    kingPos = getKingPos(board, isWhite);

    /* for (auto it = pieceLocations.begin(); it != pieceLocations.end(); it++) { */
    /*     unsigned short piece = board[it->first][it->second]; */
    /*     if (piece == WHITE_KING || piece == BLACK_KING) { */
    /*         kingPos = *it; */
    /*         break; */
    /*     } */
    /* } */

    // Need to find which pieces to check
    std::vector<unsigned char> checkPieces = potentialBoard[row][col];
    for (unsigned char piece: checkPieces) {
        // TODO Find if the piece that we are looking up is the current players piece or not
        // Have the piece id, now need to find it's current location
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                if (std::find(checkPieces.begin(), checkPieces.end(), idBoard[x][y]) != checkPieces.end()) {
                    // We are at the location of one of the pieces
                    auto moves = getPieceMoves(board, x, y);
                    if (moves.count(kingPos) > 0) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Chess::isCheck(unsigned char board[8][8], bool isWhite) {
    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal>* oppPieceLocations = isWhite ? &blackPieceLocations : &whitePieceLocations;

    // Need to find where the current players king is
    std::pair<short, short> kingPos = getKingPos(board, isWhite);

    for (auto it = oppPieceLocations->begin(); it != oppPieceLocations->end(); it++) {
        auto moves = getPieceMoves(board, it->first, it->second);
        if (moves.count(kingPos) > 0) {
            return true;
        }
    }

    return false;
}

std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> getPieceMoves(unsigned char board[8][8], short row, short col) {
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
