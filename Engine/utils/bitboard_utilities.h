#pragma once
#include "type.h"

namespace Bitboard {

    // Basic bit operations (Inlined for zero function-call overhead)
    inline bool getBit(U64 bb, Square square) {
        return (bb & (1ULL << square)) != 0;
    }

    inline void setBit(U64& bb, Square square) {
        bb |= (1ULL << square);
    }

    inline void clearBit(U64& bb, Square square) {
        bb &= ~(1ULL << square);
    }

    // Bit manipulation helpers (Inlined using compiler intrinsics)
    inline int popCount(U64 bb) {
        return __builtin_popcountll(bb);
    }

    inline int lsb(U64 bb) {
        return bb ? __builtin_ctzll(bb) : -1;
    }

    inline int popLSB(U64& bb) {
        if(!bb) return -1;
        int pos = __builtin_ctzll(bb);
        bb &= (bb - 1);
        return pos;
    }

    // Debugging (Keep out-of-line in .cpp)
    void printBitboard(U64 bb);
}