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
    U64 occ, friendlyocc;
    Color tomove;


    // layer 1
    Square getKingpos() const;
    void createAttackMap();
    CheckInfo analyzeChecks() const;
    void computePins(CheckInfo &info) const;
    void computeChecks(CheckInfo &info) const;
    void computeOrthogonalPins(CheckInfo &info) const;
    void computeDiagonalPins(CheckInfo &info) const;



    void generateKingMoves(Move moves[],CheckInfo& info);
    void generateKnightMoves(Move moves[],CheckInfo& info);
    void generatePawnMoves(Move moves[],CheckInfo& info);
    void generateQueenMoves(Move moves[],CheckInfo& info);
    void generateRookMoves(Move moves[],CheckInfo& info);
    void generateBishopMoves(Move moves[],CheckInfo& info);


    void computeBetween();
    void fillMoves(Move moves[], U64 mask, Square from, Piece moved);
    bool iskingattacked(U64 newocc);
};