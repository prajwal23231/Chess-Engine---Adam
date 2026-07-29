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

    U64 nodes = 0;
    int legal = moveGen.generateLegalMoves(moves);

    if(depth == 1) return legal;

    for(int i=0; i<legal; i++){
        board.makeMove(moves[i]);
        nodes += run(depth-1);
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