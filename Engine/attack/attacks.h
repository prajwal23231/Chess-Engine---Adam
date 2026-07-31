#pragma once
#include "utils/type.h"
#include "utils/bitboard_utilities.h"
#include "magic_instance.h"
#include <array>

class Attacks{
public:
    Attacks();
    
    inline U64 getKnightAttack(Square square) const { return knightAttack[square]; }
    inline U64 getKingAttack(Square square) const { return kingAttack[square]; }
    inline U64 getBlackPawnAttack(Square square) const { return blackPawnAttack[square]; }
    inline U64 getWhitePawnAttack(Square square) const { return whitePawnAttack[square]; }

    // for sliding pieces
    inline U64 getBishopAttack(Square square, U64 occupancy) const {
        return g_magic.getBishopAttack(square, occupancy);
    }

    inline U64 getRookAttack(Square square, U64 occupancy) const {
        return g_magic.getRookAttack(square, occupancy);
    }

    inline U64 getQueenAttack(Square square, U64 occupancy) const {
        return g_magic.getBishopAttack(square, occupancy) | g_magic.getRookAttack(square, occupancy);
    }

private:
    // precomputed
    std::array<U64,BOARD_SIZE> knightAttack;
    std::array<U64,BOARD_SIZE> kingAttack;
    std::array<U64,BOARD_SIZE> blackPawnAttack;
    std::array<U64,BOARD_SIZE> whitePawnAttack;
};


inline Attacks attacks;