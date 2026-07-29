#pragma once
#include "utils/type.h"
#include "attack/attacks.h"
#include <vector>
#include "move.h"
#include "utils/bitboard_utilities.h"


class Board;

class MoveGenerator{
public:
    MoveGenerator( Board &board, const Attacks &attacks);

    int generateLegalMoves(Move moves[]);
    int getMoveCount() const { return cnt; }

private:
    Board& board;
    const Attacks& attacks;

    int generatePseudoMoves(Move moves[]);

    int cnt;

    void generateBishopMoves(Move moves[]);
    void generateKingMoves(Move moves[]);
    void generateKnightMoves(Move moves[]);
    void generateQueenMoves(Move moves[]);
    void generateRookMoves(Move moves[]);
    void generatePawnMoves(Move moves[]);
};