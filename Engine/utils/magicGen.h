#pragma once
#include <random>
#include "type.h"
#include "attack/magic_instance.h"
#include "bitboard_utilities.h"

class MagicGen{
public:
    static U64 randomU64();
    static U64 randomMagicCandidate();

    static U64 findMagic(Square square, bool bishop);

    static void generateAll();
};