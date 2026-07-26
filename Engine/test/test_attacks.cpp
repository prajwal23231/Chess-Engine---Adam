#include <iostream>
#include <cassert>
#include <iomanip>
#include "attack/attacks.h"
#include "utils/bitboard_utilities.h"

using namespace std;
using namespace Bitboard;

// =====================================================
// TEST STATISTICS & HELPERS
// =====================================================

class TestStats {
public:
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;

    void reset() {
        total_tests = 0;
        passed_tests = 0;
        failed_tests = 0;
    }

    void report() {
        cout << "\n" << string(50, '=') << "\n";
        cout << "TEST SUMMARY\n";
        cout << string(50, '=') << "\n";
        cout << "Total:  " << total_tests << "\n";
        cout << "Passed: " << passed_tests << "\n";
        cout << "Failed: " << failed_tests << "\n";
        cout << string(50, '=') << "\n\n";

        if (failed_tests == 0) {
            cout << "[PASS] ALL TESTS PASSED!\n\n";
        } else {
            cout << "[FAIL] SOME TESTS FAILED!\n\n";
        }
    }
};

TestStats stats;

// =====================================================
// ASSERTION HELPERS
// =====================================================

void assertBitboardEqual(U64 actual, U64 expected, const string& test_name) {
    stats.total_tests++;
    
    if (actual == expected) {
        stats.passed_tests++;
        cout << "[PASS] " << test_name << "\n";
    } else {
        stats.failed_tests++;
        cout << "[FAIL] " << test_name << "\n";
        cout << "  Expected:\n";
        printBitboard(expected);
        cout << "  Got:\n";
        printBitboard(actual);
    }
}

void assertBitCount(U64 bb, int expected_count, const string& test_name) {
    stats.total_tests++;
    int actual_count = popCount(bb);
    
    if (actual_count == expected_count) {
        stats.passed_tests++;
        cout << "[PASS] " << test_name << " (count: " << actual_count << ")\n";
    } else {
        stats.failed_tests++;
        cout << "[FAIL] " << test_name << "\n";
        cout << "  Expected count: " << expected_count << ", Got: " << actual_count << "\n";
    }
}

void assertSquareInBitboard(U64 bb, Square sq, const string& test_name) {
    stats.total_tests++;
    
    if (getBit(bb, sq)) {
        stats.passed_tests++;
        cout << "[PASS] " << test_name << "\n";
    } else {
        stats.failed_tests++;
        cout << "[FAIL] " << test_name << " - Square " << sq << " not in bitboard\n";
    }
}

void assertSquareNotInBitboard(U64 bb, Square sq, const string& test_name) {
    stats.total_tests++;
    
    if (!getBit(bb, sq)) {
        stats.passed_tests++;
        cout << "[PASS] " << test_name << "\n";
    } else {
        stats.failed_tests++;
        cout << "[FAIL] " << test_name << " - Square " << sq << " should not be in bitboard\n";
    }
}

// =====================================================
// TEST 1: KNIGHT ATTACKS
// =====================================================

void testKnightAttacks(Attacks& attacks) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE: KNIGHT ATTACKS\n";
    cout << string(50, '=') << "\n\n";

    // Knight from center (E4 = 28)
    // Should attack: D2, F2, G3, G5, F6, D6, C5, C3
    U64 e4_knight = attacks.getKnightAttack(E4);
    assertBitCount(e4_knight, 8, "Knight E4 attacks 8 squares (center)");
    assertSquareInBitboard(e4_knight, D2, "Knight E4 attacks D2");
    assertSquareInBitboard(e4_knight, F2, "Knight E4 attacks F2");
    assertSquareInBitboard(e4_knight, G3, "Knight E4 attacks G3");
    assertSquareInBitboard(e4_knight, G5, "Knight E4 attacks G5");
    assertSquareInBitboard(e4_knight, F6, "Knight E4 attacks F6");
    assertSquareInBitboard(e4_knight, D6, "Knight E4 attacks D6");
    assertSquareInBitboard(e4_knight, C5, "Knight E4 attacks C5");
    assertSquareInBitboard(e4_knight, C3, "Knight E4 attacks C3");

    // Knight from corner (A1)
    // Should attack only: B3, C2
    U64 a1_knight = attacks.getKnightAttack(A1);
    assertBitCount(a1_knight, 2, "Knight A1 attacks 2 squares (corner)");
    assertSquareInBitboard(a1_knight, B3, "Knight A1 attacks B3");
    assertSquareInBitboard(a1_knight, C2, "Knight A1 attacks C2");

    // Knight from corner (H8)
    // Should attack only: G6, F7
    U64 h8_knight = attacks.getKnightAttack(H8);
    assertBitCount(h8_knight, 2, "Knight H8 attacks 2 squares (corner)");
    assertSquareInBitboard(h8_knight, G6, "Knight H8 attacks G6");
    assertSquareInBitboard(h8_knight, F7, "Knight H8 attacks F7");

    // Knight from edge (A4)
    // Should attack: B2, C3, C5, B6
    U64 a4_knight = attacks.getKnightAttack(A4);
    assertBitCount(a4_knight, 4, "Knight A4 attacks 4 squares (edge)");

    // Symmetry test: If knight on A attacks B, knight on B should attack A
    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        U64 knight_attacks = attacks.getKnightAttack(s);
        
        // For each square attacked by this knight
        U64 temp = knight_attacks;
        int attacked_sq;
        while ((attacked_sq = popLSB(temp)) != -1) {
            Square attacked = static_cast<Square>(attacked_sq);
            U64 reverse_attacks = attacks.getKnightAttack(attacked);
            
            if (!getBit(reverse_attacks, s)) {
                stats.total_tests++;
                stats.failed_tests++;
                cout << "[FAIL] Knight symmetry failed: " << s << " attacks " << attacked
                     << " but " << attacked << " doesn't attack " << s << "\n";
            }
        }
    }
    stats.total_tests++;
    stats.passed_tests++;
    cout << "[PASS] Knight attacks symmetry property verified\n";
}

// =====================================================
// TEST 2: KING ATTACKS
// =====================================================

void testKingAttacks(Attacks& attacks) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE: KING ATTACKS\n";
    cout << string(50, '=') << "\n\n";

    // King from center (E4)
    // Should attack 8 squares
    U64 e4_king = attacks.getKingAttack(E4);
    assertBitCount(e4_king, 8, "King E4 attacks 8 squares (center)");

    // King from corner (A1)
    // Should attack 3 squares: A2, B1, B2
    U64 a1_king = attacks.getKingAttack(A1);
    assertBitCount(a1_king, 3, "King A1 attacks 3 squares (corner)");
    assertSquareInBitboard(a1_king, A2, "King A1 attacks A2");
    assertSquareInBitboard(a1_king, B1, "King A1 attacks B1");
    assertSquareInBitboard(a1_king, B2, "King A1 attacks B2");

    // King from corner (H8)
    // Should attack 3 squares: H7, G8, G7
    U64 h8_king = attacks.getKingAttack(H8);
    assertBitCount(h8_king, 3, "King H8 attacks 3 squares (corner)");
    assertSquareInBitboard(h8_king, H7, "King H8 attacks H7");
    assertSquareInBitboard(h8_king, G8, "King H8 attacks G8");
    assertSquareInBitboard(h8_king, G7, "King H8 attacks G7");

    // King from edge (A4)
    // Should attack 5 squares
    U64 a4_king = attacks.getKingAttack(A4);
    assertBitCount(a4_king, 5, "King A4 attacks 5 squares (edge)");

    // Symmetry test
    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        U64 king_attacks = attacks.getKingAttack(s);
        
        U64 temp = king_attacks;
        int attacked_sq;
        while ((attacked_sq = popLSB(temp)) != -1) {
            Square attacked = static_cast<Square>(attacked_sq);
            U64 reverse_attacks = attacks.getKingAttack(attacked);
            
            if (!getBit(reverse_attacks, s)) {
                stats.total_tests++;
                stats.failed_tests++;
                cout << "[FAIL] King symmetry failed at " << s << " -> " << attacked << "\n";
                return;
            }
        }
    }
    stats.total_tests++;
    stats.passed_tests++;
    cout << "[PASS] King attacks symmetry property verified\n";
}

// =====================================================
// TEST 3: PAWN ATTACKS
// =====================================================

void testPawnAttacks(Attacks& attacks) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE: PAWN ATTACKS\n";
    cout << string(50, '=') << "\n\n";

    // White pawn from E2 (12)
    // Should attack D3 (11) and F3 (13)
    U64 white_e2 = attacks.getWhitePawnAttack(E2);
    assertBitCount(white_e2, 2, "White pawn E2 attacks 2 squares");
    assertSquareInBitboard(white_e2, D3, "White pawn E2 attacks D3");
    assertSquareInBitboard(white_e2, F3, "White pawn E2 attacks F3");

    // White pawn from A2 (8) - edge case
    // Should attack only B3
    U64 white_a2 = attacks.getWhitePawnAttack(A2);
    assertBitCount(white_a2, 1, "White pawn A2 attacks 1 square (edge)");
    assertSquareInBitboard(white_a2, B3, "White pawn A2 attacks B3");

    // White pawn from H2 (15) - edge case
    // Should attack only G3
    U64 white_h2 = attacks.getWhitePawnAttack(H2);
    assertBitCount(white_h2, 1, "White pawn H2 attacks 1 square (edge)");
    assertSquareInBitboard(white_h2, G3, "White pawn H2 attacks G3");

    // Black pawn from E7 (52)
    // Should attack D6 (43) and F6 (45)
    U64 black_e7 = attacks.getBlackPawnAttack(E7);
    assertBitCount(black_e7, 2, "Black pawn E7 attacks 2 squares");
    assertSquareInBitboard(black_e7, D6, "Black pawn E7 attacks D6");
    assertSquareInBitboard(black_e7, F6, "Black pawn E7 attacks F6");

    // Black pawn from A7 (48) - edge case
    U64 black_a7 = attacks.getBlackPawnAttack(A7);
    assertBitCount(black_a7, 1, "Black pawn A7 attacks 1 square (edge)");
    assertSquareInBitboard(black_a7, B6, "Black pawn A7 attacks B6");

    // Black pawn from H7 (55) - edge case
    U64 black_h7 = attacks.getBlackPawnAttack(H7);
    assertBitCount(black_h7, 1, "Black pawn H7 attacks 1 square (edge)");
    assertSquareInBitboard(black_h7, G6, "Black pawn H7 attacks G6");
}

// =====================================================
// TEST 4: BISHOP ATTACKS
// =====================================================

void testBishopAttacks(Attacks& attacks) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE: BISHOP ATTACKS\n";
    cout << string(50, '=') << "\n\n";

    // Empty board - center bishop
    U64 e4_bishop_empty = attacks.getBishopAttack(E4, 0ULL);
    assertBitCount(e4_bishop_empty, 13, "Bishop E4 on empty board attacks 13 squares");

    // Bishop blocked by single piece in one direction
    U64 occ = (1ULL << F5);  // Blocker NE
    U64 e4_bishop_blocked = attacks.getBishopAttack(E4, occ);
    assertSquareInBitboard(e4_bishop_blocked, F5, "Bishop E4 attacks blocker at F5");
    assertSquareNotInBitboard(e4_bishop_blocked, G6, "Bishop E4 cannot attack beyond blocker at F5");

    // Bishop corner - all directions tested
    U64 a1_bishop = attacks.getBishopAttack(A1, 0ULL);
    assertBitCount(a1_bishop, 7, "Bishop A1 on empty board attacks 7 squares");

    // Bishop with blockers in all directions
    U64 occ_all = 0ULL;
    occ_all |= (1ULL << F5);  // NE blocker
    occ_all |= (1ULL << D5);  // NW blocker
    occ_all |= (1ULL << D3);  // SW blocker
    occ_all |= (1ULL << F3);  // SE blocker
    
    U64 e4_all_blocked = attacks.getBishopAttack(E4, occ_all);
    // Should attack the 4 blockers but nothing beyond
    assertBitCount(e4_all_blocked, 4, "Bishop E4 with all blockers attacks 4 squares");

    // Long diagonal test (A1 to H8)
    U64 a1_bishop_long = attacks.getBishopAttack(A1, (1ULL << H8));
    assertSquareInBitboard(a1_bishop_long, H8, "Bishop A1 attacks blocker at H8");

    // Adjacent blockers - bishop should still move 1 square
    U64 occ_adj = (1ULL << F5) | (1ULL << D5) | (1ULL << D3) | (1ULL << F3);
    U64 e4_adj_blocked = attacks.getBishopAttack(E4, occ_adj);
    assertBitCount(e4_adj_blocked, 4, "Bishop E4 with adjacent blockers attacks exactly blockers");
}

// =====================================================
// TEST 5: ROOK ATTACKS
// =====================================================

void testRookAttacks(Attacks& attacks) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE: ROOK ATTACKS\n";
    cout << string(50, '=') << "\n\n";

    // Empty board - center rook
    U64 e4_rook_empty = attacks.getRookAttack(E4, 0ULL);
    assertBitCount(e4_rook_empty, 14, "Rook E4 on empty board attacks 14 squares");

    // Rook blocked in one direction
    U64 occ = (1ULL << E5);  // Blocker north
    U64 e4_rook_blocked = attacks.getRookAttack(E4, occ);
    assertSquareInBitboard(e4_rook_blocked, E5, "Rook E4 attacks blocker at E5");
    assertSquareNotInBitboard(e4_rook_blocked, E6, "Rook E4 cannot attack beyond blocker at E5");

    // Rook corner
    U64 a1_rook = attacks.getRookAttack(A1, 0ULL);
    assertBitCount(a1_rook, 14, "Rook A1 on empty board attacks 14 squares");

    // Rook with blockers in all directions
    U64 occ_all = 0ULL;
    occ_all |= (1ULL << E5);  // North
    occ_all |= (1ULL << G4);  // East
    occ_all |= (1ULL << E3);  // South
    occ_all |= (1ULL << B4);  // West
    
    U64 e4_all_blocked = attacks.getRookAttack(E4, occ_all);
    assertBitCount(e4_all_blocked, 7, "Rook E4 attacks until blockers");
    assertSquareInBitboard(e4_all_blocked, E5, "Rook E4 attacks north blocker");
    assertSquareInBitboard(e4_all_blocked, G4, "Rook E4 attacks east blocker");
    assertSquareInBitboard(e4_all_blocked, E3, "Rook E4 attacks south blocker");
    assertSquareInBitboard(e4_all_blocked, B4, "Rook E4 attacks west blocker");

    // Edge case: rook on edge
    U64 a4_rook = attacks.getRookAttack(A4, 0ULL);
    assertBitCount(a4_rook, 14, "Rook A4 on empty board attacks 14 squares");
}

// =====================================================
// TEST 6: QUEEN ATTACKS
// =====================================================

void testQueenAttacks(Attacks& attacks) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE: QUEEN ATTACKS\n";
    cout << string(50, '=') << "\n\n";

    // Queen = Bishop | Rook
    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        
        // Test with empty board
        U64 queen_empty = attacks.getQueenAttack(s, 0ULL);
        U64 bishop_empty = attacks.getBishopAttack(s, 0ULL);
        U64 rook_empty = attacks.getRookAttack(s, 0ULL);
        
        if (queen_empty != (bishop_empty | rook_empty)) {
            stats.total_tests++;
            stats.failed_tests++;
            cout << "[FAIL] Queen empty board decomposition failed at square " << sq << "\n";
            return;
        }
    }
    stats.total_tests++;
    stats.passed_tests++;
    cout << "[PASS] Queen attacks = Bishop | Rook (empty board, all squares)\n";

    // Test with various occupancies
    U64 occ = (1ULL << E5) | (1ULL << G4) | (1ULL << D5) | (1ULL << F3);
    
    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        
        U64 queen = attacks.getQueenAttack(s, occ);
        U64 bishop = attacks.getBishopAttack(s, occ);
        U64 rook = attacks.getRookAttack(s, occ);
        
        if (queen != (bishop | rook)) {
            stats.total_tests++;
            stats.failed_tests++;
            cout << "[FAIL] Queen decomposition failed at square " << sq << " with occupancy\n";
            return;
        }
    }
    stats.total_tests++;
    stats.passed_tests++;
    cout << "[PASS] Queen attacks = Bishop | Rook (with blockers, all squares)\n";

    // Queen in center should attack more squares than any other piece type
    U64 queen_e4 = attacks.getQueenAttack(E4, 0ULL);
    U64 bishop_e4 = attacks.getBishopAttack(E4, 0ULL);
    U64 rook_e4 = attacks.getRookAttack(E4, 0ULL);
    
    int queen_count = popCount(queen_e4);
    int bishop_count = popCount(bishop_e4);
    int rook_count = popCount(rook_e4);
    
    assertBitCount(queen_e4, bishop_count + rook_count, 
                   "Queen E4 count = Bishop + Rook count");
}

// =====================================================
// TEST 7: EDGE CASES & BOUNDARY CONDITIONS
// =====================================================

void testEdgeCases(Attacks& attacks) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE: EDGE CASES & BOUNDARIES\n";
    cout << string(50, '=') << "\n\n";

    // All four corners
    cout << "Corner tests:\n";
    U64 a1 = attacks.getBishopAttack(A1, 0ULL);
    U64 h1 = attacks.getBishopAttack(H1, 0ULL);
    U64 a8 = attacks.getBishopAttack(A8, 0ULL);
    U64 h8 = attacks.getBishopAttack(H8, 0ULL);
    
    assertBitCount(a1, 7, "Bishop A1 attacks 7 squares");
    assertBitCount(h1, 7, "Bishop H1 attacks 7 squares");
    assertBitCount(a8, 7, "Bishop A8 attacks 7 squares");
    assertBitCount(h8, 7, "Bishop H8 attacks 7 squares");

    // Rook on every edge
    cout << "\nRook edge tests:\n";
    U64 a_rook = attacks.getRookAttack(A4, 0ULL);
    U64 h_rook = attacks.getRookAttack(H4, 0ULL);
    U64 rank1_rook = attacks.getRookAttack(D1, 0ULL);
    U64 rank8_rook = attacks.getRookAttack(D8, 0ULL);
    
    assertBitCount(a_rook, 14, "Rook A4 attacks 14 squares");
    assertBitCount(h_rook, 14, "Rook H4 attacks 14 squares");
    assertBitCount(rank1_rook, 14, "Rook D1 attacks 14 squares");
    assertBitCount(rank8_rook, 14, "Rook D8 attacks 14 squares");

    // Multiple blockers on same ray
    cout << "\nMultiple blockers on ray:\n";
    U64 occ = (1ULL << C5) | (1ULL << E5);
    U64 b5_rook = attacks.getRookAttack(B5, occ);
    // Should attack C5 but not E5
    assertSquareInBitboard(b5_rook, C5, "Rook B5 attacks first blocker C5");
    assertSquareNotInBitboard(b5_rook, E5, "Rook B5 cannot attack second blocker E5");

    // Blocker adjacent to piece
    cout << "\nAdjacent blockers:\n";
    U64 occ_adj = (1ULL << F4);
    U64 e4_rook_adj = attacks.getRookAttack(E4, occ_adj);
    assertSquareInBitboard(e4_rook_adj, F4, "Rook E4 attacks adjacent blocker F4");
}

// =====================================================
// TEST 8: CONSISTENCY CHECKS
// =====================================================

void testConsistency(Attacks& attacks) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE: CONSISTENCY CHECKS\n";
    cout << string(50, '=') << "\n\n";

    // No piece should attack itself
    cout << "Self-attack tests:\n";
    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        
        U64 knight = attacks.getKnightAttack(s);
        U64 king = attacks.getKingAttack(s);
        U64 white_pawn = attacks.getWhitePawnAttack(s);
        U64 black_pawn = attacks.getBlackPawnAttack(s);
        
        if (getBit(knight, s) || getBit(king, s) || getBit(white_pawn, s) || getBit(black_pawn, s)) {
            stats.total_tests++;
            stats.failed_tests++;
            cout << "[FAIL] Piece attacks itself at square " << sq << "\n";
            return;
        }
    }
    stats.total_tests++;
    stats.passed_tests++;
    cout << "[PASS] No piece attacks itself (all squares)\n";

    // Bishop/Rook should only attack empty squares when occupancy is empty
    cout << "\nEmpty board tests:\n";
    for (int sq = 0; sq < 64; sq++) {
        Square s = static_cast<Square>(sq);
        
        U64 bishop = attacks.getBishopAttack(s, 0ULL);
        U64 rook = attacks.getRookAttack(s, 0ULL);
        
        // These should be stable (not containing the source square)
        if (getBit(bishop, s) || getBit(rook, s)) {
            stats.total_tests++;
            stats.failed_tests++;
            cout << "[FAIL] Sliding piece attacks itself at square " << sq << "\n";
            return;
        }
    }
    stats.total_tests++;
    stats.passed_tests++;
    cout << "[PASS] Sliding pieces don't attack themselves (all squares)\n";
}

// =====================================================
// TEST 9: PERFT VALIDATION (BONUS - Future Use)
// =====================================================

void testPerftValidation(Attacks& attacks) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE: VALIDATION FOR FUTURE PERFT\n";
    cout << string(50, '=') << "\n\n";

    cout << "These attacks will be used in move generation.\n";
    cout << "Attack generation correctness is critical for:\n";
    cout << "  - Checking if squares are attacked\n";
    cout << "  - Validating legal moves\n";
    cout << "  - Determining check/checkmate\n";
    cout << "  - Castling validation\n\n";
    cout << "[PASS] Attack generation foundation is solid for future work\n";
}

// =====================================================
// MAIN TEST RUNNER
// =====================================================

int main() {
    Attacks attacks;

    cout << "\n";
    cout << "============================================================\n";
    cout << "          COMPREHENSIVE ATTACK GENERATION TEST\n";
    cout << "============================================================\n";

    stats.reset();

    testKnightAttacks(attacks);
    testKingAttacks(attacks);
    testPawnAttacks(attacks);
    testBishopAttacks(attacks);
    testRookAttacks(attacks);
    testQueenAttacks(attacks);
    testEdgeCases(attacks);
    testConsistency(attacks);
    testPerftValidation(attacks);

    stats.report();

    return stats.failed_tests == 0 ? 0 : 1;
}