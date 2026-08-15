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
    static U64 generatePawnHash(const Board& board);

    static inline U64 getPieceKeys(Piece p, Square s) { return pieceKeys[p][s]; }
    static inline U64 getCastleKeys(int rights) { return castleKeys[rights]; }
    static inline U64 getEnPassantKeys(int file) { return enPassantKeys[file]; }
    static inline U64 getSideKey() { return sideKey; }
    static inline U64 getPawnKeys(Color c, Square s) { return (c == WHITE) ? pieceKeys[WP][s] : pieceKeys[BP][s]; }

private:
    static U64 pieceKeys[NUM_PIECES][BOARD_SIZE];
    static U64 castleKeys[16];
    static U64 enPassantKeys[RANK_SIZE];
    static U64 sideKey;

    static U64 generateRandom();
    static U64 generateUniqueRandom(std::unordered_set<U64> &st);
};