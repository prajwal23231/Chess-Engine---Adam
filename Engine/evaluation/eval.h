#pragma once
#include "utils/type.h"

class Board;

constexpr int QUEEN_PHASE  = 4;
constexpr int ROOK_PHASE   = 2;
constexpr int BISHOP_PHASE = 1;
constexpr int KNIGHT_PHASE = 1;
constexpr int TOTAL_PHASE = 24;
constexpr int NUM_STAGE = 2; // middle game and end game

constexpr int BISHOP_PAIR_MG = 25;
constexpr int BISHOP_PAIR_EG = 50;

constexpr int passedPawnMG[RANK_SIZE] = {0, 5, 10, 20, 35, 60, 100, 0};
constexpr int passedPawnEG[RANK_SIZE] = {0, 10, 20, 40, 70, 120, 200, 0};

constexpr int DOUBLED_PAWN_MG = 12;
constexpr int DOUBLED_PAWN_EG = 18;

constexpr int ISOLATED_PAWN_MG = 10;
constexpr int ISOLATED_PAWN_EG = 15;


struct EvalInfo{
    int mg=0;
    int eg=0;

    EvalInfo& operator+=(const EvalInfo& other){
        mg+=other.mg;
        eg+=other.eg;
        return *this;
    }
};


class Evaluator{
public:
    Evaluator();
    int evaluate(const Board& board);

private:
    static int pst[NUM_STAGE][NUM_PIECE_TYPE][BOARD_SIZE];
    static inline bool builtpst = false;
    static void createpst();


    static U64 whitePassedMask[BOARD_SIZE];
    static U64 blackPassedMask[BOARD_SIZE];
    static inline bool builtPassedMask = false;
    static void createPassedMask();


    static U64 fileMask[8];
    static inline bool builtFileMask = false;
    static void createFileMask();



    static U64 isolatedMask[8];
    static inline bool builtIsolatedMask = false;
    static void createIsolatedMask();


    int calculatePhase(const Board& board);
    int interpolate(const EvalInfo& score, int phase);

    void calculateMaterial(const Board& board,EvalInfo& score);
    void calculatePST(const Board& board,EvalInfo& score);
    void calculateBishopPair(const Board& board,EvalInfo& score);
    void calculatePassedPawns(const Board& board,EvalInfo& score);
    void calculateDoubledPawns(const Board& board,EvalInfo& score);
    void calculateIsolatedPawns(const Board& board,EvalInfo& score);
    void calculateMobility(const Board& board,EvalInfo& score);
    void calculateRook(const Board& board, EvalInfo& info);
};