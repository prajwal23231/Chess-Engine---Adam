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
	inline Square getKingSquare(Color c) const {
		U64 k = bitboards[c == WHITE ? WK : BK];
		return k ? static_cast<Square>(Bitboard::lsb(k)) : NO_SQUARE;
	}
	inline bool isRepetition() const {
		for(int i = ply - 2; i >= 0 && i >= ply - halfmoveClock; i -= 2){
			if(history[i].zobristKey == zobristKey) return true;
		}
		return false;
	}

	bool makeMove(const Move &move);
	void undoMove(const Move &move);

	inline bool isSquareAttacked(Square square, Color bySide) const;

	inline U64 getZobristKey() const { return zobristKey; }
	inline U64 getPawnKey() const{ return pawnKey; };
	inline int getGamePhase() const { return gamePhase; }

	inline int getMgScore() const { return mgScore; }
	inline int getEgScore() const { return egScore; }


	void makeNullMove();
	void undoNullMove();
	inline bool hasNonPawnMaterial(Color c) const{
		if(c == WHITE){
			return bitboards[WN] | bitboards[WB] | bitboards[WR] | bitboards[WQ];
		}

		else{
			return bitboards[BN] | bitboards[BB] | bitboards[BR] | bitboards[BQ];
		}
	}

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
	int mgScore = 0;
	int egScore = 0;

	Square parseEnPassantSquare(char pos, int rank, Color tomove);
	void rebuildBitboards();
	static char pieceToChar(Piece p);

	void updateOccupancies();

	inline void addPieceScore(Piece p, Square sq);
	inline void removePieceScore(Piece p, Square sq);
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



inline void Board::addPieceScore(Piece p,Square sq){
	int pieceType = (p<BP ? p : p-BP);

	if(p<BP){
		mgScore += mg_value[pieceType] + pst[WHITE][MG][pieceType][sq];
		egScore += eg_value[pieceType] + pst[WHITE][EG][pieceType][sq];
	}

	else{
		mgScore -= mg_value[pieceType] + pst[BLACK][MG][pieceType][sq];
		egScore -= eg_value[pieceType] + pst[BLACK][EG][pieceType][sq];
	}
}



inline void Board::removePieceScore(Piece p,Square sq){
	int pieceType = (p<BP ? p : p-BP);

	if(p<BP){
		mgScore -= mg_value[pieceType] + pst[WHITE][MG][pieceType][sq];
		egScore -= eg_value[pieceType] + pst[WHITE][EG][pieceType][sq];
	}

	else{
		mgScore += mg_value[pieceType] + pst[BLACK][MG][pieceType][sq];
		egScore += eg_value[pieceType] + pst[BLACK][EG][pieceType][sq];
	}
}