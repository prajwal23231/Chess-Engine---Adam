#include "magic.h"
using namespace std;
using namespace Bitboard;

namespace{
    constexpr int bishopMoves[4][2] = {{-1,-1}, {1,1}, {-1,1}, {1,-1}};
    constexpr int rookMoves[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};

    constexpr int getFile(int sq) { return sq % 8; }
    constexpr int getRank(int sq) { return sq / 8; }
};

Magic::Magic(){
    for(int x=0; x<BOARD_SIZE; x++){
        Square s = static_cast<Square>(x);

        bishopMasks[s] = bishopMask(s);
        rookMasks[s] = rookMask(s);
    }

    buildBishopTable();
    buildRookTable();
}


U64 Magic::bishopMask(Square square) const{
    U64 mask = 0;
    int cur_rank = getRank(square);
    int cur_file = getFile(square);

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
    int cur_rank = getRank(square);
    int cur_file = getFile(square);

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


U64 Magic::setOccupancy(int index, int relevantBits, U64 attackMask) const{
    U64 blocked = 0;

    for(int i=0; i<relevantBits; i++){
        int bit = (1<<i);
        int x = popLSB(attackMask);
        Square s = static_cast<Square>(x);

        if(index & bit){
            setBit(blocked,s);
        }
    }

    return blocked;
}


U64 Magic::getBishopAttackOTF(Square square, U64 occupancy) const{
    U64 bishopAttack = 0;

    int cur_rank = getRank(square);
    int cur_file = getFile(square);

    for (int i=0; i<4; i++){
        int new_rank = cur_rank + bishopMoves[i][0], new_file = cur_file + bishopMoves[i][1];

        while(max(new_rank , new_file) < RANK_SIZE && min(new_rank, new_file) >= 0){
            Square pos = static_cast<Square>(new_rank * RANK_SIZE + new_file);
            setBit(bishopAttack,pos);

            if(occupancy & (1ULL<<pos)) break;

            new_rank += bishopMoves[i][0];
            new_file += bishopMoves[i][1];
        }
    }

    return bishopAttack;
}


U64 Magic::getRookAttackOTF(Square square, U64 occupancy) const{
    U64 rookAttack = 0;

    int cur_rank = getRank(square);
    int cur_file = getFile(square);

    for (int i=0; i<4; i++){
        int new_rank = cur_rank + rookMoves[i][0], new_file = cur_file + rookMoves[i][1];

        while(max(new_rank , new_file) < RANK_SIZE && min(new_rank, new_file) >= 0){
            Square pos = static_cast<Square>(new_rank * RANK_SIZE + new_file);
            setBit(rookAttack,pos);

            if(occupancy & (1ULL<<pos)) break;

            new_rank += rookMoves[i][0];
            new_file += rookMoves[i][1];
        }
    }

    return rookAttack;
}


void Magic::buildBishopTable(){
    for(int x=0; x<BOARD_SIZE; x++){
        Square square = static_cast<Square>(x);
        int relevantBits = popCount(bishopMasks[square]);

        int maxInd = 1<<relevantBits;

        for(int index=0; index<maxInd; index++){
            U64 occ = setOccupancy(index, relevantBits, bishopMasks[square]);
            bishopTable[square].push_back(getBishopAttackOTF(square, occ));
        }
    }
}


void Magic::buildRookTable(){
    for(int x=0; x<BOARD_SIZE; x++){
        Square square = static_cast<Square>(x);
        int relevantBits = popCount(rookMasks[square]);

        int maxInd = 1<<relevantBits;

        for(int index=0; index<maxInd; index++){
            U64 occ = setOccupancy(index, relevantBits, rookMasks[square]);
            rookTable[square].push_back(getRookAttackOTF(square, occ));
        }
    }
}