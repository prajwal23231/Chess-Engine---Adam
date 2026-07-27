#pragma once
#include "type.h"

namespace Bitboard{

    // Basic operations
    bool getBit(U64 bb, Square square);
    void setBit(U64& bb, Square square);
    void clearBit(U64& bb, Square square);


    // Bit manipulation helpers
    int popCount(U64 bb);
    int lsb(U64 bb);
    int popLSB(U64& bb);

    // Debugging
    void printBitboard(U64 bb);
}