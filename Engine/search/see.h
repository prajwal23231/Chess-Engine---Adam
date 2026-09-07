#pragma once
#include "board/board.h"
#include "moves/move.h"
#include "utils/type.h"
#include "utils/bitboard_utilities.h"
#include "attack/attacks.h"
#include <algorithm>

namespace SEE {
    // Piece values for SEE calculations (Notice: WB=3, WR=2 in Adam's Piece enum)
    constexpr int seePieceValues[NUM_PIECES] = {
        100,  320,  500,  330,  900,  20000, // WP, WN, WR, WB, WQ, WK
        100,  320,  500,  330,  900,  20000  // BP, BN, BR, BB, BQ, BK
    };

    // Piece type values (0..5: P, N, R, B, Q, K)
    constexpr int seeTypeValue[NUM_PIECE_TYPE] = {
        100, 320, 500, 330, 900, 20000
    };

    /**
     * Returns a bitboard of all pieces of both colors attacking 'sq',
     * taking into account the dynamic 'occ' (blocking pieces and vacated squares).
     */
    inline U64 attackersTo(const Board& board, Square sq, U64 occ) {
        U64 attackers = 0;

        // Pawns: Black pawn attack mask from 'sq' points to squares where White pawns attack 'sq', and vice versa
        attackers |= attacks.getBlackPawnAttack(sq) & board.getBitboard(WP);
        attackers |= attacks.getWhitePawnAttack(sq) & board.getBitboard(BP);

        // Knights
        attackers |= attacks.getKnightAttack(sq) & (board.getBitboard(WN) | board.getBitboard(BN));

        // Sliding pieces (Magic Bitboards evaluate rays against dynamic 'occ')
        attackers |= attacks.getBishopAttack(sq, occ) & (board.getBitboard(WB) | board.getBitboard(BB));
        attackers |= attacks.getRookAttack(sq, occ) & (board.getBitboard(WR) | board.getBitboard(BR));
        attackers |= attacks.getQueenAttack(sq, occ) & (board.getBitboard(WQ) | board.getBitboard(BQ));

        // King
        attackers |= attacks.getKingAttack(sq) & (board.getBitboard(WK) | board.getBitboard(BK));

        // Crucial: Only consider pieces that actually exist in the current occupancy!
        return attackers & occ;
    }

    /**
     * Finds the Least Valuable Attacker (LVA) of 'side' from the attackers bitboard.
     * Updates 'occ' and 'attackers' by clearing the chosen piece's square.
     */
    inline Piece getLVA(const Board& board, U64& attackers, Color side, U64& occ, Square& fromSq) {
        int startPiece = (side == WHITE ? WP : BP);

        // Adam piece enum types: P=0, N=1, R=2, B=3, Q=4, K=5
        // We order by piece value: Pawn(0), Knight(1), Bishop(3), Rook(2), Queen(4), King(5)
        static const int pieceOrder[NUM_PIECE_TYPE] = {0, 1, 3, 2, 4, 5};

        for (int orderIdx = 0; orderIdx < NUM_PIECE_TYPE; orderIdx++) {
            int pType = pieceOrder[orderIdx];
            Piece p = static_cast<Piece>(startPiece + pType);

            U64 candidates = attackers & board.getBitboard(p) & occ;

            if (candidates) {
                // If the only attacker left is the King, ensure the target square is not defended by opponent
                if (pType == 5) {
                    Color opp = (side == WHITE ? BLACK : WHITE);
                    if (attackers & board.getOccupancy(opp) & occ) {
                        break; // King cannot capture into check!
                    }
                }

                fromSq = static_cast<Square>(Bitboard::lsb(candidates));
                occ &= ~(1ULL << fromSq);
                attackers &= ~(1ULL << fromSq);

                return p;
            }
        }

        fromSq = NO_SQUARE;
        return EMPTY;
    }

    /**
     * Statically evaluates an exchange sequence on the destination square of 'move'.
     * Returns centipawn score from the perspective of the moving side.
     * Positive: winning capture, 0: equal trade, Negative: losing capture.
     */
    inline int evaluate(const Board& board, const Move& move) {
        Square from = move.getFrom();
        Square to = move.getTo();
        Piece moved = move.getMovedPiece();
        Piece captured = move.getCapturedPiece();

        int gain[32];
        int depth = 0;

        // Step 1: Initial material gain
        gain[0] = (captured != EMPTY) ? seePieceValues[captured] : 0;
        if (move.isPromotion()) {
            gain[0] += seePieceValues[move.getPromotion()] - seePieceValues[moved];
            moved = move.getPromotion();
        }

        // Build simulated occupancy
        U64 occ = board.getOccupancy(BOTH);
        occ &= ~(1ULL << from);
        occ |= (1ULL << to);

        if (move.isEnPassant()) {
            Square epPawnSq = static_cast<Square>((board.getMovingSide() == WHITE ? to - 8 : to + 8));
            occ &= ~(1ULL << epPawnSq);
        }

        Color side = (board.getMovingSide() == WHITE ? BLACK : WHITE);
        U64 allattackers = attackersTo(board, to, occ);

        U64 diagSliders = board.getBitboard(WB) |
                          board.getBitboard(BB) |
                          board.getBitboard(WQ) |
                          board.getBitboard(BQ);

        U64 orthSliders = board.getBitboard(WR) |
                          board.getBitboard(BR) |
                          board.getBitboard(WQ) |
                          board.getBitboard(BQ);

        // Step 2: Simulate recapture sequence
        while (true) {
            depth++;
            gain[depth] = seePieceValues[moved] - gain[depth - 1];

            // Pruning: if even after recapturing, the side is still at a material loss, they will decline to recapture
            if (std::max(-gain[depth - 1], gain[depth]) < 0) break;

            Square nextFrom = NO_SQUARE;
            moved = getLVA(board, allattackers, side, occ, nextFrom);

            if (moved == EMPTY) break;

            // Reveal hidden sliding pieces along diagonal/orthogonal rays (X-rays)
            allattackers |= (attacks.getBishopAttack(to, occ) & diagSliders);
            allattackers |= (attacks.getRookAttack(to, occ) & orthSliders);
            allattackers &= occ;

            side = (side == WHITE ? BLACK : WHITE);
        }

        // Step 3: Minimax back-propagation
        while (--depth > 0) {
            gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);
        }

        return gain[0];
    }

    /**
     * Fast threshold check: returns true if SEE(move) >= threshold.
     */
    inline bool seeGe(const Board& board, const Move& move, int threshold = 0) {
        return evaluate(board, move) >= threshold;
    }
}