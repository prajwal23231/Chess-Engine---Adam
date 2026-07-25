#include "move.h"
#include <iostream>

using namespace std;

Move::Move(
    Square from,
    Square to,
    Piece moved,
    Piece captured,
    Piece promotion,
    MoveFlag flag
){
    U32 start = from;
    U32 dest = to << toShift;
    U32 Promoted = (promotion + PIECE_OFFSET) << promoteShift;
    U32 mflag = flag << flagShift;
    U32 movedPiece = (moved + PIECE_OFFSET) << movedPieceShift;
    U32 capturedPiece = (captured + PIECE_OFFSET) << capturedPieceShift;

    move = start | dest | movedPiece | capturedPiece | Promoted | mflag ;
}


bool Move::isCapture() const{
    U32 flag = (move >> flagShift) & flagMask;
    return flag==capture;
}


bool Move::isPromotion() const{
    U32 flag = (move >> flagShift) & flagMask;
    return flag==promotion;
}


bool Move::isCastle() const{
    U32 moveFlag = (move >> flagShift) & flagMask;
    return (moveFlag == kingSideCastle || moveFlag == queenSideCastle);
}


bool Move::isEnPassant() const{
    U32 moveFlag = (move >> flagShift) & flagMask;
    return moveFlag == enPassant;
}

Square Move::getFrom() const{
    U32 from = move & SquareMask;
    Square s = static_cast<Square>(from);
    return s;
}

Square Move::getTo() const{
    U32 to = (move >> toShift) & SquareMask;
    Square s = static_cast<Square>(to);
    return s;
}


Piece Move::getPromotion() const{
    int promotion = (move >> promoteShift) & pieceMask;
    Piece p = static_cast<Piece>(promotion - PIECE_OFFSET);
    return p;
}


MoveFlag Move::getMoveFlag() const{
    U32 flag = (move >> flagShift) & flagMask;
    MoveFlag f = static_cast<MoveFlag>(flag);
    return f;
}


Piece Move::getMovedPiece() const{
    int moved = (move >> movedPieceShift) & pieceMask;
    Piece p = static_cast<Piece>(moved - PIECE_OFFSET);
    return p;
}


Piece Move::getCapturedPiece() const{
    int captured = (move >> capturedPieceShift) & pieceMask;
    Piece p = static_cast<Piece>(captured - PIECE_OFFSET);
    return p;
}


U32 Move::getValue() const{
    return move;
}