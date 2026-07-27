#pragma once
#include "utils/type.h"
#include "utils/bitboard_utilities.h"
#include <array>
#include <vector>


struct MagicEntry
{
    U64 mask;
    U64 magic;
    uint8_t shift;
    const U64* attacks;
};


class Magic{
public:
    Magic();

private:
    std::array<MagicEntry, BOARD_SIZE> bishopMagic;
    std::array<MagicEntry, BOARD_SIZE> rookMagic;

    std::vector<U64> bishopTable;
    std::vector<U64> rookTable;

    // helper functions
    U64 bishopMask(Square square) const;
    U64 rookMask(Square square) const;
    U64 setOccupancy(int index, int relevamtBits, U64 attackMask) const;

    U64 getBishopAttackOTF(Square s, U64 occupancy) const;
    U64 getRookAttackOTF(Square s, U64 occupancy) const;

    void buildBishopTable();
    void buildRookTable();
};