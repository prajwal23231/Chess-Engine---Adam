#pragma once
#include "utils/type.h"

struct UndoInfo{
    int castlingRights;
    Square enpassant;
    int halfMoveClock;
};