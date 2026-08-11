#pragma once
#include "utils/type.h"
#include "attack/attacks.h"
#include "utils/tools.h"
#include <vector>
#include "move.h"
#include "utils/bitboard_utilities.h"


struct CheckInfo{
    U64 checkers = 0;
    U64 checkMask = 0;
    U64 pinnedPieces = 0;
    int checkerCount = 0;
    U64 pinnedRay[BOARD_SIZE];
};


class Board;


class MoveGenerator{
public:
    MoveGenerator(Board &board);

    int generateLegalMoves(Move moves[]);

private:
    Board& board;

    int cnt;
    U64 enemyAttackMap;
    U64 occ, friendlyocc;
    Color tomove;
    Square kingpos;


    // layer 1
    void computeAttackMapAndChecks(CheckInfo &info);
    void analyzeChecks(CheckInfo &info);
    void computePins(CheckInfo &info) const;
    void computeOrthogonalPins(CheckInfo &info) const;
    void computeDiagonalPins(CheckInfo &info) const;



    void generateKingMoves(Move moves[],CheckInfo& info);
    void generateKnightMoves(Move moves[],CheckInfo& info);
    void generatePawnMoves(Move moves[],CheckInfo& info);
    void generateQueenMoves(Move moves[],CheckInfo& info);
    void generateRookMoves(Move moves[],CheckInfo& info);
    void generateBishopMoves(Move moves[],CheckInfo& info);


    void fillMoves(Move moves[], U64 mask, Square from, Piece moved);
    bool iskingattacked(U64 newocc);
};