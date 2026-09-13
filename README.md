# ADAM — Handcrafted C++ Chess Engine

[![C++20](https://img.shields.io/badge/Language-C%2B%2B20%20%2F%20C%2B%2B17-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![UCI Protocol](https://img.shields.io/badge/Protocol-UCI%20Compliant-brightgreen.svg)](https://en.wikipedia.org/wiki/Universal_Chess_Interface)
[![Throughput](https://img.shields.io/badge/Throughput-2.5M%2B%20NPS%20(Single%20Core)-orange.svg)](#perft-benchmark-suite)
[![Endgames](https://img.shields.io/badge/Endgames-Syzygy%205--Piece%20%2B%20KPK%20Bitbase-purple.svg)](#endgame-bitbases--tablebases)
[![GitHub Repository](https://img.shields.io/badge/GitHub-prajwal23231%2FChess--Engine----Adam-181717.svg?logo=github)](https://github.com/prajwal23231/Chess-Engine---Adam)

**ADAM** is a high-performance, tournament-grade, UCI-compliant chess engine designed and engineered from scratch in C++. Built from first principles for extreme speed, search depth, and positional/tactical balance, ADAM features a dual bitboard/mailbox board representation, Fancy Magic Bitboards, strictly legal move generation, a 256-step tapered evaluation system backed by a dedicated 16K-entry Pawn Hash Table, an in-memory 24K-entry retrograde KPK Bitbase, integrated Syzygy 3-4-5 piece endgame tablebases, and an advanced Alpha-Beta search engine combining Principal Variation Search (PVS), dynamic Null Move Pruning (NMP), Reverse Futility Pruning (RFP), Move-Loop Futility Pruning, Late Move Reductions (LMR), and an 11-tier move ordering hierarchy.

* **Repository**: [https://github.com/prajwal23231/Chess-Engine---Adam](https://github.com/prajwal23231/Chess-Engine---Adam)
* **Author**: Prajwal
* **Language**: C++20 / C++17
* **Protocol**: UCI (Universal Chess Interface)
* **Single-Core Throughput**: 2.0M to 2.5M+ Nodes Per Second (NPS)

---

## Key Milestone & Live Benchmark Showcase

### Defeating Stockfish Level 7 on Lichess (3-Minute Blitz — 96% Accuracy)
ADAM was deployed as an autonomous bot on Lichess and challenged against **Stockfish Level 7** in a real-time **3-minute Blitz** time control:

* **Result**: Convincing Victory against Stockfish Level 7
* **Move Accuracy**: **96% Accuracy** across the entire game with zero critical blunders
* **Time Management**: Flawless dynamic clock allocation across opening, middlegame tactical skirmishes, and endgame conversion
* **Conversion Precision**: Seamless transition from middlegame king-attack positional advantages into Syzygy-verified endgame tablebase checkmates

---

## System Architecture

```text
                                 +--------------------------------+
                                 |   UCI Interface & Protocol     |
                                 |    (uci.cpp / Time Manager)    |
                                 +---------------+----------------+
                                                 |
                                                 v
                                 +--------------------------------+
                                 |  Iterative Deepening Search    |
                                 |  - Principal Variation Search  |
                                 |  - Null Move Pruning (NMP)     |
                                 |  - Reverse Futility (RFP)      |
                                 |  - Late Move Reductions (LMR)  |
                                 |  - Quiescence & Delta Pruning  |
                                 +-------+----------------+-------+
                                         |                |
             +---------------------------+                +--------------------------+
             |                                                                       |
             v                                                                       v
+----------------------------+                                         +----------------------------+
| 11-Tier Move Ordering      |                                         | Dual Board State Machine   |
| - Hash Move (TT Probe)     |                                         | - 12 Piece Bitboards       |
| - Queen/Knight Promotions  |                                         | - 3 Occupancy Bitboards    |
| - MVV-LVA Captures         |                                         | - 8x8 Mailbox Array        |
| - 2 Killer Move Slots      |                                         | - Incremental Zobrist Keys |
| - Countermove & History    |                                         +--------------+-------------+
+----------------------------+                                                        |
             |                                                                        v
             v                                                         +----------------------------+
+----------------------------+                                         | Attack & Move Generation   |
| Transposition Table (TT)   |                                         | - Fancy Magic Bitboards    |
| - 24-byte aligned entries  |                                         | - Constant-time O(1) rays  |
| - Power-of-2 bit masking   |                                         | - Strictly Legal Movegen   |
| - Exact/Lower/Upper Bounds |                                         | - Pin & Check-Ray Masks    |
+----------------------------+                                         +--------------+-------------+
             |                                                                        |
             +---------------------------+--------------------------------------------+
                                         |
                                         v
                         +--------------------------------+
                         | Evaluation & Endgame Systems   |
                         | - 256-Step Tapered Evaluation  |
                         | - 16K Pawn Hash Table Cache    |
                         | - In-Memory Retrograde KPK     |
                         | - Syzygy 3-4-5 WDL/DTZ Probing |
                         +--------------------------------+
```

---

## Table of Contents

- [Key Milestone & Live Benchmark Showcase](#key-milestone--live-benchmark-showcase)
- [System Architecture](#system-architecture)
- [Directory Layout](#directory-layout)
- [Core Engine Architecture](#core-engine-architecture)
  - [Dual Board Representation & State Machine](#dual-board-representation--state-machine)
  - [Incremental State Updates & Byte-Level Undo Symmetry](#incremental-state-updates--byte-level-undo-symmetry)
  - [Zobrist Hashing & Dynamic Repetition Detection](#zobrist-hashing--dynamic-repetition-detection)
  - [Attack Tables & Fancy Magic Bitboards](#attack-tables--fancy-magic-bitboards)
  - [Strictly Legal Move Generation](#strictly-legal-move-generation)
  - [Compact 32-bit Move Encoding](#compact-32-bit-move-encoding)
- [Transposition Table (TT) Subsystem](#transposition-table-tt-subsystem)
  - [Memory Layout & Entry Structure](#memory-layout--entry-structure)
  - [Single-Cycle Bitwise Indexing & Collision Safety](#single-cycle-bitwise-indexing--collision-safety)
  - [Bound Flags & Replacement Scheme](#bound-flags--replacement-scheme)
  - [Root-Independent Mate Score Normalization](#root-independent-mate-score-normalization)
- [Endgame Bitbases & Tablebases](#endgame-bitbases--tablebases)
  - [In-Memory KPK Retrograde Bitbase (29 Passes)](#in-memory-kpk-retrograde-bitbase-29-passes)
  - [Syzygy 3-4-5 Piece Tablebases (WDL & DTZ)](#syzygy-3-4-5-piece-tablebases-wdl--dtz)
- [Evaluation System](#evaluation-system)
  - [Game Phase & 256-Step Tapered Evaluation](#game-phase--256-step-tapered-evaluation)
  - [Dedicated 16K-Entry Pawn Hash Table](#dedicated-16k-entry-pawn-hash-table)
  - [Pawn Structure, Outposts & Mobility](#pawn-structure-outposts--mobility)
  - [Dynamic Bishop Pair & Depletion Scaling](#dynamic-bishop-pair--depletion-scaling)
  - [King Safety & Quadratic Danger Zones](#king-safety--quadratic-danger-zones)
  - [Endgame King Cornering & Mop-Up Evaluation](#endgame-king-cornering--mop-up-evaluation)
- [Search Engine & Pruning Heuristics](#search-engine--pruning-heuristics)
  - [Principal Variation Search (PVS)](#principal-variation-search-pvs)
  - [Dynamic Null Move Pruning (NMP)](#dynamic-null-move-pruning-nmp)
  - [Reverse Futility Pruning (RFP)](#reverse-futility-pruning-rfp)
  - [Move-Loop Futility Pruning](#move-loop-futility-pruning)
  - [Late Move Reductions (LMR)](#late-move-reductions-lmr)
  - [11-Tier Move Ordering Hierarchy](#11-tier-move-ordering-hierarchy)
  - [Quiescence Search & Victim-Specific Delta Pruning](#quiescence-search--victim-specific-delta-pruning)
- [Universal Chess Interface (UCI) & Adaptive Clock Manager](#universal-chess-interface-uci--adaptive-clock-manager)
- [Opening Book & Lichess Bot Integration](#opening-book--lichess-bot-integration)
- [Automated Testing & Fastchess Match Runner](#automated-testing--fastchess-match-runner)
- [Perft Benchmark Suite](#perft-benchmark-suite)
- [How to Build and Run ADAM](#how-to-build-and-run-adam)
- [Roadmap & Future Directions](#roadmap--future-directions)
- [License](#license)

---

## Directory Layout

```text
Adam/
├── ADAM.exe                      # Optimized release binary (-O3 -march=native -flto)
├── ADAM_base.exe                 # Frozen reference binary for Fastchess regression testing
├── README.md                     # Comprehensive architecture and engine documentation
├── run_test.bat                  # Interactive Windows batch test runner (Quick, Standard, SPRT)
├── run_test.ps1                  # Interactive PowerShell test runner (Fastchess automation)
├── config.json                   # Fastchess tournament match configuration
├── test_results.pgn              # Output PGN match archive from automated engine runs
├── Engine/                       # Engine Source Code
│   ├── main.cpp                  # Program entry point (initializes Zobrist, Tools, KPK, Syzygy)
│   ├── attack/                   # Attack tables & Magic Bitboard subsystem
│   │   ├── attacks.h / .cpp      # Precomputed non-sliding (Pawn, Knight, King) attack tables
│   │   ├── magic.h / .cpp        # Fancy Magic Bitboard generator & constant-time sliding lookups
│   │   ├── magicGen.h / .cpp     # Monte Carlo magic candidate search algorithm
│   │   ├── magic_instance.h / .cpp # Global magic singleton instance
│   │   └── magicCreate.cpp       # Standalone generator utility for magic numbers
│   ├── board/                    # Board state representation & state machine
│   │   ├── board.h               # Board class declaration, bitboards, occupancies & inline queries
│   │   └── board.cpp             # Make/undo state transitions, FEN parser, null move mechanics
│   ├── evaluation/               # Positional evaluation subsystem
│   │   ├── eval.h / .cpp         # Tapered evaluation engine, phase calculation, positional terms
│   │   └── pawn_table.h          # 16K-entry Pawn Hash Table (caches structure, passers, attacks)
│   ├── hash/                     # Zobrist position hashing, TT & Endgames
│   │   ├── zobrist.h / .cpp      # 64-bit pseudo-random key generation & position hashing
│   │   ├── tt.h / .cpp           # Transposition table declarations, TTEntry, TTFlag, probe/store
│   │   └── kpk.h / .cpp          # In-memory KPK retrograde bitbase table & 29-pass solver
│   ├── moves/                    # Move encoding & legal move generation
│   │   ├── move.h / .cpp         # 32-bit compact Move class & flag masks
│   │   ├── movegen.h / .cpp      # Strictly legal move generator with pin & check masks
│   │   └── undomove.h            # UndoInfo state snapshot structure
│   ├── opening book/             # Polyglot GM opening library
│   │   └── book.bin              # 170 MB opening book containing 11.1 Million positions
│   ├── perft/                    # Move generation validation & benchmarking
│   │   ├── perft.h / .cpp        # Perft tree traversal, divide debugger, NPS benchmark
│   │   └── perft_results.h       # Reference perft move counts from standard positions
│   ├── search/                   # Alpha-Beta search & move ordering
│   │   ├── search.h              # Search class declaration, time controls & heuristics
│   │   └── search.cpp            # PVS, TT probe/store, NMP, RFP, LMR, Futility, MVV-LVA, Killers
│   ├── syzygy/                   # Syzygy Endgame Tablebase Probing Library (Fathom-based)
│   │   ├── syzygy.h / .cpp       # High-level engine Syzygy wrapper (root probe & WDL probe)
│   │   ├── tbprobe.h / .c        # Fathom tablebase probing engine implementation
│   │   ├── tbchess.c             # Internal bitboard conversion utilities
│   │   ├── tbconfig.h            # Build configuration for Syzygy probing
│   │   └── stdendian.h           # Cross-platform endianness handlers
│   ├── tablebase/                # Bundled 3-4-5 Piece Syzygy Tablebase Files
│   │   ├── wdl/                  # 145 Win-Draw-Loss (.rtbw) tablebase files
│   │   └── dtz/                  # 145 Distance-To-Zero (.rtbz) tablebase files
│   ├── test/                     # Automated unit test suites & validation tools
│   │   ├── fastchess.exe         # Fastchess executable for command-line automated matches
│   │   ├── test_attacks.cpp      # Non-sliding & sliding attack validation
│   │   ├── test_magic.cpp        # Magic bitboard mask & collision invariance tests
│   │   ├── moveTester.cpp        # 32-bit move encoding/decoding verification
│   │   ├── movegen.cpp           # Strict legal move generation test runner
│   │   ├── makemove.cpp          # Board makeMove state transition tests
│   │   ├── undoMoveTest.cpp      # Byte-level make/undo state symmetry verification
│   │   ├── test_eval.cpp         # Evaluation term unit testing
│   │   ├── run_eval.cpp          # Standalone evaluation runner
│   │   └── mirror_test.py        # Color symmetry validation script across FEN suites
│   ├── uci/                      # Universal Chess Interface protocol handler
│   │   ├── uci.h                 # UCI parser & dispatcher declaration
│   │   └── uci.cpp               # UCI commands, adaptive 7-tier time allocation, option handlers
│   └── utils/                    # Bitboard utilities, constants & lookup tools
│       ├── type.h                # Core types (U64, U32), enums, evaluation constants
│       ├── bitboard_utilities.h / .cpp # Bit manipulation helpers (popCount, popLSB, lsb)
│       ├── magic_numbers.h       # Precomputed 64-bit magic numbers for all 64 squares
│       └── tools.h / .cpp        # Geometric ray masks (between, line, ray, outpost masks)
```

---

## Core Engine Architecture

### Dual Board Representation & State Machine

ADAM maintains a dual representation inside `Engine/board/board.h` and `Engine/board/board.cpp`:

1. **12 Piece Bitboards** (`bitboards[12]`): 64-bit unsigned integers representing individual piece locations:
   - White: `WP` (0), `WN` (1), `WR` (2), `WB` (3), `WQ` (4), `WK` (5)
   - Black: `BP` (6), `BN` (7), `BR` (8), `BB` (9), `BQ` (10), `BK` (11)
2. **3 Occupancy Bitboards** (`occupancies[3]`): Combined occupancy bitboards for `WHITE` (0), `BLACK` (1), and `BOTH` (2).
3. **Mailbox Array** (`board[64]`): 1D array mapping square indices (0–63) to `Piece` enums for immediate $O(1)$ piece queries.

```text
Square Index Mapping (Little-Endian Rank-File):

Rank 8 | 56 57 58 59 60 61 62 63
Rank 7 | 48 49 50 51 52 53 54 55
Rank 6 | 40 41 42 43 44 45 46 47
Rank 5 | 32 33 34 35 36 37 38 39
Rank 4 | 24 25 26 27 28 29 30 31
Rank 3 | 16 17 18 19 20 21 22 23
Rank 2 |  8  9 10 11 12 13 14 15
Rank 1 |  0  1  2  3  4  5  6  7
       -------------------------
         A  B  C  D  E  F  G  H
```

### Incremental State Updates & Byte-Level Undo Symmetry

During search, recalculating scores from scratch is computationally prohibitive. ADAM maintains evaluation terms and keys incrementally inside `makeMove()` and `undoMove()`:

- **Positional Scores**: `mgScore` and `egScore` are updated incrementally via `addPieceScore()` and `removePieceScore()`, adding and subtracting material values and Piece-Square Table (PST) values.
- **Game Phase**: `gamePhase` is updated incrementally upon non-pawn piece additions and captures.
- **Zobrist Key & Pawn Key**: Updated incrementally with single-cycle XOR operations.
- **Undo Snapshot**: Every `makeMove()` call pushes an `UndoInfo` struct onto `history[ply]` storing:
  - `castlingRights` (4-bit mask)
  - `enPassant` (target square or `NO_SQUARE`)
  - `halfmoveClock` (50-move rule counter)
  - `zobristKey` (full 64-bit position hash)
  - `pawnKey` (64-bit pawn structure hash)
  - `mgScore`, `egScore`, and `gamePhase`
- **Null Move Mechanics**: `makeNullMove()` toggles the side to move, clears en-passant, updates the Zobrist key with `sideKey` and en-passant keys, and increments the ply. `undoNullMove()` restores the exact previous state.

### Zobrist Hashing & Dynamic Repetition Detection

The `Zobrist` system initializes 64-bit pseudo-random numbers at startup:
- `pieceKeys[12][64]`: Unique key for every piece on every square.
- `castleKeys[16]`: Unique key for each of the 16 castling rights bit combinations.
- `enPassantKeys[8]`: Unique key for the en-passant target file.
- `sideKey`: Toggled when Black is to move.

#### Dynamic Repetition Strategy
During search, `board.isRepetition()` traverses backward from `ply - 2` down to `ply - halfmoveClock` in steps of 2. When a repeat is detected, it is scored as **0 centipawns (Draw)**:
- **When Behind Material** (e.g. $-350\text{ cp}$): A draw score of $0$ is superior to all losing alternatives, prompting ADAM to actively seek perpetual check or defensive repetitions.
- **When Ahead Material** (e.g. $+350\text{ cp}$): A draw score of $0$ is an unacceptable concession, ensuring ADAM avoids repetitions and plays aggressively for a win.

### Attack Tables & Fancy Magic Bitboards

#### 1. Non-Sliding Pieces
Knights, Kings, and Pawns utilize precomputed lookup arrays initialized at startup (`Engine/attack/attacks.cpp`):
- `knightAttack[64]`: 8 possible L-shaped jumps.
- `kingAttack[64]`: 8 surrounding king steps.
- `whitePawnAttack[64]` / `blackPawnAttack[64]`: Diagonal pawn capture squares.

#### 2. Sliding Pieces (Fancy Magic Bitboards)
Bishop and Rook attacks are resolved in $O(1)$ constant time using precalculated 64-bit magic numbers and shift amounts (`Engine/attack/magic.cpp`):

$$\text{masked\_occ} = \text{occ} \ \& \ \text{mask}[s]$$

$$\text{index} = \frac{\text{masked\_occ} \times \text{magic}[s]}{2^{64 - \text{shift}[s]}}$$

$$\text{attacks} = \text{attackTable}[s][\text{index}]$$

- Queen attacks are calculated via the bitwise union of Bishop and Rook attacks:
$$\text{QueenAttacks}(s, \text{occ}) = \text{getBishopAttack}(s, \text{occ}) \mid \text{getRookAttack}(s, \text{occ})$$

### Strictly Legal Move Generation

Unlike engines that generate pseudo-legal moves and test king safety inside `makeMove()`, ADAM calculates legal moves **strictly up front** (`Engine/moves/movegen.cpp`):

1. **`CheckInfo` Computation**: Locates the king square, computes opponent attack bitboards, detects all direct checkers (`checkers`), and identifies pin rays (`pinnedPieces`, `pinnedRay[64]`).
2. **Double Check Handling**: If `checkerCount >= 2`, move generation for all non-king pieces is bypassed completely; only legal king evasions are generated.
3. **Single Check Handling**: Non-king piece moves are masked with `checkMask` (requiring either capturing the attacking checker or interposing a piece on the checking ray).
4. **Absolute Pin Rays**: Pinned pieces are constrained strictly to moves along their pin ray (`pinnedRay[from]`).
5. **King Moves**: King destinations are validated against opponent attack maps using simulated occupancy to prevent moving into discovered checks along sliding rays.

### Compact 32-bit Move Encoding

Every move is stored in a compact 32-bit integer (`Move` class, `Engine/moves/move.h`):

```text
┌────────────┬────────────┬──────────────┬──────────────┬─────────────┬───────────────┬──────────┐
│  Bits 0-5  │  Bits 6-11 │  Bits 12-15  │  Bits 16-19  │ Bits 20-23  │  Bits 24-27   │ Bits 28+ │
│ From Square│  To Square │  Promotion   │   Move Flag  │ Moved Piece │ Captured Piece│ Reserved │
│   (0-63)   │   (0-63)   │  (Piece+1)   │  (MoveFlag)  │  (Piece+1)  │   (Piece+1)   │          │
└────────────┴────────────┴──────────────┴──────────────┴─────────────┴───────────────┴──────────┘
```

Move flags include: `quiet`, `capture`, `doublePawnPush`, `kingSideCastle`, `queenSideCastle`, `enPassant`, `promotion`, and `promotion_capture`.

---

## Transposition Table (TT) Subsystem

ADAM incorporates a high-performance Transposition Table implemented in `Engine/hash/tt.h` and `Engine/hash/tt.cpp`.

### Memory Layout & Entry Structure

Each entry in the table occupies a clean 24-byte structure:
```cpp
struct TTEntry {
    U64 key;          // Full 64-bit Zobrist key for collision verification
    int score;        // Minimax evaluation or bound score
    int depth;        // Depth of the searched subtree
    TTFlag flag;      // TT_EXACT, TT_LOWER, or TT_UPPER
    Move bestMove;    // Principal variation / best cutoff move (Hash Move)
};
```

### Single-Cycle Bitwise Indexing & Collision Safety
- **Power-of-2 Sizing**: The number of entries is rounded down to the nearest power of 2 ($2^N$).
- **Single-Cycle Masking**: Index lookup is performed via `key & mask` instead of modulo division (`key % size`), executing in 1 CPU cycle.
- **Collision Protection**: During probing, `entry.key == key` validates ownership, preventing hash collisions from corrupting search decisions.

### Bound Flags & Replacement Scheme
- **`TT_EXACT`**: Score was strictly between $\alpha$ and $\beta$ (true minimax PV node score).
- **`TT_LOWER`**: Beta cutoff occurred; the position is at least this good ($\ge \beta$, fail-high node).
- **`TT_UPPER`**: All moves failed low; the position is at most this good ($\le \alpha$, all-node).

### Root-Independent Mate Score Normalization
To ensure mate scores stored in the TT remain valid when retrieved at different tree depths, scores are adjusted relative to the current search ply:
```cpp
// Storing into TT:
if (score > MATE_THRESHOLD) score += ply;
else if (score < -MATE_THRESHOLD) score -= ply;

// Probing from TT:
if (score > MATE_THRESHOLD) score -= ply;
else if (score < -MATE_THRESHOLD) score += ply;
```

---

## Endgame Bitbases & Tablebases

### In-Memory KPK Retrograde Bitbase (29 Passes)

King + Pawn vs King (KPK) is the foundational pawn endgame. ADAM generates a complete 24,576-entry bitbase directly into memory at startup in **29 retrograde passes** (`Engine/hash/kpk.cpp`):
- Encodes all valid triples `(whiteKing, whitePawn, blackKing, sideToMove)`.
- Resolves game-theoretic Win/Draw status in $O(1)$ without spending search tree plies.

### Syzygy 3-4-5 Piece Tablebases (WDL & DTZ)

ADAM integrates the Fathom probing library for full Syzygy 3, 4, and 5-piece endgame tablebases:
- **145 WDL Files** (`.rtbw`): Queried within the search tree to prune or evaluate terminal positions instantly.
- **145 DTZ Files** (`.rtbz`): Queried at the root to play the fastest mathematically optimal winning moves under the 50-move rule.

---

## Evaluation System

### Game Phase & 256-Step Tapered Evaluation

ADAM calculates a dynamic game phase ($0 \le \text{phase} \le 24$) based on non-pawn material:
$$\text{phase} = (\text{Knights} \times 1) + (\text{Bishops} \times 1) + (\text{Rooks} \times 2) + (\text{Queens} \times 4)$$

The final evaluation blends Middlegame (MG) and Endgame (EG) positional evaluations across a smooth 256-step interpolation:
$$\text{score} = \frac{(\text{mgScore} \times \text{phase}) + (\text{egScore} \times (24 - \text{phase}))}{24}$$

### Dedicated 16K-Entry Pawn Hash Table

Pawn structure evaluation is computationally heavy. ADAM uses a 16,384-entry direct-mapped cache (`Engine/evaluation/pawn_table.h`) indexed by `pawnKey`:
- Caches MG and EG pawn structure scores.
- Caches passed pawn masks, pawn attacks, and shelter structures.
- Delivers a massive boost to overall search NPS by eliminating redundant pawn calculations.

### Pawn Structure, Outposts & Mobility
- **Passed Pawns**: Non-linear rank-based bonuses, rewarded heavily when supported by friendly rooks or unobstructed.
- **Isolated & Doubled Pawns**: Graded penalties scaling with open files.
- **Phalanx & Pawn Chains**: Bonus for connected, advancing pawn duos.
- **Knight Outposts**: Bonus for knights firmly anchored by friendly pawns on opponent territory.
- **Safe Piece Mobility**: Scaled bonuses based on safe ray/destination count.

### Dynamic Bishop Pair & Depletion Scaling
The Bishop Pair bonus scales dynamically as enemy and friendly pawns are removed from the board, rewarding bishops in open endgames.

### King Safety & Quadratic Danger Zones
King defense evaluation calculates attacker counts and piece weights around the king's 3×3 square zone. Danger points are mapped quadratically:
$$\text{KingPenalty} = \text{DangerScore}^2 / 64$$

---

## Search Engine & Pruning Heuristics

ADAM implements an Alpha-Beta framework with Principal Variation Search (PVS):

```text
Iterative Deepening Loop (Depth 1 -> Max Depth):
  ├── Syzygy Root Probe (if <= 5 pieces remaining)
  ├── PVS Root Window [alpha, beta]
  └── Search Tree:
        ├── Dynamic Repetition & 50-Move Check (Score = 0)
        ├── Transposition Table Probe (Cutoff / Hash Move)
        ├── Reverse Futility Pruning (RFP) at depth <= 3
        ├── Dynamic Null Move Pruning (NMP) with zugzwang guard
        ├── strictly legal move generation
        ├── 11-Tier Move Ordering (TT move -> MVV-LVA -> Killers -> History)
        ├── Move-Loop Futility Pruning (quiet moves at depth <= 2)
        ├── Late Move Reductions (LMR) for late non-tactical moves
        ├── Quiescence Search with victim-specific Delta Pruning
        └── Transposition Table Store (Exact / Lower / Upper)
```

### Key Search Optimizations
1. **Principal Variation Search (PVS)**: Explores the first move with full window $[\alpha, \beta]$, scouting subsequent moves with null-window $[\alpha, \alpha+1]$.
2. **Dynamic Null Move Pruning (NMP)**: Reduction $R = 2 + \text{depth} / 4$, protected by non-pawn material checks.
3. **Reverse Futility Pruning (RFP / Static NMP)**: Prunes at $\text{depth} \le 3$ when $\text{staticEval} - 100 \times \text{depth} \ge \beta$ (+34.86 Elo).
4. **Move-Loop Futility Pruning**: Skips quiet moves at $\text{depth} \le 2$ when $\text{staticEval} + 100 \times \text{depth} \le \alpha$ (+70.44 Elo).
5. **Late Move Reductions (LMR)**: Reduces late quiet moves by 1–2 ply while protecting checks, killers, and tactical pushes.
6. **11-Tier Move Ordering**:
   1. TT Hash Move (`10,000,000`)
   2. Queen Promotions (`9,000,000`)
   3. Knight Promotions (`8,500,000`)
   4. Winning/Equal MVV-LVA Captures (`8,000,000 + MVV - LVA`)
   5. Killer Move 1 (`7,000,000`)
   6. Killer Move 2 (`6,000,000`)
   7. Counter-move Heuristic (`5,000,000`)
   8. Advanced Pawn Advances (`4,000,000`)
   9. Castling Moves (`3,000,000`)
   10. History Heuristic (`0 – 2,000,000`)
   11. Losing Captures (Bad MVV-LVA, `1,000,000`)

---

## Universal Chess Interface (UCI) & Adaptive Clock Manager

ADAM fully supports the standard UCI protocol (`Engine/uci/uci.cpp`).

### Adaptive 7-Tier Time Allocation Algorithm
Time is allocated based on remaining clock and increment brackets:

| Bracket | Time Range | Formula | Bounds |
|:---|:---|:---|:---|
| **1. Classical** | $\ge 30\text{ min}$ | $(\text{time} / 120) + (\text{inc} \times 3 / 5)$ | $5\text{s} - 15\text{s}$ |
| **2. Long Time Control** | $20 - 30\text{ min}$ | $(\text{time} / 140) + (\text{inc} \times 3 / 5)$ | $3\text{s} - 10\text{s}$ |
| **3. Long Rapid** | $13 - 20\text{ min}$ | $(\text{time} / 180) + (\text{inc} / 2)$ | $2\text{s} - 8\text{s}$ |
| **4. Standard Rapid** | $6.5 - 13\text{ min}$ | $(\text{time} / 100) + (\text{inc} / 2)$ | $1.5\text{s} - 6\text{s}$ |
| **5. Standard Blitz** | $1.5 - 6.5\text{ min}$ | $(\text{time} / 90) + (\text{inc} \times 2 / 5)$ | $800\text{ms} - 3\text{s}$ |
| **6. Bullet / Low Clock** | $20\text{s} - 1.5\text{ min}$ | $(\text{time} / 40) + (\text{inc} \times 2 / 5)$ | $300\text{ms} - 1.5\text{s}$ |
| **7. Extreme Scramble** | $< 20\text{ seconds}$ | $(\text{time} / 20) + (\text{inc} / 4)$ | Max $600\text{ms}$ |

---

## Opening Book & Lichess Bot Integration

1. **Polyglot Opening Book (`Engine/opening book/book.bin`)**:
   - 170 MB opening book containing **11.1 Million grandmaster positions**.
   - Weighted random selection up to ply 25 for varied opening lines.
2. **Lichess Bot Architecture**:
   - Compatible with `lichess-bot` through standard UCI pipes.
   - Smooth fallback to Lichess Masters Database when leaving book theory.

---

## Automated Testing & Fastchess Match Runner

ADAM includes automated test scripts: `run_test.bat` and `run_test.ps1`.

- **Quick Sanity Test**: 20 rounds / 40 games at `5+0.05` across 6 concurrent threads.
- **Standard Match**: 100 rounds / 200 games at `10+0.1` across 6 concurrent threads.
- **SPRT Test**: Sequential Probability Ratio Test ($H_0 = 0$, $H_1 = +5\text{ Elo}$, $\alpha = 0.05$, $\beta = 0.05$) up to 2,000 rounds.

---

## Perft Benchmark Suite

Standard starting position move generation counts:

| Depth | Legal Leaf Nodes | NPS Benchmark |
|:---:|:---:|:---:|
| **1** | 20 | > 5,000,000 |
| **2** | 400 | > 5,000,000 |
| **3** | 8,902 | > 5,000,000 |
| **4** | 197,281 | > 4,800,000 |
| **5** | 4,865,609 | > 4,500,000 |
| **6** | 119,060,324 | > 4,200,000 |

---

## How to Build and Run ADAM

### Compilation Recipe

ADAM is written in standard modern C++20 (backwards-compatible with C++17). Compile using `g++` (MinGW-w64 / GCC 10+):

```bash
g++ -O3 -march=native -flto -DNDEBUG -std=c++20 \
    -IEngine -IEngine/syzygy \
    Engine/main.cpp \
    Engine/board/board.cpp \
    Engine/moves/move.cpp \
    Engine/moves/movegen.cpp \
    Engine/attack/attacks.cpp \
    Engine/attack/magic.cpp \
    Engine/attack/magic_instance.cpp \
    Engine/evaluation/eval.cpp \
    Engine/perft/perft.cpp \
    Engine/search/search.cpp \
    Engine/uci/uci.cpp \
    Engine/utils/bitboard_utilities.cpp \
    Engine/utils/tools.cpp \
    Engine/hash/zobrist.cpp \
    Engine/hash/tt.cpp \
    Engine/hash/kpk.cpp \
    Engine/syzygy/syzygy.cpp \
    Engine/syzygy/tbprobe.c \
    -o ADAM.exe
```

### CLI Interactive Session

```bash
./ADAM.exe
uci
isready
position startpos
go depth 8
```

---

## Roadmap & Future Directions

- **Multi-Threading (Lazy SMP)**: Parallel alpha-beta search with shared Transposition Table.
- **Static Exchange Evaluation (SEE)**: Accurate capture pruning in quiescence search.
- **Continuation History Heuristics**: Move ordering improvements based on piece-to-square history.
- **NNUE Evaluation Architecture**: Dual-perspective halfKP/halfKA neural network evaluation.

---

## License

This project is created by **Prajwal** for educational, competitive, and research purposes.