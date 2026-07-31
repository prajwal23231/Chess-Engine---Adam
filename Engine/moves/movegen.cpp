#include "movegen.h"
#include "board/board.h"
#include <array>

using namespace std;
using namespace Bitboard;


MoveGenerator::MoveGenerator(Board &board) : board(board){
    cnt = 0;

    if (!isBetweenInitialized) {
        computeBetween();
        isBetweenInitialized = true;
    }
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


void MoveGenerator::createAttackMap(){
    enemyAttackMap = 0;

    U64 occWithoutKing = occ ^ (1ULL << kingpos);

    // king attack
    U64 kBb = board.getBitboard(tomove == WHITE ? BK : WK);
    Square king = static_cast<Square>(popLSB(kBb));

    enemyAttackMap |= attacks.getKingAttack(king);


    // knight attack
    U64 nBb = board.getBitboard(tomove == WHITE ? BN : WN);
    
    while(nBb){
        Square knight = static_cast<Square>(popLSB(nBb));
        enemyAttackMap |= attacks.getKnightAttack(knight);
    }


    // pawn attack
    U64 pBb = board.getBitboard(tomove == WHITE ? BP : WP);
    
    while(pBb){
        Square pawn = static_cast<Square>(popLSB(pBb));

        if(tomove == WHITE) enemyAttackMap |= attacks.getBlackPawnAttack(pawn);
        else enemyAttackMap |= attacks.getWhitePawnAttack(pawn);
    }



    // bishop attack
    U64 bBb = board.getBitboard(tomove == WHITE ? BB : WB);
    
    while(bBb){
        Square bishop = static_cast<Square>(popLSB(bBb));
        enemyAttackMap |= attacks.getBishopAttack(bishop, occWithoutKing);
    }



    // rook attack
    U64 rBb = board.getBitboard(tomove == WHITE ? BR : WR);
    
    while(rBb){
        Square rook = static_cast<Square>(popLSB(rBb));
        enemyAttackMap |= attacks.getRookAttack(rook, occWithoutKing);
    }



    // queen attack
    U64 qBb = board.getBitboard(tomove == WHITE ? BQ : WQ);
    
    while(qBb){
        Square queen = static_cast<Square>(popLSB(qBb));
        enemyAttackMap |= attacks.getQueenAttack(queen, occWithoutKing);
    }
}


void MoveGenerator::analyzeChecks(CheckInfo& info) const{
    computeChecks(info);
    if(info.checkerCount < 2) computePins(info);
}


void MoveGenerator::computeChecks(CheckInfo& info) const{
    Square sq = kingpos;


    // knight attack
    U64 nBb = board.getBitboard(tomove == WHITE ? BN : WN);
    
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
    U64 pBb = board.getBitboard(tomove == WHITE ? BP : WP);
    
    while(pBb){
        Square pawn = static_cast<Square>(popLSB(pBb));
        U64 attack = 0;

        if(tomove == WHITE) attack = attacks.getBlackPawnAttack(pawn);
        else attack = attacks.getWhitePawnAttack(pawn);

        if(attack & (1ULL<<sq)){
            info.checkMask |= (1ULL<<pawn);
            info.checkerCount++;
            info.checkers |= (1ULL<<pawn);
        }
    }



    // bishop attack
    U64 bBb = board.getBitboard(tomove == WHITE ? BB : WB);
    
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
    U64 rBb = board.getBitboard(tomove == WHITE ? BR : WR);
    
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
    U64 qBb = board.getBitboard(tomove == WHITE ? BQ : WQ);
    
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

    Piece queen = (tomove == WHITE ? BQ : WQ);
    Piece bishop = (tomove == WHITE ? BB : WB);
    
    U64 bishopray = attacks.getBishopAttack(kingpos, occ);
    U64 blockers = bishopray & friendlyocc;

    if(blockers == 0) return ;

    U64 complete = attacks.getBishopAttack(kingpos, occ^blockers);
    U64 pinners = complete & (board.getBitboard(queen) | board.getBitboard(bishop));

    while(pinners){
        Square pinner = static_cast<Square>(popLSB(pinners));

        U64 betweenMask = between[kingpos][pinner];
        U64 blockers = betweenMask & friendlyocc;

        if(popCount(blockers) == 1){
            Square pinned = static_cast<Square>(popLSB(blockers));

            info.pinnedPieces |= (1ULL<<pinned);
            info.pinnedRay[pinned] = betweenMask | (1ULL<<pinner);
        }
    }
}


void MoveGenerator::computeOrthogonalPins(CheckInfo& info) const{

    Piece queen = (tomove == WHITE ? BQ : WQ);
    Piece rook = (tomove == WHITE ? BR : WR);
    
    U64 rookray = attacks.getRookAttack(kingpos, occ);
    U64 blockers = rookray & friendlyocc;

    if(blockers == 0) return ;

    U64 complete = attacks.getRookAttack(kingpos, occ^blockers);
    U64 pinners = complete & (board.getBitboard(queen) | board.getBitboard(rook));

    while(pinners){
        Square pinner = static_cast<Square>(popLSB(pinners));

        U64 betweenMask = between[kingpos][pinner];
        U64 blockers = betweenMask & friendlyocc;

        if(popCount(blockers) == 1){
            Square pinned = static_cast<Square>(popLSB(blockers));

            info.pinnedPieces |= (1ULL<<pinned);
            info.pinnedRay[pinned] = betweenMask | (1ULL<<pinner);
        }
    }
}



int MoveGenerator::generateLegalMoves(Move moves[]){
    tomove = board.getMovingSide();
    occ = board.getOccupancy(BOTH);
    friendlyocc = board.getOccupancy(tomove);

    Piece p = (tomove == WHITE ? WK : BK);
    U64 pos = board.getBitboard(p);
    kingpos = static_cast<Square>(popLSB(pos));

    createAttackMap();
    CheckInfo info = {};
    analyzeChecks(info);
    cnt = 0;

    generateKingMoves(moves, info);

    if(info.checkerCount >= 2) return cnt;

    generateKnightMoves(moves, info);
    generatePawnMoves(moves, info);
    generateBishopMoves(moves,info);
    generateRookMoves(moves, info);
    generateQueenMoves(moves, info);

    return cnt;
}




void MoveGenerator::generateKingMoves(Move moves[],CheckInfo& info){
    (void)info;
    U64 kattacks = attacks.getKingAttack(kingpos) & (~friendlyocc);

    Piece moved = (tomove == WHITE ? WK : BK);
    Castling kingside = (tomove == WHITE ? CASTLE_WK : CASTLE_BK);
    Castling queenside = (tomove == WHITE ? CASTLE_WQ : CASTLE_BQ);

    int castleRights = board.getCastlingRights();

    while(kattacks){
        int pos = popLSB(kattacks);
        if(enemyAttackMap & (1ULL<<pos)) continue;

        Square dest = static_cast<Square>(pos);
        Piece captured = board.getPieceBoard(dest);
        MoveFlag f = (captured == EMPTY ? quiet : capture);

        moves[cnt] = {kingpos, dest, moved, captured, EMPTY, f};
        cnt++;
    }


    // castle not possible
    if(enemyAttackMap & (1ULL<<kingpos)) return ;


    // king side castle
    if((castleRights & kingside)){
        static constexpr U64 wk_side = (1ULL<<F1) | (1ULL<<G1); 
        static constexpr U64 bk_side = (1ULL<<F8) | (1ULL<<G8);

        if(tomove == WHITE){
            if(!(enemyAttackMap & wk_side) && !(occ & wk_side)){
                moves[cnt] = {kingpos, G1, WK, EMPTY, EMPTY, kingSideCastle};
                cnt++;
            }
        }
        
        else{
            if(!(enemyAttackMap & bk_side) && !(occ & bk_side)){
                moves[cnt] = {kingpos, G8, BK, EMPTY, EMPTY, kingSideCastle};
                cnt++;
            }
        }
    }


    // queen side
    if(castleRights & queenside){
        static constexpr U64 wq_safe = (1ULL<<D1) | (1ULL<<C1); 
        static constexpr U64 bq_safe = (1ULL<<D8) | (1ULL<<C8);
        static constexpr U64 wq_side = wq_safe | (1ULL<<B1); 
        static constexpr U64 bq_side = bq_safe | (1ULL<<B8); 

        if(tomove == WHITE){
            if(!(enemyAttackMap & wq_safe) && !(occ & wq_side)){
                moves[cnt] = {kingpos, C1, WK, EMPTY, EMPTY, queenSideCastle};
                cnt++;
            }
        }
        
        else{
            if(!(enemyAttackMap & bq_safe) && !(occ & bq_side)){
                moves[cnt] = {kingpos, C8, BK, EMPTY, EMPTY, queenSideCastle};
                cnt++;
            }
        }
    }
}



void MoveGenerator::fillMoves(Move moves[], U64 mask, Square from, Piece moved){
    while(mask){
        Square dest = static_cast<Square>(popLSB(mask));
        Piece captured = board.getPieceBoard(dest);
        MoveFlag f = (captured == EMPTY ? quiet : capture);

        moves[cnt] = {from, dest, moved, captured, EMPTY, f};
        cnt++;
    }
}



void MoveGenerator::generateKnightMoves(Move moves[],CheckInfo &info){

    Piece moved = (tomove == WHITE ? WN : BN);
    U64 knight = board.getBitboard(moved);

    while(knight){
        int pos = popLSB(knight);
        Square from = static_cast<Square>(pos);

        // piece is pinned
        if(info.pinnedPieces & (1ULL<<pos)) continue;

        U64 attack = attacks.getKnightAttack(from) & (~friendlyocc);

        // king in check
        if(info.checkerCount == 1) attack &= info.checkMask;

        fillMoves(moves,attack,from,moved);
    }
}




void MoveGenerator::generateBishopMoves(Move moves[],CheckInfo &info){

    Piece moved = (tomove == WHITE ? WB : BB);
    U64 bishop = board.getBitboard(moved);

    while(bishop){
        int pos = popLSB(bishop);
        Square from = static_cast<Square>(pos);

        U64 attack = attacks.getBishopAttack(from, occ) & (~friendlyocc);

        // piece is pinned
        if(info.pinnedPieces & (1ULL<<pos)) attack &= info.pinnedRay[from];

        // king in check
        if(info.checkerCount == 1) attack &= info.checkMask;

        fillMoves(moves,attack,from,moved);
    }
}




void MoveGenerator::generateRookMoves(Move moves[],CheckInfo &info){

    Piece moved = (tomove == WHITE ? WR : BR);
    U64 rook = board.getBitboard(moved);

    while(rook){
        int pos = popLSB(rook);
        Square from = static_cast<Square>(pos);

        U64 attack = attacks.getRookAttack(from, occ) & (~friendlyocc);

        // piece is pinned
        if(info.pinnedPieces & (1ULL<<pos)) attack &= info.pinnedRay[from];

        // king in check
        if(info.checkerCount == 1) attack &= info.checkMask;

        fillMoves(moves,attack,from,moved);
    }
}




void MoveGenerator::generateQueenMoves(Move moves[],CheckInfo &info){

    Piece moved = (tomove == WHITE ? WQ : BQ);
    U64 queen = board.getBitboard(moved);

    while(queen){
        int pos = popLSB(queen);
        Square from = static_cast<Square>(pos);

        U64 attack = attacks.getQueenAttack(from, occ) & (~friendlyocc);

        // piece is pinned
        if(info.pinnedPieces & (1ULL<<pos)) attack &= info.pinnedRay[from];

        // king in check
        if(info.checkerCount == 1) attack &= info.checkMask;

        fillMoves(moves,attack,from,moved);
    }
}



bool MoveGenerator::iskingattacked(U64 new_occ){
    U64 bishop = board.getBitboard(tomove == WHITE ? BB : WB);
    U64 rook = board.getBitboard(tomove == WHITE ? BR : WR);
    U64 queen = board.getBitboard(tomove == WHITE ? BQ : WQ);
    Square king = kingpos;

    U64 bishopAttack = attacks.getBishopAttack(king, new_occ);
    if (bishopAttack & (bishop | queen)) return true;

    U64 rookAttack = attacks.getRookAttack(king, new_occ);
    if (rookAttack & (rook | queen)) return true;

    return false;
}



void MoveGenerator::generatePawnMoves(Move moves[],CheckInfo &info){

    Piece moved = (tomove == WHITE ? WP : BP);
    U64 pawn = board.getBitboard(moved);
    U64 enemyocc = board.getOccupancy(tomove == WHITE ? BLACK : WHITE);

    constexpr int lastRank[2] = {RANK_SIZE - 1, 0};
    constexpr int firstRank[2] = {1, 6};
    constexpr Piece allprom[2][4] = {{WN, WB, WR, WQ}, {BN, BB, BR, BQ}}; 
    constexpr int rankoffset[2] = {8, -8};

    U64 validTargets = enemyocc;
    if(board.getEnPassant() != NO_SQUARE){
        validTargets |= (1ULL << board.getEnPassant());
    }

    while(pawn){
        int pos = popLSB(pawn);
        Square from = static_cast<Square>(pos);

        U64 attack = (tomove == WHITE ? attacks.getWhitePawnAttack(from) : attacks.getBlackPawnAttack(from))
            & validTargets;

        
        // king in check
        if(info.checkerCount == 1){
            U64 prev = attack;
            attack &= info.checkMask;

            Square ep = board.getEnPassant();

            if(ep != NO_SQUARE && (prev & (1ULL<<ep))){
                Square cap = static_cast<Square>(ep + rankoffset[!tomove]);

                if(info.checkers & (1ULL<<cap)){
                    attack |= (1ULL<<ep);
                }
            }
        }


        // piece is pinned
        if(info.pinnedPieces & (1ULL<<pos)) attack &= info.pinnedRay[from];

        while(attack){
            Square dest = static_cast<Square>(popLSB(attack));
            Piece captured = EMPTY;
            MoveFlag f = quiet;

            // enpassant
            if(board.getEnPassant() == dest){
                int opp = dest + rankoffset[!tomove];
                Square cap = static_cast<Square>(opp);
                captured = board.getPieceBoard(cap);
                f = enPassant;

                // legalty check
                U64 new_occ = occ ^ (1ULL<<from) ^ (1ULL<<dest) ^ (1ULL<<cap);
                if(iskingattacked(new_occ)) continue;
            }

            // normal capture
            else if(board.getPieceBoard(dest) != EMPTY){
                captured = board.getPieceBoard(dest);
                f = capture;
            }

            // promotion
            if(getRank(dest) == lastRank[tomove]){
                if(f == capture) f = promotion_capture;
                else f = promotion;

                for(auto prom : allprom[tomove]){
                    moves[cnt] = {from, dest, moved, captured, prom, f};
                    cnt++;
                }
            }
            
            else{
                moves[cnt] = {from, dest, moved, captured, EMPTY, f};
                cnt++;
            }
        }


        // single pawn push
        Square firstpush = static_cast<Square>(from + rankoffset[tomove]);
        U64 first_mask = (1ULL<<firstpush);

        if(!(occ & first_mask)){
            U64 cur_single = first_mask;
            
            // piece is pinned
            if(info.pinnedPieces & (1ULL<<pos)) cur_single &= info.pinnedRay[from];

            // king in check
            if(info.checkerCount == 1) cur_single &= info.checkMask;

            if(cur_single){
                if (getRank(firstpush) == lastRank[tomove]){
                    for(auto prom : allprom[tomove]){
                        moves[cnt] = {from, firstpush, moved, EMPTY, prom, promotion};
                        cnt++;
                    }
                }
                else{
                    moves[cnt] = {from, firstpush, moved, EMPTY, EMPTY, quiet};
                    cnt++;
                }
            }

            // double pawn push
            if(getRank(from) == firstRank[tomove]){
                Square secondpush = static_cast<Square>(firstpush + rankoffset[tomove]);
                U64 second_mask = (1ULL<<secondpush);

                if(!(occ & second_mask)){
                    U64 cur_double = second_mask;

                    // piece is pinned
                    if(info.pinnedPieces & (1ULL<<pos)) cur_double &= info.pinnedRay[from];

                    // king in check
                    if(info.checkerCount == 1) cur_double &= info.checkMask;

                    if(cur_double){
                        moves[cnt] = {from, secondpush, moved, EMPTY, EMPTY, doublePawnPush};
                        cnt++;
                    }
                }
            }
        }
    }
}