#include "board.h"
#include <iostream>
#include <cassert>

using namespace Bitboard;
using namespace std;

namespace {
    constexpr char START_FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    constexpr int getFile(int sq) { return sq % 8; }
    constexpr int getRank(int sq) { return sq / 8; }
}

Board::Board(){
    clear();

    initPawnAttacks();
    initKingAttacks();
    initKnightAttacks();
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

    for(int i=0 ; i<BOARD_SIZE ; i++){
        Piece p = board[i];
        if(p==EMPTY) continue;
        setBit(bitboards[p], static_cast<Square>(i));
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


    // removing the moved piece from source
    clearBit(occupancies[sideToMove], from);
    setBit(occupancies[BOTH], from);
    board[from] = EMPTY;
    clearBit(bitboards[moved], from);

    // placing moved piece to dest
    setBit(occupancies[BOTH], to);
    setBit(occupancies[sideToMove], to);

    if(move.isPromotion()){
        setBit(bitboards[promotionPiece], to);
        board[to] = promotionPiece;
    }

    else{
        setBit(bitboards[moved], to);
        board[to] = moved;
    }


    // removing the captured piece
    if(move.isEnPassant()){
        int rankOffset = (sideToMove == WHITE ? -8 : 8);
        Square s = static_cast<Square>(to + rankOffset);

        clearBit(bitboards[captured], s);
        board[s] = EMPTY;
        clearBit(occupancies[opp], s);
        clearBit(occupancies[BOTH], s);
    }

    else if(move.isCapture()){
        clearBit(bitboards[captured], to);
        clearBit(occupancies[opp], to);
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


        clearBit(bitboards[p], source);
        setBit(bitboards[p], dest);
        board[source] = EMPTY;
        board[dest] = p;
        clearBit(occupancies[sideToMove], source);
        setBit(occupancies[sideToMove], dest);
        clearBit(occupancies[BOTH], source);
        setBit(occupancies[BOTH], dest);
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
        int rankOffset = (sideToMove == WHITE ? 8 : -8);
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


void Board::initPawnAttacks(){
    constexpr int whiteMove[2][2] = {{1,-1}, {1,1}};
    constexpr int blackMove[2][2] = {{-1,-1}, {-1,1}};

    for(int i=0; i<BOARD_SIZE; i++){
        Square s = static_cast<Square>(i);
        int cur_rank = getRank(i);
        int cur_file = getFile(i);

        // white
        U64 wAttack = 0;
        
        for(int j=0;j<2;j++){
            int new_rank = cur_rank + whiteMove[j][0];
            int new_file = cur_file + whiteMove[j][1];

            if(new_rank >=0 && new_file >=0 && new_rank < RANK_SIZE && new_file < RANK_SIZE){
                Square source = static_cast<Square>(new_rank*RANK_SIZE + new_file);
                setBit(wAttack, source);
            }
        }

        pawnAttacks[WHITE][s] = wAttack;


        // black
        U64 bAttack = 0;
        
        for(int j=0;j<2;j++){
            int new_rank = cur_rank + blackMove[j][0];
            int new_file = cur_file + blackMove[j][1];

            if(new_rank >=0 && new_file >=0 && new_rank < RANK_SIZE && new_file < RANK_SIZE){
                Square source = static_cast<Square>(new_rank*RANK_SIZE + new_file);
                setBit(bAttack, source);
            }
        }

        pawnAttacks[BLACK][s] = bAttack;
    }
}


void Board::initKingAttacks(){
    constexpr int kingMoves[8][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};

    for(int i=0; i<BOARD_SIZE; i++){
        Square s = static_cast<Square>(i);
        int cur_rank = getRank(i);
        int cur_file = getFile(i);

        U64 kAttack = 0;
        
        for(int j=0;j<8;j++){
            int new_rank = cur_rank + kingMoves[j][0];
            int new_file = cur_file + kingMoves[j][1];

            if(new_rank >=0 && new_file >=0 && new_rank < RANK_SIZE && new_file < RANK_SIZE){
                Square source = static_cast<Square>(new_rank*RANK_SIZE + new_file);
                setBit(kAttack, source);
            }
        }

        kingAttacks[s] = kAttack;
    }
}


void Board::initKnightAttacks(){
    constexpr int knightMoves[8][2] = {{2,1}, {2,-1}, {1,2}, {-1,2}, {-2,1}, {-2,-1}, {1,-2}, {-1,-2}};

    for(int i=0; i<BOARD_SIZE; i++){
        Square s = static_cast<Square>(i);
        int cur_rank = getRank(i);
        int cur_file = getFile(i);

        U64 nAttack = 0;
        
        for(int j=0;j<8;j++){
            int new_rank = cur_rank + knightMoves[j][0];
            int new_file = cur_file + knightMoves[j][1];

            if(new_rank >=0 && new_file >=0 && new_rank < RANK_SIZE && new_file < RANK_SIZE){
                Square source = static_cast<Square>(new_rank*RANK_SIZE + new_file);
                setBit(nAttack, source);
            }
        }

        knightAttacks[s] = nAttack;
    }
}


bool Board::isSquareAttacked(Square square, Color bySide) const{

    Piece pawn   = (bySide == WHITE ? WP : BP);
    Piece knight = (bySide == WHITE ? WN : BN);
    Piece bishop = (bySide == WHITE ? WB : BB);
    Piece rook   = (bySide == WHITE ? WR : BR);
    Piece queen  = (bySide == WHITE ? WQ : BQ);
    Piece king   = (bySide == WHITE ? WK : BK);
    Color opp = (bySide == WHITE ? BLACK : WHITE);



    // white pawn attack
    U64 pawnAttack = pawnAttacks[opp][square];
    if(pawnAttack & bitboards[pawn]) return true;
    

    // king attack
    U64 kingAttack = kingAttacks[square];
    if(kingAttack & bitboards[king]) return true;


    // knight attack
    U64 knightAttack = knightAttacks[square];
    if(knightAttack & bitboards[knight]) return true;


    // bishop attack
    U64 bishopAttack = g_magic.getBishopAttack(square, occupancies[BOTH]);
    if(bishopAttack & bitboards[bishop]) return true;


    // rook attack
    U64 rookAttack = g_magic.getRookAttack(square, occupancies[BOTH]);
    if(rookAttack & bitboards[rook]) return true;


    // queen attack
    if((bishopAttack | rookAttack) & bitboards[queen]) return true;


    return false;
}