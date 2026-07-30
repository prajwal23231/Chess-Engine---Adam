#include "movegen.h"
#include "board/board.h"
#include <array>

using namespace std;
using namespace Bitboard;


MoveGenerator::MoveGenerator(Board &board) : board(board){
    cnt = 0;

    computeBetween();
}


void MoveGenerator::computeBetween(){
    for(int i=0; i<BOARD_SIZE; i++){
        Square first = static_cast<Square>(i);
        int f_rank = getRank(first);
        int f_file = getFile(first);

        for(int j=0; j<BOARD_SIZE; j++){
            Square sec = static_cast<Square>(j);
            int s_rank = getRank(sec);
            int s_file = getFile(sec);

            U64 mask = 0;

            // same rank
            if(f_rank == s_rank){
                int cur = min(f_file, s_file) + 1;

                while(cur < max(f_file, s_file)){
                    mask |= (1ULL<<(f_rank*RANK_SIZE + cur));
                    cur++;
                }
            }

            // same file
            else if(f_file == s_file){
                int cur = min(f_rank, s_rank) + 1;

                while(cur < max(f_rank, s_rank)){
                    mask |= (1ULL<<(cur*RANK_SIZE + f_file));
                    cur++;
                }
            }


            // same diagonal
            else if(abs(f_file - s_file) == abs(f_rank - s_rank)){
                constexpr int dr[4] = {-1, -1, 1, 1};
                constexpr int df[4] = {-1, 1, -1, 1};
                int ind;

                if(f_rank > s_rank){
                    if(f_file > s_file) ind = 0;
                    else ind = 1;
                }

                else{
                    if(f_file > s_file) ind = 2;
                    else ind = 3;
                }

                int cur_rank = f_rank + dr[ind];
                int cur_file = f_file + df[ind];

                while(cur_rank != s_rank && cur_file != s_file){
                    mask |= (1ULL<<(cur_rank*RANK_SIZE + cur_file));
                    cur_rank += dr[ind];
                    cur_file += df[ind];
                }
            }

            between[first][sec] = mask;
        }
    }
}


Square MoveGenerator::getKingpos() const{
    Piece p = (board.getMovingSide() == WHITE ? WK : BK);
    U64 pos = board.getBitboard(p);
    return static_cast<Square>(popLSB(pos));
}


void MoveGenerator::createAttackMap(){
    enemyAttackMap = 0;
    Color toMove = board.getMovingSide();


    // king attack
    U64 kBb = board.getBitboard(toMove == WHITE ? BK : WK);
    Square king = static_cast<Square>(popLSB(kBb));

    enemyAttackMap |= attacks.getKingAttack(king);


    // knight attack
    U64 nBb = board.getBitboard(toMove == WHITE ? BN : WN);
    
    while(nBb){
        Square knight = static_cast<Square>(popLSB(nBb));
        enemyAttackMap |= attacks.getKnightAttack(knight);
    }


    // pawn attack
    U64 pBb = board.getBitboard(toMove == WHITE ? BP : WP);
    
    while(pBb){
        Square pawn = static_cast<Square>(popLSB(pBb));

        if(toMove == WHITE) enemyAttackMap |= attacks.getBlackPawnAttack(pawn);
        else enemyAttackMap |= attacks.getWhitePawnAttack(pawn);
    }



    U64 occ = board.getOccupancy(BOTH);

    // bishop attack
    U64 bBb = board.getBitboard(toMove == WHITE ? BB : WB);
    
    while(bBb){
        Square bishop = static_cast<Square>(popLSB(bBb));
        enemyAttackMap |= attacks.getBishopAttack(bishop, occ);
    }



    // rook attack
    U64 rBb = board.getBitboard(toMove == WHITE ? BR : WR);
    
    while(rBb){
        Square rook = static_cast<Square>(popLSB(rBb));
        enemyAttackMap |= attacks.getRookAttack(rook, occ);
    }



    // queen attack
    U64 qBb = board.getBitboard(toMove == WHITE ? BQ : WQ);
    
    while(qBb){
        Square queen = static_cast<Square>(popLSB(qBb));
        enemyAttackMap |= attacks.getQueenAttack(queen, occ);
    }
}


CheckInfo MoveGenerator::analyzeChecks() const{
    CheckInfo info = {};

    computeChecks(info);
    computePins(info);

    return info;
}


void MoveGenerator::computeChecks(CheckInfo& info) const{
    Color toMove = board.getMovingSide();
    Square sq = getKingpos();


    // knight attack
    U64 nBb = board.getBitboard(toMove == WHITE ? BN : WN);
    
    while(nBb){
        Square knight = static_cast<Square>(popLSB(nBb));
        U64 attack = attacks.getKnightAttack(knight);

        if(attack & (1ULL<<sq)){
            info.checkMask |= (1ULL<<knight);
            info.checkerCount++;
            info.checkers |= (1ULL<<knight);
        }
    }


    // pawn attack
    U64 pBb = board.getBitboard(toMove == WHITE ? BP : WP);
    
    while(pBb){
        Square pawn = static_cast<Square>(popLSB(pBb));
        U64 attack = 0;

        if(toMove == WHITE) attack = attacks.getBlackPawnAttack(pawn);
        else attack = attacks.getWhitePawnAttack(pawn);

        if(attack & (1ULL<<sq)){
            info.checkMask |= (1ULL<<pawn);
            info.checkerCount++;
            info.checkers |= (1ULL<<pawn);
        }
    }



    U64 occ = board.getOccupancy(BOTH);

    // bishop attack
    U64 bBb = board.getBitboard(toMove == WHITE ? BB : WB);
    
    while(bBb){
        Square bishop = static_cast<Square>(popLSB(bBb));
        U64 attack = attacks.getBishopAttack(bishop, occ);

        if(attack & (1ULL<<sq)){
            info.checkMask |= (attack & between[bishop][sq]) | (1ULL<<bishop);
            info.checkerCount++;
            info.checkers |= (1ULL<<bishop);
        }
    }



    // rook attack
    U64 rBb = board.getBitboard(toMove == WHITE ? BR : WR);
    
    while(rBb){
        Square rook = static_cast<Square>(popLSB(rBb));
        U64 attack = attacks.getRookAttack(rook, occ);

        if(attack & (1ULL<<sq)){
            info.checkMask |= (attack & between[rook][sq]) | (1ULL<<rook);
            info.checkerCount++;
            info.checkers |= (1ULL<<rook);
        }
    }



    // queen attack
    U64 qBb = board.getBitboard(toMove == WHITE ? BQ : WQ);
    
    while(qBb){
        Square queen = static_cast<Square>(popLSB(qBb));
        U64 attack = attacks.getQueenAttack(queen, occ);

        if(attack & (1ULL<<sq)){
            info.checkMask |= (attack & between[queen][sq]) | (1ULL<<queen);
            info.checkerCount++;
            info.checkers |= (1ULL<<queen);
        }
    }
}



void MoveGenerator::computePins(CheckInfo& info) const{
    computeOrthogonalPins(info);
    computeDiagonalPins(info);
}



void MoveGenerator::computeDiagonalPins(CheckInfo& info) const{
    Color toMove = board.getMovingSide();

    Piece queen = (toMove == WHITE ? BQ : WQ);
    Piece bishop = (toMove == WHITE ? BB : WB);

    Square kingsq = getKingpos();
    U64 occ = board.getOccupancy(BOTH);
    U64 friendOcc = board.getOccupancy(toMove);
    
    U64 bishopray = attacks.getBishopAttack(kingsq, occ);
    U64 blockers = bishopray & board.getOccupancy(toMove);

    if(blockers == 0) return ;

    U64 complete = attacks.getBishopAttack(kingsq, occ^blockers);
    U64 pinners = complete & (board.getBitboard(queen) | board.getBitboard(bishop));

    while(pinners){
        Square pinner = static_cast<Square>(popLSB(pinners));

        U64 betweenMask = between[kingsq][pinner];
        U64 blockers = betweenMask & friendOcc;

        if(popCount(blockers) == 1){
            Square pinned = static_cast<Square>(popLSB(blockers));

            info.pinnedPieces |= (1ULL<<pinned);
            info.pinnedRay[pinned] = betweenMask | (1ULL<<pinner);
        }
    }
}


void MoveGenerator::computeOrthogonalPins(CheckInfo& info) const{
    Color toMove = board.getMovingSide();

    Piece queen = (toMove == WHITE ? BQ : WQ);
    Piece rook = (toMove == WHITE ? BR : WR);

    Square kingsq = getKingpos();
    U64 occ = board.getOccupancy(BOTH);
    U64 friendOcc = board.getOccupancy(toMove);
    
    U64 rookray = attacks.getRookAttack(kingsq, occ);
    U64 blockers = rookray & board.getOccupancy(toMove);

    if(blockers == 0) return ;

    U64 complete = attacks.getRookAttack(kingsq, occ^blockers);
    U64 pinners = complete & (board.getBitboard(queen) | board.getBitboard(rook));

    while(pinners){
        Square pinner = static_cast<Square>(popLSB(pinners));

        U64 betweenMask = between[kingsq][pinner];
        U64 blockers = betweenMask & friendOcc;

        if(popCount(blockers) == 1){
            Square pinned = static_cast<Square>(popLSB(blockers));

            info.pinnedPieces |= (1ULL<<pinned);
            info.pinnedRay[pinned] = betweenMask | (1ULL<<pinner);
        }
    }
}



int MoveGenerator::generateLegalMoves(Move moves[]){
    createAttackMap();
    CheckInfo info = analyzeChecks();
}