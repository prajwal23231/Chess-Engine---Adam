#pragma once
#include "board/board.h"
#include "utils/type.h"
#include "moves/movegen.h"
#include "moves/move.h"
#include "evaluation/eval.h"
#include <chrono>


constexpr int INFINITY_SCORE = 300000;
constexpr int MATE_SCORE     = 100000;
constexpr int MATE_THRESHOLD = MATE_SCORE - MAX_PLYS;

using Clock = std::chrono::high_resolution_clock;

class Search{
public:
    Search(Board& board, MoveGenerator& movegen, Evaluator& evaluator);

    Move findBestMove(int depth);

    inline void setMoveTime(long long time){
        timeLimitMs = time;
    }

    inline void resetMoveTime(){
        timeLimitMs = 5000;
    }

private:
    Board& board;
    MoveGenerator& movegen;
    Evaluator evaluator;

    U64 nodes=0;
    long long timeLimitMs = 5000;

    int negamax(int alpha, int beta, int depth, int ply);
    int quiescence(int alpha, int beta, int ply);

    // Move ordering
    int scoreMove(const Move& move);
    void orderMoves(Move* moves, int* scores, int count);

    inline bool timeLimitReached(auto start) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count() >= timeLimitMs;
    }
};