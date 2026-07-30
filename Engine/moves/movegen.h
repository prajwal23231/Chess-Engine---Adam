#pragma once
#include "utils/type.h"
#include "attack/attacks.h"
#include <vector>
#include "move.h"
#include "utils/bitboard_utilities.h"


class Board;

class MoveGenerator{
public:
    MoveGenerator(Board &board);

    int generateLegalMoves(Move moves[]);
    int generatePseudoMoves(Move moves[]);
    int getMoveCount() const { return cnt; }

private:
    Board& board;
    int cnt;

    void generateBishopMoves(Move moves[]);
    void generateKingMoves(Move moves[]);
    void generateKnightMoves(Move moves[]);
    void generateQueenMoves(Move moves[]);
    void generateRookMoves(Move moves[]);
    void generatePawnMoves(Move moves[]);
};