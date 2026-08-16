#pragma once
#include "attack/attacks.h"
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
	inline U64 getPawnKey() const{ return pawnKey; };
	inline int getGamePhase() const { return gamePhase; }

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
	U64 pawnKey;
	int gamePhase = TOTAL_PHASE;

	Square parseEnPassantSquare(char pos, int rank, Color tomove);
	void rebuildBitboards();
	static char pieceToChar(Piece p);

	void updateOccupancies();
};

inline bool Board::isSquareAttacked(Square square, Color bySide) const {

	Piece pawn = (bySide == WHITE ? WP : BP);
	Piece knight = (bySide == WHITE ? WN : BN);
	Piece bishop = (bySide == WHITE ? WB : BB);
	Piece rook = (bySide == WHITE ? WR : BR);
	Piece queen = (bySide == WHITE ? WQ : BQ);
	Piece king = (bySide == WHITE ? WK : BK);

	U64 pawnAttack = (bySide == WHITE ? attacks.getBlackPawnAttack(square) : attacks.getWhitePawnAttack(square));
	if (pawnAttack & bitboards[pawn])
		return true;

	if (attacks.getKingAttack(square) & bitboards[king])
		return true;

	if (attacks.getKnightAttack(square) & bitboards[knight])
		return true;

	if (attacks.getBishopAttack(square, occupancies[BOTH]) & bitboards[bishop])
		return true;

	if (attacks.getRookAttack(square, occupancies[BOTH]) & bitboards[rook])
		return true;

	if (attacks.getQueenAttack(square, occupancies[BOTH]) & bitboards[queen])
		return true;

	return false;
}