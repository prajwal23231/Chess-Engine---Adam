#include "move.h"

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

    move = start | dest | movedPiece | capturedPiece | Promoted | mflag;
}