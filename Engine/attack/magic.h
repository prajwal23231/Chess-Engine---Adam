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
    U64* attacks;
};

class MagicGen;

class Magic{
public:
    friend class MagicGen;
    Magic();

    inline U64 getBishopAttack(Square square, U64 occ) const {
        occ &= bishopMagic[square].mask;
        size_t hash = (occ * bishopMagic[square].magic) >> bishopMagic[square].shift;
        return bishopMagic[square].attacks[hash];
    }

    inline U64 getRookAttack(Square square, U64 occ) const {
        occ &= rookMagic[square].mask;
        size_t hash = (occ * rookMagic[square].magic) >> rookMagic[square].shift;
        return rookMagic[square].attacks[hash];
    }

    inline U64 getBishopMask(Square s) const { return bishopMagic[s].mask; }
    inline U64 getRookMask(Square s) const { return rookMagic[s].mask; }

private:
    std::array<MagicEntry, BOARD_SIZE> bishopMagic;
    std::array<MagicEntry, BOARD_SIZE> rookMagic;

    std::vector<U64> bishopTable;
    std::vector<U64> rookTable;

    // helper functions
    U64 bishopMask(Square square) const;
    U64 rookMask(Square square) const;
    U64 setOccupancy(int index, int relevantBits, U64 attackMask) const;

    U64 getBishopAttackOTF(Square s, U64 occupancy) const;
    U64 getRookAttackOTF(Square s, U64 occupancy) const;

    void buildBishopTable();
    void buildRookTable();

    void validate() const;
};