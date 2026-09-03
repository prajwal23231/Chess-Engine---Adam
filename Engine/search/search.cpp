#include "search.h"
#include "utils/bitboard_utilities.h"
#include <iostream>
#include <algorithm>

using namespace std;
using namespace Bitboard;

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

    constexpr U64 castleShieldMask[BOARD_SIZE] = {
        // C1 (Queenside White)
        0, 0, (1ULL << A2) | (1ULL << B2) | (1ULL << C2), 0, 0, 0,
        // G1 (Kingside White)
        (1ULL << F2) | (1ULL << G2) | (1ULL << H2), 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        // C8 (Queenside Black)
        0, 0, (1ULL << A7) | (1ULL << B7) | (1ULL << C7), 0, 0, 0,
        // G8 (Kingside Black)
        (1ULL << F7) | (1ULL << G7) | (1ULL << H7), 0
    };
}



Search::Search(Board& board, MoveGenerator& movegen, Evaluator& evaluator) 
    : board(board), movegen(movegen), evaluator(evaluator) {
    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyTable, 0, sizeof(historyTable));
    tt.init(64);
}


int Search::scoreMove(const Move& move, int ply,const Move& ttMove) {
    // tt move is top priority
    if(ttMove.getValue() != 0 && move.getValue() == ttMove.getValue()){
        return 10000000;
    }


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
    int historyScore = min(historyTable[side][move.getFrom()][move.getTo()], 70000);


    if(move.isCastle()){
        Square to = move.getTo();
        Piece myPawn = (side == WHITE ? WP : BP);
        int shieldCount = popCount(board.getBitboard(myPawn) & castleShieldMask[to]);
        return historyScore + (board.getGamePhase() * 1100) + (shieldCount * 2500);
    }


    return historyScore;
}



void Search::orderMoves(Move* moves, int* scores, int count, int ply, const Move& ttMove) {
    for (int i = 0; i < count; i++) {
        scores[i] = scoreMove(moves[i], ply, ttMove);
    }
}



int Search::quiescence(int alpha, int beta, int ply) {
    if ((nodes & 1023) == 0 && isTimeUp()) return 0;
    nodes++;

    if (ply >= MAX_PLYS - 1) return evaluator.evaluate(board);

    Color movingSide = board.getMovingSide();
    Square kingSq = board.getKingSquare(movingSide);
    Color opp = (movingSide == WHITE ? BLACK : WHITE);
    bool inCheck = board.isSquareAttacked(kingSq, opp);

    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int count = movegen.generateLegalMoves(moves);

    if (count == 0) {
        if (inCheck) return -MATE_SCORE + ply;
        return 0; // Stalemate
    }

    if (inCheck) {
        // When in check in quiescence, search all evasions and never return a false high alpha
        int bestScore = -INFINITY_SCORE;
        orderMoves(moves, scores, count, ply);

        for (int i = 0; i < count; i++) {
            int bestIdx = i;
            for (int j = i + 1; j < count; j++) {
                if (scores[j] > scores[bestIdx]) bestIdx = j;
            }
            swap(scores[i], scores[bestIdx]);
            swap(moves[i], moves[bestIdx]);

            board.makeMove(moves[i]);
            int score = -quiescence(-beta, -alpha, ply + 1);
            board.undoMove(moves[i]);

            if (stopped) return 0;

            if (score > bestScore) bestScore = score;
            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        }
        return bestScore;
    }

    // Stand-pat evaluation when not in check
    int standPat = evaluator.evaluate(board);
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    orderMoves(moves, scores, count, ply);

    // Filter out quiet moves
    for (int i = 0; i < count; i++) {
        if (!(moves[i].isCapture() || moves[i].isPromotion())) {
            scores[i] = -1;
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

        if (scores[i] < 0) break;

        // Accurate Delta Pruning: only prune if even capturing this specific piece cannot reach alpha
        Piece captured = moves[i].getCapturedPiece();
        int victimType = (captured == EMPTY ? PAWN : (captured >= BP ? captured - BP : captured));
        int pieceVal = mvvPieceValues[victimType];
        if (standPat + pieceVal + 200 < alpha && !moves[i].isPromotion()) {
            continue;
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

    if (ply >= MAX_PLYS - 1) return evaluator.evaluate(board);


    int orignalAlpha = alpha;
    Move ttMove;
    TTEntry* entry = tt.probe(board.getZobristKey());

    if(entry != nullptr){
        ttMove = entry->bestMove;

        if(ply>0 && entry->depth >= depth){
            int ttScore = TranspositionTable::scoreFromTT(entry->score,ply);

            // exact pv node
            if(entry->flag == TT_EXACT){
                return ttScore;
            }

            // beta cutoff
            if(entry->flag == TT_LOWER && ttScore >= beta){
                return ttScore;
            }

            // alpha cutoff
            if(entry->flag == TT_UPPER && ttScore <= alpha){
                return ttScore;
            }
        }
    }


    Color movingSide = board.getMovingSide();
    Square kingSq = board.getKingSquare(movingSide);
    Color opp = (movingSide == WHITE ? BLACK : WHITE);
    bool inCheck = board.isSquareAttacked(kingSq, opp);

    if(depth <=2 && !inCheck && abs(beta) < MATE_THRESHOLD){
        int eval = evaluator.evaluate(board);
        int margin = 120 * depth;
        
        if(eval - margin >= beta){
            return beta;
        }
    }

    int extension = inCheck ? 1 : 0;

    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int count = movegen.generateLegalMoves(moves);

    if (count == 0) {
        if (inCheck) return -MATE_SCORE + ply;
        return 0; // Stalemate
    }

    orderMoves(moves, scores, count, ply, ttMove);
    Move bestMove;

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
        int score = 0;
        bool isQuiet = !moves[i].isCapture() && !moves[i].isPromotion();


        // LMR
        if(movesSearched >= 4 && depth >= 3 && isQuiet && !inCheck){
            score = -negamax(-alpha-1, -alpha, depth - 2 + extension, ply + 1);

            if(score > alpha){
                score = -negamax(-beta, -alpha, depth - 1 + extension, ply + 1);
            }
        }

        else{
            score = -negamax(-beta, -alpha, depth - 1 + extension, ply + 1);
        }

        board.undoMove(moves[i]);
        movesSearched++;

        if (stopped) return 0;

        if (score >= beta) {
            bestMove = moves[i];

            // Beta cutoff: update killer moves and history heuristic for quiet moves
            if (!moves[i].isCapture() && !moves[i].isPromotion()) {
                if (ply < MAX_PLYS && killerMoves[0][ply].getValue() != moves[i].getValue()) {
                    killerMoves[1][ply] = killerMoves[0][ply];
                    killerMoves[0][ply] = moves[i];
                }
                historyTable[movingSide][moves[i].getFrom()][moves[i].getTo()] += depth * depth;
            }

            tt.store(board.getZobristKey(), depth, score, TT_LOWER, bestMove, ply);
            return beta;
        }


        if (score > alpha) {
            alpha = score;
            bestMove = moves[i];
        }
    }


    TTFlag flag = (alpha > orignalAlpha) ? TT_EXACT : TT_UPPER;
    tt.store(board.getZobristKey(), depth, alpha, flag, bestMove, ply);
    
    return alpha;
}

Move Search::findBestMove(int depth) {
    nodes = 0;
    startTime = Clock::now();
    stopped = false;

    memset(killerMoves, 0, sizeof(killerMoves));
    memset(historyTable, 0, sizeof(historyTable));

    Move moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int count = movegen.generateLegalMoves(moves);

    if (count == 0) return Move();
    if (count == 1) return moves[0];

    // Order initial root moves so default pre-search move is best heuristic move
    orderMoves(moves, scores, count, 0);
    int initialBestIdx = 0;
    for (int i = 1; i < count; i++) {
        if (scores[i] > scores[initialBestIdx]) initialBestIdx = i;
    }
    swap(moves[0], moves[initialBestIdx]);
    swap(scores[0], scores[initialBestIdx]);

    Move bestMove = moves[0];
    int bestScore = -INFINITY_SCORE;

    for (int d = 1; d <= depth; d++) {
        if (isTimeUp()) break;

        int alpha = -INFINITY_SCORE, beta = INFINITY_SCORE;
        Move currentIterationBest = moves[0];
        int currentIterationScore = -INFINITY_SCORE;
        bool completedDepth = true;

        orderMoves(moves, scores, count, 0, bestMove);

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

            // If stopped or timed out during evaluation of this move, abort this depth completely!
            if (stopped || isTimeUp()) {
                completedDepth = false;
                break;
            }

            if (score > alpha) {
                alpha = score;
                currentIterationScore = score;
                currentIterationBest = moves[i];
            }
        }

        // Only commit currentIterationBest if this entire depth finished without timeout
        if (completedDepth && !stopped) {
            bestMove = currentIterationBest;
            bestScore = currentIterationScore;

            tt.store(board.getZobristKey(), d, bestScore, TT_EXACT, bestMove, 0);

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - startTime).count();
            U64 nps = elapsed > 0 ? (nodes * 1000) / elapsed : nodes;

            string pvMove = string(squareToStr[bestMove.getFrom()]) + string(squareToStr[bestMove.getTo()]);
            if (bestMove.isPromotion()) {
                Piece prom = bestMove.getPromotion();
                if (prom == WN || prom == BN) pvMove += 'n';
                else if (prom == WB || prom == BB) pvMove += 'b';
                else if (prom == WR || prom == BR) pvMove += 'r';
                else pvMove += 'q';
            }

            std::cout << "info depth " << d 
                      << " score cp " << bestScore 
                      << " nodes " << nodes 
                      << " nps " << nps 
                      << " time " << elapsed 
                      << " pv " << pvMove 
                      << "\n";
        } else {
            // Aborted mid-depth: strictly keep bestMove from the last fully completed depth!
            break;
        }

        if (stopped || isTimeUp()) break;

        // If found forced checkmate win, stop search early
        if (bestScore >= MATE_THRESHOLD) {
            break;
        }
    }

    return bestMove;
}