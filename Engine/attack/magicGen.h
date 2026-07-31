#pragma once
#include <random>
#include "utils/type.h"
#include "magic_instance.h"
#include "utils/bitboard_utilities.h"

class MagicGen{
public:
    static U64 randomU64();
    static U64 randomMagicCandidate();

    static U64 findMagic(Square square, bool bishop);

    static void generateAll();
};