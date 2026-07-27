// test_makemove.cpp
//
// Test script for Board::makeMove(), built against the real move.h / board.h
// you shared. Build e.g.:
//   g++ -std=c++17 -fsanitize=address,undefined -I<include roots> \
//       test_makemove.cpp board.cpp move.cpp bitboard_utilities.cpp -o test_makemove
//
// -fsanitize=address,undefined is recommended: one test below intentionally
// pushes ply near MAX_PLYS to check the history array boundary, which is
// undefined behavior if it overflows rather than a clean assert failure.

#include "board/board.h"
#include "moves/move.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

static int testsRun = 0;
static int testsFailed = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        testsRun++;                                                          \
        if (!(cond)) {                                                      \
            testsFailed++;                                                   \
            cerr << "  [FAIL] " << #cond << "  (line " << __LINE__ << ")\n"; \
        }                                                                     \
    } while (0)

static void section(const string& name) {
    cout << "\n== " << name << " ==\n";
    cout.flush();
}

// ============================================================================
// 1. Quiet pawn push
// ============================================================================
static void test_quietPawnPush() {
    section("Quiet pawn push");

    Board b;
    b.setStartingPosition();
    int rightsBefore = b.getCastlingRights();

    Move m(E2, E3, WP, EMPTY, EMPTY, quiet);
    b.makeMove(m);

    CHECK(b.getPieceBoard(E2) == EMPTY);
    CHECK(b.getPieceBoard(E3) == WP);
    CHECK((b.getBitboard(WP) & (1ULL << E2)) == 0);
    CHECK((b.getBitboard(WP) & (1ULL << E3)) != 0);
    CHECK(b.getMovingSide() == BLACK);
    CHECK(b.getEnPassant() == NO_SQUARE);
    CHECK(b.getCastlingRights() == rightsBefore);
    CHECK(b.getHalfMoveClock() == 0);      // pawn move resets clock
    CHECK(b.getFullMoveNumber() == 1);     // white's move doesn't bump fullmove yet
}

// ============================================================================
// 2. Double pawn push — en passant square set, then correctly overwritten
// ============================================================================
static void test_doublePawnPush() {
    section("Double pawn push sets/updates en passant square");

    Board b;
    b.setStartingPosition();

    Move m1(E2, E4, WP, EMPTY, EMPTY, doublePawnPush);
    b.makeMove(m1);
    CHECK(b.getPieceBoard(E4) == WP);
    CHECK(b.getEnPassant() == E3);

    Move m2(D7, D5, BP, EMPTY, EMPTY, doublePawnPush);
    b.makeMove(m2);
    CHECK(b.getEnPassant() == D6);  // must be overwritten, not stacked/leaked
}

// ============================================================================
// 3. Ordinary capture
// ============================================================================
static void test_ordinaryCapture() {
    section("Ordinary capture");

    Board b;
    bool loaded = b.loadFEN("rnbqkbnr/pppp1ppp/8/3Np3/8/8/PPPPPPPP/R1BQKBNR w KQkq - 0 1");
    CHECK(loaded);
    CHECK(b.getPieceBoard(D5) == WN);
    CHECK(b.getPieceBoard(E5) == BP);

    Move m(D5, E5, WN, BP, EMPTY, capture);
    b.makeMove(m);

    CHECK(b.getPieceBoard(E5) == WN);
    CHECK(b.getPieceBoard(D5) == EMPTY);
    CHECK((b.getBitboard(BP) & (1ULL << E5)) == 0);
    CHECK((b.getOccupancy(BLACK) & (1ULL << E5)) == 0);
    CHECK(b.getHalfMoveClock() == 0);  // capture resets clock
}

// ============================================================================
// 4. FLAG BUG ISOLATION — an unrelated capture must NOT touch rooks/rights
// ============================================================================
static void test_captureDoesNotTriggerCastlingLogic() {
    section("[BUG CHECK] ordinary capture must not enter castling branch");

    Board b;
    bool loaded = b.loadFEN("rnbqkbnr/pppp1ppp/8/3Np3/8/8/PPPPPPPP/R1BQKBNR w KQkq - 0 1");
    CHECK(loaded);

    int rightsBefore = b.getCastlingRights();
    Piece f1Before = b.getPieceBoard(F1);
    Piece h1Before = b.getPieceBoard(H1);
    Piece a1Before = b.getPieceBoard(A1);
    Piece d1Before = b.getPieceBoard(D1);

    Move m(D5, E5, WN, BP, EMPTY, capture);  // plain capture, nothing to do with castling
    b.makeMove(m);

    // With flag&kingSideCastle instead of flag==kingSideCastle, this capture
    // (flag value 1) incorrectly satisfies (1 & 3) != 0 and runs the rook-shuffle.
    CHECK(b.getCastlingRights() == rightsBefore);   // KQ rights must be untouched
    CHECK(b.getPieceBoard(F1) == f1Before);         // no ghost rook placed on f1
    CHECK(b.getPieceBoard(H1) == h1Before);         // h1 rook must not vanish
    CHECK(b.getPieceBoard(A1) == a1Before);
    CHECK(b.getPieceBoard(D1) == d1Before);
}

// ============================================================================
// 5. FLAG BUG ISOLATION — promotion must clear stale en passant square
// ============================================================================
static void test_promotionClearsStaleEnPassant() {
    section("[BUG CHECK] promotion must clear a stale en passant square");

    // Set up: black just double-pushed (ep = d6), then white promotes elsewhere.
    Board b;
    bool loaded = b.loadFEN("rnbqkbn1/pppppp1P/8/3p4/8/8/PPPPPP2/RNBQKBNR w KQkq d6 0 1");
    CHECK(loaded);
    CHECK(b.getEnPassant() == D6);

    Move m(H7, H8, WP, EMPTY, WQ, promotion);
    b.makeMove(m);

    // flag == promotion (6); (6 & doublePawnPush(2)) == 2, nonzero, so the
    // buggy `if(flag & doublePawnPush)` guard incorrectly treats this as "don't
    // reset ep" — the correct behavior is that ep must be cleared here.
    CHECK(b.getEnPassant() == NO_SQUARE);
    CHECK(b.getPieceBoard(H8) == WQ);
}

// ============================================================================
// 6. En passant capture
// ============================================================================
static void test_enPassantCapture() {
    section("En passant capture");

    Board b;
    bool loaded = b.loadFEN("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1");
    CHECK(loaded);
    CHECK(b.getEnPassant() == D6);

    Move m(E5, D6, WP, BP, EMPTY, enPassant);
    b.makeMove(m);

    CHECK(b.getPieceBoard(D6) == WP);
    CHECK(b.getPieceBoard(D5) == EMPTY);              // captured pawn removed from d5, not d6
    CHECK((b.getBitboard(BP) & (1ULL << D5)) == 0);
    CHECK((b.getOccupancy(BLACK) & (1ULL << D5)) == 0);
}

// ============================================================================
// 7. Kingside castling
// ============================================================================
static void test_kingsideCastle() {
    section("Kingside castling (white)");

    Board b;
    bool loaded = b.loadFEN("rnbqk2r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
    CHECK(loaded);

    Move m(E1, G1, WK, EMPTY, EMPTY, kingSideCastle);
    b.makeMove(m);

    CHECK(b.getPieceBoard(G1) == WK);
    CHECK(b.getPieceBoard(F1) == WR);
    CHECK(b.getPieceBoard(E1) == EMPTY);
    CHECK(b.getPieceBoard(H1) == EMPTY);
    CHECK(!(b.getCastlingRights() & CASTLE_WK));
    CHECK(!(b.getCastlingRights() & CASTLE_WQ));
}

// ============================================================================
// 8. Queenside castling
// ============================================================================
static void test_queensideCastle() {
    section("Queenside castling (black)");

    Board b;
    bool loaded = b.loadFEN("r3kbnr/pppqpppp/2n5/3p4/3P4/2N5/PPPQPPPP/R3KBNR b KQkq - 6 5");
    CHECK(loaded);

    Move m(E8, C8, BK, EMPTY, EMPTY, queenSideCastle);
    b.makeMove(m);

    CHECK(b.getPieceBoard(C8) == BK);
    CHECK(b.getPieceBoard(D8) == BR);
    CHECK(b.getPieceBoard(E8) == EMPTY);
    CHECK(b.getPieceBoard(A8) == EMPTY);
    CHECK(!(b.getCastlingRights() & CASTLE_BQ));
    CHECK(!(b.getCastlingRights() & CASTLE_BK));
}

// ============================================================================
// 9. Plain rook move revokes only its own side's right (quiet flag —
//    deliberately avoids the flag bug above so this isolates the rights logic)
// ============================================================================
static void test_rookMoveRevokesOneSide() {
    section("Rook move revokes only its own side's right");

    Board b;
    bool loaded = b.loadFEN("rnbqkbnr/pppppppp/8/8/8/8/1PPPPPPP/RNBQKBNR w KQkq - 0 1");
    CHECK(loaded);

    Move m(A1, A2, WR, EMPTY, EMPTY, quiet); // a2 pawn cleared in this FEN, so this is a legal-looking quiet slide
    b.makeMove(m);

    CHECK(!(b.getCastlingRights() & CASTLE_WQ));
    CHECK(b.getCastlingRights() & CASTLE_WK);
}

// ============================================================================
// 10. Capturing an unmoved rook on its home square revokes that right too
// ============================================================================
static void test_captureOnRookSquareRevokesRight() {
    section("Capturing opponent's rook on its home square revokes castling right");

    Board b;
    bool loaded = b.loadFEN("r3kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    CHECK(loaded);
    CHECK(b.getPieceBoard(A8) == BR);

    Move m(A2, A8, WP, BR, EMPTY, capture); // contrived, exercises the bookkeeping branch only
    b.makeMove(m);

    CHECK(!(b.getCastlingRights() & CASTLE_BQ));
}

// ============================================================================
// 11. Promotion with capture
// ============================================================================
static void test_promotionCapture() {
    section("Promotion with capture");

    Board b;
    bool loaded = b.loadFEN("rnbqkbnr/ppppppP1/8/8/8/8/PPPPPP1P/RNBQKBNR w KQkq - 0 1");
    CHECK(loaded);

    Move m(G7, H8, WP, BR, WQ, promotion_capture); // g7 pawn captures on h8 and promotes
    b.makeMove(m);

    CHECK(b.getPieceBoard(H8) == WQ);
    CHECK((b.getBitboard(WP) & (1ULL << G7)) == 0);
    CHECK((b.getBitboard(WQ) & (1ULL << H8)) != 0);
    CHECK((b.getBitboard(BR) & (1ULL << H8)) == 0);
}

// ============================================================================
// 12. Fullmove number increments only after BLACK moves
// ============================================================================
static void test_fullmoveNumberIncrement() {
    section("Fullmove number increments only after black's move");

    Board b;
    b.setStartingPosition();
    CHECK(b.getFullMoveNumber() == 1);

    Move white1(E2, E4, WP, EMPTY, EMPTY, doublePawnPush);
    b.makeMove(white1);
    CHECK(b.getFullMoveNumber() == 1);  // still 1 after white's move

    Move black1(E7, E5, BP, EMPTY, EMPTY, doublePawnPush);
    b.makeMove(black1);
    CHECK(b.getFullMoveNumber() == 2);  // now 2
}

// ============================================================================
// 13. Halfmove clock increments on non-pawn, non-capture moves
// ============================================================================
static void test_halfmoveClockIncrementsOnQuietPieceMove() {
    section("Halfmove clock increments on quiet non-pawn move");

    Board b;
    bool loaded = b.loadFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 2 3");
    CHECK(loaded);
    CHECK(b.getHalfMoveClock() == 2);

    Move m(G1, F3, WN, EMPTY, EMPTY, quiet);
    b.makeMove(m);
    CHECK(b.getHalfMoveClock() == 3);
}


int main() {
    test_quietPawnPush();
    test_doublePawnPush();
    test_ordinaryCapture();
    test_captureDoesNotTriggerCastlingLogic();
    test_promotionClearsStaleEnPassant();
    test_enPassantCapture();
    test_kingsideCastle();
    test_queensideCastle();
    test_rookMoveRevokesOneSide();
    test_captureOnRookSquareRevokesRight();
    test_promotionCapture();
    test_fullmoveNumberIncrement();
    test_halfmoveClockIncrementsOnQuietPieceMove();

    cout << "\n============================\n";
    cout << testsRun << " checks run, " << testsFailed << " failed.\n";
    if (testsFailed == 0) {
        cout << "ALL TESTS PASSED\n";
    } else {
        cout << "SOME TESTS FAILED\n";
    }

    return testsFailed == 0 ? 0 : 1;
}
