#include "board/board.h"
#include "attack/attacks.h"
#include "move.h"
#include "moveGenerator.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std;

static int testsRun = 0;
static int testsFailed = 0;

#define CHECK(cond, msg) do { \
    testsRun++; \
    if(!(cond)) { \
        cout << "FAIL: " << msg << " (line " << __LINE__ << ")\n"; \
        testsFailed++; \
    } \
} while(0)

// Helper: count moves matching a predicate
template<typename Pred>
int countIf(const vector<Move>& moves, Pred pred) {
    int c = 0;
    for (const auto& m : moves) if (pred(m)) c++;
    return c;
}

// Helper: does a move with this from/to/flag exist?
bool hasMove(const vector<Move>& moves, Square from, Square to, MoveFlag flag) {
    for (const auto& m : moves) {
        if (m.getFrom() == from && m.getTo() == to && m.getMoveFlag() == flag) return true;
    }
    return false;
}

bool hasMoveTo(const vector<Move>& moves, Square from, Square to) {
    for (const auto& m : moves) {
        if (m.getFrom() == from && m.getTo() == to) return true;
    }
    return false;
}

void loadOrFail(Board& b, const string& fen, const string& label) {
    bool ok = b.loadFEN(fen);
    CHECK(ok, label + ": FEN failed to load (\"" + fen + "\")");
}

int main() {
    Attacks attacks;

    // ============================================================
    // 1. STARTING POSITION - classic sanity check (20 legal moves)
    // ============================================================
    {
        Board board;
        board.setStartingPosition();
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        CHECK(moves.size() == 20, "starting position: expected 20 pseudo-legal moves, got " + to_string(moves.size()));

        int pawnMoves = countIf(moves, [](const Move& m){ return m.getMovedPiece() == WP; });
        int knightMoves = countIf(moves, [](const Move& m){ return m.getMovedPiece() == WN; });
        CHECK(pawnMoves == 16, "starting position: expected 16 pawn moves, got " + to_string(pawnMoves));
        CHECK(knightMoves == 4, "starting position: expected 4 knight moves, got " + to_string(knightMoves));

        // every pawn double-push should be flagged doublePawnPush
        int doublePush = countIf(moves, [](const Move& m){ return m.getMoveFlag() == doublePawnPush; });
        CHECK(doublePush == 8, "starting position: expected 8 double pawn pushes, got " + to_string(doublePush));

        // no captures should exist in the opening position
        int captures = countIf(moves, [](const Move& m){ return m.getMoveFlag() == capture; });
        CHECK(captures == 0, "starting position: expected 0 captures, got " + to_string(captures));
    }

    // ============================================================
    // 2. ISOLATED KNIGHT IN OPEN FIELD - 8 attack squares
    // ============================================================
    {
        Board board;
        loadOrFail(board, "8/8/3k4/8/3N4/8/8/3K4 w - - 0 1", "isolated knight");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        int knightMoves = countIf(moves, [](const Move& m){ return m.getMovedPiece() == WN; });
        int kingMoves = countIf(moves, [](const Move& m){ return m.getMovedPiece() == WK; });

        CHECK(knightMoves == 8, "central knight: expected 8 moves, got " + to_string(knightMoves));
        CHECK(kingMoves == 5, "d1 king (edge rank): expected 5 moves, got " + to_string(kingMoves));
        CHECK(moves.size() == 13, "isolated knight position: expected 13 total moves, got " + to_string(moves.size()));
    }

    // ============================================================
    // 3. KNIGHT IN THE CORNER - only 2 attack squares
    // ============================================================
    {
        Board board;
        loadOrFail(board, "3k4/8/8/8/8/8/8/N6K w - - 0 1", "corner knight");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        int knightMoves = countIf(moves, [](const Move& m){ return m.getMovedPiece() == WN; });
        CHECK(knightMoves == 2, "corner knight (a1): expected 2 moves, got " + to_string(knightMoves));
        CHECK(hasMoveTo(moves, A1, B3), "corner knight: A1->B3 should exist");
        CHECK(hasMoveTo(moves, A1, C2), "corner knight: A1->C2 should exist");
    }

    // ============================================================
    // 4. EN PASSANT CAPTURE
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "en passant setup");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        CHECK(moves.size() == 7, "en passant position: expected 7 total moves, got " + to_string(moves.size()));

        bool foundEp = false;
        for (const auto& m : moves) {
            if (m.getMoveFlag() == enPassant) {
                foundEp = true;
                CHECK(m.getFrom() == E5, "en passant: from should be E5");
                CHECK(m.getTo() == D6, "en passant: to should be D6");
                CHECK(m.getMovedPiece() == WP, "en passant: moved piece should be WP");
                CHECK(m.getCapturedPiece() == BP, "en passant: captured piece should be BP (the passed pawn)");
            }
        }
        CHECK(foundEp, "en passant: no move flagged 'enPassant' was generated");

        // f6 is empty and NOT the ep square -> must NOT generate a capture there
        CHECK(!hasMoveTo(moves, E5, F6), "en passant: E5->F6 should not exist (empty, non-ep square)");
    }

    // ============================================================
    // 5. BLACK EN PASSANT (mirror case, catches sign/offset bugs)
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1", "black en passant setup");
        // black pawn just captured position: white played e2-e4, black pawn sits on d4 already;
        // actually let's construct: black to move, black pawn on d4, white pawn just double-pushed to e4, ep square e3
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        bool foundEp = false;
        for (const auto& m : moves) {
            if (m.getMoveFlag() == enPassant) {
                foundEp = true;
                CHECK(m.getFrom() == D4, "black en passant: from should be D4");
                CHECK(m.getTo() == E3, "black en passant: to should be E3");
                CHECK(m.getMovedPiece() == BP, "black en passant: moved piece should be BP");
                CHECK(m.getCapturedPiece() == WP, "black en passant: captured piece should be WP");
            }
        }
        CHECK(foundEp, "black en passant: no move flagged 'enPassant' was generated");
    }

    // ============================================================
    // 6. DOUBLE PAWN PUSH - blocked vs open
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/8/4p3/8/4P3/4K3 w - - 0 1", "blocked double push");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        // e2 pawn: single push to e3 legal (empty), double push to e4 blocked by black pawn
        CHECK(hasMoveTo(moves, E2, E3), "blocked double push: single push E2->E3 should exist");
        CHECK(!hasMoveTo(moves, E2, E4), "blocked double push: E2->E4 should NOT exist (e4 occupied)");
        int doublePush = countIf(moves, [](const Move& m){ return m.getMoveFlag() == doublePawnPush; });
        CHECK(doublePush == 0, "blocked double push: expected 0 double-push moves, got " + to_string(doublePush));
    }

    {
        // pawn NOT on starting rank should never get a doublePawnPush even if two squares ahead are empty
        Board board;
        loadOrFail(board, "4k3/8/8/8/8/4P3/8/4K3 w - - 0 1", "pawn not on start rank");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        int doublePush = countIf(moves, [](const Move& m){ return m.getMoveFlag() == doublePawnPush; });
        CHECK(doublePush == 0, "pawn on e3 (not start rank): expected 0 double pushes, got " + to_string(doublePush));
        CHECK(hasMoveTo(moves, E3, E4), "pawn on e3: single push E3->E4 should exist");
    }

    // ============================================================
    // 7. PROMOTION + CAPTURE-PROMOTION COMBINATION (the risky case)
    // ============================================================
    {
        Board board;
        loadOrFail(board, "r1b1k3/1P6/8/8/8/8/8/4K3 w - - 0 1", "promotion matrix");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        int promoCapture = countIf(moves, [](const Move& m){ return m.getMoveFlag() == promotion_capture; });
        int promoQuiet   = countIf(moves, [](const Move& m){ return m.getMoveFlag() == promotion; });

        CHECK(promoCapture == 8, "promotion matrix: expected 8 promotion_capture moves (4 to a8 + 4 to c8), got " + to_string(promoCapture));
        CHECK(promoQuiet == 4, "promotion matrix: expected 4 straight promotion moves (to b8), got " + to_string(promoQuiet));

        // check all 4 promo pieces appear for the straight push to b8
        map<Piece,int> b8Promos;
        for (const auto& m : moves) {
            if (m.getTo() == B8 && m.getMoveFlag() == promotion) b8Promos[m.getPromotion()]++;
        }
        CHECK(b8Promos[WQ] == 1, "promotion to b8: missing WQ option");
        CHECK(b8Promos[WR] == 1, "promotion to b8: missing WR option");
        CHECK(b8Promos[WB] == 1, "promotion to b8: missing WB option");
        CHECK(b8Promos[WN] == 1, "promotion to b8: missing WN option");

        // check captured piece is correct for each capture-promotion target
        int capturedRookPromos = countIf(moves, [](const Move& m){
            return m.getMoveFlag() == promotion_capture && m.getTo() == A8 && m.getCapturedPiece() == BR;
        });
        int capturedBishopPromos = countIf(moves, [](const Move& m){
            return m.getMoveFlag() == promotion_capture && m.getTo() == C8 && m.getCapturedPiece() == BB;
        });
        CHECK(capturedRookPromos == 4, "promotion-capture on a8: expected 4 moves capturing BR, got " + to_string(capturedRookPromos));
        CHECK(capturedBishopPromos == 4, "promotion-capture on c8: expected 4 moves capturing BB, got " + to_string(capturedBishopPromos));
    }

    // ============================================================
    // 8. CASTLING - all rights present, path clear
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", "castling both sides clear");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        CHECK(hasMove(moves, E1, G1, kingSideCastle), "castling: king-side castle move missing");
        CHECK(hasMove(moves, E1, C1, queenSideCastle), "castling: queen-side castle move missing");

        int kingMoves = countIf(moves, [](const Move& m){ return m.getMovedPiece() == WK; });
        CHECK(kingMoves == 7, "castling clear: expected 7 total king moves (5 normal + 2 castle), got " + to_string(kingMoves));
    }

    // ============================================================
    // 9. CASTLING - king-side blocked by own piece
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/8/8/8/8/R3KB1R w KQ - 0 1", "castling kingside blocked");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        CHECK(!hasMove(moves, E1, G1, kingSideCastle), "castling blocked: king-side castle should NOT be generated (f1 occupied)");
        CHECK(hasMove(moves, E1, C1, queenSideCastle), "castling blocked: queen-side castle should still be generated");
    }

    // ============================================================
    // 10. CASTLING - no rights at all
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/8/8/8/8/R3K2R w - - 0 1", "castling no rights");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        int castleMoves = countIf(moves, [](const Move& m){ return m.getMoveFlag() == kingSideCastle || m.getMoveFlag() == queenSideCastle; });
        CHECK(castleMoves == 0, "castling rights absent: expected 0 castle moves, got " + to_string(castleMoves));
    }

    // ============================================================
    // 11. CASTLING - queen-side needs 3 empty squares (b1,c1,d1)
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/8/8/8/8/RN2K2R w KQ - 0 1", "castling queenside blocked by knight on b1");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        CHECK(!hasMove(moves, E1, C1, queenSideCastle), "queen-side castle blocked by piece on b1 should not be generated");
        CHECK(hasMove(moves, E1, G1, kingSideCastle), "king-side castle should still be available");
    }

    // ============================================================
    // 12. SLIDING PIECE BLOCKED BY OWN PIECE VS ENEMY PIECE
    // ============================================================
    {
        Board board;
        // white rook on d4, own pawn on d6 (blocks and should not be capturable),
        // enemy pawn on d2 (blocks but IS capturable)
        loadOrFail(board, "4k3/8/3P4/8/3R4/8/3p4/4K3 w - - 0 1", "rook blocked both directions");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        // rook should NOT be able to land on or pass through d6 (own piece)
        CHECK(!hasMoveTo(moves, D4, D6), "rook: should not capture own piece on d6");
        CHECK(!hasMoveTo(moves, D4, D7), "rook: should not see past own blocker on d6");
        CHECK(hasMoveTo(moves, D4, D5), "rook: should be able to move to empty d5");

        // rook SHOULD be able to capture enemy pawn on d2, but not go past it
        CHECK(hasMove(moves, D4, D2, capture), "rook: should be able to capture enemy pawn on d2");
        CHECK(!hasMoveTo(moves, D4, D1), "rook: should not see past enemy blocker on d2");
    }

    // ============================================================
    // 13. QUEEN COMBINES BISHOP + ROOK RAYS CORRECTLY
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/8/3Q4/8/8/4K3 w - - 0 1", "isolated queen open board");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        int queenMoves = countIf(moves, [](const Move& m){ return m.getMovedPiece() == WQ; });
        // A queen on d4 on an otherwise empty board reaches exactly 27 squares
        // (13 rook-direction + 14 bishop-direction, standard reference value)
        CHECK(queenMoves == 27, "queen on d4 open board: expected 27 moves, got " + to_string(queenMoves));
    }

    // ============================================================
    // 14. NO PIECE OF A GIVEN TYPE -> ZERO MOVES FOR THAT TYPE, NO CRASH
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/8/8/8/8/4K3 w - - 0 1", "kings only");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        CHECK(moves.size() == 5, "kings-only position: expected 5 moves for lone white king, got " + to_string(moves.size()));
    }

    // ============================================================
    // 15. SIDE TO MOVE RESPECTED - only generates for side to move
    // ============================================================
    {
        Board board;
        loadOrFail(board, "4k3/8/8/8/8/8/8/4K3 b - - 0 1", "black to move kings only");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        CHECK(moves.size() == 5, "black to move: expected 5 moves for lone black king, got " + to_string(moves.size()));
        for (const auto& m : moves) {
            CHECK(m.getMovedPiece() == BK, "black to move: all generated moves should move BK, found " + to_string(m.getMovedPiece()));
        }
    }

    // ============================================================
    // 16. KNOWN-ISSUE CHECKS: isCapture()/isPromotion() vs promotion_capture/enPassant flags
    //     These reflect the semantics implied by MoveGenerator's own usage of MoveFlag,
    //     NOT necessarily current Move:: behavior. If these fail, see explanation below.
    // ============================================================
    {
        Board board;
        loadOrFail(board, "r1b1k3/1P6/8/8/8/8/8/4K3 w - - 0 1", "known-issue promo capture check");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        Move* promoCap = nullptr;
        for (auto& m : moves) {
            if (m.getMoveFlag() == promotion_capture) { promoCap = &m; break; }
        }
        CHECK(promoCap != nullptr, "known-issue setup: expected at least one promotion_capture move to exist");
        if (promoCap) {
            CHECK(promoCap->isCapture(),   "KNOWN ISSUE: isCapture() returns false for a promotion_capture move (captures a real piece)");
            CHECK(promoCap->isPromotion(), "KNOWN ISSUE: isPromotion() returns false for a promotion_capture move (does promote)");
        }
    }
    {
        Board board;
        loadOrFail(board, "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "known-issue en passant check");
        MoveGenerator gen(board, attacks);
        vector<Move> moves;
        gen.generateMoves(moves);

        Move* ep = nullptr;
        for (auto& m : moves) {
            if (m.getMoveFlag() == enPassant) { ep = &m; break; }
        }
        CHECK(ep != nullptr, "known-issue setup: expected an enPassant move to exist");
        if (ep) {
            CHECK(ep->isCapture(), "KNOWN ISSUE: isCapture() returns false for an en passant move (it does capture a pawn)");
        }
    }

    // ============================================================
    // SUMMARY
    // ============================================================
    cout << "\n" << (testsRun - testsFailed) << "/" << testsRun << " checks passed.\n";
    if (testsFailed == 0) {
        cout << "ALL TESTS PASSED\n";
        return 0;
    } else {
        cout << testsFailed << " TEST(S) FAILED\n";
        return 1;
    }
}