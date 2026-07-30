#pragma once
#include "utils/type.h"
#include "attack/attacks.h"
#include <vector>
#include "move.h"
#include "utils/bitboard_utilities.h"


struct CheckInfo{
    U64 checkers = 0;
    U64 checkMask = 0;
    U64 pinnedRay[BOARD_SIZE] = {};
    U64 pinnedPieces = 0;
    int checkerCount = 0;
};


class Board;


class MoveGenerator{
public:
    MoveGenerator(Board &board);

    int generateLegalMoves(Move moves[]);

private:
    Board& board;

    U64 between[BOARD_SIZE][BOARD_SIZE];
    int cnt;
    U64 enemyAttackMap;


    // layer 1
    Square getKingpos() const;
    void createAttackMap();
    CheckInfo analyzeChecks() const;
    void computePins(CheckInfo &info) const;
    void computeChecks(CheckInfo &info) const;
    void computeOrthogonalPins(CheckInfo &info) const;
    void computeDiagonalPins(CheckInfo &info) const;


    void generateBishopMoves(Move moves[]);
    void generateKingMoves(Move moves[]);
    void generateKnightMoves(Move moves[]);
    void generateQueenMoves(Move moves[]);
    void generateRookMoves(Move moves[]);
    void generatePawnMoves(Move moves[]);


    void computeBetween();
};