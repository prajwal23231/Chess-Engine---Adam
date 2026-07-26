// test_move_all.cpp
#include "moves/move.h"
#include <iostream>
#include <vector>
#include <string>

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

void testRoundTrip(Square from, Square to, Piece moved, Piece captured,
                    Piece promo, MoveFlag flag, const string& label) {
    Move m(from, to, moved, captured, promo, flag);
    CHECK(m.getFrom() == from,        label + ": from mismatch");
    CHECK(m.getTo() == to,            label + ": to mismatch");
    CHECK(m.getMovedPiece() == moved, label + ": movedPiece mismatch");
    CHECK(m.getCapturedPiece() == captured, label + ": capturedPiece mismatch");
    CHECK(m.getPromotion() == promo,  label + ": promotion mismatch");
    CHECK(m.getMoveFlag() == flag,    label + ": flag mismatch");
}

int main() {
    // ===================== BASIC TESTS =====================

    testRoundTrip(E2, E4, WP, EMPTY, EMPTY, doublePawnPush, "double pawn push");
    testRoundTrip(A1, H8, WR, EMPTY, EMPTY, quiet, "corner-to-corner quiet");
    testRoundTrip(H8, A1, BQ, WP, EMPTY, quiet, "corner-to-corner capture");

    Piece allPieces[] = { WP,WN,WR,WB,WQ,WK, BP,BN,BR,BB,BQ,BK };
    for (Piece p : allPieces) testRoundTrip(D2, D4, p, EMPTY, EMPTY, quiet, "moved-piece sweep");
    for (Piece p : allPieces) testRoundTrip(D2, D4, WP, p, EMPTY, quiet, "captured-piece sweep");

    Piece promoPieces[] = { EMPTY, WN, WB, WR, WQ, BN, BB, BR, BQ };
    for (Piece p : promoPieces) testRoundTrip(A7, A8, WP, EMPTY, p, quiet, "promotion sweep");

    MoveFlag flags[] = { quiet, doublePawnPush, kingSideCastle, queenSideCastle, enPassant };
    for (MoveFlag f : flags) testRoundTrip(E1, G1, WK, EMPTY, EMPTY, f, "flag sweep");

    {
        Move quietMove(E2, E4, WP, EMPTY, EMPTY, doublePawnPush);
        CHECK(quietMove.isCapture() == false, "quiet move should not be capture");

        Move captureMove(E4, D5, WP, BP, EMPTY, quiet);
        CHECK(captureMove.isCapture() == true, "pawn capture should be capture");
    }

    {
        Move noPromo(A7, A8, WP, EMPTY, EMPTY, quiet);
        CHECK(noPromo.isPromotion() == false, "no promo field should not be promotion");

        Move withPromo(A7, A8, WP, EMPTY, WQ, quiet);
        CHECK(withPromo.isPromotion() == true, "promo field set should be promotion");
    }

    {
        Move capPromo(B7, A8, WP, BR, WQ, quiet);
        CHECK(capPromo.isCapture() == true, "capture+promo: isCapture true");
        CHECK(capPromo.isPromotion() == true, "capture+promo: isPromotion true");
        CHECK(capPromo.getCapturedPiece() == BR, "capture+promo: captured piece correct");
        CHECK(capPromo.getPromotion() == WQ, "capture+promo: promo piece correct");
    }

    {
        Move ksCastle(E1, G1, WK, EMPTY, EMPTY, kingSideCastle);
        CHECK(ksCastle.isCastle() == true, "king-side castle detected");
        CHECK(ksCastle.isEnPassant() == false, "king-side castle is not en passant");

        Move qsCastle(E1, C1, WK, EMPTY, EMPTY, queenSideCastle);
        CHECK(qsCastle.isCastle() == true, "queen-side castle detected");

        Move ep(D5, E6, WP, BP, EMPTY, enPassant);
        CHECK(ep.isEnPassant() == true, "en passant detected");
        CHECK(ep.isCastle() == false, "en passant is not castle");
        CHECK(ep.isCapture() == true, "en passant counts as capture");
    }

    {
        Move base(A2, A4, WP, EMPTY, EMPTY, doublePawnPush);
        Move onlyDiffersInCaptured(A2, A4, WP, BN, EMPTY, doublePawnPush);

        CHECK(base.getFrom() == onlyDiffersInCaptured.getFrom(), "captured field bled into 'from'");
        CHECK(base.getTo() == onlyDiffersInCaptured.getTo(), "captured field bled into 'to'");
        CHECK(base.getMovedPiece() == onlyDiffersInCaptured.getMovedPiece(), "captured field bled into 'moved'");
        CHECK(base.getMoveFlag() == onlyDiffersInCaptured.getMoveFlag(), "captured field bled into 'flag'");
        CHECK(base.getPromotion() == onlyDiffersInCaptured.getPromotion(), "captured field bled into 'promo'");
    }

    {
        Move m1(C2, C4, WP, EMPTY, EMPTY, doublePawnPush);
        Move m2(C2, C4, WP, EMPTY, EMPTY, doublePawnPush);
        CHECK(m1.getValue() == m2.getValue(), "identical moves should have identical raw encoding");
    }

    // ===================== EXTENDED TESTS =====================

    {
        bool allPassed = true;
        for (int f = A1; f <= H8; f++) {
            for (int t = A1; t <= H8; t++) {
                Move m(static_cast<Square>(f), static_cast<Square>(t), WP, EMPTY, EMPTY, quiet);
                if (m.getFrom() != f || m.getTo() != t) {
                    allPassed = false;
                    cout << "FAIL: square sweep mismatch at from=" << f << " to=" << t << "\n";
                }
            }
        }
        testsRun++;
        if (!allPassed) testsFailed++;
        CHECK(allPassed, "full 64x64 from/to sweep");
    }

    Piece blackPromoPieces[] = { BN, BB, BR, BQ };
    for (Piece p : blackPromoPieces) testRoundTrip(D2, D1, BP, EMPTY, p, quiet, "black promotion sweep");

    {
        Move blackEp(D4, C3, BP, WP, EMPTY, enPassant);
        CHECK(blackEp.isEnPassant() == true, "black en passant flag detected");
        CHECK(blackEp.isCapture() == true, "black en passant is a capture");
        CHECK(blackEp.getCapturedPiece() == WP, "black en passant captured piece is WP");
        CHECK(blackEp.getMovedPiece() == BP, "black en passant moved piece is BP");
    }

    {
        Move whiteKS(E1, G1, WK, EMPTY, EMPTY, kingSideCastle);
        Move whiteQS(E1, C1, WK, EMPTY, EMPTY, queenSideCastle);
        Move blackKS(E8, G8, BK, EMPTY, EMPTY, kingSideCastle);
        Move blackQS(E8, C8, BK, EMPTY, EMPTY, queenSideCastle);

        CHECK(whiteKS.isCastle() && whiteKS.getMovedPiece() == WK, "white king-side castle");
        CHECK(whiteQS.isCastle() && whiteQS.getTo() == C1, "white queen-side castle");
        CHECK(blackKS.isCastle() && blackKS.getMovedPiece() == BK, "black king-side castle");
        CHECK(blackQS.isCastle() && blackQS.getTo() == C8, "black queen-side castle");
        CHECK(!whiteKS.isCapture() && !whiteKS.isEnPassant(), "white KS castle is not capture/ep");
        CHECK(!blackQS.isCapture() && !blackQS.isEnPassant(), "black QS castle is not capture/ep");
    }

    {
        Piece whitePromos[] = { WN, WB, WR, WQ };
        Piece blackCaptured[] = { BN, BB, BR, BQ };
        for (int i = 0; i < 4; i++) {
            Move m(B7, A8, WP, blackCaptured[i], whitePromos[i], quiet);
            CHECK(m.getPromotion() == whitePromos[i], "white capture-promo: promo field");
            CHECK(m.getCapturedPiece() == blackCaptured[i], "white capture-promo: captured field");
            CHECK(m.isCapture() && m.isPromotion(), "white capture-promo: both flags true");
        }

        Piece blackPromos[] = { BN, BB, BR, BQ };
        Piece whiteCaptured[] = { WN, WB, WR, WQ };
        for (int i = 0; i < 4; i++) {
            Move m(B2, A1, BP, whiteCaptured[i], blackPromos[i], quiet);
            CHECK(m.getPromotion() == blackPromos[i], "black capture-promo: promo field");
            CHECK(m.getCapturedPiece() == whiteCaptured[i], "black capture-promo: captured field");
            CHECK(m.isCapture() && m.isPromotion(), "black capture-promo: both flags true");
        }
    }

    {
        vector<Move> moveList;
        moveList.push_back(Move(E2, E4, WP, EMPTY, EMPTY, doublePawnPush));
        moveList.push_back(Move(E7, E5, BP, EMPTY, EMPTY, doublePawnPush));
        moveList.push_back(Move(G1, F3, WN, EMPTY, EMPTY, quiet));
        moveList.push_back(Move(B8, C6, BN, EMPTY, EMPTY, quiet));
        moveList.push_back(Move(F1, C4, WB, EMPTY, EMPTY, quiet));
        moveList.push_back(Move(F8, C5, BB, EMPTY, EMPTY, quiet));
        moveList.push_back(Move(E1, G1, WK, EMPTY, EMPTY, kingSideCastle));

        struct Expected { Square from, to; Piece moved; MoveFlag flag; };
        Expected expected[] = {
            {E2, E4, WP, doublePawnPush}, {E7, E5, BP, doublePawnPush},
            {G1, F3, WN, quiet}, {B8, C6, BN, quiet},
            {F1, C4, WB, quiet}, {F8, C5, BB, quiet},
            {E1, G1, WK, kingSideCastle},
        };

        CHECK(moveList.size() == 7, "move list size after simulated game opening");
        for (size_t i = 0; i < moveList.size(); i++) {
            CHECK(moveList[i].getFrom() == expected[i].from, "game seq: from at index " + to_string(i));
            CHECK(moveList[i].getTo() == expected[i].to, "game seq: to at index " + to_string(i));
            CHECK(moveList[i].getMovedPiece() == expected[i].moved, "game seq: moved at index " + to_string(i));
            CHECK(moveList[i].getMoveFlag() == expected[i].flag, "game seq: flag at index " + to_string(i));
        }
    }

    {
        Move a(E2, E4, WP, EMPTY, EMPTY, doublePawnPush);
        Move b(E2, E4, WP, EMPTY, EMPTY, doublePawnPush);
        Move c(E2, E4, WP, EMPTY, EMPTY, quiet);
        CHECK(a.getValue() == b.getValue(), "identical moves compare equal via getValue");
        CHECK(a.getValue() != c.getValue(), "moves differing only by flag compare unequal");
    }

    {
        Move original(D7, D8, WP, BN, WQ, quiet);
        Move copy = original;
        CHECK(copy.getValue() == original.getValue(), "copy constructor preserves raw value");
        CHECK(copy.getFrom() == original.getFrom(), "copy preserves from");
        CHECK(copy.getPromotion() == original.getPromotion(), "copy preserves promotion");
        CHECK(copy.getCapturedPiece() == original.getCapturedPiece(), "copy preserves captured");
    }

    {
        bool allCorrect = true;
        for (Piece p : allPieces) {
            Move m(A2, A3, p, EMPTY, EMPTY, quiet);
            if (m.isCapture() || m.isPromotion() || m.isCastle() || m.isEnPassant()) {
                allCorrect = false;
                cout << "FAIL: quiet move with piece " << p << " reported a special flag\n";
            }
        }
        testsRun++;
        if (!allCorrect) testsFailed++;
        CHECK(allCorrect, "quiet moves never spuriously flagged as special");
    }

    testRoundTrip(E4, E5, WK, EMPTY, EMPTY, quiet, "king one-square move");
    testRoundTrip(E4, D5, WK, BP, EMPTY, quiet, "king diagonal capture");

    // ===================== SUMMARY =====================
    cout << "\n" << (testsRun - testsFailed) << "/" << testsRun << " checks passed.\n";
    if (testsFailed == 0) {
        cout << "ALL TESTS PASSED\n";
        return 0;
    } else {
        cout << testsFailed << " TEST(S) FAILED\n";
        return 1;
    }
}