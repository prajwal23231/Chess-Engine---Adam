#pragma once
#include "board/board.h"
#include "utils/type.h"
#include "moves/movegen.h"
#include "moves/move.h"
#include "evaluation/eval.h"


class Search{
public:
    Search(Board& board, MoveGenerator& movegen);

    Move findBestMove(int depth);

private:
    Board& board;
    MoveGenerator& movegen;
};