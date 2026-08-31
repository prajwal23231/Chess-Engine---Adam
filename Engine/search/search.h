#pragma once
#include "board/board.h"
#include "utils/type.h"
#include "moves/movegen.h"
#include "moves/move.h"
#include "evaluation/eval.h"
#include <chrono>
#include <cstring>

constexpr int INFINITY_SCORE = 300000;
constexpr int MATE_SCORE     = 100000;
constexpr int MATE_THRESHOLD = MATE_SCORE - MAX_PLYS;

using Clock = std::chrono::high_resolution_clock;

class Search {
public:
    Search(Board& board, MoveGenerator& movegen, Evaluator& evaluator);

    Move findBestMove(int depth);

    inline void setMoveTime(long long time) {
        timeLimitMs = time;
    }

    inline void stopSearch() {
        stopped = true;
    }

private:
    Board& board;
    MoveGenerator& movegen;
    Evaluator evaluator;

    U64 nodes = 0;
    long long timeLimitMs = 4000;
    Clock::time_point startTime;
    bool stopped = false;

    // Search heuristics
    Move killerMoves[2][MAX_PLYS];
    int historyTable[2][BOARD_SIZE][BOARD_SIZE];

    int negamax(int alpha, int beta, int depth, int ply);
    int quiescence(int alpha, int beta, int ply);

    // Move ordering
    int scoreMove(const Move& move, int ply);
    void orderMoves(Move* moves, int* scores, int count, int ply);

    inline bool isTimeUp() {
        if (stopped) return true;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - startTime).count() >= timeLimitMs) {
            stopped = true;
            return true;
        }
        return false;
    }
};