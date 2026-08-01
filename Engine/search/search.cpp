#include "search.h"


Search::Search(Board& board, MoveGenerator& movegen) : board(board), movegen(movegen){
}



Move Search::findBestMove(int depth){
    if(depth == 0 ) return {NO_SQUARE, NO_SQUARE, EMPTY, EMPTY, EMPTY, quiet};

    Move moves[MAX_MOVES];
    int total = movegen.generateLegalMoves(moves);

    Move bestMove ;
    int cur_eval = -1;

    if(depth == 1){
        // calculate eval and get best move
    }

    else{
        for(int i=0; i<total; i++){
            board.makeMove(moves[i]);
            Move candidate = findBestMove(depth-1);
            board.undoMove(moves[i]);

            if(candidate > cur_best){
                cur_best = candidate;
                bestMove = candidate;
            }
        }
    }
}