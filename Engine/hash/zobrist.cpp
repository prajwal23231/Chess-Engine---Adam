#include "zobrist.h"
#include "board/board.h"
#include "utils/bitboard_utilities.h"
#include <random>

U64 Zobrist::pieceKeys[NUM_PIECES][BOARD_SIZE];
U64 Zobrist::castleKeys[16];
U64 Zobrist::enPassantKeys[8];
U64 Zobrist::sideKey;

U64 Zobrist::generateRandom(){
    static std::mt19937_64 rng(0xDEADBEEFCAFEBABEULL);
    return rng();
}


U64 Zobrist::generateUniqueRandom(std::unordered_set<U64> &st){
    while(true){
        U64 candidate = generateRandom();
        if(st.insert(candidate).second) return candidate;
    }
}


void Zobrist::init(){
    std::unordered_set<U64> st;
    constexpr int totalCandidates = NUM_PIECES * BOARD_SIZE + 16 + RANK_SIZE + 1;
    st.reserve(totalCandidates);

    for(int piece=0; piece<NUM_PIECES; piece++){
        for(int square=0; square<BOARD_SIZE; square++){
            pieceKeys[piece][square] = generateUniqueRandom(st);
        }
    }


    for(int rights=0; rights<16; rights++){
        castleKeys[rights] = generateUniqueRandom(st);
    }


    for(int file=0; file<RANK_SIZE; file++){
        enPassantKeys[file] = generateUniqueRandom(st);
    }


    sideKey = generateUniqueRandom(st);
}


U64 Zobrist::generateHash(const Board &board){
    U64 hash = 0;

    for(int s=0; s<BOARD_SIZE; s++){
        Square square = static_cast<Square>(s);
        Piece p = board.getPieceBoard(square);

        if(p != EMPTY) hash ^= pieceKeys[p][square];
    }

    if(board.getMovingSide() == BLACK) hash ^= sideKey;

    hash ^= castleKeys[board.getCastlingRights()];

    Square ep = board.getEnPassant();

    if(ep != NO_SQUARE){
        int cur_file = getFile(ep);
        hash ^= enPassantKeys[cur_file];
    }


    return hash;
}


U64 Zobrist::generatePawnHash(const Board &board){
    U64 hash = 0;

    U64 wp = board.getBitboard(WP);
    U64 bp = board.getBitboard(BP);

    while(wp){
        Square s = static_cast<Square>(Bitboard::popLSB(wp));
        hash ^= pieceKeys[WP][s];
    }

    while(bp){
        Square s = static_cast<Square>(Bitboard::popLSB(bp));
        hash ^= pieceKeys[BP][s];
    }

    return hash;
}