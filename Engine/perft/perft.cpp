#include "perft.h"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace std;


Perft::Perft(Board& board, MoveGenerator& moveGen)
    : board(board), moveGen(moveGen){
}


U64 Perft::run(int depth){
    if(depth == 0) return 1;

    Move moves[MAX_MOVES];
    int count = moveGen.generatePseudoMoves(moves);

    if(depth == 1) return count;

    U64 nodes = 0;
    Color toMove = board.getMovingSide();
    Color opp = (toMove == WHITE ? BLACK : WHITE);

    for (int i = 0; i < count; i++) {
        if (moves[i].isCastle()) {
            MoveFlag f = moves[i].getMoveFlag();
            Piece moved = moves[i].getMovedPiece();
            Square kSq = (moved == WK ? E1 : E8);
            if (board.isSquareAttacked(kSq, opp)) continue;
            Square passSq = (f == kingSideCastle) ? (moved == WK ? F1 : F8) : (moved == WK ? D1 : D8);
            if (board.isSquareAttacked(passSq, opp)) continue;
        }

        board.makeMove(moves[i]);

        U64 kingbb = board.getBitboard((toMove == WHITE) ? WK : BK);
        Square kingPos = static_cast<Square>(Bitboard::lsb(kingbb));

        if (!board.isSquareAttacked(kingPos, opp)) {
            nodes += run(depth - 1);
        }

        board.undoMove(moves[i]);
    }

    return nodes;
}


void Perft::divide(int depth){
    if(depth == 0) return ;

    Move moves[MAX_MOVES];
    int legal = moveGen.generateLegalMoves(moves);

    U64 ways=0;

    for(int i=0; i<legal; i++){
        Square from = moves[i].getFrom();
        Square to = moves[i].getTo();

        board.makeMove(moves[i]);
        U64 cnt = run(depth-1);
        
        ways+=cnt;

        cout << squareToStr[from] << squareToStr[to] << " : " << cnt << "\n";

        board.undoMove(moves[i]);
    }

    cout << "total : " << ways << "\n";
}


void Perft::benchmark(int depth){
    auto start = chrono::high_resolution_clock::now();

    U64 nodes = run(depth);

    auto end = chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;

    double seconds = elapsed.count();
    U64 nps = (seconds > 0.0)
                ? static_cast<U64>(nodes / seconds)
                : 0;

    std::cout << "Depth : " << depth << '\n';
    std::cout << "Nodes : " << nodes << '\n';
    std::cout << "Time  : " << std::fixed << std::setprecision(3) << seconds << " s\n";
    std::cout << "NPS   : " << nps << '\n';
}