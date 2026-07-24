#include "attacks.h"
using namespace std;

inline int getRank(int sq) { return sq / 8; }
inline int getFile(int sq) { return sq % 8; }

Attacks::Attacks(){
    knightAttack.fill(0);
    kingAttack.fill(0);
    blackPawnAttack.fill(0);
    whitePawnAttack.fill(0);


    // knight attack generation
    static constexpr int attack_dir[8][2] = {{2,1},{2,-1},{1,-2},{-1,-2},{1,2},{-1,2},{-2,1},{-2,-1}};

    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int cur_rank = getRank(sq);
        int cur_file = getFile(sq);

        for(int j=0; j<8; j++){
            int new_rank = cur_rank + attack_dir[j][0];
            int new_file = cur_file + attack_dir[j][1];

            if(new_rank < 0 || new_rank >= RANK_SIZE || new_file < 0 || new_file >= RANK_SIZE) continue;

            int new_sq = new_rank*RANK_SIZE + new_file;
            knightAttack[sq] |= (1ULL<<new_sq);
        }
    }


    // king attack generation
    static constexpr int king_move[8][2] = {{-1,0}, {1,0}, {-1,-1}, {1,1}, {1,-1}, {-1,1}, {0,-1}, {0,1}};

    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int cur_rank = getRank(sq);
        int cur_file = getFile(sq);

        for(int j=0; j<8; j++){
            int new_rank = cur_rank + king_move[j][0];
            int new_file = cur_file + king_move[j][1];

            if(new_rank < 0 || new_rank >= RANK_SIZE || new_file < 0 || new_file >= RANK_SIZE) continue;

            int new_sq = new_rank*RANK_SIZE + new_file;
            kingAttack[sq] |= (1ULL<<new_sq);
        }
    }


    // white pawn move gen
    static constexpr int white_pawn[2][2] = {{1,-1}, {1,1}};

    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int cur_rank = getRank(sq);
        int cur_file = getFile(sq);

        for(int j=0; j<2; j++){
            int new_rank = cur_rank + white_pawn[j][0];
            int new_file = cur_file + white_pawn[j][1];

            if(new_rank < 0 || new_rank >= RANK_SIZE || new_file < 0 || new_file >= RANK_SIZE) continue;

            int new_sq = new_rank*RANK_SIZE + new_file;
            whitePawnAttack[sq] |= (1ULL<<new_sq);
        }
    }



    // black pawn move gen
    static constexpr int black_pawn[2][2] = {{-1,-1}, {-1,1}};

    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int cur_rank = getRank(sq);
        int cur_file = getFile(sq);

        for(int j=0; j<2; j++){
            int new_rank = cur_rank + black_pawn[j][0];
            int new_file = cur_file + black_pawn[j][1];

            if(new_rank < 0 || new_rank >= RANK_SIZE || new_file < 0 || new_file >= RANK_SIZE) continue;

            int new_sq = new_rank*RANK_SIZE + new_file;
            blackPawnAttack[sq] |= (1ULL<<new_sq);
        }
    }
}

U64 Attacks::getKnightAttack(Square square) {
    return knightAttack[square];
}

U64 Attacks::getKingAttack(Square square) {
    return kingAttack[square];
}

U64 Attacks::getWhitePawnAttack(Square square) {
    return whitePawnAttack[square];
}

U64 Attacks::getBlackPawnAttack(Square square) {
    return blackPawnAttack[square];
}