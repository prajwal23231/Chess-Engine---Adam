# ADAM — Handcrafted C++ Chess Engine

ADAM is a modern, tournament-grade, UCI-compliant chess engine designed and written from scratch in C++. Built from the ground up for extreme speed, search depth, and tactical precision, ADAM features a dual bitboard/mailbox board representation, Fancy Magic Bitboards, strictly legal move generation, a 256-step tapered evaluation system backed by a dedicated Pawn Hash Table and an in-memory retrograde KPK Bitbase, integrated Syzygy 3-4-5 piece endgame tablebases, and an advanced Alpha-Beta search engine combining Principal Variation Search (PVS), dynamic Null Move Pruning (NMP), Reverse Futility Pruning (RFP), Move-Loop Futility Pruning, Late Move Reductions (LMR), and multi-tier move ordering heuristics.

**Author**: Prajwal  
**Language**: C++20 / C++17  
**Protocol**: UCI (Universal Chess Interface)  
**Throughput**: 2.0+ to 2.5+ Million NPS (Single Core)  

---

## Table of Contents

- [Overview & Architectural Highlights](#overview--architectural-highlights)
- [Directory Layout](#directory-layout)
- [Core Engine Architecture](#core-engine-architecture)
  - [Dual Board Representation & State Machine](#dual-board-representation--state-machine)
  - [Incremental State Updates & Byte-Level Undo Symmetry](#incremental-state-updates--byte-level-undo-symmetry)
  - [Zobrist Hashing & Repetition Detection](#zobrist-hashing--repetition-detection)
  - [Attack Tables & Fancy Magic Bitboards](#attack-tables--fancy-magic-bitboards)
  - [Strictly Legal Move Generation](#strictly-legal-move-generation)
  - [Compact 32-bit Move Encoding](#compact-32-bit-move-encoding)
- [Transposition Table (TT) Subsystem](#transposition-table-tt-subsystem)
  - [Architecture & Memory Layout](#architecture--memory-layout)
  - [Fast Bitwise Indexing & Collision Safety](#fast-bitwise-indexing--collision-safety)
  - [Bound Flags & Replacement Scheme](#bound-flags--replacement-scheme)
  - [Root-Independent Mate Score Normalization](#root-independent-mate-score-normalization)
  - [Hash Move Ordering Synergy](#hash-move-ordering-synergy)
- [Endgame Bitbases & Tablebases](#endgame-bitbases--tablebases)
  - [In-Memory KPK Retrograde Bitbase](#in-memory-kpk-retrograde-bitbase)
  - [Syzygy 3-4-5 Piece Tablebases (WDL & DTZ)](#syzygy-3-4-5-piece-tablebases-wdl--dtz)
- [Evaluation System](#evaluation-system)
  - [Game Phase & 256-Step Tapered Evaluation](#game-phase--256-step-tapered-evaluation)
  - [Piece-Square Tables (PST)](#piece-square-tables-pst)
  - [Dedicated 16K-Entry Pawn Hash Table](#dedicated-16k-entry-pawn-hash-table)
  - [Pawn Structure Evaluation & Passed Pawn Scaling](#pawn-structure-evaluation--passed-pawn-scaling)
  - [Bishop Pair & Dynamic Pawn Depletion Scaling](#bishop-pair--dynamic-pawn-depletion-scaling)
  - [Piece Mobility & Strategic Outposts](#piece-mobility--strategic-outposts)
  - [Rook Files, Seventh Rank & Connected Rooks](#rook-files-seventh-rank--connected-rooks)
  - [Bad & Trapped Piece Penalties](#bad--trapped-piece-penalties)
  - [Development & Castling Incentives](#development--castling-incentives)
  - [Tactical Vulnerability & Hanging Pieces](#tactical-vulnerability--hanging-pieces)
  - [King Safety & Quadratic Danger Zones](#king-safety--quadratic-danger-zones)
  - [Endgame King Cornering & Mop-Up Evaluation](#endgame-king-cornering--mop-up-evaluation)
  - [Material Scale Factor & Theoretical Draw Detection](#material-scale-factor--theoretical-draw-detection)
  - [Side-to-Move Tempo Bonus](#side-to-move-tempo-bonus)
- [Search Engine](#search-engine)
  - [Negamax & Alpha-Beta Pruning](#negamax--alpha-beta-pruning)
  - [Iterative Deepening & Timeout Resilience](#iterative-deepening--timeout-resilience)
  - [Check Extensions](#check-extensions)
  - [Principal Variation Search (PVS)](#principal-variation-search-pvs)
  - [Dynamic Null Move Pruning (NMP)](#dynamic-null-move-pruning-nmp)
  - [Reverse Futility Pruning (RFP)](#reverse-futility-pruning-rfp)
  - [Move-Loop Futility Pruning](#move-loop-futility-pruning)
  - [Late Move Reductions (LMR)](#late-move-reductions-lmr)
  - [Move Ordering Hierarchy (11 Tiers)](#move-ordering-hierarchy-11-tiers)
  - [Quiescence Search & Victim-Specific Delta Pruning](#quiescence-search--victim-specific-delta-pruning)
  - [Syzygy Search & Root Probing](#syzygy-search--root-probing)
- [Universal Chess Interface (UCI) & Time Management](#universal-chess-interface-uci--time-management)
  - [UCI Commands & Options](#uci-commands--options)
  - [Adaptive 7-Tier Time Management Algorithm](#adaptive-7-tier-time-management-algorithm)
- [Opening Book & Lichess Bot Integration](#opening-book--lichess-bot-integration)
- [Automated Testing & Fastchess Match Runner](#automated-testing--fastchess-match-runner)
  - [Test Modes (Quick, Standard, SPRT)](#test-modes-quick-standard-sprt)
  - [Baseline Engine Promotion](#baseline-engine-promotion)
  - [Unit Test Suites & Validation](#unit-test-suites--validation)
  - [Perft Benchmark Suite](#perft-benchmark-suite)
- [How to Build and Run ADAM](#how-to-build-and-run-adam)
  - [Compilation Recipe](#compilation-recipe)
  - [CLI Interactive Session](#cli-interactive-session)
- [Roadmap & Future Directions](#roadmap--future-directions)
- [License](#license)

---

## Overview & Architectural Highlights

- **Dual Board Representation**: 12 piece bitboards (`WP` through `BK`), 3 occupancy bitboards (`WHITE`, `BLACK`, `BOTH`), and an 8×8 mailbox array for $O(1)$ square occupancy and piece lookups.
- **Fancy Magic Bitboards**: Constant-time $O(1)$ sliding attack lookups for Rooks, Bishops, and Queens using 64-bit magic numbers and precomputed attack arrays.
- **Strictly Legal Move Generation**: Computes pin-rays and check-masks upfront. Generates 100% legal moves with zero illegal move testing or pseudo-legal post-filtering overhead.
- **Transposition Table**: Configurable hash size (default 64 MB, dynamically scalable up to 1 TB), power-of-2 single-cycle masking, full 64-bit Zobrist key verification, depth-preferred replacement, root-independent mate score normalization, and top-priority hash move ordering (`10,000,000`).
- **Dedicated Pawn Hash Table**: 16,384-entry direct-mapped cache indexed by `pawnKey`, storing middle-game and endgame pawn structure scores along with passed pawn and pawn attack bitboard masks.
- **In-Memory KPK Bitbase**: 24,576-entry retrograde bitbase generated in 29 passes at startup, providing instant, exact game-theoretic win/draw determination for King + Pawn vs King endgames.
- **Syzygy 3-4-5 Piece Tablebase Integration**: Fully bundled with 145 WDL and 145 DTZ tablebase files. Features instantaneous root DTZ optimal move execution and search tree WDL probing.
- **Tapered Positional Evaluation**: Blends opening/middlegame and endgame piece-square tables with game phase calculation ($0 \le \text{phase} \le 24$) using a 256-step integer interpolation. Incorporates pawn phalanx/chains, graded isolated/doubled penalties, advanced passed pawn scaling, dynamic bishop pair scaling based on pawn depletion, safe piece mobility, knight outposts, rook open files, bad/trapped bishops, development/castling incentives, hanging piece detection, non-linear quadratic king danger zones, endgame king cornering/mop-up, and material draw scaling.
- **Alpha-Beta Search Engine**:
  - Principal Variation Search (PVS) with zero-window scouting and full re-search.
  - Dynamic Null Move Pruning (NMP) with adaptive reduction $R = 2 + \text{depth} / 4$ and non-pawn material zugzwang verification.
  - Reverse Futility Pruning (RFP / Static NMP) at $\text{depth} \le 3$ with margin $100 \times \text{depth}$ (+34.86 Elo).
  - Move-Loop Futility Pruning for quiet moves at $\text{depth} \le 2$ with margin $100 \times \text{depth}$ (+70.44 Elo).
  - Late Move Reductions (LMR) with 1–2 ply reductions, strictly exempting tactical moves, checks, killer moves, and 7th-rank pawn pushes.
  - Check extensions (+1 ply) during forcing check sequences.
  - 11-tier move ordering hierarchy incorporating TT hash move, promotions, MVV-LVA good/bad captures, 2 killer slots per ply, counter-move heuristic, advanced pawn advances, castling shelter score, and history heuristic.
  - Quiescence search with stand-pat, victim-specific delta pruning, and check evasion search.
- **Adaptive Time Management**: 7-tier dynamic clock allocation accommodating Classical, Rapid, Blitz, Bullet, and extreme time scrambles, with move overhead protection and hard floor limits.
- **High Performance**: Reaches **2.0M to 2.5M+ nodes per second (NPS)** on modern x86-64 hardware, solving depth 8 in under 100 ms.

---

## Directory Layout

```text
Adam/
├── ADAM.exe                      # Optimized release binary (-O3 -march=native -flto)
├── ADAM_base.exe                 # Frozen reference binary for Fastchess regression testing
├── README.md                     # Comprehensive architecture and engine documentation
├── run_test.bat                  # Interactive Windows batch test runner (Quick, Standard, SPRT)
├── run_test.ps1                  # Interactive PowerShell test runner (Fastchess automation)
├── test_results.pgn              # Output PGN match archive from automated engine runs
├── Engine/                       # Engine Source Code
│   ├── main.cpp                  # Program entry point (initializes Zobrist, Tools, KPK, Syzygy)
│   ├── attack/                   # Attack tables & Magic Bitboard subsystem
│   │   ├── attacks.h / .cpp      # Precomputed non-sliding (Pawn, Knight, King) attack tables
│   │   ├── magic.h / .cpp        # Fancy Magic Bitboard generator & constant-time sliding lookups
│   │   ├── magicGen.h / .cpp     # Monte Carlo magic candidate search algorithm
│   │   ├── magic_instance.h / .cpp # Global magic singleton instance (`g_magic`)
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
│   │   ├── run_eval.cpp          # Standalone evaluation evaluation runner
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

ADAM maintains a dual representation inside [Engine/board/board.h](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/board/board.h) and [Engine/board/board.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/board/board.cpp):

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

During search, repeatedly recalculating scores from scratch is computationally expensive. ADAM maintains evaluation terms and keys incrementally inside `makeMove()` and `undoMove()`:

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

### Zobrist Hashing & Repetition Detection

The `Zobrist` system initializes 64-bit pseudo-random numbers at startup:
- `pieceKeys[12][64]`: Unique key for every piece on every square.
- `castleKeys[16]`: Unique key for each of the 16 castling rights bit combinations.
- `enPassantKeys[8]`: Unique key for the en-passant target file.
- `sideKey`: Toggled when Black is to move.

#### Dynamic Repetition Strategy
During search, `board.isRepetition()` traverses backward from `ply - 2` down to `ply - halfmoveClock` in steps of 2. When a repeat is detected, it is scored as **0 centipawns (Draw)**:
- **When Behind Material** (e.g. $-350\text{ cp}$): A draw score of $0$ is superior to all losing alternatives, prompting ADAM to actively seek perpetual check or defensive repetitions.
- **When Ahead Material** (e.g. $+350\text{ cp}$): A draw score of $0$ is an unacceptable concession, ensuring ADAM avoids repetitions and plays for a win.

### Attack Tables & Fancy Magic Bitboards

#### 1. Non-Sliding Pieces
Knights, Kings, and Pawns utilize precomputed lookup arrays initialized at startup ([Engine/attack/attacks.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/attack/attacks.cpp)):
- `knightAttack[64]`: 8 possible L-shaped jumps.
- `kingAttack[64]`: 8 surrounding king steps.
- `whitePawnAttack[64]` / `blackPawnAttack[64]`: Diagonal pawn capture squares.

#### 2. Sliding Pieces (Fancy Magic Bitboards)
Bishop and Rook attacks are resolved in $O(1)$ constant time using precalculated 64-bit magic numbers and shift amounts ([Engine/attack/magic.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/attack/magic.cpp)):

$$\text{masked\_occ} = \text{occ} \ \& \ \text{mask}[s]$$

$$\text{index} = \frac{\text{masked\_occ} \times \text{magic}[s]}{2^{64 - \text{shift}[s]}}$$

$$\text{attacks} = \text{attackTable}[s][\text{index}]$$

- Queen attacks are calculated via the bitwise union of Bishop and Rook attacks:
$$\text{QueenAttacks}(s, \text{occ}) = \text{getBishopAttack}(s, \text{occ}) \mid \text{getRookAttack}(s, \text{occ})$$

### Strictly Legal Move Generation

Unlike engines that generate pseudo-legal moves and test king safety inside `makeMove()`, ADAM calculates legal moves **strictly up front** ([Engine/moves/movegen.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/moves/movegen.cpp)):

1. **`CheckInfo` Computation**: Locates the king square, computes opponent attack bitboards, detects all direct checkers (`checkers`), and identifies pin rays (`pinnedPieces`, `pinnedRay[64]`).
2. **Double Check Handling**: If `checkerCount >= 2`, move generation for all non-king pieces is bypassed completely; only legal king evasions are generated.
3. **Single Check Handling**: Non-king piece moves are masked with `checkMask` (requiring either capturing the attacking checker or interposing a piece on the checking ray).
4. **Absolute Pin Rays**: Pinned pieces are constrained strictly to moves along their pin ray (`pinnedRay[from]`).
5. **King Moves**: King destinations are validated against opponent attack maps using simulated occupancy to prevent moving into discovered checks along sliding rays.

### Compact 32-bit Move Encoding

Every move is stored in a compact 32-bit integer (`Move` class, [Engine/moves/move.h](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/moves/move.h)):

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

ADAM incorporates a high-performance Transposition Table implemented in [Engine/hash/tt.h](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/hash/tt.h) and [Engine/hash/tt.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/hash/tt.cpp).

### Architecture & Memory Layout

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

### Fast Bitwise Indexing & Collision Safety
- **Power-of-2 Sizing**: The number of entries is rounded down to the nearest power of 2 ($2^N$).
- **Single-Cycle Masking**: Index lookup is performed via `key & mask` instead of modulo division (`key % size`), executing in 1 CPU cycle.
- **Collision Protection**: During probing, `entry.key == key` validates ownership, preventing hash collisions from corrupting search decisions.

### Bound Flags & Replacement Scheme
- **`TT_EXACT`**: Score was strictly between $\alpha$ and $\beta$ (true minimax PV node score).
- **`TT_LOWER`**: Beta cutoff occurred; the position is at least this good ($\ge \beta$, fail-high node).
- **`TT_UPPER`**: All moves failed low; the position is at most this good ($\le \alpha$, all-node).

**Replacement Policy**: An entry is replaced if a new position hashes to the slot, if the new search depth is greater than or equal to the stored depth (`depth >= entry.depth`), or if the new result is an exact PV score (`flag == TT_EXACT`).

### Root-Independent Mate Score Normalization
To prevent mate-in-N scores from drifting across different depths and plies:
- **Storing**: `scoreToTT()` converts root-relative mate scores to position-independent scores (`score + ply` for win, `score - ply` for loss).
- **Probing**: `scoreFromTT()` converts stored scores back to current-root distance (`score - ply` for win, `score + ply` for loss).

### Hash Move Ordering Synergy
When a stored entry's depth is insufficient to trigger an immediate cutoff, its `bestMove` is extracted and assigned top priority (**`10,000,000`**) in `scoreMove()`. Searching the proven best move first yields immediate beta cutoffs, cutting search times dramatically across iterative deepening.

---

## Endgame Bitbases & Tablebases

### In-Memory KPK Retrograde Bitbase

ADAM includes a built-in King + Pawn vs King (KPK) bitbase generator and probe ([Engine/hash/kpk.h](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/hash/kpk.h) and [Engine/hash/kpk.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/hash/kpk.cpp)):

- **Compact Footprint**: 24,576 states packed into a 24 KB bit array (`std::array<U32, 6144>`).
- **Retrograde Analysis**: At startup, `KPKBitbase::init()` runs 29 backward iterations propagating terminal win/loss conditions across pawn advances and king opposition moves.
- **Endgame Scaling Integration**: When an endgame is reduced to $K+P \text{ vs } K$, `getMaterialScaleFactor()` probes the bitbase:
  - If winning, scale factor returns `128` (full evaluation).
  - If drawn (e.g. defending king controls key squares or holds opposition), scale factor returns `0`, immediately collapsing the score to a clean draw (0 centipawns).

### Syzygy 3-4-5 Piece Tablebases (WDL & DTZ)

ADAM provides complete integration with Syzygy Endgame Tablebases via the embedded Fathom probing library ([Engine/syzygy/syzygy.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/syzygy/syzygy.cpp)):

- **Bundled Files**: Includes all 145 WDL (`.rtbw`) and 145 DTZ (`.rtbz`) files covering all 3, 4, and 5-piece endgames in `Engine/tablebase/wdl` and `Engine/tablebase/dtz`.
- **Root DTZ Probing (`probeRoot`)**: If $\le 5$ pieces remain at the root, ADAM probes DTZ, identifies the game-theoretically optimal move, and plays it immediately without wasting search time.
- **Search Tree WDL Probing (`probeWDL`)**: During search, when `halfmoveClock == 0`, WDL is probed to return exact mate-bounded scores (`MATE_SCORE - MAX_PLYS + ply` for win, 0 for draw, `-MATE_SCORE + MAX_PLYS - ply` for loss).
- **Configurable Path**: Configurable at runtime via `setoption name SyzygyPath value <path>`.

---

## Evaluation System

ADAM implements a tapered evaluation combining material balance, piece-square tables, and handcrafted positional terms ([Engine/evaluation/eval.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/evaluation/eval.cpp)).

### Game Phase & 256-Step Tapered Evaluation

Game phase is calculated based on remaining non-pawn material:
- **Phase Weights**: Knight = 1, Bishop = 1, Rook = 2, Queen = 4 (`TotalPhase = 24`).
- **256-Step Interpolation**:
$$\text{Score} = \frac{\text{mgScore} \times p + \text{egScore} \times (256 - p)}{256}$$
Where $p = \text{phase256}[\text{phase}]$ provides smooth, non-linear transitions from opening to endgame.

### Piece-Square Tables (PST)
Utilizes PeSTO-derived tables tuned separately for opening/middlegame and endgame across all 6 piece types.

### Dedicated 16K-Entry Pawn Hash Table

Pawn structure calculations are computationally intensive. ADAM offloads this to a dedicated 16,384-entry direct-mapped `PawnTable` ([Engine/evaluation/pawn_table.h](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/evaluation/pawn_table.h)):
- Indexed by `pawnKey`.
- Caches middle game and endgame scores (`mg`, `eg`).
- Caches bitboards for passed pawns (`passedPawns[2]`) and pawn attacks (`pawnAttacks[2]`).
- Caching these bitboards eliminates redundant computations inside piece mobility, rook open-file scans, knight outposts, and passed pawn routines.

### Pawn Structure Evaluation & Passed Pawn Scaling

1. **Isolated Pawns**: Penalties graded by file (center files D/E: -18 MG / -25 EG; flank files A/H: -10 MG / -15 EG).
2. **Doubled Pawns**: -15 MG / -25 EG penalty for stacked pawns on the same file.
3. **Backward Pawns**: Detected when a pawn cannot be defended by neighboring pawns and its advance square is controlled by enemy pawns.
4. **Pawn Phalanx & Chains**:
   - Phalanx (adjacent pawns on same rank): +8 MG / +12 EG.
   - Chains (pawns protecting each other diagonally): +10 MG / +15 EG.
5. **Passed Pawns**:
   - Rank 4: +20 MG / +40 EG
   - Rank 5: +40 MG / +80 EG
   - Rank 6: +80 MG / +160 EG
   - Rank 7: +150 MG / +300 EG
   - **Connected Passers Bonus**: Array `[0, 10, 15, 25, 40, 65, 110, 0]`.
   - **Defended Passed Pawns**: +15 MG / +25 EG bonus if protected by a friendly pawn; -15 MG / -25 EG penalty if completely undefended in the endgame.
   - **Unblocked Marching Bonus**: Additional bonus for 5th- and 6th-rank passers when their stop square is clear.

### Bishop Pair & Dynamic Pawn Depletion Scaling
- **Base Bonus**: +35 MG / +50 EG for retaining both bishops.
- **Pawn Depletion Scaling**: In endgames with $\le 8$ pawns remaining on the board, the bishop pair bonus increases by `(8 - totalPawns) * 3` cp (up to +24 cp), rewarding open diagonals.

### Piece Mobility & Strategic Outposts
- **Safe Mobility**: Evaluates reachable destination squares excluding squares attacked by enemy pawns (`~entry->pawnAttacks[opp]`).
- **Knight Outposts**: Knights on ranks 4, 5, or 6 protected by a friendly pawn and immune to enemy pawn attacks receive +25 MG / +35 EG (+10 extra bonus if located on the central D/E files).

### Rook Files, Seventh Rank & Connected Rooks
- **Open File**: +25 MG / +20 EG for rooks on files with no pawns.
- **Semi-Open File**: +15 MG / +12 EG for rooks on files with only enemy pawns.
- **Seventh Rank**: +30 MG / +45 EG when attacking 7th-rank pawns or restricting the enemy king.
- **Connected Rooks**: +15 MG / +15 EG for rooks mutually defending each other horizontally or vertically.

### Bad & Trapped Piece Penalties
- **Bad Bishops**: Penalizes bishops blocked by friendly pawns on the same square color (-4 MG / -6 EG per blocked pawn).
- **Trapped Bishops**: -150 cp penalty for cornered or boxed-in bishops on B3/B6 (boxed by A4/C4 or A5/C5 pawns) and A7/H7 or A2/H2.

### Development & Castling Incentives
- Evaluated during the opening and middlegame ($\text{phase} \ge 18$):
  - Undeveloped minor pieces on home squares (B1, G1, C1, F1 / B8, G8, C8, F8): -18 cp penalty each.
  - Safely castled king on flank squares (G1/H1, C1/B1 / G8/H8, C8/B8): +30 cp bonus.
  - Retaining castling rights: +15 cp.
  - Trapped uncastled king in the center with lost rights: -35 cp penalty.

### Tactical Vulnerability & Hanging Pieces
- Scans minor and major pieces attacked by the opponent and undefended by friendly pieces.
- Penalty: $\text{mg\_value}[\text{pieceType}] / 4$ (-225 cp for queen, -125 cp for rook, -75 cp for minor piece).

### King Safety & Quadratic Danger Zones
- Evaluated when $\text{phase} \ge 6$:
  - **Pawn Shield**: Flank/castled kings incur penalties for missing shield pawns (`PAWN_SHIELD_MISSING = 30`), stepped pawns (`PAWN_SHIELD_STEPPED = 15`), and open files facing the king (`OPEN_FILE_NEAR_KING = 25`).
  - **Central Uncastled King**: Uncastled kings on D/E files receive penalties up to 80 cp for open/semi-open central files and 25 cp for exposed adjacent files.
  - **Virtual King Zone & Danger Table**: A $3 \times 3$ virtual zone around the enemy king extended 1 rank towards the center. Attacks are weighted: Knight = 2, Bishop = 2, Rook = 3, Queen = 5. If $\ge 2$ attackers coordinate on the zone, a non-linear `kingDangerTable` penalty is applied (halved if the enemy queen has been traded).

### Endgame King Cornering & Mop-Up Evaluation
When a side possesses a decisive material advantage against a lone king (`calculateMatingScore()`):
$$\text{Bonus} = \text{pushToEdge}(\text{LosingKing}) \times 12 + (14 - \text{distance}(\text{WinKing}, \text{LoseKing})) \times 6$$
This guides the engine to push the enemy king to the edge and close in with its own king, preventing aimless endgame shuffling.

### Material Scale Factor & Theoretical Draw Detection
- $K+N+N \text{ vs } K$: Scaled to 0 (insufficient mating material).
- Opposite-Colored Bishops with one minor piece: Scaled to 64 (50% draw reduction).
- $K+P \text{ vs } K$: Probes `KPKBitbase::probe()` to scale winning endgames to 128 and drawn endgames to 0.

### Side-to-Move Tempo Bonus
- Adds +15 centipawns to the moving side, rewarding initiative (+6.95 Elo).

---

## Search Engine

ADAM's search engine operates inside [Engine/search/search.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/search/search.cpp) using a fail-soft Alpha-Beta framework.

### Negamax & Alpha-Beta Pruning
Implements fail-soft alpha-beta pruning with full principal variation tracking and Transposition Table integration.

### Iterative Deepening & Timeout Resilience
Search starts at `depth = 1` and increments depth-by-depth:
- Root moves are ordered using the best move from the previous iteration (`bestMove`), guaranteeing immediate cutoffs.
- **Timeout Safety**: If time expires mid-depth, the search aborts immediately, discarding incomplete results and retaining the proven `bestMove` from the last completed iteration.

### Check Extensions
When the moving side is in check (`inCheck == true`), search depth is extended by **+1 ply**:
```cpp
int extension = inCheck ? 1 : 0;
score = -negamax(-beta, -alpha, depth - 1 + extension, ply + 1);
```

### Principal Variation Search (PVS)
- The first move (PV candidate) is searched with a full window `[-beta, -alpha]`.
- All subsequent moves are searched with a zero-window null scout search `[-alpha - 1, -alpha]` with reductions.
- If a scout search beats $\alpha$ (`score > alpha && score < beta`), it is re-searched at full depth and full window.

### Dynamic Null Move Pruning (NMP)
At `depth >= 3`, if not in check, static evaluation $\ge \beta$, and the side to move possesses non-pawn material (`hasNonPawnMaterial`):
```cpp
int R = 2 + depth / 4;
board.makeNullMove();
int nullScore = -negamax(-beta, -beta + 1, depth - 1 - R, ply + 1, false);
board.undoNullMove();
if (nullScore >= beta) return nullScore >= MATE_THRESHOLD ? beta : nullScore;
```

### Reverse Futility Pruning (RFP)
At shallow depths ($\text{depth} \le 3$), if static evaluation minus a margin exceeds $\beta$:
```cpp
if (depth <= 3 && !inCheck && abs(beta) < MATE_THRESHOLD) {
    int margin = 100 * depth;
    if (staticEval - margin >= beta) return beta;
}
```
Prunes nodes immediately without move generation (+34.86 Elo).

### Move-Loop Futility Pruning
For quiet, non-checking moves at $\text{depth} \le 2$:
- If the move is not a killer move, does not give check, and $\text{staticEval} + 100 \times \text{depth} \le \alpha$, the branch is pruned immediately without recursing (+70.44 Elo).

### Late Move Reductions (LMR)
Quiet moves searched late in the move list ($\ge 4$ moves) at depth $\ge 3$ are reduced:
- Reduction = 1 ply for moves $\ge 4$.
- Reduction = 2 plies for moves $\ge 8$ at depth $\ge 5$.
- **Strict Exemptions**: Checks, tactical moves (captures and promotions), killer moves, and advanced pawn pushes to the 7th rank are completely exempt from LMR.
- If a reduced search exceeds $\alpha$, it is re-searched at full depth.

### Move Ordering Hierarchy (11 Tiers)

Moves are scored and sorted via `scoreMove()` using a strict 11-tier hierarchy:

| Priority Score | Move Category | Heuristic Description |
|---|---|---|
| **10,000,000** | TT Hash Move | Best move from Transposition Table entry |
| **200,000+** | Promotions | Queen/underpromotions with victim capture bonus |
| **100,000+** | Good Captures (MVV-LVA) | Captures where $\text{Victim} \ge \text{Attacker}$ |
| **90,000** | Primary Killer Move | Quiet move causing beta cutoff at this ply (slot 0) |
| **85,000** | Counter-Move | Move refuting opponent's previous move (`searchStack[ply-1]`) |
| **82,000** | 7th-Rank Pawn Advance | Pawn push to 7th rank (1 square from promotion) |
| **80,000** | Secondary Killer Move | Quiet move causing beta cutoff at this ply (slot 1) |
| **76,000** | 6th-Rank Pawn Advance | Advanced pawn push to 6th rank |
| **75,000 max** | Castling Move | Castling safety bonus based on game phase and pawn shield |
| **70,000+** | Bad Captures (MVV-LVA) | Captures where $\text{Victim} < \text{Attacker}$ |
| **0 – 65,000** | History Heuristic | Quiet moves rewarded by $\text{depth}^2$ upon beta cutoffs |

### Quiescence Search & Victim-Specific Delta Pruning
To eliminate the horizon effect:
- **Stand-Pat**: Uses static evaluation as a lower bound when not in check.
- **In-Check Handling**: When in check during quiescence, generates all legal evasions to prevent tactical blind spots.
- **Victim-Specific Delta Pruning**:
```cpp
if (standPat + mvvPieceValues[victimType] + 200 < alpha && !move.isPromotion()) {
    continue;
}
```

### Syzygy Search & Root Probing
- **Root Probe**: Optimal moves from DTZ tablebases are played instantly if $\le 5$ pieces remain.
- **Search Tree Probe**: Probes WDL at `depth >= 1` when `halfmoveClock == 0`, returning game-theoretic win/draw/loss bounds.

---

## Universal Chess Interface (UCI) & Time Management

### UCI Commands & Options

ADAM is 100% compliant with the Universal Chess Interface standard:

| Command | Description |
|---|---|
| `uci` | Identifies the engine (`ADAM`, author `Prajwal`) and prints configurable options. |
| `isready` | Synchronizes engine readiness (`readyok`). |
| `ucinewgame` | Clears the Transposition Table and resets state for a new game. |
| `position startpos [moves ...]` | Sets up standard board position and plays move sequence. |
| `position fen <fen> [moves ...]` | Loads custom FEN string and plays move sequence. |
| `go [wtime N] [btime N] [winc N] [binc N] [movetime N] [depth N]` | Launches search with adaptive time controls or fixed limits. |
| `stop` | Aborts ongoing search and prints best move found. |
| `perft <depth>` | Runs perft verification to the specified depth. |
| `divide <depth>` | Runs perft divide diagnostic, printing move-by-move subtree leaf counts. |
| `eval` | Displays detailed static evaluation breakdown in centipawns. |
| `d` | Prints an ASCII representation of the current board state. |
| `quit` | Exits the engine process cleanly. |

#### Configurable Options (`setoption name <Name> value <Value>`)
- `Hash`: Transposition Table size in megabytes (default: `64`, range: `1` to `1048576`).
- `Clear Hash`: Clears all entries in the Transposition Table.
- `Threads`: Number of search threads (default: `1`, range: `1` to `512`).
- `Move Overhead`: Buffer time in milliseconds deducted from clock to prevent flagging (default: `100`, range: `0` to `5000`).
- `SyzygyPath`: Semicolon-delimited directory path to Syzygy tablebase files (default: `Engine/tablebase/wdl;Engine/tablebase/dtz`).
- `Ponder`, `UCI_ShowWDL`, `UCI_Chess960`: Supported standard boolean flags.

### Adaptive 7-Tier Time Management Algorithm

When clock times are provided in `go`, ADAM calculates available time:
$$\text{availableTime} = \max(10, \text{myTime} - \text{moveOverheadMs})$$

Time is allocated based on 7 time brackets:

1. **Classical ($\ge 30\text{ min}$, availableTime $\ge 1,800,000\text{ ms}$)**:
   - $\text{allocated} = (\text{time} / 120) + (\text{inc} \times 3 / 5)$, bounded between $5\text{s}$ and $15\text{s}$.
2. **Long Time Control ($20–30\text{ min}$, $1,200,000 \le \text{time} < 1,800,000\text{ ms}$)**:
   - $\text{allocated} = (\text{time} / 140) + (\text{inc} \times 3 / 5)$, bounded between $3\text{s}$ and $10\text{s}$.
3. **Long Rapid ($13–20\text{ min}$, $800,000 \le \text{time} < 1,200,000\text{ ms}$)**:
   - $\text{allocated} = (\text{time} / 180) + (\text{inc} / 2)$, bounded between $2\text{s}$ and $8\text{s}$.
4. **Standard Rapid ($6.5–13\text{ min}$, $400,000 \le \text{time} < 800,000\text{ ms}$)**:
   - $\text{allocated} = (\text{time} / 100) + (\text{inc} / 2)$, bounded between $1.5\text{s}$ and $6\text{s}$.
5. **Standard Blitz ($1.5–6.5\text{ min}$, $100,000 \le \text{time} < 400,000\text{ ms}$)**:
   - $\text{allocated} = (\text{time} / 90) + (\text{inc} \times 2 / 5)$, bounded between $800\text{ms}$ and $3\text{s}$.
6. **Bullet / Low Clock ($20\text{s}–1.5\text{ min}$, $20,000 \le \text{time} < 100,000\text{ ms}$)**:
   - $\text{allocated} = (\text{time} / 40) + (\text{inc} \times 2 / 5)$, bounded between $300\text{ms}$ and $1.5\text{s}$.
7. **Extreme Scramble ($< 20\text{ seconds}$ remaining)**:
   - $\text{allocated} = (\text{time} / 20) + (\text{inc} / 4)$, capped at $600\text{ms}$ to prevent flagging.

**Hard Safety Floor**: Allocated time is strictly capped at $\text{availableTime} - 20\text{ ms}$ with an absolute minimum of $10\text{ ms}$. For unlimited or correspondence games, ADAM defaults to a deliberate 15-second think time.

---

## Opening Book & Lichess Bot Integration

ADAM features an opening subsystem:

1. **Polyglot Opening Book (`Engine/opening book/book.bin`)**:
   - A 170 MB opening book containing **11.1 Million positions** from high-level grandmaster play.
   - Configured with weighted random selection up to ply 25, ensuring diverse, sound theoretical openings across all major defenses.
2. **Lichess Bot Architecture**:
   - Compatible with `lichess-bot` using standard UCI pipes.
   - Seamless fallback to live Lichess Masters Database when leaving standard book lines.
   - Clean transition to full engine calculation once out of opening book theory.

---

## Automated Testing & Fastchess Match Runner

ADAM includes automated testing scripts in the root directory: [run_test.bat](file:///c:/Users/sriva/OneDrive/Desktop/Adam/run_test.bat) and [run_test.ps1](file:///c:/Users/sriva/OneDrive/Desktop/Adam/run_test.ps1).

### Test Modes (Quick, Standard, SPRT)

Interactive menu options:
- **[1] Quick Sanity Test**: 20 rounds / 40 games at time control `5+0.05` across 6 concurrent threads (~1–2 minutes). Confirms engine stability and basic tactical strength.
- **[2] Standard Match**: 100 rounds / 200 games at time control `10+0.1` across 6 concurrent threads (~10–15 minutes). Verifies measurable Elo gains against reference builds.
- **[3] SPRT Test (Sequential Probability Ratio Test)**:
  - $H_0: \text{Elo}_0 = 0$, $H_1: \text{Elo}_1 = +5$, $\alpha = 0.05$, $\beta = 0.05$ at time control `8+0.08` up to 2,000 rounds. Automatically stops when a statistically significant $+5$ Elo gain is proven.

### Baseline Engine Promotion
- **[4] Save current ADAM.exe as new ADAM_base.exe**: Freezes the current optimized binary as the new benchmark reference for future feature branches.

### Unit Test Suites & Validation
The [Engine/test/](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/test/) directory provides dedicated test runners:
- `movegen.cpp`: Validates strictly legal move generation across complex tactical positions.
- `makemove.cpp` & `undoMoveTest.cpp`: Byte-level state invariance tests ensuring exact reversible symmetry across millions of random moves.
- `test_attacks.cpp` & `test_magic.cpp`: Validates non-sliding and sliding magic bitboard attack tables against naive ray-casters.
- `test_eval.cpp` & `run_eval.cpp`: Unit tests for evaluation terms and balance.
- `mirror_test.py`: Verifies evaluation symmetry across mirrored positions for White and Black.

### Perft Benchmark Suite

Standard starting position move generation counts:

| Depth | Legal Leaf Nodes | NPS Benchmark |
|:---:|:---:|:---:|
| **1** | 20 | > 5,000,000 |
| **2** | 400 | > 5,000,000 |
| **3** | 8,902 | > 5,000,000 |
| **4** | 197,281 | > 4,800,000 |
| **5** | 4,865,609 | > 4,500,000 |
| **6** | 119,060,324 | > 4,200,000 |

```bash
position startpos
perft 5
```

---

## How to Build and Run ADAM

### Compilation Recipe

ADAM is written in standard modern C++20 (fully backwards-compatible with C++17). Compile using `g++` (MinGW-w64 / GCC 10+) with full competitive optimization flags:

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

#### Compilation Flag Explanations
- **`-O3`**: Maximum compiler optimization (vectorization, loop unrolling, and inline expansion).
- **`-march=native`**: Generates CPU-specific instructions for the host processor (`POPCNT`, `BMI2`, `AVX2`).
- **`-flto`**: Enables Link-Time Optimization across all C++ and C translation units.
- **`-DNDEBUG`**: Strips assertion code and debugging overhead for maximum execution speed.
- **`-std=c++20`**: Enables modern C++20 standard features (also compiles seamlessly with `-std=c++17`).
- **`-IEngine -IEngine/syzygy`**: Adds engine root and Syzygy headers to include paths.

### CLI Interactive Session

```bash
./ADAM.exe
uci
isready
position startpos
go depth 8
```

Sample output:
```text
info string KPK Bitbase initialized in 29 retrograde passes.
info string Syzygy tablebases loaded up to 5 pieces.
id name ADAM
id author Prajwal
...
uciok
readyok
info depth 1 score cp 68 nodes 22 nps 22 time 0 pv g1f3
info depth 2 score cp 15 nodes 84 nps 84 time 0 pv g1f3
info depth 3 score cp 65 nodes 389 nps 389 time 0 pv g1f3
info depth 4 score cp 15 nodes 1360 nps 1360 time 0 pv g1f3
info depth 5 score cp 51 nodes 5182 nps 1727333 time 3 pv b1c3
info depth 6 score cp 15 nodes 14610 nps 2087142 time 7 pv b1c3
info depth 7 score cp 52 nodes 42109 nps 2105450 time 20 pv b1c3
info depth 8 score cp 18 nodes 142018 nps 2254253 time 63 pv b1c3
bestmove b1c3
```

---

## Roadmap & Future Directions

- **Multi-Threading (Lazy SMP)**: Parallel alpha-beta search with shared Transposition Table scaling across multi-core systems.
- **Static Exchange Evaluation (SEE)**: Accurate capture pruning in quiescence search and move ordering.
- **Continuation History Heuristics**: Move ordering improvements based on piece-to-square history indexed by previous moves.
- **NNUE Evaluation Architecture**: Dual-perspective halfKP/halfKA efficiently updatable neural network evaluation.

---

## License

This project is created by **Prajwal** for educational, competitive, and research purposes.