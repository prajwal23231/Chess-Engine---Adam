#pragma once
#include "utils/type.h"
#include "utils/bitboard_utilities.h"
#include <array>

class Magic{
public:
    Magic();

private:
    std::array<U64, BOARD_SIZE> bishopMasks;
    std::array<U64, BOARD_SIZE> rookMasks;

    // helper functions
    U64 bishopMask(Square square) const;
    U64 rookMask(Square square) const;
};