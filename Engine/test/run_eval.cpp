#include "board/board.h"
#include "evaluation/eval.h"
#include "hash/zobrist.h"
#include "utils/tools.h"
#include "attack/attacks.h"
#include <iostream>

using namespace std;

int main() {
    Zobrist::init();
    Tools::initTools();

    Board board;
    string fen = "5r1k/p4ppp/1p2p3/3pP3/1P1n3P/P1r3P1/3N2B1/R4R1K w - - 0 25";
    board.loadFEN(fen);

    Evaluator evaluator;
    int eval = evaluator.evaluate(board);
    cout << "Final evaluate: " << eval << " cp\n";

    return 0;
}
