#include "search.h"
#include "utils/bitboard_utilities.h"
#include <iostream>
#include <algorithm>

using namespace std;

namespace {
    // Approximate piece values for MVV-LVA sorting
    constexpr int mvvPieceValues[NUM_PIECE_TYPE] = {
        100,    // PAWN   (0)
        300,    // KNIGHT (1)
        500,    // ROOK   (2)
        300,    // BISHOP (3)
        900,    // QUEEN  (4)
        10000   // KING   (5)
    };
}

Search::Search(Board& board, MoveGenerator& movegen, Evaluator& evaluator) 
    : board(board), movegen(movegen), evaluator(evaluator) {
    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyTable, 0, sizeof(historyTable));
}

int Search::scoreMove(const Move& move, int ply) {
    if (move.isPromotion()) {
        Piece promoted = move.getPromotion();
        int promoType = (promoted >= BP ? promoted - BP : promoted);
        int score = 200000 + mvvPieceValues[promoType] * 10;
        if (move.isCapture()) {
            Piece captured = move.getCapturedPiece();
            int victimType = (captured == EMPTY ? PAWN : (captured >= BP ? captured - BP : captured));
            score += mvvPieceValues[victimType] * 10;
        }
        return score;
    }

    if (move.isCapture()) {
        Piece moved = move.getMovedPiece();
        Piece captured = move.getCapturedPiece();

        int attackerType = (moved >= BP ? moved - BP : moved);
        int victimType = (captured == EMPTY ? PAWN : (captured >= BP ? captured - BP : captured));

        return 100000 + (mvvPieceValues[victimType] * 10 - mvvPieceValues[attackerType]);
    }

    // Killer moves
    if (ply < MAX_PLYS) {
        if (killerMoves[0][ply].getValue() == move.getValue()) return 90000;
        if (killerMoves[1][ply].getValue() == move.getValue()) return 80000;
    }

    // History heuristic
    Color side = board.getMovingSide();
    return min(historyTable[side][move.getFrom()][move.getTo()], 70000);
}

void Search::orderMoves(Move* moves, int* scores, int count, int ply) {
    for (int i = 0; i < count; i++) {
        scores[i] = scoreMove(moves[i], ply);
    }
}

int Search::quiescence(int alpha, int beta, int ply) {
    if ((nodes & 1023) == 0 && isTimeUp()) return 0;
    nodes++;

    Color movingSide = board.getMovingSide();
    Square kingSq = board.getKingSquare(movingSide);
    Color opp = (movingSide == WHITE ? BLACK : WHITE);
    bool inCheck = board.isSquareAttacked(kingSq, opp);

    int standPat = 0;
    if (!inCheck) {
        standPat = evaluator.evaluate(board);
        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;
    }

    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int count = movegen.generateLegalMoves(moves);

    if (count == 0) {
        if (inCheck) return -MATE_SCORE + ply;
        return 0; // Stalemate
    }

    orderMoves(moves, scores, count, ply);

    if (!inCheck) {
        for (int i = 0; i < count; i++) {
            if (!(moves[i].isCapture() || moves[i].isPromotion())) {
                scores[i] = -1;
            }
        }
    }

    for (int i = 0; i < count; i++) {
        // Pick move with best score
        int bestIdx = i;
        for (int j = i + 1; j < count; j++) {
            if (scores[j] > scores[bestIdx]) {
                bestIdx = j;
            }
        }
        swap(scores[i], scores[bestIdx]);
        swap(moves[i], moves[bestIdx]);

        if (!inCheck) {
            if (scores[i] < 0) break;

            // Delta pruning
            if (standPat < alpha - 975 && !moves[i].isPromotion()) continue;
        }

        board.makeMove(moves[i]);
        int score = -quiescence(-beta, -alpha, ply + 1);
        board.undoMove(moves[i]);

        if (stopped) return 0;

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

int Search::negamax(int alpha, int beta, int depth, int ply) {
    if (depth <= 0) {
        return quiescence(alpha, beta, ply);
    }

    if ((nodes & 1023) == 0 && isTimeUp()) return 0;
    nodes++;

    // Draw detection (50-move rule and 3-fold repetition)
    if (ply > 0 && (board.getHalfMoveClock() >= 100 || board.isRepetition())) {
        return 0;
    }

    Color movingSide = board.getMovingSide();
    Square kingSq = board.getKingSquare(movingSide);
    Color opp = (movingSide == WHITE ? BLACK : WHITE);
    bool inCheck = board.isSquareAttacked(kingSq, opp);

    // Check extension (bounded to avoid infinite tree traps)
    if (inCheck && ply < MAX_PLYS - 10) {
        depth++;
    }

    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int count = movegen.generateLegalMoves(moves);

    if (count == 0) {
        if (inCheck) return -MATE_SCORE + ply;
        return 0; // Stalemate
    }

    orderMoves(moves, scores, count, ply);

    int movesSearched = 0;
    for (int i = 0; i < count; i++) {
        // Pick best scored move
        int bestIdx = i;
        for (int j = i + 1; j < count; j++) {
            if (scores[j] > scores[bestIdx]) {
                bestIdx = j;
            }
        }
        swap(scores[i], scores[bestIdx]);
        swap(moves[i], moves[bestIdx]);

        board.makeMove(moves[i]);
        int score = -negamax(-beta, -alpha, depth - 1, ply + 1);
        board.undoMove(moves[i]);

        if (stopped) return 0;

        movesSearched++;

        if (score >= beta) {
            // Beta cutoff: update killer moves and history heuristic for quiet moves
            if (!moves[i].isCapture() && !moves[i].isPromotion()) {
                if (ply < MAX_PLYS && killerMoves[0][ply].getValue() != moves[i].getValue()) {
                    killerMoves[1][ply] = killerMoves[0][ply];
                    killerMoves[0][ply] = moves[i];
                }
                historyTable[movingSide][moves[i].getFrom()][moves[i].getTo()] += depth * depth;
            }
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

Move Search::findBestMove(int depth) {
    nodes = 0;
    startTime = Clock::now();
    stopped = false;

    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyTable, 0, sizeof(historyTable));

    Move bestMove;
    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int count = movegen.generateLegalMoves(moves);

    if (count == 0) return bestMove;
    if (count == 1) return moves[0];

    bestMove = moves[0];

    for (int d = 1; d <= depth; d++) {
        if (isTimeUp()) break;

        int alpha = -INFINITY_SCORE, beta = INFINITY_SCORE;
        Move currentBest = bestMove;
        int bestScore = -INFINITY_SCORE;
        bool completedDepth = true;

        orderMoves(moves, scores, count, 0);

        // Always search current bestMove from previous iteration first
        if (d > 1) {
            for (int i = 0; i < count; i++) {
                if (moves[i].getValue() == bestMove.getValue()) {
                    scores[i] = 10000000;
                    break;
                }
            }
        }

        for (int i = 0; i < count; i++) {
            int bestIdx = i;
            for (int j = i + 1; j < count; j++) {
                if (scores[j] > scores[bestIdx]) {
                    bestIdx = j;
                }
            }
            swap(scores[i], scores[bestIdx]);
            swap(moves[i], moves[bestIdx]);

            board.makeMove(moves[i]);
            int score = -negamax(-beta, -alpha, d - 1, 1);
            board.undoMove(moves[i]);

            if (stopped || isTimeUp()) {
                completedDepth = false;
                break;
            }

            if (score > alpha) {
                alpha = score;
                bestScore = score;
                currentBest = moves[i];
            }
        }

        if (completedDepth && !stopped) {
            bestMove = currentBest;
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - startTime).count();
            U64 nps = elapsed > 0 ? (nodes * 1000) / elapsed : nodes;
            std::cout << "info depth " << d 
                      << " score cp " << bestScore 
                      << " nodes " << nodes 
                      << " nps " << nps 
                      << " time " << elapsed 
                      << " pv " << (string(squareToStr[bestMove.getFrom()]) + string(squareToStr[bestMove.getTo()])) 
                      << "\n";
        }

        if (stopped || isTimeUp()) break;

        // If found checkmate, stop search early
        if (bestScore >= MATE_THRESHOLD || bestScore <= -MATE_THRESHOLD) {
            break;
        }
    }

    return bestMove;
}