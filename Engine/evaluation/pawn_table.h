#pragma once
#include "utils/type.h"
#include <vector>

struct PawnTableEntry{
    U64 key = 0;
    int mg = 0;
    int eg = 0;
    U64 passedPawns[2] = {0,0};
    U64 pawnAttacks[2] = {0,0};
};

class PawnTable{
public:
    static constexpr size_t DEFAULT_SIZE = 16384;

    PawnTable(size_t size = DEFAULT_SIZE) : table(size), mask(size-1){}

    void clear() { std::fill(table.begin(),table.end(), PawnTableEntry{}); }

    PawnTableEntry* probe(U64 key){
        PawnTableEntry& entry = table[key&mask];
        return entry.key == key ? &entry : nullptr;
    }

    void store(U64 key,int mg,int eg,U64 wPassed,U64 bPassed,U64 wAttacks,U64 bAttacks){
        PawnTableEntry& entry = table[key & mask];
        entry = {key, mg, eg, {wPassed,bPassed}, {wAttacks, bAttacks}};
    }

private:
    std::vector<PawnTableEntry> table;
    size_t mask;
};