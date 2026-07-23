#include "uci/uci.h"
#include "board/board.h"
#include <iostream>

int main() {
    // UCI uci;
    // uci.loop();

    Board board;
    board.setStartingPosition();
    board.print();

    return 0;
}