#include "eval.h"
#include "board/board.h"
#include "utils/bitboard_utilities.h"
#include <algorithm>

using namespace std;
using namespace Bitboard;


Evaluator::Evaluator(){
    if(!builtpst){
        createpst();
        builtpst = true;
    }
}


void Evaluator::createpst(){
    for (int piece = 0; piece < 6; ++piece) {
        for (int sq = 0; sq < 64; ++sq) {
            pst[MG][piece][sq ^ 56] = mgTables[piece][sq];
            pst[EG][piece][sq ^ 56] = egTables[piece][sq];
        }
    }
}


int Evaluator::evaluate(const Board& board){
    EvalInfo score = {};
    int phase = calculatePhase(board);

    calculateMaterial(board, score);
    calculatePST(board, score);
    calculateBishopPair(board, score);

    return interpolate(score, phase);
}


int Evaluator::calculatePhase(const Board& board){
    int phase = 0;

    phase += QUEEN_PHASE*(popCount(board.getBitboard(WQ)) + popCount(board.getBitboard(BQ)));
    phase += ROOK_PHASE*(popCount(board.getBitboard(WR)) + popCount(board.getBitboard(BR)));
    phase += KNIGHT_PHASE*(popCount(board.getBitboard(WN)) + popCount(board.getBitboard(BN)));
    phase += BISHOP_PHASE*(popCount(board.getBitboard(WB)) + popCount(board.getBitboard(BB)));

    return clamp(phase, 0, TOTAL_PHASE);
}


int Evaluator::interpolate(const EvalInfo& score, int phase){
    int totalScore = (score.mg*phase + score.eg*(TOTAL_PHASE - phase))/TOTAL_PHASE;
    return totalScore;
}


void Evaluator::calculateMaterial(const Board& board, EvalInfo& score){
    for(int i=0; i<NUM_PIECE_TYPE; i++){
        Piece wp = static_cast<Piece>(WP + i);
        Piece bp = static_cast<Piece>(BP + i);

        int cnt = popCount(board.getBitboard(wp)) - popCount(board.getBitboard(bp));
        score.mg += mg_value[i] * cnt;
        score.eg += eg_value[i] * cnt;
    }
}



void Evaluator::calculatePST(const Board& board, EvalInfo& score){
    for(int i=0; i<NUM_PIECE_TYPE; i++){
        Piece wp = static_cast<Piece>(WP + i);
        Piece bp = static_cast<Piece>(BP + i);

        U64 wBb = board.getBitboard(wp);
        U64 bBb = board.getBitboard(bp);

        while(wBb){
            Square s = static_cast<Square>(popLSB(wBb));
            
            score.mg += pst[MG][i][s];
            score.eg += pst[EG][i][s];
        }


        while(bBb){
            Square s = static_cast<Square>(popLSB(bBb));
            
            score.mg -= pst[MG][i][s^56];
            score.eg -= pst[EG][i][s^56];
        }
    }
}



void Evaluator::calculateBishopPair(const Board& board, EvalInfo& score){
    U64 wBb = board.getBitboard(WB);
    U64 bBb = board.getBitboard(BB);

    int total_white = popCount(wBb);
    int total_black = popCount(bBb);

    if(total_white >= 2){
        score.mg += BISHOP_PAIR_MG;
        score.eg += BISHOP_PAIR_EG;
    }

    if(total_black >= 2){
        score.mg -= BISHOP_PAIR_MG;
        score.eg -= BISHOP_PAIR_EG;
    }
}