#include "attacks.h"
using namespace std;
using namespace Bitboard;

namespace{
    constexpr int kingMove[8][2] = {{-1,0}, {1,0}, {-1,-1}, {1,1}, {1,-1}, {-1,1}, {0,-1}, {0,1}};
    constexpr int knightMove[8][2] = {{2,1},{2,-1},{1,-2},{-1,-2},{1,2},{-1,2},{-2,1},{-2,-1}};
    constexpr int whitePawnMove[2][2] = {{1,-1}, {1,1}};
    constexpr int blackPawnMove[2][2] = {{-1,-1}, {-1,1}};
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
            setBit(knightAttack[sq], new_sq);
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
            setBit(kingAttack[sq], new_sq);
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
            setBit(whitePawnAttack[sq], new_sq);
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
            setBit(blackPawnAttack[sq], new_sq);
        }
    }
}
