#include "magic.h"
#include "utils/magic_numbers.h"
#include <cassert>
using namespace std;
using namespace Bitboard;

namespace{
    constexpr int bishopMoves[4][2] = {{-1,-1}, {1,1}, {-1,1}, {1,-1}};
    constexpr int rookMoves[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};

    constexpr int getFile(int sq) { return sq % 8; }
    constexpr int getRank(int sq) { return sq / 8; }
};

Magic::Magic(){
    int sz1=0, sz2=0, relBits1=0, relBits2=0;

    for(int x=0; x<BOARD_SIZE; x++){
        Square s = static_cast<Square>(x);

        bishopMagic[s].mask = bishopMask(s);
        rookMagic[s].mask = rookMask(s);

        // getting size
        relBits1 = popCount(bishopMagic[s].mask);
        sz1 += (1ULL<<relBits1);

        relBits2 = popCount(rookMagic[s].mask);
        sz2 += (1ULL<<relBits2);

        bishopMagic[s].shift = 64 - relBits1;
        rookMagic[s].shift = 64 - relBits2;

        bishopMagic[s].magic = bishopMagicNum[s];
        rookMagic[s].magic = rookMagicNum[s];
    }


    bishopTable.resize(sz1);
    rookTable.resize(sz2);

    buildBishopTable();
    buildRookTable();

    validate();
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
    size_t offset=0;

    for(int x=0; x<BOARD_SIZE; x++){
        Square square = static_cast<Square>(x);
        int relevantBits = 64 - bishopMagic[square].shift;
        size_t tableSize = 1ULL<<relevantBits;

        bishopMagic[square].attacks = &bishopTable[offset];

        for(size_t index=0; index<tableSize; index++){
            U64 occ = setOccupancy(index, relevantBits, bishopMagic[square].mask);
            size_t hash = (occ * bishopMagic[square].magic) >> bishopMagic[square].shift;

            assert(hash < tableSize);
            
            bishopMagic[square].attacks[hash] = getBishopAttackOTF(square, occ);
        }

        offset += tableSize;
    }
}


void Magic::buildRookTable(){
    size_t offset=0;

    for(int x=0; x<BOARD_SIZE; x++){
        Square square = static_cast<Square>(x);
        int relevantBits = 64 - rookMagic[square].shift;
        size_t tableSize = 1ULL<<relevantBits;

        rookMagic[square].attacks = &rookTable[offset];

        for(size_t index=0; index<tableSize; index++){
            U64 occ = setOccupancy(index, relevantBits, rookMagic[square].mask);
            size_t hash = (occ * rookMagic[square].magic) >> rookMagic[square].shift;

            assert(hash < tableSize);
            
            rookMagic[square].attacks[hash] = getRookAttackOTF(square, occ);
        }

        offset += tableSize;
    }
}


U64 Magic::getBishopAttack(Square square, U64 occ) const{
    occ &= bishopMagic[square].mask;

    size_t hash = (occ * bishopMagic[square].magic) >> bishopMagic[square].shift;
    return bishopMagic[square].attacks[hash];
}


U64 Magic::getRookAttack(Square square, U64 occ) const{
    occ &= rookMagic[square].mask;

    size_t hash = (occ * rookMagic[square].magic) >> rookMagic[square].shift;
    return rookMagic[square].attacks[hash];
}


U64 Magic::getBishopMask(Square s) const{
    return bishopMagic[s].mask;
}


U64 Magic::getRookMask(Square s) const{
    return rookMagic[s].mask;
}


void Magic::validate() const{
    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);

        int bits = popCount(getBishopMask(s));
        int count = 1 << bits;

        for (int i = 0; i < count; i++) {
            U64 occ = setOccupancy(i, bits, getBishopMask(s));

            assert(
                getBishopAttack(s, occ) ==
                getBishopAttackOTF(s, occ)
            );
        }
    }


    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);

        int bits = popCount(getRookMask(s));
        int count = 1 << bits;

        for (int i = 0; i < count; i++) {
            U64 occ = setOccupancy(i, bits, getRookMask(s));

            assert(
                getRookAttack(s, occ) ==
                getRookAttackOTF(s, occ)
            );
        }
    }
}