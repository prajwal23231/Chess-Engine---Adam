#include "utils/type.h"

namespace Bitboard{

    // Basic operations
    bool getBit(U64 bb, int square);
    void setBit(U64& bb, int square);
    void clearBit(U64& bb, int square);


    // Bit manipulation helpers
    int popCount(U64 bb);
    int lsb(U64 bb);
    int popLSB(U64& bb);

    // Debugging
    void printBitboard(U64 bb);
}