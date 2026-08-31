#include "uci/uci.h"
#include "board/board.h"
#include "utils/type.h"
#include "attack/attacks.h"
#include "hash/zobrist.h"
#include "utils/tools.h"
#include <iostream>

using namespace std;

int main() {
    Zobrist::init();
    Tools::initTools();

    Board board;
    MoveGenerator movegen(board);
    Evaluator evaluator;

    Search search(board,movegen,evaluator);

    UCI uci(board, movegen, evaluator,search);
    uci.loop();
}