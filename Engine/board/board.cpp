#include "board.h"
#include <iostream>
#include <cassert>

using namespace Bitboard;
using namespace std;

namespace {
    constexpr char START_FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    constexpr int castlingRightsMask[BOARD_SIZE] = {
        13, 15, 15, 15, 12, 15, 15, 14,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
         7, 15, 15, 15,  3, 15, 15, 11
    };
}

Board::Board(){
    clear();

    // initialised at the end
    zobristKey = Zobrist::generateHash(*this);
    pawnKey = Zobrist::generatePawnHash(*this);
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

    zobristKey = 0;
    pawnKey = 0;
    gamePhase = 0;

    mgScore = 0;
    egScore = 0;
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
    U64 key = 0;


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
            temp_board[j] = p;

            if(p == WP) key ^= Zobrist::getPawnKeys(WHITE, static_cast<Square>(j));
            else if(p == BP) key ^= Zobrist::getPawnKeys(BLACK, static_cast<Square>(j));

            j++;
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

    zobristKey = Zobrist::generateHash(*this);
    pawnKey = key;

    return true;
}


void Board::rebuildBitboards(){
    bitboards.fill(0);
    gamePhase = 0;
    mgScore = 0;
    egScore = 0;

    for(int i=0 ; i<BOARD_SIZE ; i++){
        Piece p = board[i];
        if(p==EMPTY) continue;
        setBit(bitboards[p], static_cast<Square>(i));
        gamePhase += piecePhase[p];
        addPieceScore(p, static_cast<Square>(i));
    }
}


void Board::updateOccupancies(){
    occupancies.fill(0);

    occupancies[WHITE] = bitboards[WP] | bitboards[WN] | bitboards[WB] | bitboards[WK] | bitboards[WQ] | bitboards[WR];
    occupancies[BLACK] = bitboards[BP] | bitboards[BN] | bitboards[BB] | bitboards[BK] | bitboards[BQ] | bitboards[BR];
    occupancies[BOTH] = occupancies[WHITE] | occupancies[BLACK];
}


void Board::setStartingPosition(){
    bool ok = loadFEN(START_FEN);
    assert(ok);
    (void)ok;
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
    for(int i=RANK_SIZE ; i>0 ; i--){
        int start_file = (i-1)*RANK_SIZE;

        cout<<i<<"  ";

        for(int j=0 ; j<RANK_SIZE ; j++){
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





bool Board::makeMove(const Move &move){
    assert(ply<MAX_PLYS);

    history[ply++] = {
        castlingRights,
        enPassant,
        halfmoveClock,
        zobristKey,
        pawnKey,
        gamePhase,
        mgScore,
        egScore
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

    
    // removing old enpassant
    if(enPassant != NO_SQUARE){
        zobristKey ^= Zobrist::getEnPassantKeys(getFile(enPassant));
    }


    // removing the moved piece from source
    clearBit(occupancies[sideToMove], from);
    clearBit(occupancies[BOTH], from);
    board[from] = EMPTY;
    clearBit(bitboards[moved], from);
    zobristKey ^= Zobrist::getPieceKeys(moved, from);
    removePieceScore(moved,from);

    // placing moved piece to dest
    setBit(occupancies[BOTH], to);
    setBit(occupancies[sideToMove], to);


    if(moved == WP || moved == BP){
        pawnKey ^= Zobrist::getPawnKeys(sideToMove, from);
    }
    

    if(move.isPromotion()){
        setBit(bitboards[promotionPiece], to);
        board[to] = promotionPiece;
        zobristKey ^= Zobrist::getPieceKeys(promotionPiece, to);
        gamePhase += piecePhase[promotionPiece];
        addPieceScore(promotionPiece,to);
    }

    else{
        setBit(bitboards[moved], to);
        board[to] = moved;
        zobristKey ^= Zobrist::getPieceKeys(moved, to);
        addPieceScore(moved,to);

        if(moved == WP || moved == BP){
            pawnKey ^= Zobrist::getPawnKeys(sideToMove, to);
        }
    }


    // removing the captured piece
    if(move.isEnPassant()){
        int rankOffset = (sideToMove == WHITE ? -RANK_SIZE : RANK_SIZE);
        Square s = static_cast<Square>(to + rankOffset);

        clearBit(bitboards[captured], s);
        board[s] = EMPTY;
        clearBit(occupancies[opp], s);
        clearBit(occupancies[BOTH], s);
        zobristKey ^= Zobrist::getPieceKeys(captured, s);

        pawnKey ^= Zobrist::getPawnKeys(opp, s);
        removePieceScore(captured,s);
    }

    else if(move.isCapture()){
        clearBit(bitboards[captured], to);
        clearBit(occupancies[opp], to);
        zobristKey ^= Zobrist::getPieceKeys(captured, to);

        if(captured == WP || captured == BP){
            pawnKey ^= Zobrist::getPawnKeys(opp, to);
        }

        gamePhase -= piecePhase[captured];
        removePieceScore(captured,to);
    }



    // checking for double pawn push
    if(flag == doublePawnPush){
        int rankOffset = (sideToMove == WHITE ? -RANK_SIZE : RANK_SIZE);
        enPassant = static_cast<Square>(to + rankOffset);
        zobristKey ^= Zobrist::getEnPassantKeys(getFile(enPassant));
    }


    // removing previous castling rights
    zobristKey ^= Zobrist::getCastleKeys(castlingRights);

    // checking for castling
    if((flag  == kingSideCastle) || (flag == queenSideCastle)){
        Square source = NO_SQUARE;
        Square dest = NO_SQUARE;
        Piece p = EMPTY;

        if(sideToMove == WHITE){
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


        clearBit(bitboards[p], source);
        setBit(bitboards[p], dest);
        board[source] = EMPTY;
        board[dest] = p;
        clearBit(occupancies[sideToMove], source);
        setBit(occupancies[sideToMove], dest);
        clearBit(occupancies[BOTH], source);
        setBit(occupancies[BOTH], dest);
        zobristKey ^= Zobrist::getPieceKeys(p, source);
        zobristKey ^= Zobrist::getPieceKeys(p, dest);

        removePieceScore(p,source);
        addPieceScore(p,dest);
    }



    // removing castling rights branchlessly
    castlingRights &= castlingRightsMask[from] & castlingRightsMask[to];

    // adding new castling rights
    zobristKey ^= Zobrist::getCastleKeys(castlingRights);


    if(moved == WP || moved == BP || move.isCapture()) halfmoveClock = 0;
    else ++halfmoveClock;

    sideToMove = opp;
    if(sideToMove == WHITE) ++fullmoveNumber;

    if(flag != doublePawnPush) enPassant = NO_SQUARE;

    zobristKey ^= Zobrist::getSideKey();

    return true;
}


void Board::undoMove(const Move &move){
    assert(ply > 0);
    UndoInfo data = history[--ply];

    castlingRights = data.castlingRights;
    enPassant = data.enpassant;
    halfmoveClock = data.halfMoveClock;
    zobristKey = data.zobristKey;
    pawnKey = data.pawnKey;
    gamePhase = data.gamePhase;
    mgScore = data.mgScore;
    egScore = data.egScore;

    Piece moved = move.getMovedPiece();
    Piece captured = move.getCapturedPiece();
    Square from = move.getFrom();
    Square to = move.getTo();
    MoveFlag flag = move.getMoveFlag();
    Piece promotionPiece = move.getPromotion();

    Color opp = (sideToMove == WHITE ? BLACK : WHITE);


    // bringing piece back to from
    setBit(bitboards[moved], from);
    board[from] = moved;
    setBit(occupancies[opp], from);
    setBit(occupancies[BOTH], from);
    clearBit(occupancies[opp], to);
    clearBit(occupancies[BOTH], to);
    board[to] = EMPTY;

    if(move.isPromotion()){
        clearBit(bitboards[promotionPiece], to);
    }

    else{
        clearBit(bitboards[moved], to);
    }

    
    if(move.isEnPassant()){
        int rankOffset = (sideToMove == WHITE ? RANK_SIZE : -RANK_SIZE);
        Square s = static_cast<Square>(to + rankOffset);

        setBit(bitboards[captured], s);
        board[s] = captured;
        setBit(occupancies[sideToMove], s);
        setBit(occupancies[BOTH], s);
    }

    else if(move.isCapture()){
        setBit(bitboards[captured], to);
        setBit(occupancies[sideToMove], to);
        board[to] = captured;
        setBit(occupancies[BOTH], to);
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


        setBit(bitboards[p], source);
        clearBit(bitboards[p], dest);
        board[source] = p;
        board[dest] = EMPTY;
        setBit(occupancies[opp], source);
        clearBit(occupancies[opp], dest);
        setBit(occupancies[BOTH], source);
        clearBit(occupancies[BOTH], dest);
    }

    
    sideToMove = opp;
    if(sideToMove == BLACK) --fullmoveNumber;
}


