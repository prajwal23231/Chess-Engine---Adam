#pragma once
#include "utils/type.h"
#include "moves/undomove.h"
#include "moves/move.h"
#include "moves/movegen.h"
#include "utils/bitboard_utilities.h"
#include "attack/magic_instance.h"
#include "utils/zobrist.h"
#include <array>
#include <string>

class Board {
public:
    Board();

    void clear();
    void setStartingPosition();
    void print() const;
    bool loadFEN(const std::string &fen);
    Color getMovingSide() const;
    Square getEnPassant() const;
    U64 getBitboard(Piece p) const;
    U64 getOccupancy(Color c) const;
    Piece getPieceBoard(Square s) const;
    int getCastlingRights() const;

    int getHalfMoveClock() const;
    int getFullMoveNumber() const;

    bool makeMove(const Move &move);
    void undoMove(const Move &move);

    bool isSquareAttacked(Square square, Color bySide) const;

private:
    std::array<U64, NUM_PIECES> bitboards;
    std::array<Piece, BOARD_SIZE> board;
    std::array<U64, NUM_COLORS> occupancies;

    Color sideToMove;
    int castlingRights;

    Square enPassant;

    int halfmoveClock;
    int fullmoveNumber;

    std::array<UndoInfo, MAX_PLYS> history;
    int ply = 0;

    U64 zobristKey;
    
    Piece charToPiece(char c);
    Square parseEnPassantSquare(char pos,int rank,Color tomove);
    void rebuildBitboards();
    static char pieceToChar(Piece p);

    void updateOccupancies();


    // square attack lookup
    U64 pawnAttacks[2][BOARD_SIZE];
    U64 knightAttacks[BOARD_SIZE];
    U64 kingAttacks[BOARD_SIZE];

    void initPawnAttacks();
    void initKnightAttacks();
    void initKingAttacks();
};