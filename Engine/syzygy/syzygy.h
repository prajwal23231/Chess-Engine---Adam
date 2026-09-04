#pragma once
#include "board/board.h"
#include "moves/move.h"
#include <string>


enum WDLResult{
    TB_RESULT_FAIL = -1,
    TB_RESULT_LOSS = 0,
    TB_RESULT_DRAW = 2,
    TB_RESULT_WIN = 4
};


class Syzygy{
public:
    static bool init(const std::string& path="Engine/tablebase/wdl;Engine/tablebase/dtz");
    static int getMaxPieces();
    static WDLResult probeWDL(const Board& board);
    static bool probeRoot(const Board& board, Move& bestMove);
};