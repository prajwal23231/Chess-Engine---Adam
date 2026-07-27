#include "magic.h"
using namespace std;
using namespace Bitboard;

namespace{
    constexpr int bishopMoves[4][2] = {{-1,-1}, {1,1}, {-1,1}, {1,-1}};
    constexpr int rookMoves[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
};

Magic::Magic(){
    for(int x=0; x<BOARD_SIZE; x++){
        Square s = static_cast<Square>(x);

        bishopMasks[s] = bishopMask(s);
        rookMasks[s] = rookMask(s);
    }
}


U64 Magic::bishopMask(Square square) const{
    U64 mask = 0;
    int cur_rank = square / RANK_SIZE;
    int cur_file = square % RANK_SIZE;

    for (int i=0; i<4; i++){
        int new_rank = cur_rank + bishopMoves[i][0], new_file = cur_file + bishopMoves[i][1];

        while(max(new_rank , new_file) < RANK_SIZE-1 && min(new_rank, new_file) > 0){
            Square pos = static_cast<Square>(new_rank * RANK_SIZE + new_file);
            setBit(mask,pos);

            new_rank += bishopMoves[i][0];
            new_file += bishopMoves[i][1];
        }
    }

    return mask;
}


U64 Magic::rookMask(Square square) const{
    U64 mask = 0;
    int cur_rank = square / RANK_SIZE;
    int cur_file = square % RANK_SIZE;

    for (int i=0; i<4; i++){
        int new_rank = cur_rank + rookMoves[i][0], new_file = cur_file + rookMoves[i][1];

        while(max(new_rank , new_file) < RANK_SIZE-1 && min(new_rank, new_file) > 0){
            Square pos = static_cast<Square>(new_rank * RANK_SIZE + new_file);
            setBit(mask,pos);

            new_rank += rookMoves[i][0];
            new_file += rookMoves[i][1];
        }
    }

    return mask;
}