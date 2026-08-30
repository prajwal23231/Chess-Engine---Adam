#pragma once
#include "board/board.h"
#include "utils/type.h"
#include "moves/movegen.h"
#include "moves/move.h"
#include "evaluation/eval.h"


constexpr int INFINITY_SCORE = 300000;
constexpr int MATE_SCORE     = 100000;
constexpr int MATE_THRESHOLD = MATE_SCORE - MAX_PLYS;


class Search{
public:
    Search(Board& board, MoveGenerator& movegen, Evaluator& evaluator);

    Move findBestMove(int depth);

private:
    Board& board;
    MoveGenerator& movegen;
    Evaluator evaluator;

    U64 nodes=0;

    int quiescence(int alpha,int beta, int ply);

    // Move ordering
    int scoreMove(const Move& move);
    void orderMoves(Move* moves, int* scores, int count);
};