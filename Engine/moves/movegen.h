#pragma once
#include "utils/type.h"
#include "attack/attacks.h"
#include <vector>
#include "move.h"
#include "utils/bitboard_utilities.h"


class Board;

class MoveGenerator{
public:
    MoveGenerator(
        const Board &board,
        const Attacks &attacks
    );

    int generateMoves(Move moves[]);
    int getMoveCount() const { return cnt; }

private:
    const Board& board;
    const Attacks& attacks;

    int cnt;

    void generateBishopMoves(Move moves[]);
    void generateKingMoves(Move moves[]);
    void generateKnightMoves(Move moves[]);
    void generateQueenMoves(Move moves[]);
    void generateRookMoves(Move moves[]);
    void generatePawnMoves(Move moves[]);
};