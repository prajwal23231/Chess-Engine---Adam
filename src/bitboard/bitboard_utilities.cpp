#include "bitboard_utilities.h"
#include <iostream>
using namespace::std;

bool Bitboard::getBit(U64 bb,int square) {
    U64 mask = (1ULL<<square);
    return bb&mask;
}

void Bitboard::setBit(U64 &bb,int square){
    U64 mask = (1ULL<<square);
    bb |= mask;
}

void Bitboard::clearBit(U64 &bb,int square){
    U64 mask = ~(1ULL<<square);
    bb &= mask;
}

int Bitboard::popCount(U64 bb) {
    return __builtin_popcountll(bb);
}

int Bitboard::lsb(U64 bb) {
    return bb ? __builtin_ctzll(bb) : -1;
}

int Bitboard::popLSB(U64& bb){
    int pos = lsb(bb);
    if(pos==-1) return pos;

    bb &= bb-1;
    return pos;
}

void Bitboard::printBitboard(U64 bb){
    for(int i=RANK_SIZE; i>0; i--){
        cout<<i<<"  ";

        for(int j=0; j<RANK_SIZE; j++){
            int square = (i-1)*RANK_SIZE + j;

            cout<<getBit(bb,square)<<" ";
        }

        cout<<"\n";
    }

    cout<<"\n   ";

    for(char c='a'; c<='h'; c++){
        cout<<c<<" ";
    }

    cout<<"\n";
}