#pragma once
#include "utils/type.h"
#include "utils/bitboard_utilities.h"
#include "magic_instance.h"
#include <array>

class Attacks{
public:
    Attacks();
    
    U64 getKnightAttack(Square square) const;
    U64 getKingAttack(Square square) const;
    U64 getBlackPawnAttack(Square square) const;
    U64 getWhitePawnAttack(Square square) const;

    // for sliding pices
    U64 getBishopAttack(Square square, U64 occupancy) const;
    U64 getQueenAttack(Square square, U64 occupancy) const;
    U64 getRookAttack(Square square, U64 occupancy) const;

private:
    // precoumputed
    std::array<U64,BOARD_SIZE> knightAttack;
    std::array<U64,BOARD_SIZE> kingAttack;
    std::array<U64,BOARD_SIZE> blackPawnAttack;
    std::array<U64,BOARD_SIZE> whitePawnAttack;
};


inline Attacks attacks;