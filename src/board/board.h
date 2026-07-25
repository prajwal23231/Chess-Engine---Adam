#pragma once
#include "utils/type.h"

#include <array>
#include <string>

class Board {
public:
    Board();

    void clear();
    void setStartingPosition();
    void updateOccupancies();
    void print() const;
    bool loadFEN(const std::string &fen);
    Color getMovingSide() const;
    Square getEnPassant() const;
    U64 getBitboard(Piece p) const;
    U64 getOccupancy(Color c) const;
    Piece getPieceBoard(Square s) const;

private:
    std::array<U64, NUM_PIECES> bitboards;
    std::array<Piece, BOARD_SIZE> board;
    std::array<U64, NUM_COLORS> occupancies;

    Color sideToMove;
    int castlingRights;

    Square enPassant;

    int halfmoveClock;
    int fullmoveNumber;
    
    Piece charToPiece(char c);
    Square parseEnPassantSquare(char pos,int rank,Color tomove);
    void rebuildBitboards();
    static char pieceToChar(Piece p);
};