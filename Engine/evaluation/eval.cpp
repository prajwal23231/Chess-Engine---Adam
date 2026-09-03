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

    constexpr int connectedPasserBonus[8] = { 0, 15, 30, 60, 100, 180, 300, 0 };
}


int Evaluator::evaluate(const Board& board){
    EvalInfo score = {board.getMgScore(), board.getEgScore()};
    int phase = std::clamp(board.getGamePhase(), 0, TOTAL_PHASE);

    calculateBishopPair(board, score);
    calculatePawns(board, score);

    U64 key = board.getPawnKey();
    PawnEntry* entry = pawntable.probe(key);

    calculateRook(board, score, entry);
    calculateKnightOutpost(board, score, entry);
    calculateMobility(board, score, entry);
    calculateKingSafety(board, score);
    calculateDevelopment(board, score);
    calculateHangingPieces(board, score);
    
    score.eg = calculateMatingScore(board, score.eg);
    int scaleFactor = getMaterialScaleFactor(board);
    score.eg = (score.eg*scaleFactor)/128;

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

    U64 occ = board.getOccupancy(BOTH);


    while(wp){
        Square s = static_cast<Square>(popLSB(wp));
        int file = getFile(s), rank = getRank(s);
        bool passed = !(whitePassedMask[s] & (blackPawns | (fileMask[file] & whitePawns)));

        if(passed){
            int bonusMG = passedPawnMG[rank];
            int bonusEG = passedPawnEG[rank];

            Square stopSq = static_cast<Square>(s + 8);
            Square promoSq = static_cast<Square>(getFile(s) + 56);

            if (board.getKingSquare(BLACK) == stopSq) {
                bonusEG /= 2;
            }
            
            else if (occ & (1ULL << stopSq)) {
                bonusEG -= 25;
            }

            else if (rank >= 4) {
                bonusEG += (rank - 3) * 20;
            }
            
            if (board.isSquareAttacked(promoSq, BLACK)) {
                bonusEG -= 30;
            }
            
            Square wKing = board.getKingSquare(WHITE);
            Square bKing = board.getKingSquare(BLACK);
            bonusEG += (5 - distance(wKing, s)) * 6;
            bonusEG -= (5 - distance(bKing, s)) * 6;

            if(attacks.getBlackPawnAttack(s) & whitePawns){
                bonusMG += connectedPasserBonus[rank]/2;
                bonusEG += connectedPasserBonus[rank];
            }

            if(adjacentFileMask[file] & (rankMask[rank] | rankMask[min(7,rank+1)]) & whitePawns){
                bonusMG += connectedPasserBonus[rank]/2;
                bonusEG += connectedPasserBonus[rank];
            }

            if(board.getGamePhase() == 0){
                int promoDist = 7-rank;
                if(rank == 1) promoDist--;
                if(board.getMovingSide() == WHITE) promoDist--;

                int kingDist = distance(bKing,promoSq);
                if(kingDist > promoDist){
                    bonusEG += 800;
                }
            }

            pawnMG += bonusMG;
            pawnEG += bonusEG;

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
            int bonusMG = passedPawnMG[mirrored];
            int bonusEG = passedPawnEG[mirrored];

            Square stopSq = static_cast<Square>(s - 8);
            Square promoSq = static_cast<Square>(getFile(s)); // 1st rank
            
            if (board.getKingSquare(WHITE) == stopSq) {
                bonusEG /= 2;
            }
            
            else if (occ & (1ULL << stopSq)) {
                bonusEG -= 25;
            }

            else if (mirrored >= 4) {
                bonusEG += (mirrored - 3) * 20;
            }
            
            if (board.isSquareAttacked(promoSq, WHITE)) {
                bonusEG -= 30;
            }
            
            Square wKing = board.getKingSquare(WHITE);
            Square bKing = board.getKingSquare(BLACK);
            bonusEG += (5 - distance(bKing, s)) * 6;
            bonusEG -= (5 - distance(wKing, s)) * 6;

            if(attacks.getWhitePawnAttack(s) & blackPawns){
                bonusMG += connectedPasserBonus[mirrored]/2;
                bonusEG += connectedPasserBonus[mirrored];
            }

            if(adjacentFileMask[file] & (rankMask[rank] | rankMask[max(0, rank-1)]) & blackPawns){
                bonusMG += connectedPasserBonus[mirrored]/2;
                bonusEG += connectedPasserBonus[mirrored];
            }

            if(board.getGamePhase() == 0){
                int promoDist = rank;
                if(rank == 6) promoDist--;
                if(board.getMovingSide() == BLACK) promoDist--;

                int kingDist = distance(wKing,promoSq);
                if(kingDist > promoDist){
                    bonusEG += 800;
                }
            }

            pawnMG -= bonusMG;
            pawnEG -= bonusEG;

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
                score.mg += connectedRooks[MG];
                score.eg += connectedRooks[EG];
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
    U64 occ = board.getOccupancy(BOTH);


    if (board.getGamePhase() < 12) {
        // White king advanced into enemy territory (rank >= 4)
        if (getRank(wKingSq) >= 4) {
            score.eg += (getRank(wKingSq) - 3) * 15;
        }

        // Black king advanced into White territory (rank <= 3)
        if (getRank(bKingSq) <= 3) {
            score.eg -= (4 - getRank(bKingSq)) * 15;
        }
    }


    // White king safety - only apply pawn shield penalty when King is castled / on flanks
    int wFile = getFile(wKingSq);
    int wShieldPenalty = 0;

    if (wFile <= 2 || wFile >= 5) {
        int wClampFile = clamp(wFile, 1, 6);
        for(int f = wClampFile-1; f <= wClampFile+1; f++){
            U64 pawnOnFile = wp & fileMask[f];

            if(!pawnOnFile){
                wShieldPenalty += PAWN_SHIELD_MISSING;

                if(!(bp & fileMask[f])){
                    wShieldPenalty += OPEN_FILE_NEAR_KING;
                }

                else{
                    wShieldPenalty += OPEN_FILE_NEAR_KING/2;
                }
            }


            else{
                int lowestRank = getRank(static_cast<Square>(lsb(pawnOnFile)));

                if(lowestRank==2){
                    wShieldPenalty += PAWN_SHIELD_STEPPED;
                }

                else if(lowestRank>=3){
                    wShieldPenalty += PAWN_SHIELD_MISSING;
                }
            }
        }
    }
    
    else {
        // King is uncastled in the center (d or e file)
        bool ownPawnOnKingFile = (wp & fileMask[wFile]);
        bool enemyPawnOnKingFile = (bp & fileMask[wFile]);

        if (!ownPawnOnKingFile) {
            wShieldPenalty += 45; // Semi-open file in front of central King!
            if (!enemyPawnOnKingFile) {
                wShieldPenalty += 35; // Fully open file right in front of central King (total 80 cp)!
            }
        }

        int adjCenter = (wFile == 4) ? 3 : 4; // Check adjacent d/e file
        if (!(wp & fileMask[adjCenter])) {
            wShieldPenalty += 25;
        }
    }



    // Black king safety - only apply pawn shield penalty when King is castled / on flanks
    int bFile = getFile(bKingSq);
    int bShieldPenalty = 0;

    if (bFile <= 2 || bFile >= 5) {
        int bClampFile = clamp(bFile, 1, 6);
        for(int f = bClampFile-1; f <= bClampFile+1; f++){
            U64 pawnOnFile = bp & fileMask[f];

            if(!pawnOnFile){
                bShieldPenalty += PAWN_SHIELD_MISSING;

                if(!(wp & fileMask[f])){
                    bShieldPenalty += OPEN_FILE_NEAR_KING;
                }

                else{
                    bShieldPenalty += OPEN_FILE_NEAR_KING/2;
                }
            }


            else{
                int highestRank = getRank(static_cast<Square>(63 - __builtin_clzll(pawnOnFile)));


                if(highestRank==5){
                    bShieldPenalty += PAWN_SHIELD_STEPPED;
                }

                else if(highestRank<=4){
                    bShieldPenalty += PAWN_SHIELD_MISSING;
                }
            }
        }
    } 
    
    else {
        // King is uncastled in the center (d or e file)
        bool ownPawnOnKingFile = (bp & fileMask[bFile]);
        bool enemyPawnOnKingFile = (wp & fileMask[bFile]);

        if (!ownPawnOnKingFile) {
            bShieldPenalty += 45; // Semi-open file in front of central King!
            if (!enemyPawnOnKingFile) {
                bShieldPenalty += 35; // Fully open file right in front of central King (total 80 cp)!
            }
        }

        int adjCenter = (bFile == 4) ? 3 : 4; // Check adjacent d/e file
        if (!(bp & fileMask[adjCenter])) {
            bShieldPenalty += 25;
        }
    }



    U64 wKingZone = attacks.getKingAttack(wKingSq) | (1ULL<<wKingSq);
    wKingZone |= wKingZone<<8;

    U64 bKingZone = attacks.getKingAttack(bKingSq) | (1ULL<<bKingSq);
    bKingZone |= bKingZone>>8;



    int bAttackUnits = 0;
    int bAttackerCount = 0;
    int bDangerScore = 0;

    U64 bn = board.getBitboard(BN);

    while(bn){
        Square sq = static_cast<Square>(popLSB(bn));
        U64 hits = attacks.getKnightAttack(sq) & wKingZone;

        if(hits){
            bAttackUnits += popCount(hits) * KNIGHT_ATTACK_WEIGHT;
            bAttackerCount++;
        }
    }


    U64 bb = board.getBitboard(BB);

    while(bb){
        Square sq = static_cast<Square>(popLSB(bb));
        U64 hits = attacks.getBishopAttack(sq,occ) & wKingZone;

        if(hits){
            bAttackUnits += popCount(hits) * BISHOP_ATTACK_WEIGHT;
            bAttackerCount++;
        }
    }


    U64 br = board.getBitboard(BR);

    while(br){
        Square sq = static_cast<Square>(popLSB(br));
        U64 hits = attacks.getRookAttack(sq,occ) & wKingZone;

        if(hits){
            bAttackUnits += popCount(hits) * ROOK_ATTACK_WEIGHT;
            bAttackerCount++;
        }
    }


    U64 bq = board.getBitboard(BQ);

    while(bq){
        Square sq = static_cast<Square>(popLSB(bq));
        U64 hits = attacks.getQueenAttack(sq,occ) & wKingZone;

        if(hits){
            bAttackUnits += popCount(hits) * QUEEN_ATTACK_WEIGHT;
            bAttackerCount++;
        }
    }


    if(bAttackerCount >= 2){
        bDangerScore = kingDangerTable[min(bAttackUnits,99)];

        if(!board.getBitboard(BQ)){
            bDangerScore /= 2;
        }
    }




    int wAttackUnits = 0;
    int wAttackerCount = 0;
    int wDangerScore = 0;

    U64 wn = board.getBitboard(WN);

    while(wn){
        Square sq = static_cast<Square>(popLSB(wn));
        U64 hits = attacks.getKnightAttack(sq) & bKingZone;

        if(hits){
            wAttackUnits += popCount(hits) * KNIGHT_ATTACK_WEIGHT;
            wAttackerCount++;
        }
    }


    U64 wb = board.getBitboard(WB);

    while(wb){
        Square sq = static_cast<Square>(popLSB(wb));
        U64 hits = attacks.getBishopAttack(sq,occ) & bKingZone;

        if(hits){
            wAttackUnits += popCount(hits) * BISHOP_ATTACK_WEIGHT;
            wAttackerCount++;
        }
    }


    U64 wr = board.getBitboard(WR);

    while(wr){
        Square sq = static_cast<Square>(popLSB(wr));
        U64 hits = attacks.getRookAttack(sq,occ) & bKingZone;

        if(hits){
            wAttackUnits += popCount(hits) * ROOK_ATTACK_WEIGHT;
            wAttackerCount++;
        }
    }


    U64 wq = board.getBitboard(WQ);

    while(wq){
        Square sq = static_cast<Square>(popLSB(wq));
        U64 hits = attacks.getQueenAttack(sq,occ) & bKingZone;

        if(hits){
            wAttackUnits += popCount(hits) * QUEEN_ATTACK_WEIGHT;
            wAttackerCount++;
        }
    }


    if(wAttackerCount >= 2){
        wDangerScore = kingDangerTable[min(wAttackUnits,99)];
        
        if(!board.getBitboard(WQ)){
            wDangerScore /= 2;
        }
    }


    score.mg -= wShieldPenalty;
    score.mg += bShieldPenalty;

    score.mg -= bDangerScore;
    score.mg += wDangerScore;
}




void Evaluator::calculateDevelopment(const Board& board, EvalInfo& score){
    if(board.getGamePhase() < 18) return ;

    constexpr int UNDEVELOPED_PENALTY = 18;

    if (board.getPieceBoard(B1) == WN) score.mg -= UNDEVELOPED_PENALTY;
    if (board.getPieceBoard(G1) == WN) score.mg -= UNDEVELOPED_PENALTY;
    if (board.getPieceBoard(C1) == WB) score.mg -= UNDEVELOPED_PENALTY;
    if (board.getPieceBoard(F1) == WB) score.mg -= UNDEVELOPED_PENALTY;
    // Black undeveloped minors on home squares
    if (board.getPieceBoard(B8) == BN) score.mg += UNDEVELOPED_PENALTY;
    if (board.getPieceBoard(G8) == BN) score.mg += UNDEVELOPED_PENALTY;
    if (board.getPieceBoard(C8) == BB) score.mg += UNDEVELOPED_PENALTY;
    if (board.getPieceBoard(F8) == BB) score.mg += UNDEVELOPED_PENALTY;
    // Castling state incentives (Middlegame)
    Square wKing = board.getKingSquare(WHITE);
    Square bKing = board.getKingSquare(BLACK);

    // White castling evaluation
    if (wKing == G1 || wKing == C1) {
        score.mg += 30; // Safely castled!
    } else if (board.getCastlingRights() & (CASTLE_WK | CASTLE_WQ)) {
        score.mg += 15; // Still holds right to castle
    } else {
        score.mg -= 35; // Trapped in center with no castling rights!
    }

    // Black castling evaluation
    if (bKing == G8 || bKing == C8) {
        score.mg -= 30; // Safely castled!
    } else if (board.getCastlingRights() & (CASTLE_BK | CASTLE_BQ)) {
        score.mg -= 15; // Still holds right to castle
    } else {
        score.mg += 35; // Trapped in center with no castling rights!
    }
}



void Evaluator::calculateHangingPieces(const Board& board, EvalInfo& score){
    for(int p=WN; p<=WQ; p++){
        U64 bb = board.getBitboard(static_cast<Piece>(p));

        while(bb){
            Square sq = static_cast<Square>(popLSB(bb));
            
            if(board.isSquareAttacked(sq, BLACK) && !board.isSquareAttacked(sq, WHITE)){
                int penalty = mg_value[p] / 4;
                score.mg -= penalty;
                score.eg -= penalty;
            }
        }
    }


    for(int p=BN; p<=BQ; p++){
        U64 bb = board.getBitboard(static_cast<Piece>(p));

        while(bb){
            Square sq = static_cast<Square>(popLSB(bb));
            
            if(board.isSquareAttacked(sq, WHITE) && !board.isSquareAttacked(sq, BLACK)){
                int penalty = mg_value[p] / 4;
                score.mg += penalty;
                score.eg += penalty;
            }
        }
    }
}


int Evaluator::calculateMatingScore(const Board& board, int egScore){
    if(abs(egScore) < 350) return egScore;

    Color winingSide = (egScore > 0) ? WHITE : BLACK;
    Color losingSide = (egScore > 0) ? BLACK : WHITE;

    Square winKing = board.getKingSquare(winingSide);
    Square loseKing = board.getKingSquare(losingSide);

    if(winKing == NO_SQUARE || loseKing == NO_SQUARE) return egScore;

    int lRank = getRank(loseKing);
    int lFile = getFile(loseKing);

    int centerDistRank = max(3-lRank, lRank-4);
    int centerDistFile = max(3-lFile, lFile-4);
    int pushToEdge = (centerDistRank + centerDistFile) * 12;

    int kingdDist = distance(winKing, loseKing);
    int closeIn = (14 - kingdDist) * 6;

    int bonus = pushToEdge + closeIn;
    return winingSide == WHITE ? egScore+bonus : egScore-bonus;
}


int Evaluator::getMaterialScaleFactor(const Board& board){
    U64 wp = board.getBitboard(WP);
    U64 bp = board.getBitboard(BP);
    U64 wn = board.getBitboard(WN);
    U64 bn = board.getBitboard(BN);
    U64 wb = board.getBitboard(WB);
    U64 bb = board.getBitboard(BB);
    U64 wr = board.getBitboard(WR);
    U64 br = board.getBitboard(BR);
    U64 wq = board.getBitboard(WQ);
    U64 bq = board.getBitboard(BQ);
    Square wk = board.getKingSquare(WHITE);
    Square bk = board.getKingSquare(BLACK);

    int wMajors = popCount(wr | wq);
    int bMajors = popCount(br | bq);
    int wMinors = popCount(wn | wb);
    int bMinors = popCount(bn | bb);


    if(wMajors + bMajors + wMinors + bMinors == 0){
        if(popCount(wp) == 1 && bp == 0){
            Square psq = static_cast<Square>(lsb(wp));

            bool iswon = KPKBitbase::probe(wk, psq, bk, board.getMovingSide());
            return iswon ? 128 : 0;
        }

        if(popCount(bp) == 1 && wp == 0){
            Square psq = static_cast<Square>(lsb(bp)^56);
            Square whiteKing = static_cast<Square>(lsb(wp) ^ 56);
            Square blackKing = static_cast<Square>(lsb(bp) ^ 56);

            Color stm = (board.getMovingSide() == WHITE ? BLACK : WHITE);
            bool iswon = KPKBitbase::probe(bk, psq, wk, BLACK);

            return iswon ? 128 : 0;
        }
    }

    if(!wp && wMajors == 0 && popCount(wn) == 2 && wMinors == 2 && (bMajors + bMinors + popCount(bp)) == 0){
        return 0;
    }

    if(!bp && bMajors == 0 && popCount(bn) == 2 && bMinors == 2 && (wMajors + wMinors + popCount(wp)) == 0){
        return 0;
    }


    if(wMajors == 0 && bMajors == 0 && wMinors == 1 && bMinors == 1 && wb && bb){
        bool wLight = (wb & LIGHT_SQUARES);
        bool bLight = (bb & LIGHT_SQUARES);
        if (wLight != bLight && (popCount(wp) + popCount(bp)) <= 2) {
            return 64; // 50% draw scaling
        }
    }


    return 128;
}



void Evaluator::calculateBishopTrappedAndBad(const Board& board, EvalInfo& score) {
    U64 wb = board.getBitboard(WB);
    U64 bb = board.getBitboard(BB);
    U64 wp = board.getBitboard(WP);
    U64 bp = board.getBitboard(BP);

    while (wb) {
        Square sq = static_cast<Square>(popLSB(wb));
        U64 colorMask = ((1ULL<<sq) & LIGHT_SQUARES) ? LIGHT_SQUARES : DARK_SQUARES;

        int blockedPawns = popCount(wp & colorMask);
        score.mg -= blockedPawns * 4;
        score.eg -= blockedPawns * 6;

        if(sq == B3 && (bp & (1ULL<<A4)) && (bp & (1ULL<<C4))){
            score.mg -= 150;
        }
    }

    while (bb) {
        Square sq = static_cast<Square>(popLSB(bb));
        U64 colorMask = ((1ULL<<sq) & LIGHT_SQUARES) ? LIGHT_SQUARES : DARK_SQUARES;

        int blockedPawns = popCount(bp & colorMask);
        score.mg += blockedPawns * 4;
        score.eg += blockedPawns * 6;

        if(sq == B6 && (wp & (1ULL<<A5)) && (wp & (1ULL<<C5))){
            score.mg += 150;
        }
    }
}