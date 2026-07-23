#pragma once

#include <array>
#include <string>
#include <cstdint>

using U64 = uint64_t;

enum Color { WHITE, BLACK, BOTH };

enum Piece {
    EMPTY = -1,

    WP = 0, WN = 1, WR = 2, WB = 3, WQ = 4, WK = 5,
    BP = 6, BN = 7, BR = 8, BB = 9, BQ = 10, BK = 11
};

enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,

    NO_SQUARE
};

enum Castling { CASTLE_WK = 1, CASTLE_WQ = 2, CASTLE_BK = 4, CASTLE_BQ = 8 };

constexpr int NUM_PIECES = 12;
constexpr int NUM_SQUARES = 64;
constexpr int NUM_COLORS = 3;

class Board {
public:
    Board();

    void clear();
    void setStartingPosition();
    void updateOccupancies();
    void print() const;
    bool loadFEN(const std::string &fen);

private:
    std::array<U64, NUM_PIECES> bitboards;
    std::array<Piece, NUM_SQUARES> board;
    std::array<U64, NUM_COLORS> occupancies;

    Color sideToMove;
    int castlingRights;

    Square enPassant;

    int halfmoveClock;
    int fullmoveNumber;
    
    Piece charToPiece(char c);
    Square parseEnPassantSquare(char pos,int rank,Color tomove);
    void rebuildBitboards();
};