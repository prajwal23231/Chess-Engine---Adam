#include "attacks.h"
using namespace std;

namespace{
    constexpr int kingMove[8][2] = {{-1,0}, {1,0}, {-1,-1}, {1,1}, {1,-1}, {-1,1}, {0,-1}, {0,1}};
    constexpr int knightMove[8][2] = {{2,1},{2,-1},{1,-2},{-1,-2},{1,2},{-1,2},{-2,1},{-2,-1}};
    constexpr int whitePawnMove[2][2] = {{1,-1}, {1,1}};
    constexpr int blackPawnMove[2][2] = {{-1,-1}, {-1,1}};

    constexpr int getFile(int sq) { return sq % 8; }
    constexpr int getRank(int sq) { return sq / 8; }
}

Attacks::Attacks(){
    knightAttack.fill(0);
    kingAttack.fill(0);
    blackPawnAttack.fill(0);
    whitePawnAttack.fill(0);


    // knight attack generation

    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int cur_rank = getRank(sq);
        int cur_file = getFile(sq);

        for(int j=0; j<8; j++){
            int new_rank = cur_rank + knightMove[j][0];
            int new_file = cur_file + knightMove[j][1];

            if(new_rank < 0 || new_rank >= RANK_SIZE || new_file < 0 || new_file >= RANK_SIZE) continue;

            Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);
            knightAttack[sq] |= (1ULL<<new_sq);
        }
    }


    // king attack generation

    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int cur_rank = getRank(sq);
        int cur_file = getFile(sq);

        for(int j=0; j<8; j++){
            int new_rank = cur_rank + kingMove[j][0];
            int new_file = cur_file + kingMove[j][1];

            if(new_rank < 0 || new_rank >= RANK_SIZE || new_file < 0 || new_file >= RANK_SIZE) continue;

            Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);
            kingAttack[sq] |= (1ULL<<new_sq);
        }
    }


    // white pawn move gen

    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int cur_rank = getRank(sq);
        int cur_file = getFile(sq);

        for(int j=0; j<2; j++){
            int new_rank = cur_rank + whitePawnMove[j][0];
            int new_file = cur_file + whitePawnMove[j][1];

            if(new_rank < 0 || new_rank >= RANK_SIZE || new_file < 0 || new_file >= RANK_SIZE) continue;

            Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);
            whitePawnAttack[sq] |= (1ULL<<new_sq);
        }
    }



    // black pawn move gen

    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int cur_rank = getRank(sq);
        int cur_file = getFile(sq);

        for(int j=0; j<2; j++){
            int new_rank = cur_rank + blackPawnMove[j][0];
            int new_file = cur_file + blackPawnMove[j][1];

            if(new_rank < 0 || new_rank >= RANK_SIZE || new_file < 0 || new_file >= RANK_SIZE) continue;

            Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);
            blackPawnAttack[sq] |= (1ULL<<new_sq);
        }
    }
}

U64 Attacks::getKnightAttack(Square square) const{
    return knightAttack[square];
}

U64 Attacks::getKingAttack(Square square) const{
    return kingAttack[square];
}

U64 Attacks::getWhitePawnAttack(Square square) const{
    return whitePawnAttack[square];
}

U64 Attacks::getBlackPawnAttack(Square square) const{
    return blackPawnAttack[square];
}

U64 Attacks::getBishopAttack(Square square, U64 occupancy) const{
    U64 bishopAttack = 0;

    int cur_rank = getRank(square);
    int cur_file = getFile(square);
    int new_rank = cur_rank+1, new_file = cur_file+1;

    while(new_rank < RANK_SIZE && new_file < RANK_SIZE){
        Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);

        U64 mask = (1ULL<<new_sq);
        bishopAttack |= mask;

        if(occupancy & mask) break;
        new_rank++, new_file++;
    }

    new_rank = cur_rank+1, new_file = cur_file-1;

    while(new_rank < RANK_SIZE && new_file >= 0){
        Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);

        U64 mask = (1ULL<<new_sq);
        bishopAttack |= mask;

        if(occupancy & mask) break;
        new_rank++, new_file--;
    }

    new_rank = cur_rank-1, new_file = cur_file-1;

    while(new_rank >= 0 && new_file >= 0){
        Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);

        U64 mask = (1ULL<<new_sq);
        bishopAttack |= mask;

        if(occupancy & mask) break;
        new_rank--, new_file--;
    }

    new_rank = cur_rank-1, new_file = cur_file+1;

    while(new_rank >= 0 && new_file < RANK_SIZE){
        Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);

        U64 mask = (1ULL<<new_sq);
        bishopAttack |= mask;

        if(occupancy & mask) break;
        new_rank--, new_file++;
    }

    return bishopAttack;
}

U64 Attacks::getRookAttack(Square square, U64 occupancy) const{
    U64 RookAttack = 0;

    int cur_rank = getRank(square);
    int cur_file = getFile(square);
    int new_rank = cur_rank+1, new_file = cur_file;

    while(new_rank < RANK_SIZE){
        Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);

        U64 mask = (1ULL<<new_sq);
        RookAttack |= mask;

        if(occupancy & mask) break;
        new_rank++;
    }

    new_rank = cur_rank, new_file = cur_file+1;

    while(new_file < RANK_SIZE){
        Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);

        U64 mask = (1ULL<<new_sq);
        RookAttack |= mask;

        if(occupancy & mask) break;
        new_file++;
    }

    new_rank = cur_rank-1, new_file = cur_file;

    while(new_rank >= 0){
        Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);

        U64 mask = (1ULL<<new_sq);
        RookAttack |= mask;

        if(occupancy & mask) break;
        new_rank--;
    }

    new_rank = cur_rank, new_file = cur_file-1;

    while(new_file >= 0){
        Square new_sq = static_cast<Square>(new_rank * RANK_SIZE + new_file);

        U64 mask = (1ULL<<new_sq);
        RookAttack |= mask;

        if(occupancy & mask) break;
        new_file--;
    }

    return RookAttack;
}

U64 Attacks::getQueenAttack(Square square, U64 occupancy) const{
    return getBishopAttack(square, occupancy) | getRookAttack(square, occupancy);
}