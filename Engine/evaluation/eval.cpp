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
    calculateMobility(board, score);
    calculateRook(board, score);
    calculatePawns(board, score);
    calculateKnightOutpost(board, score);


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
    U64 whitePawns = board.getBitboard(WP);
    U64 blackPawns = board.getBitboard(BP);

    U64 wp = whitePawns;
    U64 bp = blackPawns;

    U64 wr = board.getBitboard(WR);
    U64 br = board.getBitboard(BR);


    while(wp){
        Square s = static_cast<Square>(popLSB(wp));
        int file = getFile(s), rank = getRank(s);
        bool passed = !(whitePassedMask[s] & (blackPawns | (fileMask[file] & whitePawns)));

        if(passed){
            score.mg += passedPawnMG[rank];
            score.eg += passedPawnEG[rank];

            // rook and passed pawn
            U64 rookmask = wr & fileMask[file];
            U64 enemyrook = br & fileMask[file];

            while(rookmask){
                Square rooksq = static_cast<Square>(popLSB(rookmask));

                if(getRank(rooksq) > rank){
                    score.mg += rookInFrontOwnPassedPawn[MG];
                    score.eg += rookInFrontOwnPassedPawn[EG];
                }

                else{
                    score.mg += rookBehindOwnPassedPawn[MG];
                    score.eg += rookBehindOwnPassedPawn[EG];
                }
            }


            while(enemyrook){
                Square rooksq = static_cast<Square>(popLSB(enemyrook));

                if(getRank(rooksq) > rank){
                    score.mg -= rookInFrontEnemyPassedPawn[MG];
                    score.eg -= rookInFrontEnemyPassedPawn[EG];
                }

                else{
                    score.mg -= rookBehindEnemyPassedPawn[MG];
                    score.eg -= rookBehindEnemyPassedPawn[EG];
                }
            }
        }


        // isolated pawn
        if(!(isolatedMask[file] & whitePawns)){
            score.mg -= ISOLATED_PAWN_MG;
            score.eg -= ISOLATED_PAWN_EG;

            // semi open
            bool semiopen = !(fileMask[file] & blackPawns);
            bool blockedbyown = !(whitePassedMask[s] & fileMask[file] & whitePawns);

            if(semiopen && blockedbyown){
                score.mg -= ISOLATED_PAWN_SEMI_OPEN_MG;
                score.eg -= ISOLATED_PAWN_SEMI_OPEN_EG;
            }
        }


        // connected pawns
        if(adjacentFileMask[file] & rankMask[rank] & whitePawns){
            score.mg += connectedPawnMG[rank];
            score.eg += connectedPawnEG[rank];
        }


        // protected pawns
        if(attacks.getBlackPawnAttack(s) & whitePawns){
            score.mg += protectedPawnMG[rank];
            score.eg += protectedPawnEG[rank];
        }

        // backward pawns
        U64 backMask = blackPassedMask[s] & adjacentFileMask[file] & whitePawns;
        U64 stopMask = attacks.getWhitePawnAttack(static_cast<Square>(s+8)) & blackPawns;

        if(stopMask && !backMask){
            score.mg -= BACKWARD_PAWN_MG;
            score.eg -= BACKWARD_PAWN_EG;
        }
    }


    while(bp){
        Square s = static_cast<Square>(popLSB(bp));
        int mirrored = RANK_SIZE - 1 - getRank(s);
        int file = getFile(s), rank = getRank(s);
        bool passed = !(blackPassedMask[s] & (whitePawns | (fileMask[file] & blackPawns)));


        if(passed){
            score.mg -= passedPawnMG[mirrored];
            score.eg -= passedPawnEG[mirrored];
            
            // rook and passed pawn
            U64 rookmask = br & fileMask[file];
            U64 enemyrook = wr & fileMask[file];

            while(rookmask){
                Square rooksq = static_cast<Square>(popLSB(rookmask));

                if(getRank(rooksq) < rank){
                    score.mg -= rookInFrontOwnPassedPawn[MG];
                    score.eg -= rookInFrontOwnPassedPawn[EG];
                }

                else{
                    score.mg -= rookBehindOwnPassedPawn[MG];
                    score.eg -= rookBehindOwnPassedPawn[EG];
                }
            }


            while(enemyrook){
                Square rooksq = static_cast<Square>(popLSB(enemyrook));

                if(getRank(rooksq) < rank){
                    score.mg += rookInFrontEnemyPassedPawn[MG];
                    score.eg += rookInFrontEnemyPassedPawn[EG];
                }

                else{
                    score.mg += rookBehindEnemyPassedPawn[MG];
                    score.eg += rookBehindEnemyPassedPawn[EG];
                }
            }
        }


        // isolated pawn
        if(!(isolatedMask[file] & blackPawns)){
            score.mg += ISOLATED_PAWN_MG;
            score.eg += ISOLATED_PAWN_EG;

            // semi open
            bool semiopen = !(fileMask[file] & whitePawns);
            bool blockedbyown = !(blackPassedMask[s] & fileMask[file] & blackPawns);

            if(semiopen && blockedbyown){
                score.mg += ISOLATED_PAWN_SEMI_OPEN_MG;
                score.eg += ISOLATED_PAWN_SEMI_OPEN_EG;
            }
        }


        // connected pawns
        if(adjacentFileMask[file] & rankMask[rank] & blackPawns){
            score.mg -= connectedPawnMG[mirrored];
            score.eg -= connectedPawnEG[mirrored];
        }


        // protected pawns
        if(attacks.getWhitePawnAttack(s) & blackPawns){
            score.mg -= protectedPawnMG[mirrored];
            score.eg -= protectedPawnEG[mirrored];
        }

        // backward pawns
        U64 backMask = whitePassedMask[s] & adjacentFileMask[file] & blackPawns;
        U64 stopMask = attacks.getBlackPawnAttack(static_cast<Square>(s-8)) & whitePawns;

        if(stopMask && !backMask){
            score.mg += BACKWARD_PAWN_MG;
            score.eg += BACKWARD_PAWN_EG;
        }
    }


    // doubled pawns
    for(int file=0; file<8; file++){
        U64 wmask = whitePawns & fileMask[file];
        U64 bmask = blackPawns & fileMask[file];

        int whiteDoubled = max(popCount(wmask) - 1, 0);
        int blackDoubled = max(popCount(bmask) - 1, 0);

        score.mg -= (whiteDoubled - blackDoubled)*DOUBLED_PAWN_MG;
        score.eg -= (whiteDoubled - blackDoubled)*DOUBLED_PAWN_EG;
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
    }
}


void Evaluator::calculateKnightOutpost(const Board &board, EvalInfo &score){
    U64 wn = board.getBitboard(WN);
    U64 bn = board.getBitboard(BN);

    U64 wp = board.getBitboard(WP);
    U64 bp = board.getBitboard(BP);

    while(wn){
        Square s = static_cast<Square>(popLSB(wn));
        int rank = getRank(s);        
        if((bp & whiteOutpostMask[s]) || rank < 3) continue;

        if(attacks.getBlackPawnAttack(s) & wp){
            score.mg += knightOutpost[MG];
            score.eg += knightOutpost[EG];
        }
    }

    while(bn){
        Square s = static_cast<Square>(popLSB(bn));
        int rank = getRank(s);        
        if((wp & blackOutpostMask[s]) || rank > 4) continue;

        if(attacks.getWhitePawnAttack(s) & bp){
            score.mg -= knightOutpost[MG];
            score.eg -= knightOutpost[EG];
        }
    }
}