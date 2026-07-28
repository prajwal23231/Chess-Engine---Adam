#include <iostream>
#include <cassert>
#include <iomanip>
#include <vector>
#include <random>
#include <string>
#include "attack/magic.h"
#include "utils/bitboard_utilities.h"
#include "utils/type.h"

using namespace std;
using namespace Bitboard;

// =====================================================
// TEST STATISTICS & ASSERTION HELPERS
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
        cout << "MAGIC BITBOARD TEST SUMMARY\n";
        cout << string(50, '=') << "\n";
        cout << "Total:  " << total_tests << "\n";
        cout << "Passed: " << passed_tests << "\n";
        cout << "Failed: " << failed_tests << "\n";
        cout << string(50, '=') << "\n\n";

        if (failed_tests == 0) {
            cout << "[PASS] ALL MAGIC BITBOARD TESTS PASSED!\n\n";
        } else {
            cout << "[FAIL] SOME MAGIC BITBOARD TESTS FAILED!\n\n";
        }
    }
};

static TestStats stats;

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

void assertTrue(bool condition, const string& test_name) {
    stats.total_tests++;
    if (condition) {
        stats.passed_tests++;
        cout << "[PASS] " << test_name << "\n";
    } else {
        stats.failed_tests++;
        cout << "[FAIL] " << test_name << "\n";
    }
}

// Reference On-The-Fly (OTF) attack calculators for verification
static U64 getBishopAttackOTF(Square square, U64 occupancy) {
    U64 bishopAttack = 0;
    int cur_rank = square / 8;
    int cur_file = square % 8;
    constexpr int bishopMoves[4][2] = {{-1,-1}, {1,1}, {-1,1}, {1,-1}};

    for (int i = 0; i < 4; i++) {
        int r = cur_rank + bishopMoves[i][0];
        int f = cur_file + bishopMoves[i][1];
        while (r >= 0 && r < 8 && f >= 0 && f < 8) {
            Square pos = static_cast<Square>(r * 8 + f);
            setBit(bishopAttack, pos);
            if (occupancy & (1ULL << pos)) break;
            r += bishopMoves[i][0];
            f += bishopMoves[i][1];
        }
    }
    return bishopAttack;
}

static U64 getRookAttackOTF(Square square, U64 occupancy) {
    U64 rookAttack = 0;
    int cur_rank = square / 8;
    int cur_file = square % 8;
    constexpr int rookMoves[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};

    for (int i = 0; i < 4; i++) {
        int r = cur_rank + rookMoves[i][0];
        int f = cur_file + rookMoves[i][1];
        while (r >= 0 && r < 8 && f >= 0 && f < 8) {
            Square pos = static_cast<Square>(r * 8 + f);
            setBit(rookAttack, pos);
            if (occupancy & (1ULL << pos)) break;
            r += rookMoves[i][0];
            f += rookMoves[i][1];
        }
    }
    return rookAttack;
}

// =====================================================
// TEST SUITES
// =====================================================

void testMagicInitialization(const Magic& magic) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE 1: MAGIC INITIALIZATION & MASKS\n";
    cout << string(50, '=') << "\n\n";

    // Bishop mask relevant bit count check for key squares
    // Corners (A1, H1, A8, H8) -> 6 relevant bits
    assertBitCount(magic.getBishopMask(A1), 6, "Bishop A1 mask relevant bits count == 6");
    assertBitCount(magic.getBishopMask(H1), 6, "Bishop H1 mask relevant bits count == 6");
    assertBitCount(magic.getBishopMask(A8), 6, "Bishop A8 mask relevant bits count == 6");
    assertBitCount(magic.getBishopMask(H8), 6, "Bishop H8 mask relevant bits count == 6");

    // Center (E4, D4, E5, D5) -> 9 relevant bits
    assertBitCount(magic.getBishopMask(E4), 9, "Bishop E4 mask relevant bits count == 9");
    assertBitCount(magic.getBishopMask(D4), 9, "Bishop D4 mask relevant bits count == 9");

    // Edge (A4, H4, E1, E8) -> 5 relevant bits
    assertBitCount(magic.getBishopMask(A4), 5, "Bishop A4 mask relevant bits count == 5");
    assertBitCount(magic.getBishopMask(E1), 5, "Bishop E1 mask relevant bits count == 5");

    // Rook mask relevant bit count check for key squares
    // Corner (A1) -> 12 relevant bits
    assertBitCount(magic.getRookMask(A1), 12, "Rook A1 mask relevant bits count == 12");
    assertBitCount(magic.getRookMask(H8), 12, "Rook H8 mask relevant bits count == 12");

    // Center (E4) -> 10 relevant bits
    assertBitCount(magic.getRookMask(E4), 10, "Rook E4 mask relevant bits count == 10");

    // Edge (A4) -> 11 relevant bits
    assertBitCount(magic.getRookMask(A4), 11, "Rook A4 mask relevant bits count == 11");

    // Verify all 64 squares have non-zero masks
    bool all_masks_valid = true;
    for (int s = 0; s < 64; s++) {
        Square sq = static_cast<Square>(s);
        if (magic.getBishopMask(sq) == 0ULL || magic.getRookMask(sq) == 0ULL) {
            all_masks_valid = false;
            break;
        }
    }
    assertTrue(all_masks_valid, "All 64 squares have valid non-zero Bishop and Rook masks");
}

void testBishopAttacks(const Magic& magic) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE 2: BISHOP MAGIC ATTACKS\n";
    cout << string(50, '=') << "\n\n";

    // 1. Empty Board Tests
    U64 e4_bishop_empty = magic.getBishopAttack(E4, 0ULL);
    assertBitCount(e4_bishop_empty, 13, "Bishop E4 empty board attack count == 13");

    U64 a1_bishop_empty = magic.getBishopAttack(A1, 0ULL);
    assertBitCount(a1_bishop_empty, 7, "Bishop A1 empty board attack count == 7");

    // 2. Single Blocker
    U64 occ_f5 = (1ULL << F5);
    U64 e4_bishop_blocked = magic.getBishopAttack(E4, occ_f5);
    assertSquareInBitboard(e4_bishop_blocked, F5, "Bishop E4 attacks blocker at F5");
    assertSquareNotInBitboard(e4_bishop_blocked, G6, "Bishop E4 blocked beyond F5 (G6 absent)");
    assertSquareNotInBitboard(e4_bishop_blocked, H7, "Bishop E4 blocked beyond F5 (H7 absent)");
    assertSquareInBitboard(e4_bishop_blocked, D5, "Bishop E4 attacks unblocked D5");

    // 3. Multiple Blockers
    U64 occ_multi = (1ULL << F5) | (1ULL << D3);
    U64 e4_multi = magic.getBishopAttack(E4, occ_multi);
    assertSquareInBitboard(e4_multi, F5, "Bishop E4 attacks NE blocker at F5");
    assertSquareNotInBitboard(e4_multi, G6, "Bishop E4 blocked beyond F5");
    assertSquareInBitboard(e4_multi, D3, "Bishop E4 attacks SW blocker at D3");
    assertSquareNotInBitboard(e4_multi, C2, "Bishop E4 blocked beyond D3");

    // 4. Fully Occupied Board
    U64 e4_full = magic.getBishopAttack(E4, ~0ULL);
    assertBitCount(e4_full, 4, "Bishop E4 fully occupied board attack count == 4");
    assertSquareInBitboard(e4_full, F5, "Bishop E4 full board attacks adjacent F5");
    assertSquareInBitboard(e4_full, D5, "Bishop E4 full board attacks adjacent D5");
    assertSquareInBitboard(e4_full, D3, "Bishop E4 full board attacks adjacent D3");
    assertSquareInBitboard(e4_full, F3, "Bishop E4 full board attacks adjacent F3");

    U64 a1_full = magic.getBishopAttack(A1, ~0ULL);
    assertBitCount(a1_full, 1, "Bishop A1 fully occupied board attack count == 1 (B2 only)");
    assertSquareInBitboard(a1_full, B2, "Bishop A1 full board attacks B2");
}

void testRookAttacks(const Magic& magic) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE 3: ROOK MAGIC ATTACKS\n";
    cout << string(50, '=') << "\n\n";

    // 1. Empty Board (Rook on any square on an empty board must attack 14 squares)
    bool all_empty_14 = true;
    for (int s = 0; s < 64; s++) {
        Square sq = static_cast<Square>(s);
        if (popCount(magic.getRookAttack(sq, 0ULL)) != 14) {
            all_empty_14 = false;
            cout << "Rook on square " << s << " empty board count != 14\n";
            break;
        }
    }
    assertTrue(all_empty_14, "Rook on empty board attacks 14 squares across all 64 squares");

    // 2. Single Blocker
    U64 occ_e6 = (1ULL << E6);
    U64 e4_rook_blocked = magic.getRookAttack(E4, occ_e6);
    assertSquareInBitboard(e4_rook_blocked, E5, "Rook E4 attacks square E5 before blocker");
    assertSquareInBitboard(e4_rook_blocked, E6, "Rook E4 attacks blocker at E6");
    assertSquareNotInBitboard(e4_rook_blocked, E7, "Rook E4 blocked beyond E6 (E7 absent)");
    assertSquareNotInBitboard(e4_rook_blocked, E8, "Rook E4 blocked beyond E6 (E8 absent)");

    // 3. Blockers on All 4 Rays
    U64 occ_4way = (1ULL << E6) | (1ULL << E2) | (1ULL << G4) | (1ULL << C4);
    U64 e4_4way = magic.getRookAttack(E4, occ_4way);
    assertSquareInBitboard(e4_4way, E6, "Rook E4 attacks N blocker E6");
    assertSquareNotInBitboard(e4_4way, E7, "Rook E4 blocked beyond E6");
    assertSquareInBitboard(e4_4way, E2, "Rook E4 attacks S blocker E2");
    assertSquareNotInBitboard(e4_4way, E1, "Rook E4 blocked beyond E2");
    assertSquareInBitboard(e4_4way, G4, "Rook E4 attacks E blocker G4");
    assertSquareNotInBitboard(e4_4way, H4, "Rook E4 blocked beyond G4");
    assertSquareInBitboard(e4_4way, C4, "Rook E4 attacks W blocker C4");
    assertSquareNotInBitboard(e4_4way, B4, "Rook E4 blocked beyond C4");

    // 4. Fully Occupied Board
    U64 e4_full = magic.getRookAttack(E4, ~0ULL);
    assertBitCount(e4_full, 4, "Rook E4 fully occupied board attack count == 4");
    assertSquareInBitboard(e4_full, E5, "Rook E4 full board attacks E5");
    assertSquareInBitboard(e4_full, E3, "Rook E4 full board attacks E3");
    assertSquareInBitboard(e4_full, F4, "Rook E4 full board attacks F4");
    assertSquareInBitboard(e4_full, D4, "Rook E4 full board attacks D4");

    U64 a1_full = magic.getRookAttack(A1, ~0ULL);
    assertBitCount(a1_full, 2, "Rook A1 fully occupied board attack count == 2 (A2, B1)");
    assertSquareInBitboard(a1_full, A2, "Rook A1 full board attacks A2");
    assertSquareInBitboard(a1_full, B1, "Rook A1 full board attacks B1");
}

void testMaskInvariance(const Magic& magic) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE 4: MASK INVARIANCE & HASHING\n";
    cout << string(50, '=') << "\n\n";

    // Flipping occupancy bits outside a piece's mask MUST NOT alter the returned attack table!
    mt19937_64 rng(1337);
    bool bishop_invariant = true;
    bool rook_invariant = true;

    for (int s = 0; s < 64; s++) {
        Square sq = static_cast<Square>(s);
        U64 bMask = magic.getBishopMask(sq);
        U64 rMask = magic.getRookMask(sq);

        for (int i = 0; i < 50; i++) {
            U64 random_occ = rng();
            U64 masked_b_occ = random_occ & bMask;
            U64 masked_r_occ = random_occ & rMask;

            // Attacks with raw random occupancy vs masked occupancy must be identical
            if (magic.getBishopAttack(sq, random_occ) != magic.getBishopAttack(sq, masked_b_occ)) {
                bishop_invariant = false;
                cout << "Bishop mask invariance failure at square " << s << "\n";
                break;
            }

            if (magic.getRookAttack(sq, random_occ) != magic.getRookAttack(sq, masked_r_occ)) {
                rook_invariant = false;
                cout << "Rook mask invariance failure at square " << s << "\n";
                break;
            }
        }
    }

    assertTrue(bishop_invariant, "Bishop attack lookup is invariant to occupancy bits outside mask");
    assertTrue(rook_invariant, "Rook attack lookup is invariant to occupancy bits outside mask");
}

void testExhaustiveOTFComparison(const Magic& magic) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE 5: EXHAUSTIVE COMPARISON vs REFERENCE OTF\n";
    cout << string(50, '=') << "\n\n";

    mt19937_64 rng(42);
    bool bishop_matches = true;
    bool rook_matches = true;

    // Test 1000 random occupancy bitboards across all 64 squares
    const int NUM_SAMPLES = 1000;
    for (int s = 0; s < 64; s++) {
        Square sq = static_cast<Square>(s);

        // Always check edge occupancies: 0ULL and ~0ULL
        if (magic.getBishopAttack(sq, 0ULL) != getBishopAttackOTF(sq, 0ULL) ||
            magic.getBishopAttack(sq, ~0ULL) != getBishopAttackOTF(sq, ~0ULL)) {
            bishop_matches = false;
        }

        if (magic.getRookAttack(sq, 0ULL) != getRookAttackOTF(sq, 0ULL) ||
            magic.getRookAttack(sq, ~0ULL) != getRookAttackOTF(sq, ~0ULL)) {
            rook_matches = false;
        }

        for (int i = 0; i < NUM_SAMPLES; i++) {
            U64 occ = rng();
            if (magic.getBishopAttack(sq, occ) != getBishopAttackOTF(sq, occ)) {
                bishop_matches = false;
                cout << "Bishop OTF mismatch on square " << s << "\n";
                break;
            }
            if (magic.getRookAttack(sq, occ) != getRookAttackOTF(sq, occ)) {
                rook_matches = false;
                cout << "Rook OTF mismatch on square " << s << "\n";
                break;
            }
        }
    }

    assertTrue(bishop_matches, "Bishop magic attack equals OTF reference for 64,000+ random occupancies");
    assertTrue(rook_matches, "Rook magic attack equals OTF reference for 64,000+ random occupancies");
}

void testSelfAttackAndSymmetry(const Magic& magic) {
    cout << "\n" << string(50, '=') << "\n";
    cout << "TEST SUITE 6: SELF-ATTACK & UNBLOCKED SYMMETRY\n";
    cout << string(50, '=') << "\n\n";

    bool no_self_attack = true;
    bool unblocked_symmetry = true;

    mt19937_64 rng(999);

    for (int s = 0; s < 64; s++) {
        Square sq = static_cast<Square>(s);

        for (int i = 0; i < 20; i++) {
            U64 occ = rng();
            U64 bAttack = magic.getBishopAttack(sq, occ);
            U64 rAttack = magic.getRookAttack(sq, occ);

            if (getBit(bAttack, sq) || getBit(rAttack, sq)) {
                no_self_attack = false;
                cout << "Self-attack detected on square " << s << "\n";
                break;
            }
        }

        // Symmetry test on unblocked board:
        // If sq1 attacks sq2 on empty board, then sq2 must attack sq1 on empty board
        U64 bEmpty = magic.getBishopAttack(sq, 0ULL);
        U64 rEmpty = magic.getRookAttack(sq, 0ULL);

        for (int s2 = 0; s2 < 64; s2++) {
            Square sq2 = static_cast<Square>(s2);
            if (getBit(bEmpty, sq2)) {
                if (!getBit(magic.getBishopAttack(sq2, 0ULL), sq)) {
                    unblocked_symmetry = false;
                    cout << "Bishop symmetry failed between " << s << " and " << s2 << "\n";
                }
            }
            if (getBit(rEmpty, sq2)) {
                if (!getBit(magic.getRookAttack(sq2, 0ULL), sq)) {
                    unblocked_symmetry = false;
                    cout << "Rook symmetry failed between " << s << " and " << s2 << "\n";
                }
            }
        }
    }

    assertTrue(no_self_attack, "No sliding piece attacks its own square under any occupancy");
    assertTrue(unblocked_symmetry, "Unblocked sliding attack reciprocity (sq1 <-> sq2) verified across all square pairs");
}

// =====================================================
// MAIN TEST RUNNER
// =====================================================

int main() {
    cout << "\n";
    cout << "============================================================\n";
    cout << "          COMPREHENSIVE MAGIC BITBOARD TEST SUITE           \n";
    cout << "============================================================\n";

    stats.reset();

    cout << "Instantiating Magic object (triggers internal table generation & validate())...\n";
    Magic magic;
    cout << "Magic initialization complete.\n";

    testMagicInitialization(magic);
    testBishopAttacks(magic);
    testRookAttacks(magic);
    testMaskInvariance(magic);
    testExhaustiveOTFComparison(magic);
    testSelfAttackAndSymmetry(magic);

    stats.report();

    return stats.failed_tests == 0 ? 0 : 1;
}
