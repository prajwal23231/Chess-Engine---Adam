#pragma once
#include "attack/magic_instance.h"
#include "moves/move.h"
#include "moves/movegen.h"
#include "moves/undomove.h"
#include "utils/bitboard_utilities.h"
#include "utils/type.h"
#include "hash/zobrist.h"
#include <array>
#include <string>


class Board {
public:
  Board();

  void clear();
  void setStartingPosition();
  void print() const;
  bool loadFEN(const std::string &fen);
  inline Color getMovingSide() const { return sideToMove; }
  inline Square getEnPassant() const { return enPassant; }
  inline U64 getBitboard(Piece p) const { return bitboards[p]; }
  inline U64 getOccupancy(Color c) const { return occupancies[c]; }
  inline Piece getPieceBoard(Square s) const { return board[s]; }
  inline int getCastlingRights() const { return castlingRights; }

  inline int getHalfMoveClock() const { return halfmoveClock; }
  inline int getFullMoveNumber() const { return fullmoveNumber; }
  inline Square getKingSquare(Color c) const { return kingSquare[c]; }

  bool makeMove(const Move &move);
  void undoMove(const Move &move);

  inline bool isSquareAttacked(Square square, Color bySide) const;

  inline U64 getZobristKey() const { return zobristKey; }

private:
  std::array<U64, NUM_PIECES> bitboards;
  std::array<Piece, BOARD_SIZE> board;
  std::array<U64, NUM_COLORS> occupancies;

  Color sideToMove;
  int castlingRights;

  Square enPassant;
  std::array<Square, 2> kingSquare = {E1, E8};

  int halfmoveClock;
  int fullmoveNumber;

  std::array<UndoInfo, MAX_PLYS> history;
  int ply = 0;

  U64 zobristKey;

  Square parseEnPassantSquare(char pos, int rank, Color tomove);
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

inline bool Board::isSquareAttacked(Square square, Color bySide) const {

  Piece pawn = (bySide == WHITE ? WP : BP);
  Piece knight = (bySide == WHITE ? WN : BN);
  Piece bishop = (bySide == WHITE ? WB : BB);
  Piece rook = (bySide == WHITE ? WR : BR);
  Piece queen = (bySide == WHITE ? WQ : BQ);
  Piece king = (bySide == WHITE ? WK : BK);
  Color opp = (bySide == WHITE ? BLACK : WHITE);

  if (pawnAttacks[opp][square] & bitboards[pawn])
    return true;

  if (kingAttacks[square] & bitboards[king])
    return true;

  if (knightAttacks[square] & bitboards[knight])
    return true;

  if (g_magic.getBishopAttack(square, occupancies[BOTH]) & bitboards[bishop])
    return true;

  if (g_magic.getRookAttack(square, occupancies[BOTH]) & bitboards[rook])
    return true;

  if ((g_magic.getBishopAttack(square, occupancies[BOTH]) |
       g_magic.getRookAttack(square, occupancies[BOTH])) &
      bitboards[queen])
    return true;

  return false;
}