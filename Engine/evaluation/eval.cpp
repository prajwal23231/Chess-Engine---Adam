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

    if(!builtPassedMask){
        createPassedMask();
        builtPassedMask = true;
    }

    if(!builtFileMask){
        createFileRankMask();
        builtFileMask = true;
    }

    if(!builtIsolatedMask){
        createIsolatedMask();
        builtIsolatedMask = true;
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



void Evaluator::createPassedMask(){
    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int rank = getRank(sq);
        int file = getFile(sq);

        U64 whiteMask = 0;

        for(int r=rank+1; r<RANK_SIZE; r++){
            // current file
            whiteMask |= (1ULL<<(r*RANK_SIZE + file));

            // left file
            if(file > 0) whiteMask |= (1ULL<<(r*RANK_SIZE + file - 1));

            // right file
            if(file < RANK_SIZE-1) whiteMask |= (1ULL<<(r*RANK_SIZE + file + 1));
        }

        whitePassedMask[sq] = whiteMask;



        U64 blackMask = 0;

        for(int r=rank-1; r>=0; r--){
            // current file
            blackMask |= (1ULL<<(r*RANK_SIZE + file));

            // left file
            if(file > 0) blackMask |= (1ULL<<(r*RANK_SIZE + file - 1));

            // right file
            if(file < RANK_SIZE-1) blackMask |= (1ULL<<(r*RANK_SIZE + file + 1));
        }

        blackPassedMask[sq] = blackMask;
    }
}



void Evaluator::createFileRankMask(){
    for(int file=0; file<8; file++){
        U64 mask = 0;

        for (int rank = 0; rank < RANK_SIZE; rank++) {
            mask |= (1ULL << (rank * RANK_SIZE + file));
        }

        fileMask[file] = mask;
    }


    for(int rank=0; rank<8; rank++){
        U64 mask = 0;

        for (int file = 0; file < RANK_SIZE; file++) {
            mask |= (1ULL << (rank * RANK_SIZE + file));
        }

        rankMask[rank] = mask;
    }
}



void Evaluator::createIsolatedMask(){
    for(int file=0; file<8; file++){
        U64 mask = 0;

        for (int rank = 0; rank < RANK_SIZE; rank++) {
            // left file
            if(file>0) mask |= (1ULL << (rank * RANK_SIZE + file-1));

            // right file
            if(file<RANK_SIZE-1) mask |= (1ULL << (rank * RANK_SIZE + file+1));
        }

        isolatedMask[file] = mask;
    }
}



int Evaluator::evaluate(const Board& board){
    EvalInfo score = {};
    int phase = calculatePhase(board);

    calculateMaterial(board, score);
    calculatePST(board, score);
    calculateBishopPair(board, score);
    calculatePassedPawns(board, score);
    calculateDoubledPawns(board, score);
    calculateIsolatedPawns(board, score);
    calculateMobility(board, score);
    calculateRook(board, score);


    int mult = (board.getMovingSide() == WHITE ? 1 : -1);
    return mult * interpolate(score, phase);
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



void Evaluator::calculatePassedPawns(const Board& board,EvalInfo& score){
    U64 whitePawns = board.getBitboard(WP);
    U64 blackPawns = board.getBitboard(BP);

    U64 wp = whitePawns;
    U64 bp = blackPawns;

    while(wp){
        Square s = static_cast<Square>(popLSB(wp));
        if(whitePassedMask[s] & blackPawns) continue;

        score.mg += passedPawnMG[getRank(s)];
        score.eg += passedPawnEG[getRank(s)];
    }


    while(bp){
        Square s = static_cast<Square>(popLSB(bp));
        if(blackPassedMask[s] & whitePawns) continue;

        int mirrored = RANK_SIZE - 1 - getRank(s);

        score.mg -= passedPawnMG[mirrored];
        score.eg -= passedPawnEG[mirrored];
    }
}


void Evaluator::calculateDoubledPawns(const Board& board, EvalInfo& score){
    U64 wp = board.getBitboard(WP);
    U64 bp = board.getBitboard(BP);

    for(int file=0; file<8; file++){
        U64 wmask = wp & fileMask[file];
        U64 bmask = bp & fileMask[file];

        int whiteDoubled = max(popCount(wmask) - 1, 0);
        int blackDoubled = max(popCount(bmask) - 1, 0);

        score.mg -= (whiteDoubled - blackDoubled)*DOUBLED_PAWN_MG;
        score.eg -= (whiteDoubled - blackDoubled)*DOUBLED_PAWN_EG;
    }
}


void Evaluator::calculateIsolatedPawns(const Board& board, EvalInfo& score){
    U64 whitePawns = board.getBitboard(WP);
    U64 blackPawns = board.getBitboard(BP);

    U64 wp = whitePawns;
    U64 bp = blackPawns;

    while(wp){
        Square s = static_cast<Square>(popLSB(wp));
        if(isolatedMask[getFile(s)] & whitePawns) continue;

        score.mg -= ISOLATED_PAWN_MG;
        score.eg -= ISOLATED_PAWN_EG;
    }


    while(bp){
        Square s = static_cast<Square>(popLSB(bp));
        if(isolatedMask[getFile(s)] & blackPawns) continue;

        score.mg += ISOLATED_PAWN_MG;
        score.eg += ISOLATED_PAWN_EG;
    }
}



void Evaluator::calculateMobility(const Board& board, EvalInfo& score){
    U64 wn = board.getBitboard(WN);
    U64 bn = board.getBitboard(BN);

    U64 wb = board.getBitboard(WB);
    U64 bb = board.getBitboard(BB);

    U64 wr = board.getBitboard(WR);
    U64 br = board.getBitboard(BR);

    U64 wq = board.getBitboard(WQ);
    U64 bq = board.getBitboard(BQ);

    U64 wocc = board.getOccupancy(WHITE);
    U64 bocc = board.getOccupancy(BLACK);
    U64 bothocc = board.getOccupancy(BOTH);



    while(wn){
        Square s = static_cast<Square>(popLSB(wn));
        U64 attack = attacks.getKnightAttack(s) & (~wocc);

        int mobility = popCount(attack);

        score.mg += knightMobilityMG[mobility];
        score.eg += knightMobilityEG[mobility];
    }


    while(bn){
        Square s = static_cast<Square>(popLSB(bn));
        U64 attack = attacks.getKnightAttack(s) & (~bocc);

        int mobility = popCount(attack);

        score.mg -= knightMobilityMG[mobility];
        score.eg -= knightMobilityEG[mobility];
    }



    while(wb){
        Square s = static_cast<Square>(popLSB(wb));
        U64 attack = attacks.getBishopAttack(s, bothocc) & (~wocc);

        int mobility = popCount(attack);

        score.mg += bishopMobilityMG[mobility];
        score.eg += bishopMobilityEG[mobility];
    }


    while(bb){
        Square s = static_cast<Square>(popLSB(bb));
        U64 attack = attacks.getBishopAttack(s, bothocc) & (~bocc);

        int mobility = popCount(attack);

        score.mg -= bishopMobilityMG[mobility];
        score.eg -= bishopMobilityEG[mobility];
    }



    while(wr){
        Square s = static_cast<Square>(popLSB(wr));
        U64 attack = attacks.getRookAttack(s, bothocc) & (~wocc);

        int mobility = popCount(attack);

        score.mg += rookMobilityMG[mobility];
        score.eg += rookMobilityEG[mobility];
    }


    while(br){
        Square s = static_cast<Square>(popLSB(br));
        U64 attack = attacks.getRookAttack(s, bothocc) & (~bocc);

        int mobility = popCount(attack);

        score.mg -= rookMobilityMG[mobility];
        score.eg -= rookMobilityEG[mobility];
    }



    while(wq){
        Square s = static_cast<Square>(popLSB(wq));
        U64 attack = attacks.getQueenAttack(s, bothocc) & (~wocc);

        int mobility = popCount(attack);

        score.mg += queenMobilityMG[mobility];
        score.eg += queenMobilityEG[mobility];
    }


    while(bq){
        Square s = static_cast<Square>(popLSB(bq));
        U64 attack = attacks.getQueenAttack(s, bothocc) & (~bocc);

        int mobility = popCount(attack);

        score.mg -= queenMobilityMG[mobility];
        score.eg -= queenMobilityEG[mobility];
    }
}




void Evaluator::calculateRook(const Board& board, EvalInfo& score){
    U64 wr = board.getBitboard(WR);
    U64 br = board.getBitboard(BR);

    U64 wp = board.getBitboard(WP);
    U64 bp = board.getBitboard(BP);

    U64 wkmask = board.getBitboard(WK) & rankMask[7];
    U64 bkmask = board.getBitboard(BK) & rankMask[0];

    U64 wrmask = wr & rankMask[6];
    U64 brmask = br & rankMask[1];


    // 7th rank boost
    if(wrmask && (bkmask || (bp & rankMask[6]))){
        score.mg += popCount(wrmask) * rookSeventhRank[MG];
        score.eg += popCount(wrmask) * rookSeventhRank[EG];
    }

    if(brmask && (wkmask || (wp & rankMask[1]))){
        score.mg -= popCount(brmask) * rookSeventhRank[MG];
        score.eg -= popCount(brmask) * rookSeventhRank[EG];
    }



    // open file
    while(wr){
        Square s = static_cast<Square>(popLSB(wr));
        int file = getFile(s);

        U64 bpmask = bp & fileMask[file];
        U64 wpmask = wp & fileMask[file];

        if(!wpmask){
            if(bpmask){
                score.mg += rookSemiOpenFile[MG];
                score.eg += rookSemiOpenFile[EG];
            }

            else{
                score.mg += rookOpenFile[MG];
                score.eg += rookOpenFile[EG];
            }
        }
    }

    while(br){
        Square s = static_cast<Square>(popLSB(br));
        int file = getFile(s);

        U64 bpmask = bp & fileMask[file];
        U64 wpmask = wp & fileMask[file];

        if(!bpmask){
            if(wpmask){
                score.mg -= rookSemiOpenFile[MG];
                score.eg -= rookSemiOpenFile[EG];
            }

            else{
                score.mg -= rookOpenFile[MG];
                score.eg -= rookOpenFile[EG];
            }
        }
    }
}



