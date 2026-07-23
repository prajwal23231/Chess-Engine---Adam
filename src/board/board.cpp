#include "board.h"
#include <iostream>
using namespace std;

Board::Board(){
    clear();
}

void Board::clear(){
    bitboards.fill(0);
    occupancies.fill(0);
    board.fill(EMPTY);

    sideToMove = WHITE;
    castlingRights = 0;
    enPassant = NO_SQUARE;
    
    halfmoveClock = 0;
    fullmoveNumber = 0;
}

Piece Board::charToPiece(char c){
    if(c=='r') return BR;
    else if(c=='R') return WR;
    else if(c=='k') return BK;
    else if(c=='K') return WK;
    else if(c=='q') return BQ;
    else if(c=='Q') return WQ;
    else if(c=='n') return BN;
    else if(c=='N') return WN;
    else if(c=='b') return BB;
    else if(c=='B') return WB;
    else if(c=='p') return BP;
    else if(c=='P') return WP;
    return EMPTY;
}

Square Board::parseEnPassantSquare(char pos,int rank,Color tomove){
    if(rank==3 && tomove==BLACK){
        if(pos=='a') return A3;
        else if(pos=='b') return B3;
        else if(pos=='c') return C3;
        else if(pos=='d') return D3;
        else if(pos=='e') return E3;
        else if(pos=='f') return F3;
        else if(pos=='g') return G3;
        else if(pos=='h') return H3;
        else return NO_SQUARE;
    }

    else if(rank==6 && tomove==WHITE){
        if(pos=='a') return A6;
        else if(pos=='b') return B6;
        else if(pos=='c') return C6;
        else if(pos=='d') return D6;
        else if(pos=='e') return E6;
        else if(pos=='f') return F6;
        else if(pos=='g') return G6;
        else if(pos=='h') return H6;
        else return NO_SQUARE;
    }

    else return NO_SQUARE;
}


// bool -> to validate the fen string
bool Board::loadFEN(const string &fen){

    // filling the board with the help of field 1
    int i=0,j=56,cur=56;
    array<Piece, NUM_SQUARES> temp_board;
    temp_board.fill(EMPTY);


    while(i<fen.size() && fen[i] != ' '){

        // rank change on board
        if(fen[i]=='/') {
            // invalid length of fen
            if(j != cur+8 || cur==0) return false;
            j=cur-8;
            cur=j;
        }

        //  invalid length of fen
        else if(j >= cur+8){
            return false;
        }

        // empty pos on board
        else if(fen[i]>='0' && fen[i]<='9'){
            int num = fen[i] - '0';

            // given board exceeds actual board
            if(num ==0 || num == 9 || j+num > cur+8) return false;

            while(num--){
                temp_board[j++] = EMPTY;
            }
        }

        // piece on the board
        else{
            Piece p = charToPiece(fen[i]);
            // invalid character
            if(p==EMPTY) return false;
            temp_board[j++] = p;
        }

        i++;
    }

    if(i==fen.size()) return false; // checking if string is exhausted before time

    // invalid length of last rank
    if(j != cur+8 || cur!=0) return false;

    i++; // skipping spaces
    if(i==fen.size()) return false; // checking if string is exhausted before time






    //  deciding side to move by field 2
    Color cur_move=BOTH;

    if(fen[i] == 'w') cur_move = WHITE;
    else if(fen[i] == 'b') cur_move = BLACK;
    else return false;

    i++;
    if(i==fen.size() || fen[i] != ' ') return false;

    i++; // skipping spaces
    if(i==fen.size() || fen[i] == ' ') return false;







    // deciding castling rights by field 3
    int castle_val=0,len=0;

    while(i<fen.size() && fen[i] != ' '){

        if(fen[i] == 'K' && !(castle_val&CASTLE_WK)) castle_val |= CASTLE_WK;
        else if(fen[i] == 'Q' && !(castle_val&CASTLE_WQ)) castle_val |= CASTLE_WQ;
        else if(fen[i] == 'k' && !(castle_val&CASTLE_BK)) castle_val |= CASTLE_BK;
        else if(fen[i] == 'q' && !(castle_val&CASTLE_BQ)) castle_val |= CASTLE_BQ;
        else if(fen[i] != '-' || (fen[i] == '-' && len)) return false;

        i++;
        len++;
    }

    if(i==fen.size() || len>4 || len==0) return false;

    i++; // skipping spaces
    if(i==fen.size()) return false; // checking if string is exhausted before time






    // deciding enpassant square by field 4
    Square s=NO_SQUARE;

    if(fen[i] != '-'){
        if(i == fen.size()-1) return false;

        s = parseEnPassantSquare(fen[i],fen[i+1]-'0',cur_move);
        if(s == NO_SQUARE) return false;

        i+=2;
    }

    // if no enpassant skip the curernt
    else i++;

    if(i==fen.size() || fen[i]!=' ') return false;
    i++; // skipping spaces

    if(i==fen.size() || fen[i]==' ') return false; // checking if string is exhausted before time





    // parsing field 5 -> halfmove clock
    int halfmove=0,cnt=0;

    while(i<fen.size() && fen[i]!=' '){
        int d = fen[i]-'0';
        if(d<0 || d>9) return false;
        halfmove = halfmove*10 + d;
        i++;
        cnt++;
    }

    if(!cnt || i==fen.size()) return false;

    i++; // skipping spaces

    if(i==fen.size() || fen[i]==' ') return false;




    // parsing field 6 -> fullmovenumber
    int fullmove=0;
    cnt=0;

    while(i<fen.size()){
        int d = fen[i]-'0';
        if(d<0 || d>9) return false;
        fullmove = fullmove*10 + d;
        i++;
        cnt++;
    }

    if(!cnt || i!=fen.size()) return false;
    if(fullmove==0) return false;


    // only filling when all fields are valid;
    swap(board,temp_board);
    sideToMove = cur_move;
    castlingRights = castle_val;
    enPassant = s;
    halfmoveClock = halfmove;
    fullmoveNumber = fullmove;
    rebuildBitboards();
    updateOccupancies();

    return true;
}


void Board::rebuildBitboards(){
    bitboards.fill(0);

    for(int i=0;i<64;i++){
        if(board[i]==EMPTY) continue;

        Piece p = board[i];
        bitboards[p] |= (1ULL<<i);
    }
}


void Board::updateOccupancies(){
    occupancies.fill(0);

    occupancies[WHITE] = bitboards[WP] | bitboards[WN] | bitboards[WB] | bitboards[WK] | bitboards[WQ] | bitboards[WR];
    occupancies[BLACK] = bitboards[BP] | bitboards[BN] | bitboards[BB] | bitboards[BK] | bitboards[BQ] | bitboards[BR];
    occupancies[BOTH] = occupancies[WHITE] | occupancies[BLACK];
}


void Board::setStartingPosition(){

}