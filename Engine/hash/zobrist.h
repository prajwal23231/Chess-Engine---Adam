#pragma once
#include "utils/type.h"
#include <unordered_set>

// 16 -> castling rights is stored as 4 bits

class Board;

class Zobrist{
public:
    Zobrist() = delete;
    static void init();
    static U64 generateHash(const Board& board);

    static U64 getPieceKeys(Piece p, Square s);
    static U64 getCastleKeys(int right);
    static U64 getEnPassantKeys(int rank);
    static U64 getSideKey();

private:
    static U64 pieceKeys[NUM_PIECES][BOARD_SIZE];
    static U64 castleKeys[16];
    static U64 enPassantKeys[RANK_SIZE];
    static U64 sideKey;

    static U64 generateRandom();
    static U64 generateUniqueRandom(std::unordered_set<U64> &st);
};