#pragma once
#include "utils/type.h"
#include "utils/bitboard_utilities.h"
#include <array>
#include <vector>

class Magic{
public:
    Magic();

private:
    std::array<U64, BOARD_SIZE> bishopMasks;
    std::array<U64, BOARD_SIZE> rookMasks;
    std::array<std::vector<U64>, BOARD_SIZE> bishopTable;
    std::array<std::vector<U64>, BOARD_SIZE> rookTable;

    // helper functions
    U64 bishopMask(Square square) const;
    U64 rookMask(Square square) const;
    U64 setOccupancy(int index, int relevamtBits, U64 attackMask) const;

    U64 getBishopAttackOTF(Square s, U64 occupancy) const;
    U64 getRookAttackOTF(Square s, U64 occupancy) const;

    void buildBishopTable();
    void buildRookTable();
};