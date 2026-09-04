#include "syzygy.h"
#include "tbprobe.h"
#include "iostream"

using namespace std;
using namespace Bitboard;


bool Syzygy::init(const string& path){
    bool ok = tb_init(path.c_str());

    if (ok && TB_LARGEST > 0) {
        cout << "info string Syzygy tablebases loaded up to " << TB_LARGEST << " pieces." << endl;
    }
    
    else {
        cout << "info string No Syzygy tablebases found at: " << path << endl;
    }

    return ok;
}

int Syzygy::getMaxPieces(){
    return TB_LARGEST;
}


WDLResult Syzygy::probeWDL(const Board& board){
    if(TB_LARGEST == 0) return TB_RESULT_FAIL;

    int pieces = popCount(board.getOccupancy(BOTH));
    if(pieces > TB_LARGEST) return TB_RESULT_FAIL;

    // Fathom requires castling == 0 and halfmove == 0 for WDL
    if (board.getCastlingRights() != 0 || board.getHalfMoveClock() != 0) {
        return TB_RESULT_FAIL;
    }

    unsigned res = tb_probe_wdl(
        board.getOccupancy(WHITE),
        board.getOccupancy(BLACK),
        board.getBitboard(WK) | board.getBitboard(BK),
        board.getBitboard(WQ) | board.getBitboard(BQ),
        board.getBitboard(WR) | board.getBitboard(BR),
        board.getBitboard(WB) | board.getBitboard(BB),
        board.getBitboard(WN) | board.getBitboard(BN),
        board.getBitboard(WP) | board.getBitboard(BP),
        board.getHalfMoveClock(),
        board.getCastlingRights(),
        board.getEnPassant() != NO_SQUARE ? board.getEnPassant() : 0,
        board.getMovingSide() == WHITE
    );

    if(res == TB_RESULT_FAILED) return TB_RESULT_FAIL;
    if(res == TB_WIN) return TB_RESULT_WIN;
    if(res == TB_LOSS) return TB_RESULT_LOSS;

    return TB_RESULT_DRAW;
}



bool Syzygy::probeRoot(const Board& board, Move& bestMove) {
    if(TB_LARGEST == 0) return false;

    int pieces = popCount(board.getOccupancy(BOTH));
    if(pieces > TB_LARGEST) return false;

    // Probe DTZ at root
    unsigned result = tb_probe_root(
        board.getOccupancy(WHITE),
        board.getOccupancy(BLACK),
        board.getBitboard(WK) | board.getBitboard(BK),
        board.getBitboard(WQ) | board.getBitboard(BQ),
        board.getBitboard(WR) | board.getBitboard(BR),
        board.getBitboard(WB) | board.getBitboard(BB),
        board.getBitboard(WN) | board.getBitboard(BN),
        board.getBitboard(WP) | board.getBitboard(BP),
        board.getHalfMoveClock(),
        board.getCastlingRights(),
        board.getEnPassant() != NO_SQUARE ? board.getEnPassant() : 0,
        board.getMovingSide() == WHITE,
        nullptr
    );

    if(result == TB_RESULT_FAILED || result == TB_RESULT_CHECKMATE || result == TB_RESULT_STALEMATE) {
        return false;
    } 

    Square from = static_cast<Square>(TB_GET_FROM(result));
    Square to   = static_cast<Square>(TB_GET_TO(result));
    Piece moved = board.getPieceBoard(from);
    Piece captured = board.getPieceBoard(to);
    MoveFlag flag = (captured != EMPTY) ? capture : quiet;
    Piece promo = EMPTY;

    if(TB_GET_EP(result)) flag = enPassant;

    unsigned tbProm = TB_GET_PROMOTES(result);

    if(tbProm != TB_PROMOTES_NONE){
        flag = (captured != EMPTY) ? promotion_capture : promotion;
        Color stm = board.getMovingSide();

        if(tbProm == TB_PROMOTES_QUEEN) promo = (stm == WHITE ? WQ : BQ);
        else if(tbProm == TB_PROMOTES_BISHOP) promo = (stm == WHITE ? WB : BB);
        else if(tbProm == TB_PROMOTES_KNIGHT) promo = (stm == WHITE ? WN : BN);
        else if(tbProm == TB_PROMOTES_ROOK) promo = (stm == WHITE ? WR : BR);
    }

    bestMove = Move(from, to, moved, captured, promo, flag);
    return true;
}