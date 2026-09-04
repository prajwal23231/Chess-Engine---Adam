#pragma once
#include "utils/type.h"
#include <array>
#include <cstdint>
#include "utils/bitboard_utilities.h"


class KPKBitbase{
public:
    static void init();
    static bool probe(Square wking, Square pawnSq, Square bking, Color stm);

private:
    static inline std::array<U32, 6144> bitbase = {0};
    static inline bool initialised = false;

    static int encodeIndex(Square wking, Square pawnSq, Square bking, Color stm);

    static inline void setWin(Square wking, Square pawnsq, Square bking, Color stm){
        int idx = encodeIndex(wking, pawnsq, bking, stm);
        bitbase[idx/32] |= (1U<<(idx%32));
    }

    static inline bool isWin(Square wking, Square pawnsq, Square bking, Color stm){
        int idx = encodeIndex(wking, pawnsq, bking, stm);
        return (bitbase[idx/32] >> (idx%32)) & 1;
    }
};