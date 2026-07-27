#include "board.h"
#include <iostream>
#include <cassert>
using namespace std;

namespace {
constexpr char START_FEN[] =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

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
    fullmoveNumber = 1;
    ply=0;
}

Piece Board::charToPiece(char c){
    switch (c) {
        case 'P': return WP;
        case 'N': return WN;
        case 'B': return WB;
        case 'R': return WR;
        case 'Q': return WQ;
        case 'K': return WK;

        case 'p': return BP;
        case 'n': return BN;
        case 'b': return BB;
        case 'r': return BR;
        case 'q': return BQ;
        case 'k': return BK;

        default:  return EMPTY;
    }
}

Square Board::parseEnPassantSquare(char file,int rank,Color tomove){
    if(rank==3 && tomove==BLACK){
        if(file=='a') return A3;
        else if(file=='b') return B3;
        else if(file=='c') return C3;
        else if(file=='d') return D3;
        else if(file=='e') return E3;
        else if(file=='f') return F3;
        else if(file=='g') return G3;
        else if(file=='h') return H3;
        else return NO_SQUARE;
    }

    else if(rank==6 && tomove==WHITE){
        if(file=='a') return A6;
        else if(file=='b') return B6;
        else if(file=='c') return C6;
        else if(file=='d') return D6;
        else if(file=='e') return E6;
        else if(file=='f') return F6;
        else if(file=='g') return G6;
        else if(file=='h') return H6;
        else return NO_SQUARE;
    }

    else return NO_SQUARE;
}


// bool -> to validate the fen string
bool Board::loadFEN(const string &fen){

    // filling the board with the help of field 1
    int i=0, j=BOARD_SIZE-RANK_SIZE , cur=BOARD_SIZE-RANK_SIZE;
    array<Piece, BOARD_SIZE> temp_board;
    temp_board.fill(EMPTY);


    while(i<fen.size() && fen[i] != ' '){

        // rank change on board
        if(fen[i]=='/') {
            // invalid length of fen
            if(j != cur+RANK_SIZE || cur==0) return false;
            j=cur-RANK_SIZE;
            cur=j;
        }

        //  invalid length of fen
        else if(j >= cur+RANK_SIZE){
            return false;
        }

        // empty file on board
        else if(fen[i]>='0' && fen[i]<='9'){
            int num = fen[i] - '0';

            // given board exceeds actual board
            if(num <=0 || num > RANK_SIZE || j+num > cur+RANK_SIZE) return false;

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
    if(j != cur+RANK_SIZE || cur!=0) return false;

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

    ply=0;

    return true;
}


void Board::rebuildBitboards(){
    bitboards.fill(0);

    for(size_t i=0 ; i<BOARD_SIZE ; i++){
        Piece p = board[i];
        if(p==EMPTY) continue;
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
    assert(loadFEN(START_FEN));
}

char Board::pieceToChar(Piece p){
    switch (p) {
        case WP: return 'P';
        case WN: return 'N';
        case WB: return 'B';
        case WR: return 'R';
        case WQ: return 'Q';
        case WK: return 'K';

        case BP: return 'p';
        case BN: return 'n';
        case BB: return 'b';
        case BR: return 'r';
        case BQ: return 'q';
        case BK: return 'k';

        default: return '.';
    }
}

void Board::print() const{
    for(size_t i=RANK_SIZE ; i>0 ; i--){
        size_t start_file = (i-1)*RANK_SIZE;

        cout<<i<<"  ";

        for(size_t j=0 ; j<RANK_SIZE ; j++){
            char c = pieceToChar(board[start_file+j]);
            cout<<c<<" ";
        }

        cout<<"\n";
    }

    cout<<"   ";

    for(char c='a';c<='h';c++){
        cout<<c<<" ";
    }

    cout<<"\n";
}


Color Board::getMovingSide() const{
    return sideToMove;
}

Square Board::getEnPassant() const{
    return enPassant;
}

U64 Board::getBitboard(Piece p) const{
    return bitboards[p];
}

U64 Board::getOccupancy(Color c) const{
    return occupancies[c];
}

Piece Board::getPieceBoard(Square s) const{
    return board[s];
}

int Board::getCastlingRights() const{
    return castlingRights;
}

int Board::getHalfMoveClock() const{
    return halfmoveClock;
}

int Board::getFullMoveNumber() const{
    return fullmoveNumber;
}


bool Board::makeMove(const Move &move){
    assert(ply<MAX_PLYS);

    history[ply++] = {
        castlingRights,
        enPassant,
        halfmoveClock
    };

    Piece moved = move.getMovedPiece();
    Piece captured = move.getCapturedPiece();
    Square from = move.getFrom();
    Square to = move.getTo();
    MoveFlag flag = move.getMoveFlag();
    Piece promotionPiece = move.getPromotion();

    Color opp = (sideToMove == WHITE ? BLACK : WHITE);

    assert(board[from]==moved);
    if(!move.isEnPassant()) assert(board[to]==captured);

    U64 fromMask = (1ULL<<from);
    U64 toMask = (1ULL<<to);

    // removing the moved piece from source
    occupancies[sideToMove] &= ~fromMask;
    occupancies[BOTH] &= ~fromMask;
    board[from] = EMPTY;
    bitboards[moved] &= ~fromMask;

    // placing moved piece to dest
    occupancies[BOTH] |= toMask;
    occupancies[sideToMove] |= toMask;

    if(move.isPromotion()){
        bitboards[promotionPiece] |= toMask;
        board[to] = promotionPiece;
    }

    else{
        bitboards[moved] |= toMask;
        board[to] = moved;
    }


    // removing the captured piece
    if(move.isEnPassant()){
        int rankOffset = (sideToMove == WHITE ? -8 : 8);
        Square s = static_cast<Square>(to + rankOffset);

        bitboards[captured] &= ~(1ULL<<s);
        board[s] = EMPTY;
        occupancies[opp] &= ~(1ULL<<s);
        occupancies[BOTH] &= ~(1ULL<<s);
    }

    else if(move.isCapture()){
        bitboards[captured] &= ~toMask;
        occupancies[opp] &= ~toMask;
    }



    // checking for double pawn push
    if(flag == doublePawnPush){
        int rankOffset = (sideToMove == WHITE ? -8 : 8);
        enPassant = static_cast<Square>(to + rankOffset);
    }


    // checking for castling
    if((flag  == kingSideCastle) || (flag == queenSideCastle)){
        Square source = NO_SQUARE;
        Square dest = NO_SQUARE;
        Piece p = EMPTY;

        if(sideToMove == WHITE){
            if(flag == kingSideCastle){
                castlingRights &= (~CASTLE_WK);
                source = H1;
                dest = F1;
            }

            else{
                castlingRights &= (~CASTLE_WQ);
                source = A1;
                dest = D1;
            }

            p = WR;
        }

        else{
            if(flag == kingSideCastle){
                castlingRights &= (~CASTLE_BK);
                source = H8;
                dest = F8;
            }

            else{
                castlingRights &= (~CASTLE_BQ);
                source = A8;
                dest = D8;
            }

            p = BR;
        }


        bitboards[p] &= ~(1ULL<<source);
        bitboards[p] |= (1ULL<<dest);
        board[source] = EMPTY;
        board[dest] = p;
        occupancies[sideToMove] &= ~(1ULL<<source);
        occupancies[sideToMove] |= (1ULL<<dest);
        occupancies[BOTH] &= ~(1ULL<<source);
        occupancies[BOTH] |= (1ULL<<dest);
    }



    // removing castling rights
    if(moved == WK){
        if(castlingRights & (CASTLE_WK | CASTLE_WQ)){
            castlingRights &= ~(CASTLE_WK | CASTLE_WQ);
        }
    }

    else if(moved == BK){
        if(castlingRights & (CASTLE_BK | CASTLE_BQ)){
            castlingRights &= ~(CASTLE_BK | CASTLE_BQ);
        }
    }

    else if(moved == WR){
        if(from == A1){
            castlingRights &= ~CASTLE_WQ;
        }

        else if(from == H1){
            castlingRights &= ~CASTLE_WK;
        }
    }


    else if(moved == BR){
        if(from == A8){
            castlingRights &= ~CASTLE_BQ;
        }

        else if(from == H8){
            castlingRights &= ~CASTLE_BK;
        }
    }


    else if(captured == BR){
        if(to == A8){
            castlingRights &= ~CASTLE_BQ;
        }

        else if(to == H8){
            castlingRights &= ~CASTLE_BK;
        }
    }

    else if(captured == WR){
        if(to == A1){
            castlingRights &= ~CASTLE_WQ;
        }

        else if(to == H1){
            castlingRights &= ~CASTLE_WK;
        }
    }



    if(moved == WP || moved == BP || move.isCapture()) halfmoveClock = 0;
    else ++halfmoveClock;

    sideToMove = opp;
    if(sideToMove == WHITE) ++fullmoveNumber;
    if(flag != doublePawnPush) enPassant = NO_SQUARE;

    return true;
}


void Board::undoMove(const Move &move){
    assert(ply > 0);
    UndoInfo data = history[--ply];

    castlingRights = data.castlingRights;
    enPassant = data.enpassant;
    halfmoveClock = data.halfMoveClock;

    Piece moved = move.getMovedPiece();
    Piece captured = move.getCapturedPiece();
    Square from = move.getFrom();
    Square to = move.getTo();
    MoveFlag flag = move.getMoveFlag();
    Piece promotionPiece = move.getPromotion();

    Color opp = (sideToMove == WHITE ? BLACK : WHITE);

    U64 fromMask = (1ULL<<from);
    U64 toMask = (1ULL<<to);


    // bringing piece back to from
    bitboards[moved] |= fromMask;
    board[from] = moved;
    occupancies[opp] |= fromMask;
    occupancies[BOTH] |= fromMask;
    occupancies[opp] &= ~toMask;
    occupancies[BOTH] &= ~toMask;
    board[to] = EMPTY;

    if(move.isPromotion()){
        bitboards[promotionPiece] &= ~toMask;
    }

    else{
        bitboards[moved] &= ~toMask;
    }

    
    if(move.isEnPassant()){
        int rankOffset = (sideToMove == WHITE ? 8 : -8);
        Square s = static_cast<Square>(to + rankOffset);

        bitboards[captured] |= (1ULL<<s);
        board[s] = captured;
        occupancies[sideToMove] |= (1ULL<<s);
        occupancies[BOTH] |= (1ULL<<s);
    }

    else if(move.isCapture()){
        bitboards[captured] |= toMask;
        occupancies[sideToMove] |= toMask;
        board[to] = captured;
        occupancies[BOTH] |= toMask;
    }



    // checking for castling
    if((flag  == kingSideCastle) || (flag == queenSideCastle)){
        Square source = NO_SQUARE;
        Square dest = NO_SQUARE;
        Piece p = EMPTY;

        if(opp == WHITE){
            if(flag == kingSideCastle){
                source = H1;
                dest = F1;
            }

            else{
                source = A1;
                dest = D1;
            }

            p = WR;
        }

        else{
            if(flag == kingSideCastle){
                source = H8;
                dest = F8;
            }

            else{
                source = A8;
                dest = D8;
            }

            p = BR;
        }


        bitboards[p] |= (1ULL<<source);
        bitboards[p] &= ~(1ULL<<dest);
        board[source] = p;
        board[dest] = EMPTY;
        occupancies[opp] |= (1ULL<<source);
        occupancies[opp] &= ~(1ULL<<dest);
        occupancies[BOTH] |= (1ULL<<source);
        occupancies[BOTH] &= ~(1ULL<<dest);
    }

    
    sideToMove = opp;
    if(sideToMove == BLACK) --fullmoveNumber;
}