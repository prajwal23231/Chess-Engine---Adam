#include "tools.h"
using namespace std;

U64 between[BOARD_SIZE][BOARD_SIZE];

U64 fileMask[8];
U64 rankMask[8];

U64 whitePassedMask[BOARD_SIZE];
U64 blackPassedMask[BOARD_SIZE];

U64 whiteOutpostMask[BOARD_SIZE];
U64 blackOutpostMask[BOARD_SIZE];

U64 adjacentFileMask[8];
U64 isolatedMask[8];

int pst[NUM_STAGE][NUM_PIECE_TYPE][BOARD_SIZE];

void computeBetween(){
    for(int i=0; i<BOARD_SIZE; i++){
        Square first = static_cast<Square>(i);
        int f_rank = getRank(first);
        int f_file = getFile(first);

        for(int j=0; j<BOARD_SIZE; j++){
            Square sec = static_cast<Square>(j);
            int s_rank = getRank(sec);
            int s_file = getFile(sec);

            U64 mask = 0;

            // same rank
            if(f_rank == s_rank){
                int cur = min(f_file, s_file) + 1;

                while(cur < max(f_file, s_file)){
                    mask |= (1ULL<<(f_rank*RANK_SIZE + cur));
                    cur++;
                }
            }

            // same file
            else if(f_file == s_file){
                int cur = min(f_rank, s_rank) + 1;

                while(cur < max(f_rank, s_rank)){
                    mask |= (1ULL<<(cur*RANK_SIZE + f_file));
                    cur++;
                }
            }


            // same diagonal
            else if(abs(f_file - s_file) == abs(f_rank - s_rank)){
                constexpr int dr[4] = {-1, -1, 1, 1};
                constexpr int df[4] = {-1, 1, -1, 1};
                int ind;

                if(f_rank > s_rank){
                    if(f_file > s_file) ind = 0;
                    else ind = 1;
                }

                else{
                    if(f_file > s_file) ind = 2;
                    else ind = 3;
                }

                int cur_rank = f_rank + dr[ind];
                int cur_file = f_file + df[ind];

                while(cur_rank != s_rank && cur_file != s_file){
                    mask |= (1ULL<<(cur_rank*RANK_SIZE + cur_file));
                    cur_rank += dr[ind];
                    cur_file += df[ind];
                }
            }

            between[first][sec] = mask;
        }
    }
}


void createpst(){
    for (int piece = 0; piece < 6; ++piece) {
        for (int sq = 0; sq < 64; ++sq) {
            pst[MG][piece][sq ^ 56] = mgTables[piece][sq];
            pst[EG][piece][sq ^ 56] = egTables[piece][sq];
        }
    }
}



void createPassedMask(){
    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int rank = getRank(sq);
        int file = getFile(sq);

        U64 whiteMask = 0;

        for(int r=rank+1; r<RANK_SIZE; r++){
            // current file
            whiteMask |= (1ULL<<(r*RANK_SIZE + file));

            // left file
            if(file > 0) whiteMask |= (1ULL<<(r*RANK_SIZE + file - 1));

            // right file
            if(file < RANK_SIZE-1) whiteMask |= (1ULL<<(r*RANK_SIZE + file + 1));
        }

        whitePassedMask[sq] = whiteMask;



        U64 blackMask = 0;

        for(int r=rank-1; r>=0; r--){
            // current file
            blackMask |= (1ULL<<(r*RANK_SIZE + file));

            // left file
            if(file > 0) blackMask |= (1ULL<<(r*RANK_SIZE + file - 1));

            // right file
            if(file < RANK_SIZE-1) blackMask |= (1ULL<<(r*RANK_SIZE + file + 1));
        }

        blackPassedMask[sq] = blackMask;
    }
}



void createFileRankMask(){
    for(int file=0; file<8; file++){
        U64 mask = 0;

        for (int rank = 0; rank < RANK_SIZE; rank++) {
            mask |= (1ULL << (rank * RANK_SIZE + file));
        }

        fileMask[file] = mask;
    }


    for(int rank=0; rank<8; rank++){
        U64 mask = 0;

        for (int file = 0; file < RANK_SIZE; file++) {
            mask |= (1ULL << (rank * RANK_SIZE + file));
        }

        rankMask[rank] = mask;
    }
}


void createIsolatedMask(){
    for(int file=0; file<8; file++){
        U64 mask = 0;

        for (int rank = 0; rank < RANK_SIZE; rank++) {
            // left file
            if(file>0) mask |= (1ULL << (rank * RANK_SIZE + file-1));

            // right file
            if(file<RANK_SIZE-1) mask |= (1ULL << (rank * RANK_SIZE + file+1));
        }

        isolatedMask[file] = mask;
        adjacentFileMask[file] = mask;
    }
}



void createOutpostMask(){
    for(int i=0; i<BOARD_SIZE; i++){
        Square sq = static_cast<Square>(i);
        int rank = getRank(sq);
        int file = getFile(sq);

        U64 whiteMask = 0;
        for(int r=rank+1; r<RANK_SIZE; r++){
            if(file > 0) whiteMask |= (1ULL<<(r*RANK_SIZE + file - 1));
            if(file < RANK_SIZE-1) whiteMask |= (1ULL<<(r*RANK_SIZE + file + 1));
        }
        whiteOutpostMask[sq] = whiteMask;

        U64 blackMask = 0;
        for(int r=rank-1; r>=0; r--){
            if(file > 0) blackMask |= (1ULL<<(r*RANK_SIZE + file - 1));
            if(file < RANK_SIZE-1) blackMask |= (1ULL<<(r*RANK_SIZE + file + 1));
        }
        blackOutpostMask[sq] = blackMask;
    }
}



void Tools::initTools(){
    computeBetween();
    createFileRankMask();
    createIsolatedMask();
    createPassedMask();
    createOutpostMask();
    createpst();
}