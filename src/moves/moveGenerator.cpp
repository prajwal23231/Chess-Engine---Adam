#include "moveGenerator.h"
#include <array>

using namespace std;
using namespace Bitboard;


MoveGenerator::MoveGenerator(const Board &board, const Attacks &attacks)
    : board(board), attacks(attacks){
}

void MoveGenerator::generateMoves(vector<Move>& moves) const{
    moves.clear();

    generateKnightMoves(moves);
    generateKingMoves(moves);
    generatePawnMoves(moves);
    generateBishopMoves(moves);
    generateRookMoves(moves);
    generateQueenMoves(moves);
}

void MoveGenerator::generateKnightMoves(vector<Move>& moves) const{
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide==WHITE ? WN : BN);

    U64 bb = board.getBitboard(p);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    while(bb != 0){
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack = attacks.getKnightAttack(s);
        
        all_attack = all_attack & (~movingSideOcc);

        while(all_attack != 0){
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece Captured = board.getPieceBoard(dest);

            MoveFlag f = (Captured == EMPTY ? quiet : capture);

            moves.emplace_back(s, dest, p, Captured, EMPTY, f);
        }
    }
}


void MoveGenerator::generateBishopMoves(vector<Move>& moves) const{
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide==WHITE ? WB : BB);

    U64 bb = board.getBitboard(p);
    U64 both_occ =  board.getOccupancy(BOTH);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    while(bb != 0){
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack = attacks.getBishopAttack(s, both_occ);
        
        all_attack = all_attack & (~movingSideOcc);

        while(all_attack != 0){
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece Captured = board.getPieceBoard(dest);

            MoveFlag f = (Captured == EMPTY ? quiet : capture);

            moves.emplace_back(s, dest, p, Captured, EMPTY, f);
        }
    }
}


void MoveGenerator::generateQueenMoves(vector<Move>& moves) const{
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide==WHITE ? WQ : BQ);

    U64 bb = board.getBitboard(p);
    U64 both_occ =  board.getOccupancy(BOTH);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    while(bb != 0){
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack = attacks.getQueenAttack(s, both_occ);
        
        all_attack = all_attack & (~movingSideOcc);

        while(all_attack != 0){
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece Captured = board.getPieceBoard(dest);

            MoveFlag f = (Captured == EMPTY ? quiet : capture);

            moves.emplace_back(s, dest, p, Captured, EMPTY, f);
        }
    }
}

void MoveGenerator::generatePawnMoves(vector<Move>& moves) const{
    Color movingSide = board.getMovingSide();
    Piece p = (movingSide==WHITE ? WP : BP);

    U64 bb = board.getBitboard(p);
    U64 movingSideOcc = board.getOccupancy(movingSide);

    const array<Piece,4> all_prom = (
        movingSide == WHITE ?
        array<Piece,4> {WQ, WR, WB, WN} :
        array<Piece,4> {BQ, BR , BB, BN});

    const int fin_rank = (movingSide==WHITE ? RANK_SIZE-1 : 0);

    const int rankOffset = (movingSide==WHITE ? 1 : -1);

    const int first_rank = (movingSide == WHITE ? 1 : 6);



    while(bb != 0){
        Square s = static_cast<Square>(popLSB(bb));

        U64 all_attack ;

        if(movingSide == WHITE) all_attack = attacks.getWhitePawnAttack(s);
        else all_attack = attacks.getBlackPawnAttack(s);
        
        all_attack = all_attack & (~movingSideOcc);

        // capture move

        while(all_attack != 0){
            Square dest = static_cast<Square>(popLSB(all_attack));
            Piece Captured = board.getPieceBoard(dest);

            // enpassant check
            int cur_rank = dest / RANK_SIZE;

            MoveFlag flag = capture;

            if(board.getEnPassant() == dest && Captured == EMPTY){
                flag = enPassant;
                Captured = (movingSide == WHITE ? BP : WP);
            }

            if(Captured == EMPTY && flag != enPassant) continue;


            // promotion check
            if(cur_rank == fin_rank ) {
                for(int i=0; i<4; i++){
                    moves.emplace_back(s, dest, p, Captured, all_prom[i], promotion_capture);
                }
            }

            else{
                moves.emplace_back(s, dest, p, Captured, EMPTY, flag);
            }
        }


        // single push
        int cur_rank = s / RANK_SIZE ;
        int new_rank = cur_rank + rankOffset;
        int cur_file = s % RANK_SIZE;

        Square single_push = static_cast<Square>(new_rank*RANK_SIZE + cur_file);

        if(board.getPieceBoard(single_push) != EMPTY) continue;

        // promotion check
        if(new_rank == fin_rank){
            for(int i=0; i<4; i++){
                moves.emplace_back(s, single_push, p, EMPTY, all_prom[i], promotion);
            }
        }

        else{
            moves.emplace_back(s, single_push, p, EMPTY, EMPTY, quiet);
        }


        // double pawn push check
        Square double_push = static_cast<Square>((cur_rank + 2*rankOffset)*RANK_SIZE + cur_file);

        if(cur_rank != first_rank || 
        board.getPieceBoard(double_push) != EMPTY){
            continue;
        }

        moves.emplace_back(s, double_push, p, EMPTY, EMPTY, doublePawnPush);
    }
}



