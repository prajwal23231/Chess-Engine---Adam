#include "movegen.h"
#include "board/board.h"
#include <array>

using namespace std;
using namespace Bitboard;


MoveGenerator::MoveGenerator(Board &board, const Attacks &attacks)
    : board(board), attacks(attacks) {
    cnt = 0;
}


int MoveGenerator::generateLegalMoves(Move moves[]) {
    generatePseudoMoves(moves);

    Color toMove = board.getMovingSide();
    int legal = 0;

    for (int i = 0; i < cnt; i++) {
        if(moves[i].isCastle()){
            MoveFlag f = moves[i].getMoveFlag();
            Piece moved = moves[i].getMovedPiece();


            if(moved == WK){
                if(board.isSquareAttacked(E1)) continue;
            }

            else{
                if(board.isSquareAttacked(E8)) continue;
            }



            if(f == kingSideCastle){
                if(moved == WK){
                    if(board.isSquareAttacked(F1)) continue;
                }

                else{
                    if(board.isSquareAttacked(F8)) continue;
                }
            }

            else{
                if(moved == WK){
                    if(board.isSquareAttacked(D1)) continue;
                }

                else{
                    if(board.isSquareAttacked(D8)) continue;
                }
            }
        }

        board.makeMove(moves[i]);

        U64 kingbb = board.getBitboard((toMove == WHITE) ? WK : BK);
        Square kingPos = static_cast<Square>(popLSB(kingbb));

        if (!board.isSquareAttacked(kingPos)) {
            moves[legal] = moves[i];
            legal++;
        }

        board.undoMove(moves[i]);
    }

    cnt = legal;
    return legal;
}


int MoveGenerator::generatePseudoMoves(Move moves[]) {
    cnt = 0;
    generateKnightMoves(moves);
    generateKingMoves(moves);
    generatePawnMoves(moves);
    generateBishopMoves(moves);
    generateRookMoves(moves);
    generateQueenMoves(moves);
    return cnt;
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