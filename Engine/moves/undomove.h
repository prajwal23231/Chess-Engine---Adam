#pragma once
#include "utils/type.h"

struct UndoInfo{
    int castlingRights;
    Square enpassant;
    int halfMoveClock;
    U64 zobristKey;
    U64 pawnKey;
};