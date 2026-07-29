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

    void generateMoves(std::vector<Move>& moves) const;

private:
    const Board& board;
    const Attacks& attacks;

    void generateBishopMoves(std::vector<Move> &moves) const;
    void generateKingMoves(std::vector<Move> &moves) const;
    void generateKnightMoves(std::vector<Move> &moves) const;
    void generateQueenMoves(std::vector<Move> &moves) const;
    void generateRookMoves(std::vector<Move> &moves) const;
    void generatePawnMoves(std::vector<Move> &moves) const;
};