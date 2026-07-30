#include "movegen.h"
#include "board/board.h"
#include <array>

using namespace std;
using namespace Bitboard;


MoveGenerator::MoveGenerator(Board &board) : board(board){
    cnt = 0;

    createAttackMap();
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


void MoveGenerator::generateKnightMoves(Move moves[]) {
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide == WHITE ? WN : BN);

    U64 bb = board.getBitboard(p);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    while (bb != 0) {
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack = attacks.getKnightAttack(s);

        all_attack = all_attack & (~movingSideOcc);

        while (all_attack != 0) {
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece captured = board.getPieceBoard(dest);

            MoveFlag f = (captured == EMPTY ? quiet : capture);

            moves[cnt++] = Move(s, dest, p, captured, EMPTY, f);
        }
    }
}

void MoveGenerator::generatePawnMoves(Move moves[]) {
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide == WHITE ? WP : BP);

    U64 bb = board.getBitboard(p);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    const array<Piece, 4> all_prom =
        (movingSide == WHITE ? array<Piece, 4>{WQ, WR, WB, WN}
                            : array<Piece, 4>{BQ, BR, BB, BN});

    const int fin_rank = (movingSide == WHITE ? RANK_SIZE - 1 : 0);

    const int rankOffset = (movingSide == WHITE ? 1 : -1);

    const int first_rank = (movingSide == WHITE ? 1 : 6);

    while (bb != 0) {
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack;

        if (movingSide == WHITE)
        all_attack = attacks.getWhitePawnAttack(s);
        else
        all_attack = attacks.getBlackPawnAttack(s);

        all_attack = all_attack & (~movingSideOcc);

        // capture move

        while (all_attack != 0) {
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece captured = board.getPieceBoard(dest);

            // enpassant check
            int cur_rank = getRank(dest);

            MoveFlag flag = capture;

            if (board.getEnPassant() == dest && captured == EMPTY) {
                flag = enPassant;
                captured = (movingSide == WHITE ? BP : WP);
            }

            if (captured == EMPTY && flag != enPassant)
                continue;

            // promotion check
            if (cur_rank == fin_rank) {
                for (Piece prom : all_prom) {
                moves[cnt++] = Move(s, dest, p, captured, prom, promotion_capture);
                }
            }

            else {
                moves[cnt++] = Move(s, dest, p, captured, EMPTY, flag);
            }
        }

        // single push
        int cur_rank = getRank(s);
        int new_rank = cur_rank + rankOffset;
        int cur_file = getFile(s);

        Square single_push = static_cast<Square>(new_rank * RANK_SIZE + cur_file);

        if (board.getPieceBoard(single_push) != EMPTY)
            continue;

        // promotion check
        if (new_rank == fin_rank) {
            for (Piece prom : all_prom) {
                moves[cnt++] = Move(s, single_push, p, EMPTY, prom, promotion);
            }
        }

        else {
            moves[cnt++] = Move(s, single_push, p, EMPTY, EMPTY, quiet);
        }

        // double pawn push check
        Square double_push =
            static_cast<Square>((cur_rank + 2 * rankOffset) * RANK_SIZE + cur_file);

        if (cur_rank != first_rank || board.getPieceBoard(double_push) != EMPTY) {
            continue;
        }

        moves[cnt++] = Move(s, double_push, p, EMPTY, EMPTY, doublePawnPush);
    }
}

void MoveGenerator::generateKingMoves(Move moves[]) {
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide == WHITE ? WK : BK);

    U64 bb = board.getBitboard(p);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    // capture or normal move
    while (bb != 0) {
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack = attacks.getKingAttack(s);

        all_attack = all_attack & (~movingSideOcc);

        while (all_attack != 0) {
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece captured = board.getPieceBoard(dest);

            MoveFlag f = (captured == EMPTY ? quiet : capture);

            moves[cnt++] = Move(s, dest, p, captured, EMPTY, f);
        }
    }

    // castling part
    int castle = board.getCastlingRights();

    if (movingSide == WHITE) {
        if (castle & CASTLE_WK) {
        // checking for empty path
            if (board.getPieceBoard(F1) == EMPTY &&
                board.getPieceBoard(G1) == EMPTY) {
                moves[cnt++] = Move(E1, G1, p, EMPTY, EMPTY, kingSideCastle);
            }
        }

        if (castle & CASTLE_WQ) {
        // checking for empty path
            if (board.getPieceBoard(D1) == EMPTY &&
                board.getPieceBoard(C1) == EMPTY &&
                board.getPieceBoard(B1) == EMPTY) {
                moves[cnt++] = Move(E1, C1, p, EMPTY, EMPTY, queenSideCastle);
            }
        }
    }

    else {
        if (castle & CASTLE_BK) {
        // checking for empty path
            if (board.getPieceBoard(F8) == EMPTY &&
                board.getPieceBoard(G8) == EMPTY) {
                moves[cnt++] = Move(E8, G8, p, EMPTY, EMPTY, kingSideCastle);
            }
        }

        if (castle & CASTLE_BQ) {
        // checking for empty path
            if (board.getPieceBoard(D8) == EMPTY &&
                board.getPieceBoard(C8) == EMPTY &&
                board.getPieceBoard(B8) == EMPTY) {
                moves[cnt++] = Move(E8, C8, p, EMPTY, EMPTY, queenSideCastle);
            }
        }
    }
}

void MoveGenerator::generateBishopMoves(Move moves[]) {
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide == WHITE ? WB : BB);

    U64 bb = board.getBitboard(p);
    U64 both_occ = board.getOccupancy(BOTH);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    while (bb != 0) {
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack = attacks.getBishopAttack(s, both_occ);

        all_attack = all_attack & (~movingSideOcc);

        while (all_attack != 0) {
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece captured = board.getPieceBoard(dest);

            MoveFlag f = (captured == EMPTY ? quiet : capture);

            moves[cnt++] = Move(s, dest, p, captured, EMPTY, f);
        }
    }
}

void MoveGenerator::generateRookMoves(Move moves[]) {
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide == WHITE ? WR : BR);

    U64 bb = board.getBitboard(p);
    U64 both_occ = board.getOccupancy(BOTH);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    while (bb != 0) {
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack = attacks.getRookAttack(s, both_occ);

        all_attack = all_attack & (~movingSideOcc);

        while (all_attack != 0) {
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece captured = board.getPieceBoard(dest);

            MoveFlag f = (captured == EMPTY ? quiet : capture);

            moves[cnt++] = Move(s, dest, p, captured, EMPTY, f);
        }
    }
}

void MoveGenerator::generateQueenMoves(Move moves[]) {
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide == WHITE ? WQ : BQ);

    U64 bb = board.getBitboard(p);
    U64 both_occ = board.getOccupancy(BOTH);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    while (bb != 0) {
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack = attacks.getQueenAttack(s, both_occ);

        all_attack = all_attack & (~movingSideOcc);

        while (all_attack != 0) {
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece captured = board.getPieceBoard(dest);

            MoveFlag f = (captured == EMPTY ? quiet : capture);

            moves[cnt++] = Move(s, dest, p, captured, EMPTY, f);
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

    Square kingSq = getKingpos();

    int cur_rank = getRank(kingSq);
    int cur_file = getFile(kingSq);

    constexpr int dr[4] = {1, 1, -1, -1};
    constexpr int df[4] = {1, -1, 1, -1};

    U64 occ = board.getOccupancy(toMove);

    for(int i=0; i<4; i++){
        int new_rank = cur_rank + dr[i];
        int new_file = cur_file + df[i];
        Square pinned = NO_SQUARE;

        U64 mask = 0;

        while(new_rank >= 0 && new_file >= 0 && new_rank < RANK_SIZE && new_file < RANK_SIZE){
            Square cur_sq = static_cast<Square>(new_rank*RANK_SIZE + new_file);
            Piece p = board.getPieceBoard(cur_sq);

            if(occ & (1ULL<<cur_sq)){
                if(pinned == NO_SQUARE) pinned = cur_sq;
                
                else{
                    pinned = NO_SQUARE;
                    break;
                }
            }

            else if(p != EMPTY){
                if(p == bishop || p == queen){
                    if(pinned != NO_SQUARE){
                        mask |= between[kingSq][cur_sq] | (1ULL << cur_sq); 
                    }
                }

                break;
            }

            new_rank += dr[i];
            new_file += df[i];
        }

        if(pinned != NO_SQUARE && mask){
            info.pinnedRay[pinned] = mask;
            info.pinnedPieces |= (1ULL<<pinned);
        }
    }
}



void MoveGenerator::computeOrthogonalPins(CheckInfo& info) const{
    Color toMove = board.getMovingSide();

    Piece rook = (toMove == WHITE ? BR : WR);
    Piece queen = (toMove == WHITE ? BQ : WQ);

    Square kingSq = getKingpos();

    int cur_rank = getRank(kingSq);
    int cur_file = getFile(kingSq);

    constexpr int dr[4] = {1, 0, -1, 0};
    constexpr int df[4] = {0, -1, 0, 1};

    U64 occ = board.getOccupancy(toMove);

    for(int i=0; i<4; i++){
        int new_rank = cur_rank + dr[i];
        int new_file = cur_file + df[i];
        Square pinned = NO_SQUARE;

        U64 mask = 0;

        while(new_rank >= 0 && new_file >= 0 && new_rank < RANK_SIZE && new_file < RANK_SIZE){
            Square cur_sq = static_cast<Square>(new_rank*RANK_SIZE + new_file);
            Piece p = board.getPieceBoard(cur_sq);

            if(occ & (1ULL<<cur_sq)){
                if(pinned == NO_SQUARE) pinned = cur_sq;

                else{
                    pinned = NO_SQUARE;
                    break;
                }
            }

            else if(p != EMPTY){
                if(p == rook || p == queen){
                    if(pinned != NO_SQUARE){
                        mask |= between[kingSq][cur_sq] | (1ULL << cur_sq);
                    }
                }

                break;
            }

            new_rank += dr[i];
            new_file += df[i];
        }

        if(pinned != NO_SQUARE && mask){
            info.pinnedRay[pinned] = mask;
            info.pinnedPieces |= (1ULL<<pinned);
        }
    }
}