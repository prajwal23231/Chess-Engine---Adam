#pragma once
#include "utils/type.h"

class Board;

constexpr int QUEEN_PHASE  = 4;
constexpr int ROOK_PHASE   = 2;
constexpr int BISHOP_PHASE = 1;
constexpr int KNIGHT_PHASE = 1;
constexpr int TOTAL_PHASE = 24;


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

    int calculatePhase(const Board& board);
    int interpolate(const EvalInfo& score, int phase);

    void calculateMaterial(const Board& board,EvalInfo& score);
};