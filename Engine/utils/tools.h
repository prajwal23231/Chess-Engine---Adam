#pragma once
#include "type.h"

namespace Tools{
    void initTools();
};


extern U64 between[BOARD_SIZE][BOARD_SIZE];

extern U64 fileMask[8];
extern U64 rankMask[8];

extern U64 whitePassedMask[BOARD_SIZE];
extern U64 blackPassedMask[BOARD_SIZE];

extern U64 whiteOutpostMask[BOARD_SIZE];
extern U64 blackOutpostMask[BOARD_SIZE];

extern U64 adjacentFileMask[8];
extern U64 isolatedMask[8];

extern int pst[NUM_STAGE][NUM_PIECE_TYPE][BOARD_SIZE];