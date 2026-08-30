#include "eval.h"
#include "board/board.h"
#include "utils/bitboard_utilities.h"
#include "utils/tools.h"
#include "attack/attacks.h"
#include <algorithm>

using namespace std;
using namespace Bitboard;

namespace {
    constexpr int phase256[TOTAL_PHASE + 1] = {
          0,  11,  21,  32,  43,  53,  64,  75,
         85,  96, 107, 117, 128, 139, 149, 160,
        171, 181, 192, 203, 213, 224, 235, 245, 256
    };
}


int Evaluator::evaluate(const Board& board){
    EvalInfo score = {};
    int phase = std::clamp(board.getGamePhase(), 0, TOTAL_PHASE);

    calculateMaterial(board, score);
    calculatePST(board, score);
    calculateBishopPair(board, score);
    calculatePawns(board, score);

    U64 key = board.getPawnKey();
    PawnEntry* entry = pawntable.probe(key);

    calculateRook(board, score, entry);
    calculateKnightOutpost(board, score, entry);
    calculateMobility(board, score, entry);
    calculateKingSafety(board, score);


    int mult = (board.getMovingSide() == WHITE ? 1 : -1);
    return mult * interpolate(score, phase);
}


int Evaluator::interpolate(const EvalInfo& score, int phase){
    int p = phase256[phase];
    return (score.mg * p + score.eg * (256 - p)) >> 8;
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
            
            score.mg += pst[WHITE][MG][i][s];
            score.eg += pst[WHITE][EG][i][s];
        }


        while(bBb){
            Square s = static_cast<Square>(popLSB(bBb));
            
            score.mg -= pst[BLACK][MG][i][s];
            score.eg -= pst[BLACK][EG][i][s];
        }
    }
}



void Evaluator::calculateBishopPair(const Board& board, EvalInfo& score){
    U64 wBb = board.getBitboard(WB);
    U64 bBb = board.getBitboard(BB);

    if((wBb & LIGHT_SQUARES) && (wBb & DARK_SQUARES)){
        score.mg += BISHOP_PAIR_MG;
        score.eg += BISHOP_PAIR_EG;
    }

    if((bBb & LIGHT_SQUARES) && (bBb & DARK_SQUARES)){
        score.mg -= BISHOP_PAIR_MG;
        score.eg -= BISHOP_PAIR_EG;
    }
}


void Evaluator::calculatePawns(const Board& board,EvalInfo& score){
    U64 key = board.getPawnKey();
    PawnEntry* entry = pawntable.probe(key);

    if(entry){
        score.mg += entry->mg;
        score.eg += entry->eg;
        return ;
    }


    int pawnMG = 0, pawnEG = 0;
    U64 wPassed = 0, bPassed = 0;
    U64 wAttacks = 0, bAttacks = 0;

    U64 whitePawns = board.getBitboard(WP);
    U64 blackPawns = board.getBitboard(BP);

    U64 wp = whitePawns;
    U64 bp = blackPawns;


    while(wp){
        Square s = static_cast<Square>(popLSB(wp));
        int file = getFile(s), rank = getRank(s);
        bool passed = !(whitePassedMask[s] & (blackPawns | (fileMask[file] & whitePawns)));

        if(passed){
            pawnMG += passedPawnMG[rank];
            pawnEG += passedPawnEG[rank];

            wPassed |= 1ULL<<s;
        }


        // isolated pawn
        if(!(isolatedMask[file] & whitePawns)){
            pawnMG -= ISOLATED_PAWN_MG;
            pawnEG -= ISOLATED_PAWN_EG;

            // semi open
            bool semiopen = !(fileMask[file] & blackPawns);
            bool blockedbyown = !(whitePassedMask[s] & fileMask[file] & whitePawns);

            if(semiopen && blockedbyown){
                pawnMG -= ISOLATED_PAWN_SEMI_OPEN_MG;
                pawnEG -= ISOLATED_PAWN_SEMI_OPEN_EG;
            }
        }


        // connected pawns
        if(adjacentFileMask[file] & rankMask[rank] & whitePawns){
            pawnMG += connectedPawnMG[rank];
            pawnEG += connectedPawnEG[rank];
        }


        // protected pawns
        if(attacks.getBlackPawnAttack(s) & whitePawns){
            pawnMG += protectedPawnMG[rank];
            pawnEG += protectedPawnEG[rank];
        }

        // backward pawns
        U64 backMask = blackPassedMask[s] & adjacentFileMask[file] & whitePawns;
        U64 stopMask = attacks.getWhitePawnAttack(static_cast<Square>(s+8)) & blackPawns;

        if(stopMask && !backMask){
            pawnMG -= BACKWARD_PAWN_MG;
            pawnEG -= BACKWARD_PAWN_EG;
        }


        wAttacks |= attacks.getWhitePawnAttack(s);
    }


    while(bp){
        Square s = static_cast<Square>(popLSB(bp));
        int mirrored = RANK_SIZE - 1 - getRank(s);
        int file = getFile(s), rank = getRank(s);
        bool passed = !(blackPassedMask[s] & (whitePawns | (fileMask[file] & blackPawns)));


        if(passed){
            pawnMG -= passedPawnMG[mirrored];
            pawnEG -= passedPawnEG[mirrored];

            bPassed |= 1ULL<<s;
        }


        // isolated pawn
        if(!(isolatedMask[file] & blackPawns)){
            pawnMG += ISOLATED_PAWN_MG;
            pawnEG += ISOLATED_PAWN_EG;

            // semi open
            bool semiopen = !(fileMask[file] & whitePawns);
            bool blockedbyown = !(blackPassedMask[s] & fileMask[file] & blackPawns);

            if(semiopen && blockedbyown){
                pawnMG += ISOLATED_PAWN_SEMI_OPEN_MG;
                pawnEG += ISOLATED_PAWN_SEMI_OPEN_EG;
            }
        }


        // connected pawns
        if(adjacentFileMask[file] & rankMask[rank] & blackPawns){
            pawnMG -= connectedPawnMG[mirrored];
            pawnEG -= connectedPawnEG[mirrored];
        }


        // protected pawns
        if(attacks.getWhitePawnAttack(s) & blackPawns){
            pawnMG -= protectedPawnMG[mirrored];
            pawnEG -= protectedPawnEG[mirrored];
        }

        // backward pawns
        U64 backMask = whitePassedMask[s] & adjacentFileMask[file] & blackPawns;
        U64 stopMask = attacks.getBlackPawnAttack(static_cast<Square>(s-8)) & whitePawns;

        if(stopMask && !backMask){
            pawnMG += BACKWARD_PAWN_MG;
            pawnEG += BACKWARD_PAWN_EG;
        }


        bAttacks |= attacks.getBlackPawnAttack(s);
    }


    // doubled pawns
    for(int file=0; file<8; file++){
        U64 wmask = whitePawns & fileMask[file];
        U64 bmask = blackPawns & fileMask[file];

        int whiteDoubled = max(popCount(wmask) - 1, 0);
        int blackDoubled = max(popCount(bmask) - 1, 0);

        pawnMG -= (whiteDoubled - blackDoubled)*DOUBLED_PAWN_MG;
        pawnEG -= (whiteDoubled - blackDoubled)*DOUBLED_PAWN_EG;
    }


    pawntable.store(key, pawnMG, pawnEG, wPassed, bPassed, wAttacks, bAttacks);
    score.mg+=pawnMG;
    score.eg+=pawnEG;
}


void Evaluator::calculateMobility(const Board& board, EvalInfo& score, PawnEntry* entry){
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

    U64 bpattacks = entry->pawnAttacks[BLACK];
    U64 wpattacks = entry->pawnAttacks[WHITE];

    U64 wMobilityArea = ~(wocc | bpattacks);
    U64 bMobilityArea = ~(bocc | wpattacks);


    while(wn){
        Square s = static_cast<Square>(popLSB(wn));
        U64 attack = attacks.getKnightAttack(s) & wMobilityArea;

        int mobility = popCount(attack);

        score.mg += knightMobilityMG[mobility];
        score.eg += knightMobilityEG[mobility];
    }


    while(bn){
        Square s = static_cast<Square>(popLSB(bn));
        U64 attack = attacks.getKnightAttack(s) & bMobilityArea;

        int mobility = popCount(attack);

        score.mg -= knightMobilityMG[mobility];
        score.eg -= knightMobilityEG[mobility];
    }



    while(wb){
        Square s = static_cast<Square>(popLSB(wb));
        U64 attack = attacks.getBishopAttack(s, bothocc) & wMobilityArea;

        int mobility = popCount(attack);

        score.mg += bishopMobilityMG[mobility];
        score.eg += bishopMobilityEG[mobility];
    }


    while(bb){
        Square s = static_cast<Square>(popLSB(bb));
        U64 attack = attacks.getBishopAttack(s, bothocc) & bMobilityArea;

        int mobility = popCount(attack);

        score.mg -= bishopMobilityMG[mobility];
        score.eg -= bishopMobilityEG[mobility];
    }



    while(wr){
        Square s = static_cast<Square>(popLSB(wr));
        U64 attack = attacks.getRookAttack(s, bothocc) & wMobilityArea;

        int mobility = popCount(attack);

        score.mg += rookMobilityMG[mobility];
        score.eg += rookMobilityEG[mobility];
    }


    while(br){
        Square s = static_cast<Square>(popLSB(br));
        U64 attack = attacks.getRookAttack(s, bothocc) & bMobilityArea;

        int mobility = popCount(attack);

        score.mg -= rookMobilityMG[mobility];
        score.eg -= rookMobilityEG[mobility];
    }



    while(wq){
        Square s = static_cast<Square>(popLSB(wq));
        U64 attack = attacks.getQueenAttack(s, bothocc) & wMobilityArea;

        int mobility = popCount(attack);

        score.mg += queenMobilityMG[mobility];
        score.eg += queenMobilityEG[mobility];
    }


    while(bq){
        Square s = static_cast<Square>(popLSB(bq));
        U64 attack = attacks.getQueenAttack(s, bothocc) & bMobilityArea;

        int mobility = popCount(attack);

        score.mg -= queenMobilityMG[mobility];
        score.eg -= queenMobilityEG[mobility];
    }
}



void Evaluator::calculateRook(const Board& board, EvalInfo& score, PawnEntry* entry){
    U64 wr = board.getBitboard(WR);
    U64 br = board.getBitboard(BR);

    U64 whiteRook = wr;
    U64 blackRook = br;

    U64 wp = board.getBitboard(WP);
    U64 bp = board.getBitboard(BP);

    U64 wmask = (board.getBitboard(WK) & rankMask[0]) | (board.getBitboard(WP) & rankMask[1]);
    U64 bmask = (board.getBitboard(BK) & rankMask[7]) | (board.getBitboard(BP) & rankMask[6]);

    U64 wrmask = wr & rankMask[6];
    U64 brmask = br & rankMask[1];

    U64 occ = board.getOccupancy(BOTH);


    // 7th rank boost
    if(wrmask && bmask){
        score.mg += popCount(wrmask) * rookSeventhRank[MG];
        score.eg += popCount(wrmask) * rookSeventhRank[EG];
    }

    if(brmask && wmask){
        score.mg -= popCount(brmask) * rookSeventhRank[MG];
        score.eg -= popCount(brmask) * rookSeventhRank[EG];
    }



    // open file
    while(wr){
        Square s = static_cast<Square>(popLSB(wr));
        int file = getFile(s), rank=getRank(s);

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


        // connected rook
        U64 rmask = (whiteRook^(1ULL<<s)) & rankMask[rank];
        U64 fmask = (whiteRook^(1ULL<<s)) & fileMask[file];

        while(rmask){
            Square other = static_cast<Square>(popLSB(rmask));
            if(!(between[s][other] & occ)){
                score.mg += connectedRooks[MG];
                score.eg += connectedRooks[EG];
            }
        }

        while(fmask){
            Square other = static_cast<Square>(popLSB(fmask));
            if(!(between[s][other] & occ)){
                score.mg -= connectedRooks[MG];
                score.eg -= connectedRooks[EG];
            }
        }


        // rook and passed pawn
        U64 rattack = attacks.getRookAttack(s,occ) & fileMask[file];

        U64 wtotalPassed = entry->passedPawns[WHITE] & rattack;
        U64 btotalPassed = entry->passedPawns[BLACK] & rattack;

        while(wtotalPassed){
            Square wpassed = static_cast<Square>(popLSB(wtotalPassed));

            if(getRank(wpassed)<rank){
                score.mg += rookInFrontOwnPassedPawn[MG];
                score.eg += rookInFrontOwnPassedPawn[EG];
            }

            else{
                score.mg += rookBehindOwnPassedPawn[MG];
                score.eg += rookBehindOwnPassedPawn[EG];
            }
        }

        while(btotalPassed){
            Square bpassed = static_cast<Square>(popLSB(btotalPassed));

            if(getRank(bpassed)>rank){
                score.mg += rookInFrontEnemyPassedPawn[MG];
                score.eg += rookInFrontEnemyPassedPawn[EG];
            }

            else{
                score.mg += rookBehindEnemyPassedPawn[MG];
                score.eg += rookBehindEnemyPassedPawn[EG];
            }
        }
    }

    while(br){
        Square s = static_cast<Square>(popLSB(br));
        int file = getFile(s), rank = getRank(s);

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


        // connected rook
        U64 rmask = (blackRook^(1ULL<<s)) & rankMask[rank];
        U64 fmask = (blackRook^(1ULL<<s)) & fileMask[file];

        while(rmask){
            Square other = static_cast<Square>(popLSB(rmask));
            if(!(between[s][other] & occ)){
                score.mg -= connectedRooks[MG];
                score.eg -= connectedRooks[EG];
            }
        }

        while(fmask){
            Square other = static_cast<Square>(popLSB(fmask));
            if(!(between[s][other] & occ)){
                score.mg -= connectedRooks[MG];
                score.eg -= connectedRooks[EG];
            }
        }


        // rook and passed pawn
        U64 rattack = attacks.getRookAttack(s,occ) & fileMask[file];

        U64 wtotalPassed = entry->passedPawns[WHITE] & rattack;
        U64 btotalPassed = entry->passedPawns[BLACK] & rattack;

        while(btotalPassed){
            Square bpassed = static_cast<Square>(popLSB(btotalPassed));

            if(getRank(bpassed)>rank){
                score.mg -= rookInFrontOwnPassedPawn[MG];
                score.eg -= rookInFrontOwnPassedPawn[EG];
            }

            else{
                score.mg -= rookBehindOwnPassedPawn[MG];
                score.eg -= rookBehindOwnPassedPawn[EG];
            }
        }

        while(wtotalPassed){
            Square wpassed = static_cast<Square>(popLSB(wtotalPassed));

            if(getRank(wpassed)<rank){
                score.mg -= rookInFrontEnemyPassedPawn[MG];
                score.eg -= rookInFrontEnemyPassedPawn[EG];
            }

            else{
                score.mg -= rookBehindEnemyPassedPawn[MG];
                score.eg -= rookBehindEnemyPassedPawn[EG];
            }
        }
    }
}


void Evaluator::calculateKnightOutpost(const Board &board, EvalInfo &score, PawnEntry* entry){
    U64 wn = board.getBitboard(WN) & entry->pawnAttacks[WHITE];
    U64 bn = board.getBitboard(BN) & entry->pawnAttacks[BLACK];

    U64 wp = board.getBitboard(WP);
    U64 bp = board.getBitboard(BP);

    while(wn){
        Square s = static_cast<Square>(popLSB(wn));
        int rank = getRank(s);        
        if((bp & whiteOutpostMask[s]) || rank < 3) continue;

        score.mg += knightOutpost[MG];
        score.eg += knightOutpost[EG];
    }

    while(bn){
        Square s = static_cast<Square>(popLSB(bn));
        int rank = getRank(s);        
        if((wp & blackOutpostMask[s]) || rank > 4) continue;

        score.mg -= knightOutpost[MG];
        score.eg -= knightOutpost[EG];
    }
}



void Evaluator::calculateKingSafety(const Board& board, EvalInfo &score){
    if(board.getGamePhase() < 6) return ;

    Square wKingSq = board.getKingSquare(WHITE);
    Square bKingSq = board.getKingSquare(BLACK);

    U64 wp = board.getBitboard(WP);
    U64 bp = board.getBitboard(BP);



    // White king safety
    int wFile = getFile(wKingSq);

    
}