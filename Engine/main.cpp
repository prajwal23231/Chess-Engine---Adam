#include "uci/uci.h"
#include "board/board.h"
#include "utils/type.h"
#include "attack/attacks.h"
#include "utils/zobrist.h"
#include <iostream>

using namespace std;

int main() {
    Zobrist::init();

    Board board;
    MoveGenerator movegen(board);

    UCI uci(board, movegen);
    uci.loop();
}