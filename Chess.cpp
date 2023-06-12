#include <algorithm>
#include <crow/websocket.h>
#include <cstring>
#include <iostream>
#include <iterator>
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
            /* whitePieceLocations.insert(std::make_pair(x, y)); */
            /* blackPieceLocations.insert(std::make_pair(7 - x, y)); */
            populatePotentialMoves(x, y);
            populatePotentialMoves(7 - x, y);
        }
    }
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 8; y++) {
            whiteMoves.insert(std::make_pair(std::make_pair(x, y), getValidMoves(x, y)));
            blackMoves.insert(std::make_pair(std::make_pair(7 - x, y), getValidMoves(7 - x, y)));
        }
    }
}

bool Chess::performMove(std::string move, bool isWhite) {
    std::cout << "Performing move: " << move << std::endl;
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

//    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> moves = getPieceMoves(board, startXY.first, startXY.second);
    std::unordered_map<std::pair<short, short>, std::unordered_set<std::pair<short, short>, pair_hash, pair_equal>, pair_hash, pair_equal>* possibleMoves = isWhite ? &whiteMoves : &blackMoves;
    if (!possibleMoves->count(startXY)) {
        return false;
    }
    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> moves = possibleMoves->at(startXY);

    // The second element is the destination
    std::string dest = sm[2].str();
    std::pair<short, short> destXY = convertToXY(dest);

    if (!moves.count(destXY)) {
        return false;
    }

//    bool isValid = moves.count(destXY);
//    if (!isValid) {
//        return false;
//    }

    // Here we are going to check for check
//    unsigned char tmpBoard[8][8];
//    std::memcpy(tmpBoard, board, sizeof(board));
    // Now we need to perform the move
    // Probably should have a lock here so no one can read the board in a bad state
//    tmpBoard[destXY.first][destXY.second] = tmpBoard[startXY.first][startXY.second];
//    tmpBoard[startXY.first][startXY.second] = EMPTY;


//    if (tmpBoard[destXY.first][destXY.second] == WHITE_KING || tmpBoard[destXY.first][destXY.second] == BLACK_KING) {
//        if (moveCreatesCheck(tmpBoard, destXY.first, destXY.second, isWhiteTurn)) {
//            return false;
//        }
//    } else {
//        if (moveCreatesCheck(tmpBoard, startXY.first, startXY.second, isWhiteTurn)) {
//            return false;
//        }
//    }

    /* if (isCheck(tmpBoard, isWhiteTurn)) { */
    /*     return false; */
    /* } */

    performMoveMutex.lock();
    // This could be the case if the user tried to send two moves
    if (isWhiteTurn != isWhite) {
        performMoveMutex.unlock();
        return false;
    }

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            potentialMoves[x][y].erase(startXY);
            potentialMoves[x][y].erase(destXY);
        }
    }

    unsigned char tmpBoard[8][8];
    std::memcpy(tmpBoard, board, sizeof(board));

    // Now we need to perform the move
    // Probably should have a lock here so no one can read the board in a bad state
    // idBoard[destXY.first][destXY.second] = idBoard[startXY.first][startXY.second];
    board[destXY.first][destXY.second] = board[startXY.first][startXY.second];

    // idBoard[startXY.first][startXY.second] = 0;
    board[startXY.first][startXY.second] = EMPTY;

    // Here we are finding the moves that the current player did put the opponent into check
    isBlackInCheck = false;
    isWhiteInCheck = false;
    bool opponentInCheck = false;
    std::pair<short, short> kingPos = getKingPos(board, !isWhiteTurn);
    auto movedPieceMoves = getValidMoves(destXY.first, destXY.second);

    if (isWhite) {
        whiteMoves.erase(startXY);
        blackMoves.erase(destXY);
        whiteMoves.insert(std::make_pair(destXY, movedPieceMoves));

        if (movedPieceMoves.count(kingPos)) {
            isBlackInCheck = true;
            opponentInCheck = true;
        }
    } else {
        blackMoves.erase(startXY);
        whiteMoves.erase(destXY);
        blackMoves.insert(std::make_pair(destXY, movedPieceMoves));
        if (movedPieceMoves.count(kingPos)) {
            isWhiteInCheck = true;
            opponentInCheck = true;
        }
    }

    // Creating a vector of all the pieces that will be affected
    std::vector<std::pair<short, short>> affectedPiecesT;
    std::vector<std::pair<short, short>> affectedPieces;

    // Constructed by finding the pieces that will be affected by the source and destination changing
    auto srcPieces = potentialMoves[startXY.first][startXY.second];
    auto dstPieces = potentialMoves[destXY.first][destXY.second];

    std::vector<std::pair<short, short>> attackedPiecesSrc = getAttackedPieces(tmpBoard, startXY.first, startXY.second);

    this->populatePotentialMoves(destXY.first, destXY.second);

    std::set_union(srcPieces.begin(), srcPieces.end(), dstPieces.begin(), dstPieces.end(), std::inserter(affectedPiecesT, affectedPiecesT.end()));

    std::vector<std::pair<short, short>> attackedPiecesDst = getAttackedPieces(board, destXY.first, destXY.second);

    std::vector<std::pair<short, short>> attackedPieces;
    std::set_union(attackedPiecesDst.begin(), attackedPiecesDst.end(), attackedPiecesSrc.begin(), attackedPiecesSrc.end(), std::inserter(attackedPieces, attackedPieces.end()));

    std::set_union(affectedPiecesT.begin(), affectedPiecesT.end(), attackedPieces.begin(), attackedPieces.end(), std::inserter(affectedPieces, affectedPieces.end()));

    if (move == "d8a5") {
        std::cout << "d8a5" << std::endl;
    }

    for (auto piece : affectedPieces) {
        auto newMoves = getValidMoves(piece.first, piece.second);

        if (isPieceWhite(board[piece.first][piece.second])) {
            whiteMoves.erase(piece);
            whiteMoves.insert(std::make_pair(piece, newMoves));

            if (!isWhiteTurn) {
                if (newMoves.count(kingPos)) {
                    // This means that the opponent's king is in check
                    opponentInCheck = true;
                    isBlackInCheck = true;
                }
            }
        } else {
            blackMoves.erase(piece);
            blackMoves.insert(std::make_pair(piece, newMoves));

            if (isWhiteTurn) {
                if (newMoves.count(kingPos)) {
                    // This means that the opponent's king is in check
                    opponentInCheck = true;
                    isWhiteInCheck = true;
                }
            }
        }
    }

    if (opponentInCheck) {
        auto opponentMoves = isWhiteTurn ? blackMoves : whiteMoves;

        std::for_each(opponentMoves.begin(), opponentMoves.end(), [this](auto &move){
            std::cout << "Updating " << move.first.first << " " << move.first.second << std::endl;
            move.second = getValidMoves(move.first.first, move.first.second);
        });
        /* for (std::pair<std::pair<short, short>, std::unordered_set<std::pair<short, short>, pair_hash, pair_equal>> move : opponentMoves) { */
        /*     std::cout << "Updating " << move.first.first << " " << move.first.second << std::endl; */
            /* opponentMoves->erase(move.first); */
            /* opponentMoves->insert(std::make_pair(move.first, getValidMoves(move.first.first, move.first.second))); */
        /*     move.second = getValidMoves(move.first.first, move.first.second); */
        /* } */

        std::cout << "Updated all opponent moves" << std::endl;

        whiteMoves = opponentMoves;
    }

    isWhiteTurn = !isWhiteTurn;
    performMoveMutex.unlock();
    std::cout << "Performed " << move << std::endl;

    /* std::stringstream moveString; */
    /* moveString << id << ":" << move; */

    /* connections[isWhiteTurn]->send_text(moveString.str()); */
    if (isWhiteTurn) {
        connections[1]->send_text("true");
        connections[0]->send_text(move);
    } else {
        connections[0]->send_text("true");
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

    auto *pieceLocations = isWhite ? &whiteMoves : &blackMoves;

    bool first = true;

    output << "[";
    for (auto pieceMoves = pieceLocations->begin(); pieceMoves != pieceLocations->end(); pieceMoves++) {
        if (first) {
            first = false;
        } else {
            output << ",";
        }

        output << "{\"piece\": \"" << pieceToString(board[pieceMoves->first.first][pieceMoves->first.second]) << "\", \"location\": \"" << stringFromXY(pieceMoves->first) << "\", \"moves\": [";

        bool firstMove = true;
        // Here is where I want to get the valid moves for the current piece
        for (auto move: pieceMoves->second) {
            if (firstMove) {
                firstMove = false;
            } else {
                output << ",";
            }
            output << "\"" << stringFromXY(pieceMoves->first) << stringFromXY(move) << "\"";
        }
        output << "]}";
    }
    output << "]";

    return output;
}

std::stringstream Chess::getPieceLocationsList(bool isWhite) {
    std::stringstream output;

    auto *pieceLocations = isWhite ? &whiteMoves : &blackMoves;

    bool first = true;

    output << "[";
    for (auto pieceMoves = pieceLocations->begin(); pieceMoves != pieceLocations->end(); pieceMoves++) {
        if (pieceMoves->second.size() == 0) {
            continue;
        }

        // Here is where I want to get the valid moves for the current piece
        for (auto move: pieceMoves->second) {
            if (first) {
                first = false;
            } else {
                output << ",";
            }

            output << "\"" << stringFromXY(pieceMoves->first) << stringFromXY(move) << "\"";
        }
    }
    output << "]";

    return output;
}

std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> Chess::getValidMoves(unsigned char x, unsigned char y) {
    // First we get all of the moves
    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> moves = getPieceMoves(board, x, y);
    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> invalidMoves;

    for (std::pair<short, short> move : moves) {
        // TODO This can be made more efficient
        unsigned char tmpBoard[8][8];
        std::memcpy(tmpBoard, board, sizeof(board));

        tmpBoard[move.first][move.second] = tmpBoard[x][y];
        tmpBoard[x][y] = EMPTY;

        if (moveCreatesCheck(tmpBoard, x, y, isPieceWhite(board[x][y]))) {
            std::cout << "INVALID: " << (int)x << " " << (int)y << " " << move.first << " " << move.second << std::endl;
            invalidMoves.insert(move);
        } else {
            /* std::cout << "VALID: " << move.first << " " << move.second << " " << x << " " << y << std::endl; */
        }
    }

    for (std::pair<short, short> move : invalidMoves) {
        moves.erase(move);
    }

    return moves;
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
std::stringstream Chess::getBoardStateBasic() {
    std::stringstream output;

    for (int x = 7; x >= 0; x--) {
        for (int y = 0; y < 8; y++) {
            if (y != 0) {
                output << "|";
            }

            if (board[x][y] == EMPTY) {
                output << "  ";
            } else {
                output << pieceToString(board[x][y]);
            }
        }
        output << "\n";
    }

    return output;
}

std::vector<std::pair<short, short>> Chess::getAttackedPieces(unsigned char board[8][8], short row, short col) {
    std::vector<std::pair<short, short>> out;
    unsigned char piece = board[row][col];

    bool isWhite = isPieceWhite(piece);

    switch (piece) {
        case WHITE_QUEEN:
        case BLACK_QUEEN:
        case WHITE_ROOK:
        case BLACK_ROOK:
            // Starting by checking row movement going up
            for (int i = row + 1; i < 8; i++) {
                short curPiece = board[i][col];
                if (curPiece != EMPTY && isPieceWhite(curPiece) != isWhite) {
                    out.push_back({i, col});
                    break;
                }

                if (curPiece != EMPTY) {
                    break;
                }
            }

            // Checking row movement going down
            for (int i = row - 1; i >= 0; i--) {
                short curPiece = board[i][col];
                if (curPiece != EMPTY && isPieceWhite(curPiece) != isWhite) {
                    out.push_back({i, col});
                    break;
                }

                if (curPiece != EMPTY) {
                    break;
                }
            }

            // Checking column movement going right
            for (int i = col + 1; i < 8; i++) {
                short curPiece = board[row][i];
                if (curPiece != EMPTY && isPieceWhite(curPiece) != isWhite) {
                    out.push_back({row, i});
                    break;
                }

                if (curPiece != EMPTY) {
                    break;
                }
            }

            // Checking column movement going left
            for (int i = col - 1; i >= 0; i--) {
                short curPiece = board[row][i];
                if (curPiece != EMPTY && isPieceWhite(curPiece) != isWhite) {
                    out.push_back({row, i});
                    break;
                }

                if (curPiece != EMPTY) {
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

                if (board[curX][curY] != EMPTY && isWhite != isPieceWhite(board[curX][curY])) {
                    out.push_back({curX, curY});
                    break;
                }
                if (board[curX][curY] != EMPTY) {
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

                if (board[curX][curY] != EMPTY && isWhite != isPieceWhite(board[curX][curY])) {
                    out.push_back({curX, curY});
                    break;
                }
                if (board[curX][curY] != EMPTY) {
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

                if (board[curX][curY] != EMPTY && isWhite != isPieceWhite(board[curX][curY])) {
                    out.push_back({curX, curY});
                    break;
                }
                if (board[curX][curY] != EMPTY) {
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

                if (board[curX][curY] != EMPTY && isWhite != isPieceWhite(board[curX][curY])) {
                    out.push_back({curX, curY});
                    break;
                }
                if (board[curX][curY] != EMPTY) {
                    break;
                }
                curX++;
                curY--;
            }
            break;
        }

    }

    return out;
}

void Chess::populatePotentialMoves(short row, short col) {
    // First thing we need to do is find which piece it is
    unsigned char piece = board[row][col];
    // unsigned char piecePos = idBoard[row][col];
    std::pair<short, short> piecePos = {row, col};

    bool isWhite = isPieceWhite(piece);
    short forwardDir = isWhite ? 1 : -1;

    switch (piece) {
        case WHITE_PAWN:
        case BLACK_PAWN:
            // Seeing if the pawn can move forward one
            if (row + forwardDir >= 0 && row + forwardDir < 8) {
                potentialMoves[row + forwardDir][col].insert(piecePos);

                // If the pawn is in the starting row see if it can move two spaces forward
                if ((isWhite && row == 1) || (!isWhite && row == 6)) {
                    potentialMoves[row + 2 * forwardDir][col].insert(piecePos);
                }

                if (col + 1 < 8) {
                    potentialMoves[row + forwardDir][col + 1].insert(piecePos);
                }
                if (col - 1 >= 0) {
                    potentialMoves[row + forwardDir][col - 1].insert(piecePos);
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
                        potentialMoves[row + x][col + y].insert(piecePos);
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
                potentialMoves[i][col].insert(piecePos);
            }

            // Checking column movement
            for (int i = 0; i < 8; i++) {
                potentialMoves[row][i].insert(piecePos);
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
                potentialMoves[curX][curY].insert(piecePos);

                curX++;
                curY++;
            }

            curX = row - 1;
            curY = col - 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                potentialMoves[curX][curY].insert(piecePos);
                curX--;
                curY--;
            }

            curX = row - 1;
            curY = col + 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                potentialMoves[curX][curY].insert(piecePos);
                curX--;
                curY++;
            }

            curX = row + 1;
            curY = col - 1;
            while(true) {
                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) {
                    break;
                }

                potentialMoves[curX][curY].insert(piecePos);
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
                        potentialMoves[row + tx][col + ty].insert(piecePos);
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
    if (isWhite && isWhiteInCheck) {
        return naiveCheck(board, row, col, isWhite);
    } else if (!isWhite && isBlackInCheck) {
        return naiveCheck(board, row, col, isWhite);
    }

    std::pair<short, short> kingPos;
    kingPos = getKingPos(board, isWhite);

    // Need to find which pieces to check
    std::unordered_set<std::pair<short, short>, pair_hash, pair_equal> checkPieces = potentialMoves[row][col];
    for (std::pair<short, short> piece: checkPieces) {
        // Checking to see if the piece is the opponents since the players own piece can't put them in check
        if (isPieceWhite(board[piece.first][piece.second]) != isWhite) {
            auto moves = getPieceMoves(board, piece.first, piece.second);
            if (moves.count(kingPos) > 0) {
                return true;
            }
        }
    }
    return false;
}

bool Chess::naiveCheck(unsigned char board[8][8], short row, short col, bool isWhite) {
    std::pair<short, short> kingPos;
    kingPos = getKingPos(board, isWhite);

    // Need to find which pieces to check
    auto* opponentPieces = isWhite ? &blackMoves : &whiteMoves;
    for (auto piece: *opponentPieces) {
        auto moves = getPieceMoves(board, piece.first.first, piece.first.second);
        if (moves.count(kingPos) > 0) {
            return true;
        }
    }
    return false;
}

bool Chess::isCheck(unsigned char board[8][8], bool isWhite) {
    /* std::unordered_set<std::pair<short, short>, pair_hash, pair_equal>* oppPieceLocations = isWhite ? &blackPieceLocations : &whitePieceLocations; */
    auto* oppPieceLocations = isWhite ? &blackMoves : &whiteMoves;

    // Need to find where the current players king is
    std::pair<short, short> kingPos = getKingPos(board, isWhite);

    for (auto it = oppPieceLocations->begin(); it != oppPieceLocations->end(); it++) {
        auto moves = getPieceMoves(board, it->first.first, it->first.second);
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
                if (col + 1 < 8) {
                    short diagRight = board[row + forwardDir][col + 1];
                    if (diagRight != EMPTY && isWhite != isPieceWhite(diagRight)) {
                        out.insert({row + forwardDir, col + 1});
                    }
                }

                if (col - 1 >= 0) {
                    short diagLeft = board[row + forwardDir][col - 1];
                    if (diagLeft != EMPTY && isWhite != isPieceWhite(diagLeft)) {
                        out.insert({row + forwardDir, col - 1});
                    }
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

                    if (row + tx > 7 || row + tx < 0 || col + ty > 7 || col + ty < 0) {
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
