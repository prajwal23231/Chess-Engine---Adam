#pragma once
#include <string>
#include <array>
#include <string_view>
#include <cstdint>

using U64 = uint64_t;
using U32 = uint32_t;
using U8 = uint8_t;


constexpr int NUM_PIECES = 12;
constexpr int NUM_COLORS = 3;
constexpr int BOARD_SIZE = 64;
constexpr int RANK_SIZE = 8;
constexpr int MAX_PLYS = 1024;
constexpr int MAX_MOVES = 256;
constexpr int NUM_PIECE_TYPE = 6;


enum Color { WHITE, BLACK, BOTH };

enum Piece {
    EMPTY = -1,

    WP = 0, WN = 1, WR = 2, WB = 3, WQ = 4, WK = 5,
    BP = 6, BN = 7, BR = 8, BB = 9, BQ = 10, BK = 11
};

enum Square {
    NO_SQUARE = -1,
    
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
};

enum Castling { CASTLE_WK = 1, CASTLE_WQ = 2, CASTLE_BK = 4, CASTLE_BQ = 8 };


constexpr int getRank(Square s) {return s/RANK_SIZE;}
constexpr int getFile(Square s) {return s%RANK_SIZE;}


constexpr Square charToSquare(char file, char rank) {
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8')
        return NO_SQUARE;

    int f = file - 'a';
    int r = rank - '1';

    return static_cast<Square>(r * 8 + f);
}


constexpr Piece charToPiece(char c) {
    switch (c) {
        case 'P': return WP;
        case 'N': return WN;
        case 'B': return WB;
        case 'R': return WR;
        case 'Q': return WQ;
        case 'K': return WK;

        case 'p': return BP;
        case 'n': return BN;
        case 'b': return BB;
        case 'r': return BR;
        case 'q': return BQ;
        case 'k': return BK;

        default:  return EMPTY;
    }
}

inline constexpr std::array<std::string_view, 64> squareToStr = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};



enum Stage {
    MG,
    EG
};

enum PieceType {
    PAWN,
    KNIGHT,
    ROOK,
    BISHOP,
    QUEEN,
    KING
};




// =========================
// Evaluation Constants
// =========================
constexpr U64 DARK_SQUARES  = 0xAA55AA55AA55AA55ULL;
constexpr U64 LIGHT_SQUARES = 0x55AA55AA55AA55AAULL;

constexpr int QUEEN_PHASE = 4;
constexpr int ROOK_PHASE = 2;
constexpr int BISHOP_PHASE = 1;
constexpr int KNIGHT_PHASE = 1;
constexpr int TOTAL_PHASE = 24;
constexpr int NUM_STAGE = 2; // middle game and end game

constexpr int piecePhase[NUM_PIECES] = {
    0, KNIGHT_PHASE, ROOK_PHASE, BISHOP_PHASE, QUEEN_PHASE, 0, // WP, WN, WR, WB, WQ, WK
    0, KNIGHT_PHASE, ROOK_PHASE, BISHOP_PHASE, QUEEN_PHASE, 0  // BP, BN, BR, BB, BQ, BK
};

constexpr int BISHOP_PAIR_MG = 25;
constexpr int BISHOP_PAIR_EG = 50;

constexpr int passedPawnMG[RANK_SIZE] = {0, 5, 10, 20, 35, 60, 100, 0};
constexpr int passedPawnEG[RANK_SIZE] = {0, 10, 20, 40, 70, 120, 200, 0};

constexpr int DOUBLED_PAWN_MG = 12;
constexpr int DOUBLED_PAWN_EG = 18;

constexpr int ISOLATED_PAWN_MG = 10;
constexpr int ISOLATED_PAWN_EG = 15;
constexpr int ISOLATED_PAWN_SEMI_OPEN_MG = 8;
constexpr int ISOLATED_PAWN_SEMI_OPEN_EG = 10;

constexpr int rookOpenFile[2] = {20, 10};
constexpr int rookSemiOpenFile[2] = {10, 5};
constexpr int rookSeventhRank[2] = {15, 25};
constexpr int rookBehindOwnPassedPawn[2] = {20, 35};
constexpr int rookBehindEnemyPassedPawn[2] = {10, 20};
constexpr int rookInFrontOwnPassedPawn[2] = {-10, -20};
constexpr int rookInFrontEnemyPassedPawn[2] = {5, 10};
constexpr int connectedRooks[2] = {5, 8};

constexpr int connectedPawnMG[8] = {0, 4, 6, 8, 10, 14, 18, 0};
constexpr int connectedPawnEG[8] = {0, 6, 8, 12, 16, 22, 30, 0};
constexpr int protectedPawnMG[8] = {0, 2, 3, 5, 7, 10, 14, 0};
constexpr int protectedPawnEG[8] = {0, 2, 4, 6, 9, 12, 16, 0};

constexpr int BACKWARD_PAWN_MG = 14;
constexpr int BACKWARD_PAWN_EG = 18;
constexpr int BACKWARD_PAWN_SEMI_OPEN_MG = 20;
constexpr int BACKWARD_PAWN_SEMI_OPEN_EG = 24;

constexpr int knightOutpost[2] = {18, 12};

constexpr int PAWN_SHIELD_MISSING = 25; // Penalty for each missing shield pawn (MG)
constexpr int PAWN_SHIELD_STEPPED = 10; // Penalty if shield pawn moved from rank 2 to rank 3/4
constexpr int OPEN_FILE_NEAR_KING = 20; // Penalty for open file next to the King

constexpr int KNIGHT_ATTACK_WEIGHT = 2;
constexpr int BISHOP_ATTACK_WEIGHT = 2;
constexpr int ROOK_ATTACK_WEIGHT   = 3;
constexpr int QUEEN_ATTACK_WEIGHT  = 5;

// Non-linear king attack danger lookup table (indexed by total attack units 0..99)
constexpr int kingDangerTable[100] = {
      0,   0,   5,  10,  20,  35,  55,  80, 110, 145,
    185, 230, 280, 335, 395, 460, 530, 605, 685, 770,
    860, 955, 1050, 1150, 1250, 1350, 1450, 1550, 1650, 1750,
    1850, 1950, 2050, 2150, 2250, 2350, 2450, 2550, 2650, 2750,
    2850, 2950, 3050, 3150, 3250, 3350, 3450, 3550, 3650, 3750,
    3850, 3950, 4050, 4150, 4250, 4350, 4450, 4550, 4650, 4750,
    4850, 4950, 5050, 5150, 5250, 5350, 5450, 5550, 5650, 5750,
    5850, 5950, 6050, 6150, 6250, 6350, 6450, 6550, 6650, 6750,
    6850, 6950, 7050, 7150, 7250, 7350, 7450, 7550, 7650, 7750,
    7850, 7950, 8050, 8150, 8250, 8350, 8450, 8550, 8650, 8750
};


constexpr int mg_value[6] = { 82, 337, 477, 365, 1025, 0};
constexpr int eg_value[6] = { 94, 281, 512, 297,  936, 0};

constexpr int mg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     98, 134,  61,  95,  68, 126,  34, -11,
     -6,   7,  26,  31,  65,  56,  25, -20,
    -14,  13,   6,  21,  23,  12,  17, -23,
    -27,  -2,  -5,  12,  17,   6,  10, -25,
    -26,  -4,  -4, -10,   3,   3,  33, -12,
    -35,  -1, -20, -23, -15,  24,  38, -22,
      0,   0,   0,   0,   0,   0,   0,   0,
};

constexpr int eg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,
};

constexpr int mg_knight_table[64] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

constexpr int eg_knight_table[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};

constexpr int mg_bishop_table[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

constexpr int eg_bishop_table[64] = {
    -14, -21, -11,  -8,  -7,  -9, -17, -24,
     -8,  -4,   7, -12,  -3, -13,  -4, -14,
      2,  -8,   0,  -1,  -2,   6,   0,   4,
     -3,   9,  12,   9,  14,  10,   3,   2,
     -6,   3,  13,  19,   7,  10,  -3,  -9,
    -12,  -3,   8,  10,  13,   3,  -7, -15,
    -14, -18,  -7,  -1,   4,  -9, -15, -27,
    -23,  -9, -23,  -5,  -9, -16,  -5, -17,
};

constexpr int mg_rook_table[64] = {
     32,  42,  32,  51,  63,   9,  31,  43,
     27,  32,  58,  62,  80,  67,  26,  44,
     -5,  19,  26,  36,  17,  45,  61,  16,
    -24, -11,   7,  26,  24,  35,  -8, -20,
    -36, -26, -12,  -1,   9,  -7,   6, -23,
    -45, -25, -16, -17,   3,   0,  -5, -33,
    -44, -16, -20,  -9,  -1,  11,  -6, -71,
    -19, -13,   1,  17,  16,   7, -37, -26,
};

constexpr int eg_rook_table[64] = {
     13,  10,  18,  15,  12,  12,   8,   5,
     11,  13,  13,  11,  -3,   3,   8,   3,
      7,   7,   7,   5,   4,  -3,  -5,  -3,
      4,   3,  13,   1,   2,   1,  -1,   2,
      3,   5,   8,   4,  -5,  -6,  -8, -11,
     -4,   0,  -5,  -1,  -7, -12,  -8, -16,
     -6, -6,   0,   2,  -9,  -9, -11,  -3,
     -9,   2,   3,  -1,  -5, -13,   4, -20,
};

constexpr int mg_queen_table[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

constexpr int eg_queen_table[64] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

constexpr int mg_king_table[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

constexpr int eg_king_table[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

constexpr int gamephaseInc[6] = {0, 1, 2, 1, 4, 0};

constexpr const int* mgTables[6] = {
    mg_pawn_table, mg_knight_table, mg_rook_table,
    mg_bishop_table, mg_queen_table, mg_king_table
};

constexpr const int* egTables[6] = {
    eg_pawn_table, eg_knight_table, eg_rook_table,
    eg_bishop_table, eg_queen_table, eg_king_table
};



constexpr int knightMobilityMG[9] = {
    -40, -25, -12, -4, 4, 10, 16, 22, 28
};

constexpr int knightMobilityEG[9] = {
    -30, -18, -8, 0, 6, 12, 18, 24, 30
};



constexpr int bishopMobilityMG[14] = {
    -20, -12, -6, -2, 2, 6, 10,
    14, 18, 22, 26, 30, 34, 38
};

constexpr int bishopMobilityEG[14] = {
    -15, -8, -2, 2, 6, 10, 14,
    18, 22, 26, 30, 34, 38, 42
};



constexpr int rookMobilityMG[15] = {
    -15, -8, -2, 2, 6, 10, 14, 18,
    22, 26, 30, 34, 38, 42, 46
};

constexpr int rookMobilityEG[15] = {
    -10, -4, 2, 8, 14, 20, 26, 32,
    38, 44, 50, 56, 62, 68, 74
};



constexpr int queenMobilityMG[28] = {
    -10, -8, -6, -4, -2, 0, 2, 4,
    6, 8, 10, 12, 14, 16,
    18, 20, 22, 24, 26, 28,
    30, 32, 34, 36, 38, 40,
    42, 44
};

constexpr int queenMobilityEG[28] = {
    -8, -6, -4, -2, 0, 2, 4, 6,
    8, 10, 12, 14, 16, 18,
    20, 22, 24, 26, 28, 30,
    32, 34, 36, 38, 40, 42,
    44, 46
};