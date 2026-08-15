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

    inline Square getFrom() const {
        return static_cast<Square>(move & SquareMask);
    }

    inline Square getTo() const {
        return static_cast<Square>((move >> toShift) & SquareMask);
    }

    inline Piece getPromotion() const {
        int promotion = (move >> promoteShift) & pieceMask;
        return static_cast<Piece>(promotion - PIECE_OFFSET);
    }

    inline MoveFlag getMoveFlag() const {
        return static_cast<MoveFlag>((move >> flagShift) & flagMask);
    }

    inline Piece getMovedPiece() const {
        int moved = (move >> movedPieceShift) & pieceMask;
        return static_cast<Piece>(moved - PIECE_OFFSET);
    }

    inline Piece getCapturedPiece() const {
        int captured = (move >> capturedPieceShift) & pieceMask;
        return static_cast<Piece>(captured - PIECE_OFFSET);
    }

    inline U32 getValue() const {
        return move;
    }

    // helper functions
    inline bool isCapture() const {
        U32 flag = (move >> flagShift) & flagMask;
        return flag == capture || flag == promotion_capture || flag == enPassant;
    }

    inline bool isPromotion() const {
        U32 flag = (move >> flagShift) & flagMask;
        return flag == promotion || flag == promotion_capture;
    }

    inline bool isCastle() const {
        U32 moveFlag = (move >> flagShift) & flagMask;
        return (moveFlag == kingSideCastle || moveFlag == queenSideCastle);
    }

    inline bool isEnPassant() const {
        U32 moveFlag = (move >> flagShift) & flagMask;
        return moveFlag == enPassant;
    }

private:
    U32 move = 0;
};