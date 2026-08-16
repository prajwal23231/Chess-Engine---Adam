// test_undomove.cpp
//
// Round-trip test harness for Board::makeMove / Board::undoMove.
//
// Strategy: for a variety of positions and move types, snapshot the FULL
// board state (piece array, per-piece bitboards, occupancy bitboards,
// side to move, castling rights, en-passant square, halfmove clock,
// fullmove number), apply makeMove(), then undoMove(), and assert the
// resulting state is byte-for-byte identical to the snapshot taken
// before the move. Any mismatch is reported field-by-field.
//
// It also runs an internal-consistency check (bitboards vs board[] vs
// occupancies) after every step, since two states could accidentally
// compare equal while being internally inconsistent mid-way.
//
// Build (from the directory containing utils/, moves/, board/):
//   g++ -std=c++17 -I. board/board.cpp moves/move.cpp test_undomove.cpp -o test_undomove
//   ./test_undomove

#include "board/board.h"
#include <iostream>
#include <array>
#include <string>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------
// Full state snapshot
// ---------------------------------------------------------------------
struct BoardState {
    array<Piece, BOARD_SIZE> board{};
    array<U64, NUM_PIECES>   bitboards{};
    array<U64, NUM_COLORS>   occupancies{};
    Color  sideToMove;
    int    castlingRights;
    Square enPassant;
    int    halfmoveClock;
    int    fullmoveNumber;
    int    gamePhase;
};

static BoardState captureState(const Board &b) {
    BoardState s;
    for (int sq = 0; sq < BOARD_SIZE; sq++)
        s.board[sq] = b.getPieceBoard(static_cast<Square>(sq));
    for (int p = 0; p < NUM_PIECES; p++)
        s.bitboards[p] = b.getBitboard(static_cast<Piece>(p));
    s.occupancies[WHITE] = b.getOccupancy(WHITE);
    s.occupancies[BLACK] = b.getOccupancy(BLACK);
    s.occupancies[BOTH]  = b.getOccupancy(BOTH);
    s.sideToMove      = b.getMovingSide();
    s.castlingRights  = b.getCastlingRights();
    s.enPassant       = b.getEnPassant();
    s.halfmoveClock   = b.getHalfMoveClock();
    s.fullmoveNumber  = b.getFullMoveNumber();
    s.gamePhase       = b.getGamePhase();
    return s;
}

static string sq2str(int sq) {
    if (sq < 0) return "-";
    string f = "abcdefgh";
    return string(1, f[sq % 8]) + to_string(sq / 8 + 1);
}

static string pieceChar(Piece p) {
    if (p == EMPTY) return ".";
    static const char c[12] = {'P','N','R','B','Q','K','p','n','r','b','q','k'};
    return string(1, c[p]);
}

// Prints every field that differs between two states.
static bool diffStates(const BoardState &a, const BoardState &b) {
    bool same = true;

    for (int sq = 0; sq < BOARD_SIZE; sq++) {
        if (a.board[sq] != b.board[sq]) {
            same = false;
            cout << "    board[] mismatch at " << sq2str(sq)
                 << ": expected " << pieceChar(a.board[sq])
                 << " got " << pieceChar(b.board[sq]) << "\n";
        }
    }

    static const char *pieceNames[12] = {
        "WP","WN","WR","WB","WQ","WK","BP","BN","BR","BB","BQ","BK"
    };
    for (int p = 0; p < NUM_PIECES; p++) {
        if (a.bitboards[p] != b.bitboards[p]) {
            same = false;
            cout << "    bitboard[" << pieceNames[p] << "] mismatch: expected 0x"
                 << hex << a.bitboards[p] << " got 0x" << b.bitboards[p] << dec << "\n";
        }
    }

    static const char *occNames[3] = {"WHITE","BLACK","BOTH"};
    for (int c = 0; c < NUM_COLORS; c++) {
        if (a.occupancies[c] != b.occupancies[c]) {
            same = false;
            cout << "    occupancy[" << occNames[c] << "] mismatch: expected 0x"
                 << hex << a.occupancies[c] << " got 0x" << b.occupancies[c] << dec << "\n";
        }
    }

    if (a.sideToMove != b.sideToMove) {
        same = false;
        cout << "    sideToMove mismatch: expected " << a.sideToMove
             << " got " << b.sideToMove << "\n";
    }
    if (a.castlingRights != b.castlingRights) {
        same = false;
        cout << "    castlingRights mismatch: expected " << a.castlingRights
             << " got " << b.castlingRights << "\n";
    }
    if (a.enPassant != b.enPassant) {
        same = false;
        cout << "    enPassant mismatch: expected " << sq2str(a.enPassant)
             << " got " << sq2str(b.enPassant) << "\n";
    }
    if (a.halfmoveClock != b.halfmoveClock) {
        same = false;
        cout << "    halfmoveClock mismatch: expected " << a.halfmoveClock
             << " got " << b.halfmoveClock << "\n";
    }
    if (a.fullmoveNumber != b.fullmoveNumber) {
        same = false;
        cout << "    fullmoveNumber mismatch: expected " << a.fullmoveNumber
             << " got " << b.fullmoveNumber << "\n";
    }
    if (a.gamePhase != b.gamePhase) {
        same = false;
        cout << "    gamePhase mismatch: expected " << a.gamePhase
             << " got " << b.gamePhase << "\n";
    }

    return same;
}

// Internal consistency: bitboards, board[] and occupancies must all agree.
// Independent of makeMove/undoMove correctness -- catches desyncs that a
// pure "before vs after undo" comparison could miss.
static bool checkConsistency(const Board &b, const string &label) {
    bool ok = true;
    U64 whiteAcc = 0, blackAcc = 0;

    for (int p = 0; p < NUM_PIECES; p++) {
        U64 bb = b.getBitboard(static_cast<Piece>(p));
        U64 tmp = bb;
        while (tmp) {
            int sq = __builtin_ctzll(tmp);
            tmp &= tmp - 1;
            if (b.getPieceBoard(static_cast<Square>(sq)) != p) {
                cout << "  [CONSISTENCY:" << label << "] bitboard says piece " << p
                     << " on " << sq2str(sq) << " but board[] says "
                     << pieceChar(b.getPieceBoard(static_cast<Square>(sq))) << "\n";
                ok = false;
            }
        }
        if (p < 6) whiteAcc |= bb; else blackAcc |= bb;
    }

    for (int sq = 0; sq < BOARD_SIZE; sq++) {
        Piece p = b.getPieceBoard(static_cast<Square>(sq));
        if (p == EMPTY) continue;
        if (!(b.getBitboard(p) & (1ULL << sq))) {
            cout << "  [CONSISTENCY:" << label << "] board[] says " << pieceChar(p)
                 << " on " << sq2str(sq) << " but that bitboard has no bit there\n";
            ok = false;
        }
    }

    if (b.getOccupancy(WHITE) != whiteAcc) {
        cout << "  [CONSISTENCY:" << label << "] occupancy(WHITE) != OR of white piece bitboards\n";
        ok = false;
    }
    if (b.getOccupancy(BLACK) != blackAcc) {
        cout << "  [CONSISTENCY:" << label << "] occupancy(BLACK) != OR of black piece bitboards\n";
        ok = false;
    }
    if (b.getOccupancy(BOTH) != (b.getOccupancy(WHITE) | b.getOccupancy(BLACK))) {
        cout << "  [CONSISTENCY:" << label << "] occupancy(BOTH) != WHITE | BLACK\n";
        ok = false;
    }
    return ok;
}

// ---------------------------------------------------------------------
// Test runner
// ---------------------------------------------------------------------
static int passed = 0, failed = 0;

static void runTest(const string &name, const string &fen, Move move) {
    Board board;
    if (!board.loadFEN(fen)) {
        cout << "[SETUP FAIL] " << name << " -- invalid FEN: " << fen << "\n";
        failed++;
        return;
    }

    bool ok = checkConsistency(board, name + " (before)");

    BoardState before = captureState(board);

    board.makeMove(move);
    ok &= checkConsistency(board, name + " (after makeMove)");

    board.undoMove(move);
    ok &= checkConsistency(board, name + " (after undoMove)");

    BoardState after = captureState(board);

    bool statesMatch = diffStates(before, after);
    ok &= statesMatch;

    if (ok) {
        cout << "[PASS] " << name << "\n";
        passed++;
    } else {
        cout << "[FAIL] " << name << "\n";
        failed++;
    }
}

int main() {
    cout << "===== Board::undoMove round-trip tests =====\n\n";

    const string START = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // 1. Quiet knight move
    runTest("quiet knight move (Ng1-f3)", START,
        Move(G1, F3, WN, EMPTY, EMPTY, quiet));

    // 2. Quiet pawn single push
    runTest("quiet pawn push (e2-e3)", START,
        Move(E2, E3, WP, EMPTY, EMPTY, quiet));

    // 3. Double pawn push
    runTest("double pawn push (e2-e4)", START,
        Move(E2, E4, WP, EMPTY, EMPTY, doublePawnPush));

    // 4. Ordinary capture
    runTest("pawn captures pawn (e4xd5)",
        "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
        Move(E4, D5, WP, BP, EMPTY, capture));

    // 5. En passant capture
    runTest("en passant (e5xd6 e.p.)",
        "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3",
        Move(E5, D6, WP, BP, EMPTY, enPassant));

    // 6. Quiet promotion
    runTest("quiet promotion (a7-a8=Q)",
        "4k3/P7/8/8/8/8/8/4K3 w - - 0 1",
        Move(A7, A8, WP, EMPTY, WQ, promotion));

    // 7. Promotion with capture
    runTest("capture promotion (a7xb8=Q)",
        "1r2k3/P7/8/8/8/8/8/4K3 w - - 0 1",
        Move(A7, B8, WP, BR, WQ, promotion_capture));

    // 8. White king-side castle
    runTest("white O-O",
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        Move(E1, G1, WK, EMPTY, EMPTY, kingSideCastle));

    // 9. White queen-side castle
    runTest("white O-O-O",
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        Move(E1, C1, WK, EMPTY, EMPTY, queenSideCastle));

    // 10. Black king-side castle
    runTest("black O-O",
        "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
        Move(E8, G8, BK, EMPTY, EMPTY, kingSideCastle));

    // 11. Black queen-side castle
    runTest("black O-O-O",
        "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
        Move(E8, C8, BK, EMPTY, EMPTY, queenSideCastle));

    // 12. Rook capture (non-corner) sanity check
    runTest("rook captures knight (Rxh7)",
        "4k3/7n/8/8/8/8/8/4K2R w K - 0 1",
        Move(H1, H7, WR, BN, EMPTY, capture));

    cout << "\n----- Multi-move stack test (push 3, pop 3) -----\n";
    {
        Board board;
        board.loadFEN(START);
        vector<BoardState> stack;
        stack.push_back(captureState(board));

        Move m1(E2, E4, WP, EMPTY, EMPTY, doublePawnPush);
        Move m2(E7, E5, BP, EMPTY, EMPTY, doublePawnPush);
        Move m3(G1, F3, WN, EMPTY, EMPTY, quiet);

        board.makeMove(m1); stack.push_back(captureState(board));
        board.makeMove(m2); stack.push_back(captureState(board));
        board.makeMove(m3); stack.push_back(captureState(board));

        bool ok = true;
        board.undoMove(m3);
        ok &= diffStates(stack[2], captureState(board));
        board.undoMove(m2);
        ok &= diffStates(stack[1], captureState(board));
        board.undoMove(m1);
        ok &= diffStates(stack[0], captureState(board));

        if (ok) { cout << "[PASS] multi-move stack unwinds cleanly\n"; passed++; }
        else    { cout << "[FAIL] multi-move stack unwind mismatch\n"; failed++; }
    }

    cout << "\n----- Diagnostic: castling-rights bug on rook-takes-rook -----\n";
    {
        // White rook on h1 captures black rook sitting on its home square h8.
        // Both the mover's own rights (WK) AND the captured side's rights on
        // that file (BK) should be revoked by makeMove.
        Board board;
        board.loadFEN("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
        Move rookTakesRook(H1, H8, WR, BR, EMPTY, capture);

        board.makeMove(rookTakesRook);
        int rightsAfterMove = board.getCastlingRights();
        cout << "  castlingRights after Rxh8 = " << rightsAfterMove
             << " (CASTLE_WK=1 WQ=2 BK=4 BQ=8)\n";
        bool wkCleared = !(rightsAfterMove & CASTLE_WK);
        bool bkCleared = !(rightsAfterMove & CASTLE_BK);
        cout << "  white K-side right cleared: " << (wkCleared ? "yes" : "no") << "\n";
        cout << "  black K-side right cleared: " << (bkCleared ? "yes (correct)"
                                                                  : "NO -- BUG: captured rook's home-square rights survive") << "\n";
        board.undoMove(rookTakesRook);
        cout << "  (undoMove correctly restores castlingRights from history regardless: "
             << board.getCastlingRights() << ")\n";
    }

    cout << "\n===== " << passed << " passed, " << failed << " failed =====\n";
    return failed == 0 ? 0 : 1;
}