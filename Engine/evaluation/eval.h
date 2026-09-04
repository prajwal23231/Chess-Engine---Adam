#pragma once
#include "utils/type.h"
#include "pawn_table.h"
#include "hash/kpk.h"

class Board;

struct EvalInfo {
	int mg = 0;
	int eg = 0;

	EvalInfo &operator+=(const EvalInfo &other) {
		mg += other.mg;
		eg += other.eg;
		return *this;
	}
};


class Evaluator {
public:
	int evaluate(const Board &board);

private:
	PawnTable pawntable;

	int interpolate(const EvalInfo &score, int phase);

	void calculateMaterial(const Board &board, EvalInfo &score);
	void calculatePST(const Board &board, EvalInfo &score);
	void calculateBishopPair(const Board &board, EvalInfo &score);
	void calculateMobility(const Board &board, EvalInfo &score, PawnTableEntry* entry);
	void calculateRook(const Board &board, EvalInfo &info, PawnTableEntry* entry);
	void calculateKnightOutpost(const Board &board, EvalInfo &score,PawnTableEntry* entry);
	void calculatePawns(const Board &board, EvalInfo &score);
	void calculateKingSafety(const Board& board, EvalInfo & score);
	void calculateDevelopment(const Board& board, EvalInfo &score);
	void calculateHangingPieces(const Board& board, EvalInfo& score);
	int calculateMatingScore(const Board& board, int egScore);
	int getMaterialScaleFactor(const Board& board);
	void calculateBishopTrappedAndBad(const Board& board, EvalInfo& score);
};