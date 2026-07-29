#pragma once
#include "utils/type.h"

// move -
// from - 6 bits, destination - 6 bits
// pawn promotion - 4 bits, move flag - 4 bits, moved piece - 4 bits, captured piece - 4 bits
// reserved - remaining bits (no use)
// pawn promotion - 5 val - none, queen, rook, bishop, knight -> using piece enum
// move flag - enumerated
// moved piece and cpatured - piece enum -> 4 bits


constexpr U32 PIECE_OFFSET = 1;


constexpr U32 toShift = 6;
constexpr U32 promoteShift = 12;
constexpr U32 flagShift = 16;
constexpr U32 movedPieceShift = 20;
constexpr U32 capturedPieceShift = 24;

constexpr U32 SquareMask = 0x3F;          // 6 bits
constexpr U32 flagMask = 0xF;           // 3 bits
constexpr U32 pieceMask = 0xF;          // 4 bits


enum MoveFlag{
    quiet,
    capture,
    doublePawnPush,
    kingSideCastle,
    queenSideCastle,
    enPassant,
    promotion,
    promotion_capture
};


class Move{
public:
    Move() = default;
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