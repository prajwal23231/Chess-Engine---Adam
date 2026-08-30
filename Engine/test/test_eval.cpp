#include "board/board.h"
#include "evaluation/eval.h"
#include "hash/zobrist.h"
#include "utils/tools.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;

struct PositionTest {
    string name;
    string fen;
};

void printEval(const string &title, const Board &board, Evaluator &evaluator) {
    int score = evaluator.evaluate(board);
    Color side = board.getMovingSide();

    cout << "=======================================================\n";
    cout << " Position: " << title << "\n";
    cout << "=======================================================\n";
    board.print();
    cout << "\n";
    cout << " Side to move: " << (side == WHITE ? "White" : "Black") << "\n";
    cout << " Game Phase:   " << board.getGamePhase() << " / " << TOTAL_PHASE << "\n";
    cout << " Base MG PSQT: " << board.getMgScore() << " cp\n";
    cout << " Base EG PSQT: " << board.getEgScore() << " cp\n";
    cout << " ---------------------------------------------------\n";
    cout << " Final Eval (Side-to-move perspective): " 
         << (score > 0 ? "+" : "") << score << " cp (" 
         << fixed << setprecision(2) << (score / 100.0) << " pawns)\n";
    cout << " Final Eval (White perspective):        " 
         << (side == WHITE ? (score > 0 ? "+" : "") : (score < 0 ? "+" : "")) 
         << (side == WHITE ? score : -score) << " cp\n";
    cout << "=======================================================\n\n";
}

int main() {
    // Initialize required lookup tables
    Zobrist::init();
    Tools::initTools();

    Board board;
    Evaluator evaluator;

    cout << "\n#######################################################\n";
    cout << "             ADAM CHESS ENGINE - EVALUATION TEST       \n";
    cout << "#######################################################\n\n";

    vector<PositionTest> tests = {
        {
            "Starting Position (Equal)",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
        },
        {
            "White up a Queen",
            "rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
        },
        {
            "White up a Bishop Pair vs Knights",
            "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/2N5/PPPP1PPP/R1BQK1NR w KQkq - 0 1"
        },
        {
            "Passed Pawn on 7th Rank (White Winning)",
            "8/4P3/8/8/8/8/8/4K2k w - - 0 1"
        },
        {
            "Damaged King Pawn Shield (Black Attacking)",
            "r1b2rk1/pp1p1ppp/2n1pn2/8/8/2N1PN2/PPP2PPP/R2QKB1R w KQ - 0 1"
        },
        {
            "Kiwipete Complex Position",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
        }
    };

    for (const auto &test : tests) {
        if (board.loadFEN(test.fen)) {
            printEval(test.name, board, evaluator);
        } else {
            cout << "[ERROR] Failed to parse FEN: " << test.fen << "\n";
        }
    }

    // Symmetry Test: startpos eval should be symmetric from White and Black POV
    cout << "=======================================================\n";
    cout << "                 EVALUATION SYMMETRY TEST              \n";
    cout << "=======================================================\n";

    board.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    int whiteStart = evaluator.evaluate(board);

    board.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    int blackStart = evaluator.evaluate(board);

    cout << " Startpos (White to move): " << whiteStart << " cp\n";
    cout << " Startpos (Black to move): " << blackStart << " cp\n";

    if (whiteStart == blackStart) {
        cout << " [PASS] Symmetry holds perfectly!\n";
    } else {
        cout << " [NOTE] Slight asymmetry: " << (whiteStart - blackStart) << " cp\n";
    }
    cout << "=======================================================\n\n";

    return 0;
}
