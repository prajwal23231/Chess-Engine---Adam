#include "kpk.h"
#include "attack/attacks.h"
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Bitboard;

Square indexToSq(int pidx){
    return static_cast<Square>((pidx/4 + 1)*8 + pidx%4);
}

int KPKBitbase::encodeIndex(Square wking, Square pawnSq, Square bking, Color stm){
    int file = getFile(pawnSq);
    int rank = getRank(pawnSq);

    if(file >= 4){
        file = 7 - file;
        wking = static_cast<Square>(wking ^ 7);
        bking = static_cast<Square>(bking ^ 7);
    }

    int pIdx = (rank-1)*4 + file;
    int idx = (stm*24*64*64) + (pIdx*64*64) + (wking*64) + bking;

    return idx;
}


void KPKBitbase::init(){
    if(initialised) return ;

    for(int pRank=1; pRank<=6; pRank++){
        for(int pFile=0; pFile<4; pFile++){
            Square pSq = static_cast<Square>(pRank*8 + pFile);
            Square promoSq = static_cast<Square>(56 + pFile);

            for(int wk=0; wk<64; wk++){
                for(int bk=0; bk<64; bk++){
                    if(wk==bk || wk==pSq || bk==pSq) continue;
                    if(distance(static_cast<Square>(wk),static_cast<Square>(bk)) <= 1) continue;

                    if(pRank == 6){
                        if(bk != promoSq && distance(static_cast<Square>(wk), promoSq) <= 1){
                            int idxW = encodeIndex(static_cast<Square>(wk), pSq, static_cast<Square>(bk), WHITE);
                            bitbase[idxW/32] |= (1U << (idxW % 32));
                        }
                    }
                }
            }
        }
    }

    bool changed = true;
    int iteration = 0;

    while(changed){
        changed = false;
        iteration++;

        for(int pidx=0; pidx<24; pidx++){
            Square psq = indexToSq(pidx);
            int pRank = getRank(psq);

            for(int wk=0; wk<64; wk++){
                Square wking = static_cast<Square>(wk);

                for(int bk=0; bk<64; bk++){
                    Square bking = static_cast<Square>(bk);

                    if(bking == wking || wking == psq || bking == psq) continue;
                    if(distance(wking,bking) <= 1) continue;

                    // =========================================================
                    // CASE A: White to Move
                    // If ANY legal White move leads to a position where White is WIN:
                    // this position is a WIN!
                    // =======================================================

                    if(!isWin(wking, psq, bking, WHITE)) {
                        bool canWin = false;

                        // Legal King moves 
                        U64 wKingMoves = attacks.getKingAttack(wking);
                        wKingMoves &= ~(1ULL<<psq);

                        while (wKingMoves && !canWin) {
                            Square to = static_cast<Square>(popLSB(wKingMoves));
                            if(distance(to, bking) <= 1) continue;

                            if(isWin(to, psq, bking, BLACK)) canWin = true;
                        }

                        Square singlePush = static_cast<Square>(psq+8);
                        Square doublePush = static_cast<Square>(psq+16);


                        // pawn single push
                        if(!canWin && pRank < 6){
                            if(singlePush != wking && singlePush != bking){
                                canWin = isWin(wking, singlePush, bking, WHITE);
                            }
                        }


                        // double pawn push
                        if(!canWin && pRank == 1){
                            if(singlePush != wking && singlePush != bking && doublePush != wking && doublePush != bking){
                                canWin = isWin(wking, doublePush, bking, WHITE);
                            }
                        }

                        if(canWin) {
                            setWin(wking, psq, bking, WHITE);
                            changed = true;
                        }
                    }


                    // =========================================================
                    // CASE B: Black to Move
                    // Black wants to draw.
                    // Position is ONLY a WIN if ALL legal Black moves lead to a WIN!
                    // =========================================================


                    if(!isWin(wking, psq, bking, BLACK)){
                        bool canWin = false;

                        // Legal King moves 
                        U64 bKingMoves = attacks.getKingAttack(bking);
                        bKingMoves &= ~(1ULL<<psq);

                        int legalMoves = 0;
                        int losingMoves = 0;
                        bool pawnCaptureAndDrawn = false;

                        while(bKingMoves){
                            Square to = static_cast<Square>(popLSB(bKingMoves));

                            if(distance(to, wking) <= 1) continue;
                            
                            if(to == psq){
                                if(distance(wking,psq) > 1){
                                    pawnCaptureAndDrawn = true;
                                    break;
                                }

                                continue;
                            }

                            legalMoves++;

                            if(isWin(wking, psq, to, WHITE)) losingMoves++;
                        }

                        if(!pawnCaptureAndDrawn && legalMoves > 0 && losingMoves == legalMoves){
                            setWin(wking, psq, bking, BLACK);
                            changed = true;
                        }
                    }
                }
            }
        }
    }

    initialised = true;

    cout << "info string KPK Bitbase initialized in " << iteration << " retrograde passes." << endl;
}



bool KPKBitbase::probe(Square wking, Square pawnsq, Square bking, Color stm){
    if(!initialised) init();
    return isWin(wking, pawnsq, bking, stm);
}