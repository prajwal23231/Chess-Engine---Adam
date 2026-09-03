# ADAM — Handcrafted C++ Chess Engine

ADAM is a modern, high-performance, handcrafted UCI-compliant chess engine written from scratch in C++. Built with bitboard representations, precalculated magic bitboards, strictly legal move generation, tapered positional evaluation, and alpha-beta search enhanced with a 64 MB Transposition Table, check extensions, late move reductions, and iterative deepening.

**Author**: Prajwal

---

## Table of Contents

- [Overview & Architectural Highlights](#overview--architectural-highlights)
- [Directory Layout](#directory-layout)
- [Core Engine Architecture](#core-engine-architecture)
  - [Board Representation & State Machine](#board-representation--state-machine)
  - [Zobrist Hashing & Repetition Detection](#zobrist-hashing--repetition-detection)
  - [Attack Tables & Magic Bitboards](#attack-tables--magic-bitboards)
  - [Strictly Legal Move Generation](#strictly-legal-move-generation)
  - [Compact 32-bit Move Encoding](#compact-32-bit-move-encoding)
- [Transposition Table (TT) Subsystem](#transposition-table-tt-subsystem)
  - [Architecture & Memory Layout](#architecture--memory-layout)
  - [Fast Bitwise Indexing & Collision Safety](#fast-bitwise-indexing--collision-safety)
  - [Bound Flags & Replacement Scheme](#bound-flags--replacement-scheme)
  - [Root-Independent Mate Score Normalization](#root-independent-mate-score-normalization)
  - [Hash Move Ordering Synergy](#hash-move-ordering-synergy)
- [Evaluation System](#evaluation-system)
  - [Game Phase & Tapered Evaluation](#game-phase--tapered-evaluation)
  - [Piece-Square Tables (PST)](#piece-square-tables-pst)
  - [Passed Pawn Endgame Dynamics](#passed-pawn-endgame-dynamics)
  - [Positional & Tactical Evaluation Terms](#positional--tactical-evaluation-terms)
  - [King Safety & Flank Shield Evaluation](#king-safety--flank-shield-evaluation)
  - [Endgame King Cornering & Mop-Up Evaluation](#endgame-king-cornering--mop-up-evaluation)
- [Search Engine](#search-engine)
  - [Negamax & Alpha-Beta Pruning](#negamax--alpha-beta-pruning)
  - [Iterative Deepening](#iterative-deepening)
  - [Check Extensions](#check-extensions)
  - [Reverse Futility Pruning (RFP)](#reverse-futility-pruning-rfp)
  - [Late Move Reductions (LMR)](#late-move-reductions-lmr)
  - [Move Ordering Hierarchy](#move-ordering-hierarchy)
  - [Quiescence Search & Delta Pruning](#quiescence-search--delta-pruning)
  - [Search Safety & Timeout Protection](#search-safety--timeout-protection)
- [Universal Chess Interface (UCI) & Time Management](#universal-chess-interface-uci--time-management)
- [How to Build, Start, and Operate ADAM](#how-to-build-start-and-operate-adam)
  - [Optimized Compilation Flags](#optimized-compilation-flags)
  - [Running ADAM in Terminal](#running-adam-in-terminal)
  - [Lichess Bot & Opening Book Integration](#lichess-bot--opening-book-integration)
- [Validation & Test Suites](#validation--test-suites)
  - [Perft Validation](#perft-validation)
  - [Automated Unit Test Suites](#automated-unit-test-suites)
- [Future Goals & Roadmap](#future-goals--roadmap)
- [License](#license)

---

## Overview & Architectural Highlights

- **Dual Board Representation**: 12 piece bitboards + 3 occupancy bitboards combined with an 8×8 mailbox array for $O(1)$ square queries and fast bitwise transformations.
- **Magic Bitboards**: Constant-time $O(1)$ sliding attack lookups for Rooks, Bishops, and Queens using 64-bit magic multiplications and precomputed occupancy tables.
- **Strictly Legal Move Generator**: Direct pin-ray mask calculation and check-mask computation before move construction, guaranteeing zero illegal moves are generated or tested.
- **64 MB Transposition Table**: Full 64-bit Zobrist key collision verification, power-of-2 fast bitmask indexing, depth-preferred replacement, root-relative mate score normalization, and top-priority hash move ordering (`10,000,000` priority).
- **Tapered Positional Evaluation**: Blends opening/middlegame and endgame piece-square tables with game phase calculation, heavy passed pawn endgame scaling, connected passers, unblocked marching bonuses, minor piece outposts, open file bonuses, and cornering mop-up evaluation.
- **Alpha-Beta Search**: Negamax search with iterative deepening, check extensions, Reverse Futility Pruning (RFP), Late Move Reductions (LMR), dynamic draw repetition detection, MVV-LVA, 2-tier killer moves, history heuristic, and quiescence search with delta pruning.
- **High-Throughput Performance**: Reaches **2.2+ Million nodes per second (NPS)** on modern single-core execution with depth 8 completing in $\approx 100\text{ ms}$.

---

## Directory Layout

```text
Adam/
├── ADAM.exe                      # Fully optimized compiled binary (-O3 -march=native -flto)
├── README.md                     # Complete engine architecture and operations guide
├── assests/                      # Chess piece SVG vector assets
└── Engine/                       # Engine Source Code
    ├── main.cpp                  # Program entry point (initializes lookup tables and UCI loop)
    ├── attack/                   # Attack tables & Magic Bitboard subsystem
    │   ├── attacks.h / .cpp      # Precomputed non-sliding (Pawn, Knight, King) attack tables
    │   ├── magic.h / .cpp        # Magic bitboard generator & constant-time sliding attack lookups
    │   ├── magicGen.h / .cpp     # Monte Carlo magic candidate search algorithm
    │   ├── magic_instance.h / .cpp # Global magic singleton instance (`g_magic`)
    │   └── magicCreate.cpp       # Standalone generator utility for magic numbers
    ├── board/                    # Board state representation & state machine
    │   ├── board.h               # Board class declaration & inline state queries
    │   └── board.cpp             # Make/undo state transitions, occupancies, FEN parser
    ├── evaluation/               # Positional evaluation subsystem
    │   ├── eval.h / .cpp         # Tapered evaluation engine, phase calculation, positional terms
    │   └── pawn_table.h          # Dedicated Pawn Hash Table (caches pawn structure by pawnKey)
    ├── hash/                     # Zobrist position hashing & Transposition Table
    │   ├── zobrist.h / .cpp      # 64-bit pseudo-random key generation & position hashing
    │   ├── tt.h                  # Transposition table declarations, TTEntry, TTFlag, mate math
    │   └── tt.cpp                # TT allocation, power-of-2 masking, probe and store methods
    ├── moves/                    # Move encoding & legal move generation
    │   ├── move.h / .cpp         # 32-bit compact Move class & flag masks
    │   ├── movegen.h / .cpp      # Strictly legal move generator with pin & check masks
    │   └── undomove.h            # UndoInfo state snapshot structure
    ├── perft/                    # Move generation validation & benchmarking
    │   ├── perft.h / .cpp        # Perft tree traversal, divide debugger, NPS benchmark
    │   └── perft_results.h       # Reference perft move counts from standard positions
    ├── search/                   # Alpha-Beta search & move ordering
    │   ├── search.h              # Search class declaration, time controls & heuristics
    │   └── search.cpp            # Negamax, TT probe/store, LMR, RFP, MVV-LVA, Killers, History
    ├── uci/                      # Universal Chess Interface protocol handler
    │   ├── uci.h                 # UCI parser & dispatcher declaration
    │   └── uci.cpp               # UCI commands, adaptive time allocation, Hash options
    ├── utils/                    # Bitboard utilities, constants & lookup tools
    │   ├── type.h                # Core types (U64, U32), enums, evaluation constants
    │   ├── bitboard_utilities.h / .cpp # Bit manipulation helpers (popCount, popLSB)
    │   ├── magic_numbers.h       # Precomputed 64-bit magic numbers for all 64 squares
    │   └── tools.h / .cpp        # Geometric ray masks (between, line, ray, outpost masks)
    └── test/                     # Automated unit test suites
        ├── test_attacks.cpp      # Non-sliding & sliding attack validation
        ├── test_magic.cpp        # Magic bitboard mask & collision invariance tests
        ├── moveTester.cpp        # 32-bit move encoding/decoding verification
        ├── movegen.cpp           # Strict legal move generation test runner
        ├── makemove.cpp          # Board makeMove state transition tests
        ├── undoMoveTest.cpp      # Byte-level make/undo state symmetry verification
        └── test_eval.cpp         # Evaluation term unit testing
```

---

## Core Engine Architecture

### Board Representation & State Machine

The engine maintains a dual representation inside `Board`:
1. **12 Piece Bitboards** (`bitboards[12]`): 64-bit unsigned integers representing individual piece locations (`WP`, `WN`, `WR`, `WB`, `WQ`, `WK`, `BP`, `BN`, `BR`, `BB`, `BQ`, `BK`).
2. **3 Occupancy Bitboards** (`occupancies[3]`): Combined occupancy for `WHITE`, `BLACK`, and `BOTH`.
3. **Mailbox Array** (`board[64]`): 1D array mapping square indices (0–63) to `Piece` enums for $O(1)$ piece-at-square lookups.

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

#### State Transitions (`makeMove` & `undoMove`)
- Every `makeMove()` call pushes an `UndoInfo` snapshot onto `history[ply]` saving `castlingRights`, `enPassant` target square, `halfmoveClock`, `zobristKey`, `pawnKey`, and positional scores.
- Moving pieces, capturing pieces, en-passant removals, castling rook relocations, and promotions update the bitboards, mailbox array, and Zobrist key incrementally via XOR operations.
- `undoMove()` pops the snapshot, restores board state, and synchronizes occupancies, guaranteeing 100% byte-for-byte reversible board transitions.

---

### Zobrist Hashing & Repetition Detection

The `Zobrist` system provides fast 64-bit pseudo-random keys:
- `pieceKeys[12][64]`: Unique key for every piece on every square.
- `castleKeys[16]`: Unique key for each of the 16 castling rights bit combinations.
- `enPassantKeys[8]`: Unique key for the en-passant target file.
- `sideKey`: Key toggled when Black is to move.

#### Dynamic Repetition Strategy
During search, `board.isRepetition()` probes the history stack from `ply - halfmoveClock` to detect repeated positions. Repetition is evaluated as exactly **0 centipawns (Draw)**:
* **When Losing**: If the engine is down material (e.g. -400 cp), a 0-score repetition is superior to all alternatives, causing ADAM to aggressively force a draw (e.g. perpetual check).
* **When Winning**: If the engine is ahead (e.g. +400 cp), a 0-score repetition represents an unacceptable concession, ensuring ADAM strictly avoids repeats and plays for a win.

---

### Attack Tables & Magic Bitboards

#### 1. Non-Sliding Pieces
Knights, Kings, and Pawns utilize precomputed lookup arrays initialized at startup (`knightAttack[64]`, `kingAttack[64]`, `whitePawnAttack[64]`, `blackPawnAttack[64]`).

#### 2. Sliding Pieces (Magic Bitboards)
Bishop and Rook attacks are resolved in $O(1)$ constant time using Fancy Magic Bitboards:
$$\text{masked\_occ} = \text{occ} \ \& \ \text{mask}[s]$$
$$\text{index} = \frac{\text{masked\_occ} \times \text{magic}[s]}{2^{64 - \text{shift}[s]}}$$
$$\text{attacks} = \text{attackTable}[s][\text{index}]$$

- Queen attacks are calculated via the union of Bishop and Rook attacks:
$$\text{QueenAttacks}(s, \text{occ}) = \text{getBishopAttack}(s, \text{occ}) \mid \text{getRookAttack}(s, \text{occ})$$

---

### Strictly Legal Move Generation

ADAM generates **strictly legal moves up front**, avoiding pseudo-legal filtering overhead:
1. **Check Analysis (`CheckInfo`)**: Identifies attackers checking the king (`checkMask`) and calculates pin rays (`pinnedPieces`, `pinnedRay[64]`).
2. **Double Check**: If `checkerCount >= 2`, non-king move generation is bypassed entirely; only legal king evasions are computed.
3. **Single Check**: Non-king piece moves are masked with `checkMask` (only blocks or captures).
4. **King Move Validation**: King target squares are filtered against opponent attack maps and validated with simulated occupancy.

---

### Compact 32-bit Move Encoding

Every move is stored in a 32-bit unsigned integer (`Move` class):

```text
┌────────────┬────────────┬──────────────┬──────────────┬─────────────┬───────────────┬──────────┐
│  Bits 0-5  │  Bits 6-11 │  Bits 12-15  │  Bits 16-19  │ Bits 20-23  │  Bits 24-27   │ Bits 28+ │
│ From Square│  To Square │  Promotion   │   Move Flag  │ Moved Piece │ Captured Piece│ Reserved │
│   (0-63)   │   (0-63)   │  (Piece+1)   │ (MoveFlag)   │  (Piece+1)  │   (Piece+1)   │          │
└────────────┴────────────┴──────────────┴──────────────┴─────────────┴───────────────┴──────────┘
```

Flags include: `quiet`, `capture`, `doublePawnPush`, `kingSideCastle`, `queenSideCastle`, `enPassant`, `promotion`, and `promotion_capture`.

---

## Transposition Table (TT) Subsystem

ADAM incorporates a full-featured Transposition Table implemented in [Engine/hash/tt.h](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/hash/tt.h) and [Engine/hash/tt.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/hash/tt.cpp).

### Architecture & Memory Layout

Each entry in the table occupies a compact structure:
```cpp
struct TTEntry {
    U64 key;          // Full 64-bit Zobrist key for collision verification
    int score;        // Minimax evaluation or bound
    int depth;        // Depth of the searched subtree
    TTFlag flag;      // TT_EXACT, TT_LOWER, or TT_UPPER
    Move bestMove;    // Principal variation / best cutoff move (Hash Move)
};
```

### Fast Bitwise Indexing & Collision Safety
- **Power-of-2 Sizing**: Allocated memory is rounded down to the nearest power of 2 ($2^N$ entries).
- **Single-Cycle Masking**: Index lookup uses `key & mask` instead of modulo division (`key % size`), executing in 1 CPU cycle.
- **Full Key Verification**: When probing, `entry.key == key` verifies that the entry belongs to the current position, preventing hash collisions from corrupting search.

### Bound Flags & Replacement Scheme
1. **`TT_EXACT`**: Score was strictly between $\alpha$ and $\beta$ (true minimax score).
2. **`TT_LOWER`**: Beta cutoff occurred; the position is at least this good ($\ge \beta$).
3. **`TT_UPPER`**: All moves failed low; the position is at most this good ($\le \alpha$).

**Replacement Rule**: An entry is overwritten if a new position collides into the slot, if the new search depth is greater than or equal to the stored depth (`depth >= entry.depth`), or if the new result is an exact PV score (`flag == TT_EXACT`).

### Root-Independent Mate Score Normalization
To prevent mate-in-N scores from drifting across different depths and plies:
* **Storing**: `scoreToTT()` converts root-relative scores to position-independent scores (`score + ply`).
* **Probing**: `scoreFromTT()` converts stored scores back to current-root distance (`score - ply`).

### Hash Move Ordering Synergy
Even when a stored entry was searched to a shallower depth and cannot provide an immediate score cutoff, its `bestMove` is extracted and given a top-priority score of **`10,000,000`** in `scoreMove()`. Searching the proven best move first yields immediate beta cutoffs, cutting depth 8 search times down to **109 ms** (a $23\times$ speedup).

---

## Evaluation System

ADAM uses a tapered evaluation architecture combining material balance, piece-square tables, and handcrafted positional heuristics.

### Game Phase & Tapered Evaluation

The engine calculates the game phase based on remaining non-pawn material:
- **Phase Scale**: `TotalPhase = 24` (Knight = 1, Bishop = 1, Rook = 2, Queen = 4).
- **Interpolation**: Blends Middle Game (`mgScore`) and End Game (`egScore`):
$$\text{Score} = \frac{\text{mgScore} \times \text{phase} + \text{egScore} \times (24 - \text{phase})}{24}$$

### Piece-Square Tables (PST)
Utilizes PeSTO-derived positional tables tuned separately for opening/middlegame and endgame across all 6 piece types.

### Passed Pawn Endgame Dynamics
Passed pawns are scaled aggressively in the endgame to reflect promotion urgency:
* **Rank 4**: +60 cp
* **Rank 5**: +110 cp
* **Rank 6**: +200 cp (equivalent to 2 full pawns)
* **Rank 7**: +350 cp (equivalent to a full minor piece)
* **Connected Passers**: Scaled dynamically up to +300 cp in the endgame.
* **Unblocked Marching Bonus**: Additional bonuses when the stop square is unoccupied for 5th- and 6th-rank passers (`(rank - 3) * 20`).

### Positional & Tactical Evaluation Terms
1. **Pawn Structure & Dedicated Pawn Table**:
   - **Pawn Hash Table (`PawnTable`)**: Caches pawn structure evaluations by `pawnKey` to avoid recalculating static pawn structure.
   - **Isolated Pawns**: Penalties for pawns with no friendly pawns on adjacent files.
   - **Doubled Pawns**: Penalties for stacked pawns on the same file.
   - **Pawn Phalanx & Chains**: Bonuses for connected and defending pawn configurations.
2. **Piece Mobility & Placement**:
   - **Bishop Pair**: Positional bonus (+25 MG, +50 EG) for retaining both bishops.
   - **Bad & Trapped Bishops**: Penalizes bishops blocked by friendly pawns on the same color, with explicit trap detection for cornered bishops on `b3`/`b6`.
   - **Minor Piece Outposts**: Protected central squares that cannot be driven away by enemy pawns.
   - **Rook on Open / Semi-Open Files**: Bonuses for rooks on files with no friendly or enemy pawns.
   - **Rook on 7th Rank**: +15 MG / +25 EG bonus when cutting off the enemy king or attacking baseline pawns.
   - **Connected Rooks**: Bonuses for rooks defending each other horizontally or vertically.
3. **Tactical Vulnerability & Material Scaling**:
   - **Hanging Pieces**: Detection and penalties for undefended friendly pieces under enemy attack.
   - **Material Draw Scaling (`getMaterialScaleFactor`)**: Detects theoretical draws (e.g. 2 Knights vs lone King scaled to 0; opposite-colored bishop endgames with $\le 2$ pawns scaled by 50%).

### King Safety & Flank Shield Evaluation
* Rescaled `kingDangerTable` capped at realistic curves to prevent unsound sacrifices.
* Pawn shield evaluation strictly constrained to flank/castled files (A, B, C or F, G, H), preventing uncastled central pawn trades from artificially deflating king safety.

### Endgame King Cornering & Mop-Up Evaluation
When a side possesses decisive endgame material (e.g. $K+R \text{ vs } K$ or $K+Q \text{ vs } K$), `calculateMatingScore()` activates:
$$\text{Bonus} = \text{pushToEdge}(\text{LosingKing}) + \text{closeIn}(\text{WinningKing}, \text{LosingKing})$$
This forces the opponent's bare king toward corner squares and brings the friendly king into mating proximity, completely eliminating aimless endgame shuffling.

---

## Search Engine

### Negamax & Alpha-Beta Pruning
The search implements fail-soft Alpha-Beta pruning in a Negamax framework with check extensions, TT probing/storing, and early mate detection.

### Iterative Deepening
Search starts at `depth = 1` and increments depth-by-depth. The best move found at depth $D-1$ is prioritized at depth $D$ via the Transposition Table, maximizing branch cutoffs early.

### Check Extensions
When the moving side is in check (`inCheck == true`), search depth is automatically extended by **+1 ply**:
```cpp
int extension = inCheck ? 1 : 0;
score = -negamax(-beta, -alpha, depth - 1 + extension, ply + 1);
```
This eliminates the horizon effect during forcing check sequences.

### Reverse Futility Pruning (RFP)
At shallow depths ($d \le 2$), if the static evaluation minus a safety margin exceeds $\beta$:
```cpp
if (depth <= 2 && !inCheck && abs(beta) < MATE_THRESHOLD) {
    if (eval - 120 * depth >= beta) return beta;
}
```
The node is pruned immediately without move generation.

### Late Move Reductions (LMR)
Quiet moves searched late in the move list ($\ge 4$ moves) at depth $\ge 3$ are searched with a reduced depth ($-1$ ply). If the move refutes the reduction ($score > \alpha$), it is re-searched at full depth.

### Move Ordering Hierarchy
Moves are sorted using a strict efficiency hierarchy:
1. **TT Hash Move**: Best move from the Transposition Table (`10,000,000`).
2. **Capturing Promotions**: Promoted captures (`200,000 + promo + victim`).
3. **Quiet Promotions**: Underpromotions and queen promotions (`200,000 + promo`).
4. **Captures (MVV-LVA)**: Most Valuable Victim - Least Valuable Attacker (`100000 + victim * 10 - attacker`).
5. **Killer Moves**: Up to 2 quiet moves per ply that caused beta cutoffs in sibling branches (`90,000` and `80,000`).
6. **Castling & Pawn Shield**: Evaluated based on castling safety.
7. **History Heuristic**: Quiet moves rewarded by `depth^2` when causing beta cutoffs (`0–70,000`).

### Quiescence Search & Delta Pruning
To avoid the horizon effect:
- Evaluates noisy tactical moves (captures and promotions) until a quiet, non-tactical position is reached.
- **Stand-Pat**: Uses static evaluation as a lower bound when not in check.
- **Delta Pruning**: Prunes captures that cannot raise $\alpha$ even with a piece value margin.

---

## Universal Chess Interface (UCI) & Time Management

ADAM supports the full standard UCI protocol:

| Command | Action |
|---|---|
| `uci` | Identifies engine (`ADAM`, author `Prajwal`) and outputs configurable options. |
| `isready` | Synchronizes engine readiness (`readyok`). |
| `ucinewgame` | Clears the Transposition Table and resets history for a fresh game. |
| `setoption name Hash value <MB>` | Dynamically resizes the Transposition Table. |
| `setoption name Clear Hash` | Wipes the Transposition Table clean. |
| `position startpos [moves ...]` | Sets up standard board and plays move list. |
| `position fen <fen> [moves ...]` | Loads custom FEN string and plays move list. |
| `go [wtime N] [btime N] [winc N] [binc N] [movetime N] [depth N]` | Starts search with adaptive time controls or fixed limits. |
| `stop` | Forces immediate search abort and best move output. |
| `perft <depth>` | Runs perft verification at the specified depth. |
| `divide <depth>` | Runs perft divide diagnostic, printing leaf counts per legal root move. |
| `eval` | Outputs the static evaluation breakdown in centipawns. |
| `d` | Displays an ASCII representation of the current board state. |
| `quit` | Terminates the engine process cleanly. |

---

## How to Build, Start, and Operate ADAM

### Optimized Compilation Flags

Compile ADAM using standard C++17 with full competitive optimization flags:

```bash
g++ -O3 -march=native -flto -DNDEBUG -std=c++17 -IEngine \
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
    -o ADAM.exe
```

* **`-O3`**: Maximum aggressive optimization (vectorization, unrolling, function inlining).
* **`-march=native`**: Enables native CPU instruction set extensions (`POPCNT`, `BMI2`, `AVX2`).
* **`-flto`**: Link-Time Optimization across all translation units.
* **`-DNDEBUG`**: Disables all assertion overhead for maximum speed.

---

### Running ADAM in Terminal

```bash
./ADAM.exe
uci
isready
position startpos
go depth 8
```

Sample Benchmark Output:
```text
info depth 1 score cp 83 nodes 20 nps 20 time 0 pv g1f3
info depth 2 score cp 0 nodes 80 nps 80 time 0 pv g1f3
info depth 3 score cp 80 nodes 629 nps 629 time 0 pv g1f3
info depth 4 score cp 0 nodes 1893 nps 1893000 time 1 pv g1f3
info depth 5 score cp 66 nodes 7974 nps 1594800 time 5 pv d2d4
info depth 6 score cp 0 nodes 30566 nps 1698111 time 18 pv d2d4
info depth 7 score cp 66 nodes 68801 nps 1638119 time 42 pv d2d4
info depth 8 score cp 20 nodes 248979 nps 2284211 time 109 pv d2d4
bestmove d2d4
```

---

### Lichess Bot & Opening Book Integration

ADAM is integrated with [lichess-bot](file:///c:/Users/sriva/OneDrive/Desktop/Lichess%20Bot/lichess-bot/config.yml) featuring a dual-layer opening system:
1. **Local Polyglot Opening Book (`book.bin`)**: 170 MB grandmaster book containing **11.1 million positions**, configured with `selection: weighted_random` and `max_depth: 25` to provide diverse, sound opening play across all variations.
2. **Cloud Opening Explorer**: Seamless fallback to the live Lichess Masters Database if a game reaches an uncommon sideline.
3. **Midgame Transition**: Cleanly hands off positions to `ADAM.exe` once out of theory.

---

## Validation & Test Suites

### Perft Validation

```bash
position startpos
perft 5
```

Reference leaf nodes:
- Depth 1: 20
- Depth 2: 400
- Depth 3: 8,902
- Depth 4: 197,281
- Depth 5: 4,865,609
- Depth 6: 119,060,324

---

## Future Goals & Roadmap

1. **Null Move Pruning (NMP)**: Dynamic adaptive $R$-value reductions for non-zugzwang quiet cutoffs.
2. **Principal Variation Search (PVS)**: Zero-window scouting on non-PV moves.
3. **Static Exchange Evaluation (SEE)**: Accurate capture pruning in quiescence and move ordering.
4. **Endgame Tablebase Probing**: Syzygy 3-4-5 piece WDL/DTZ integration.
5. **Multi-Threading**: Lazy SMP shared transposition table parallelism.
6. **NNUE Architecture**: Dual-perspective efficiently updatable neural network evaluation.

---

## License

This project is created for educational and personal use.