#pragma once
#include "utils/type.h"

// move -
// from - 6 bits, destination - 6 bits
// pawn promotion - 4 bits, move flag - 3 bits, moved piece - 4 bits, captured piece - 4 bits
// reserved - remaining bits (no use)

// pawn promotion - 5 val - none, queen, rook, bishop, knight -> 0 1 2 4 8
// move flag - enumerated
// moved piece and cpatured - piece enum -> 4 bits


constexpr U32 PIECE_OFFSET = 1;


constexpr U32 toShift = 6;
constexpr U32 promoteShift = 12;
constexpr U32 flagShift = 16;
constexpr U32 movedPieceShift = 19;
constexpr U32 capturedPieceShift = 23;

constexpr U32 SquareMask = 0x3F;          // 6 bits
constexpr U32 flagMask = 0x7;           // 3 bits
constexpr U32 pieceMask = 0xF;          // 4 bits


enum MoveFlag{
    quiet,
    doublePawnPush,
    kingSideCastle,
    queenSideCastle,
    enPassant
};


class Move{
public:
    Move(
        Square from,
        Square to,
        Piece moved,
        Piece captured,
        Piece promotion,
        MoveFlag flag
    );

    Square getFrom() const;
    Square getTo() const;
    Piece getPromotion() const;
    MoveFlag getMoveFlag() const;
    Piece getMovedPiece() const;
    Piece getCapturedPiece() const;
    U32 getValue() const;

    // helper functions
    bool isCapture() const;
    bool isPromotion() const;
    bool isCastle() const;
    bool isEnPassant() const;

private:
    U32 move;
};