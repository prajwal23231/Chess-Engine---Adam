#pragma once
#include "utils/type.h"
#include <array>
#include "board/board.h"

class Attacks{
public:
    Attacks();
    
    U64 getKnightAttack(Square square);
    U64 getKingAttack(Square square);
    U64 getBlackPawnAttack(Square square);
    U64 getWhitePawnAttack(Square square);

    // for sliding pices
    U64 getBishopAttack(Square square, U64 occupancy);
    U64 getQueenAttack(Square square, U64 occupancy);
    U64 getRookAttack(Square square, U64 occupancy);

private:
    // precoumputed
    std::array<U64,BOARD_SIZE> knightAttack;
    std::array<U64,BOARD_SIZE> kingAttack;
    std::array<U64,BOARD_SIZE> blackPawnAttack;
    std::array<U64,BOARD_SIZE> whitePawnAttack;
};