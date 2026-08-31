#include "search.h"
using namespace std;

namespace {
    // Approximate piece values for MVV-LVA sorting
    constexpr int mvvPieceValues[NUM_PIECE_TYPE] = {
        100,    // PAWN   (0)
        300,    // KNIGHT (1)
        500,    // ROOK   (2)
        300,    // BISHOP (3)
        900,    // QUEEN  (4)
        10000   // KING   (5)
    };
}


Search::Search(Board& board, MoveGenerator& movegen, Evaluator& evaluator) : board(board), movegen(movegen), evaluator(evaluator){
}


int Search::scoreMove(const Move& move){
    int score=0;
    constexpr int promotionBase = 10000;

    if(move.isCapture()){
        Piece moved = move.getMovedPiece();
        Piece captured = move.getCapturedPiece();

        int attackerType = (moved >= BP ? moved - BP : moved);
        int victimeType = (captured == EMPTY ? PAWN : (captured >= BP ? captured - BP : captured));

        score = mvvPieceValues[victimeType] * 10 - mvvPieceValues[attackerType] + 10000;
    }

    if(move.isPromotion()){
        Piece promoted = move.getPromotion();
        int promotionVal = 0;

        int promoType = (promoted >= BP ? promoted - BP : promoted);
        promotionVal = mvvPieceValues[promoType]*10;

        score += promotionVal;

        if(!move.isCapture()) score+=promotionBase;
    }

    return score;
}



void Search::orderMoves(Move* moves,int* scores,int count){
    for(int i=0; i<count; i++){
        scores[i] = scoreMove(moves[i]);
    }
}



int Search::quiescence(int alpha, int beta, int ply){
    nodes++;
    int standPat = evaluator.evaluate(board);

    constexpr int BIG_DELTA = 975;

    if(standPat >= beta) return beta;
    if(standPat > alpha) alpha = standPat;

    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int count = movegen.generateLegalMoves(moves);
    orderMoves(moves, scores, count);


    for(int i=0; i<count; i++){
        int idx = i;

        for(int j=i+1; j<count; j++){
            if(scores[idx]<scores[j]){
                idx=j;
            }
        }

        swap(scores[i],scores[idx]);
        swap(moves[i],moves[idx]);

        if(!(moves[i].isCapture() || moves[i].isPromotion())) continue;
        if (standPat < alpha - BIG_DELTA && !moves[i].isPromotion()) continue;

        board.makeMove(moves[i]);
        int score = -quiescence(-beta, -alpha, ply+1);

        board.undoMove(moves[i]);

        if(score >= beta) return beta;
        if(score > alpha) alpha = score;
    }

    return alpha;
}




int Search::negamax(int alpha, int beta, int depth, int ply){
    if(depth<=0){
        return quiescence(alpha, beta, ply);
    }
    
    nodes++;

    if(board.getHalfMoveClock() >= 100) return 0;

    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int count = movegen.generateLegalMoves(moves);

    if(count == 0){
        Color movingSide = board.getMovingSide();
        Square kingSq = board.getKingSquare(movingSide);
        Color opp = (movingSide == WHITE ? BLACK : WHITE);

        if(board.isSquareAttacked(kingSq, opp)) return -MATE_SCORE+ply;
        else return 0;
    }

    orderMoves(moves, scores, count);

    for(int i=0; i<count; i++){
        int idx = i;

        for(int j=i+1; j<count; j++){
            if(scores[idx]<scores[j]){
                idx=j;
            }
        }

        swap(scores[i],scores[idx]);
        swap(moves[i],moves[idx]);

        board.makeMove(moves[i]);
        int score = -negamax(-beta, -alpha, depth-1, ply+1);

        board.undoMove(moves[i]);

        if(score >= beta) return beta;
        if(score > alpha) alpha = score;
    }

    return alpha;
}



Move Search::findBestMove(int depth){
    nodes=0;
    auto start = Clock::now();

    Move bestMove;
    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int count = movegen.generateLegalMoves(moves);

    if(count == 0) return bestMove;
    else if(count == 1) return moves[0];

    for(int d = 1; d <= depth; d++){
        int alpha = -INFINITY_SCORE, beta = INFINITY_SCORE;

        orderMoves(moves, scores, count);

        if(d > 1){
            for(int i = 0; i < count; i++){
                if(moves[i].getValue() == bestMove.getValue()){
                    scores[i] = 1000000;
                    break;
                }
            }
        }
        
        for(int i=0; i<count; i++){
            int idx = i;

            for(int j=i+1; j<count; j++){
                if(scores[idx]<scores[j]){
                    idx=j;
                }
            }

            swap(scores[i],scores[idx]);
            swap(moves[i],moves[idx]);

            board.makeMove(moves[i]);
            int score = -negamax(-beta, -alpha, d-1, 1);

            board.undoMove(moves[i]);

            if(score > alpha){
                alpha = score;
                bestMove = moves[i];
            }

            if(timeLimitReached(start)) return bestMove;
        }
    }

    return bestMove;
}