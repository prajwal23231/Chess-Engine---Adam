#pragma once
#include "utils/type.h"
#include "moves/move.h"
#include <cstring>
#include <algorithm>

enum TTFlag : uint8_t {
    TT_NONE = 0,
    TT_EXACT = 1,
    TT_LOWER = 2,
    TT_UPPER = 3
};


struct TTEntry {
    U64 key;
    int score;
    int depth;
    TTFlag flag;
    Move bestMove;
};


class TranspositionTable {
public:
    TranspositionTable();
    ~TranspositionTable();

    void init(int sizeMB);
    void clear();

    TTEntry* probe(U64 key);
    void store(U64 key, int depth, int score, TTFlag flag, Move bestMove, int ply);

    static inline int scoreToTT(int score, int ply) {
        if (score > MATE_THRESHOLD) return score + ply;
        if (score < -MATE_THRESHOLD) return score - ply;
        return score;
    }


    static inline int scoreFromTT(int score, int ply) {
        if (score > MATE_THRESHOLD) return score - ply;
        if (score < -MATE_THRESHOLD) return score + ply;
        return score;
    }

private:
    TTEntry* table;
    U64 numEntries;
    U64 mask;
};