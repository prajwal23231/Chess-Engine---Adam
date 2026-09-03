#include "tt.h"
#include <cstring>
#include <algorithm>

using namespace std;


TranspositionTable::TranspositionTable() :
table(nullptr), numEntries(0), mask(0) {}

TranspositionTable::~TranspositionTable(){
    delete[] table;
}


void TranspositionTable::init(int sizeMB) {
    if(sizeMB < 1) sizeMB = 1;
    delete[] table;

    U64 bytes = static_cast<U64>(sizeMB) * 1024 * 1024;
    numEntries = bytes / sizeof(TTEntry);

    U64 pow2 = 1;

    while((pow2<<1) <= numEntries){
        pow2 <<= 1;
    }

    numEntries = pow2;
    mask = numEntries - 1;

    table = new TTEntry[numEntries]();
}


void TranspositionTable::clear(){
    if(table){
        std::memset(table, 0, numEntries * sizeof(TTEntry));
    }
}


TTEntry* TranspositionTable::probe(U64 key){
    if(!table) return nullptr;

    TTEntry& entry = table[key & mask];
    if(entry.key == key && entry.flag != TT_NONE){
        return &entry;
    }

    return nullptr;
}



void TranspositionTable::store(U64 key, int depth, int score, TTFlag flag, Move bestMove, int ply){
    if(!table) return ;
    TTEntry& entry = table[key & mask];

    if(entry.key != key || depth >= entry.depth || flag == TT_EXACT){
        entry.key = key;
        entry.flag = flag;
        entry.depth = depth;
        entry.score = scoreToTT(score, ply);

        if(bestMove.getValue() != 0 || entry.key != key){
            entry.bestMove = bestMove;
        }
    }
}