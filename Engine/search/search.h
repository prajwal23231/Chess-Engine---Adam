#pragma once
#include "board/board.h"
#include "utils/type.h"
#include "moves/movegen.h"
#include "moves/move.h"
#include "evaluation/eval.h"
#include <chrono>
#include <cstring>
#include "hash/tt.h"

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

    inline TranspositionTable& getTT() { return tt; }

private:
    Board& board;
    MoveGenerator& movegen;
    Evaluator evaluator;
    TranspositionTable tt;

    U64 nodes = 0;
    long long timeLimitMs = 4000;
    Clock::time_point startTime;
    bool stopped = false;

    // Search heuristics
    Move killerMoves[2][MAX_PLYS];
    int historyTable[2][BOARD_SIZE][BOARD_SIZE];

    int negamax(int alpha, int beta, int depth, int ply, bool allowNull = true);
    int quiescence(int alpha, int beta, int ply);

    // Move ordering
    int scoreMove(const Move& move, int ply, const Move& ttMove = Move());
    void orderMoves(Move* moves, int* scores, int count, int ply, const Move& ttMove = Move());

    inline bool isTimeUp() {
        if (stopped) return true;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - startTime).count() >= timeLimitMs) {
            stopped = true;
            return true;
        }
        return false;
    }
};