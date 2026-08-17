#pragma once
#include "utils/type.h"
#include "pawn_table.h"

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
	void calculateMobility(const Board &board, EvalInfo &score);
	void calculateRook(const Board &board, EvalInfo &info);
	void calculateKnightOutpost(const Board &board, EvalInfo &score);
	void calculatePawns(const Board &board, EvalInfo &score);
	void calculateRooksOnPassedPawns(const Board &board, EvalInfo &score, U64 wPassed, U64 bPassed);
};