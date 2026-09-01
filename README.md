# ADAM — Handcrafted C++ Chess Engine

ADAM is a modern, high-performance, handcrafted UCI-compatible chess engine written from scratch in C++. Built with bitboard representations, precalculated magic bitboards, strictly legal move generation, tapered positional evaluation, and alpha-beta search with iterative deepening.

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
- [Evaluation System](#evaluation-system)
  - [Game Phase & Tapered Evaluation](#game-phase--tapered-evaluation)
  - [Piece-Square Tables (PST)](#piece-square-tables-pst)
  - [Positional & Tactical Evaluation Terms](#positional--tactical-evaluation-terms)
- [Search Engine](#search-engine)
  - [Negamax & Alpha-Beta Pruning](#negamax--alpha-beta-pruning)
  - [Iterative Deepening](#iterative-deepening)
  - [Move Ordering Heuristics](#move-ordering-heuristics)
  - [Quiescence Search & Delta Pruning](#quiescence-search--delta-pruning)
  - [Search Safety & Timeout Protection](#search-safety--timeout-protection)
- [Universal Chess Interface (UCI) & Time Management](#universal-chess-interface-uci--time-management)
- [How to Build, Start, and Operate ADAM](#how-to-build-start-and-operate-adam)
  - [Compilation](#compilation)
  - [Running ADAM in Terminal](#running-adam-in-terminal)
  - [Connecting to GUIs and Lichess-Bot](#connecting-to-guis-and-lichess-bot)
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
- **Tapered Positional Evaluation**: Blends opening/middlegame and endgame piece-square tables with game phase calculation, pawn structure evaluation (passed, isolated, doubled, defended), piece mobility, knight/bishop outposts, open file bonuses, and king safety pawn shields.
- **Alpha-Beta Search**: Negamax search with iterative deepening, check extensions, draw detection (50-move rule and 3-fold repetition), MVV-LVA move ordering, 2-tier killer moves, history heuristic, and quiescence search with delta pruning.
- **Fault-Tolerant Time Allocation**: Dynamic clock management across Bullet, Blitz, Rapid, and Classical time controls with non-destructive timeout handling.

---

## Directory Layout

```text
Adam/
├── ADAM.exe                      # Compiled engine executable
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
    │   ├── eval_tables.h         # PeSTO-style piece-square tables (MG / EG)
    │   └── pawn_table.h          # Pawn structure evaluation heuristics
    ├── hash/                     # Zobrist position hashing & repetition tracking
    │   ├── zobrist.h             # Zobrist keys declaration
    │   └── zobrist.cpp           # 64-bit random key initialization & position hashing
    ├── moves/                    # Move encoding & legal move generation
    │   ├── move.h / .cpp         # 32-bit compact Move class & flag masks
    │   ├── movegen.h / .cpp      # Strictly legal move generator with pin & check masks
    │   └── undomove.h            # UndoInfo state snapshot structure
    ├── perft/                    # Move generation validation & benchmarking
    │   ├── perft.h / .cpp        # Perft tree traversal, divide debugger, NPS benchmark
    │   └── perft_results.h       # Reference perft move counts from standard positions
    ├── search/                   # Alpha-Beta search & move ordering
    │   ├── search.h              # Search class declaration, time controls & heuristics
    │   └── search.cpp            # Negamax, Quiescence, MVV-LVA, Killers, History, Iterative Deepening
    ├── uci/                      # Universal Chess Interface protocol handler
    │   ├── uci.h                 # UCI parser & dispatcher declaration
    │   └── uci.cpp               # UCI commands, adaptive time allocation, go/stop handling
    ├── utils/                    # Bitboard utilities, constants & lookup tools
    │   ├── type.h                # Core types (U64, U32), enums (Piece, Square, Color, MoveFlag)
    │   ├── bitboard_utilities.h / .cpp # Bit manipulation helpers (popCount, popLSB, printBitboard)
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
- Every `makeMove()` call pushes an `UndoInfo` snapshot onto `history[ply]` saving `castlingRights`, `enPassant` target square, `halfmoveClock`, and `zobristKey`.
- Moving pieces, capturing pieces, en-passant removals, castling rook relocations, and promotions update the bitboards, mailbox array, and Zobrist key incrementally via XOR operations.
- `undoMove()` pops the snapshot, restores board state, and synchronizes occupancies, guaranteeing 100% byte-for-byte reversible board transitions.

---

### Zobrist Hashing & Repetition Detection

The `Zobrist` system provides fast 64-bit pseudo-random keys:
- `pieceKeys[12][64]`: Unique key for every piece on every square.
- `castleKeys[16]`: Unique key for each of the 16 castling rights bit combinations.
- `enPassantKeys[8]`: Unique key for the en-passant target file.
- `sideKey`: Key toggled when Black is to move.

During search, `board.isRepetition()` probes the history stack from `ply - halfmoveClock` to detect 3-fold repetition draws in $O(\text{ply})$ time without hash collisions.

---

### Attack Tables & Magic Bitboards

#### 1. Non-Sliding Pieces
- **Knights, Kings, and Pawns** utilize precomputed lookup arrays initialized at startup (`knightAttack[64]`, `kingAttack[64]`, `whitePawnAttack[64]`, `blackPawnAttack[64]`).

#### 2. Sliding Pieces (Magic Bitboards)
Bishop and Rook attacks are resolved in $O(1)$ constant time using Fancy Magic Bitboards:
$$\text{masked\_occ} = \text{occ} \ \& \ \text{mask}[s]$$
$$\text{index} = \frac{\text{masked\_occ} \times \text{magic}[s]}{2^{64 - \text{shift}[s]}}$$
$$\text{attacks} = \text{attackTable}[s][\text{index}]$$

- Queen attacks are calculated via the union of Bishop and Rook attacks:
$$\text{QueenAttacks}(s, \text{occ}) = \text{getBishopAttack}(s, \text{occ}) \mid \text{getRookAttack}(s, \text{occ})$$
- The global magic instance `g_magic` is shared engine-wide to eliminate table duplication.

---

### Strictly Legal Move Generation

Unlike engines that generate pseudo-legal moves and filter them via post-move legality checks, ADAM generates **strictly legal moves up front**:

1. **Check Analysis (`CheckInfo`)**:
   - `checkers`: Bitmask of enemy pieces checking the king.
   - `checkMask`: Bitmask of valid landing squares to resolve check (the checker's square or ray squares between attacker and king).
   - `pinnedPieces` & `pinnedRay[64]`: Identifies pinned friendly pieces and constrains their movement strictly along the pin ray.
2. **Double Check**: If `checkerCount >= 2`, non-king move generation is bypassed entirely; only legal king evasions are computed.
3. **Single Check**: Non-king piece moves are masked with `checkMask` (only blocks or captures).
4. **King Move Validation**: King target squares are filtered against opponent attack maps and validated with simulated occupancy to prevent moving into sliding piece rays.

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

## Evaluation System

ADAM uses a tapered evaluation architecture combining material balance, piece-square tables, and handcrafted positional heuristics.

### Game Phase & Tapered Evaluation

The engine calculates the game phase based on remaining non-pawn material:
- **Phase Scale**: `TotalPhase = 24` (Knight = 1, Bishop = 1, Rook = 2, Queen = 4).
- **Interpolation**: Blends Middle Game (`mgScore`) and End Game (`egScore`):
$$\text{Score} = \frac{\text{mgScore} \times \text{phase} + \text{egScore} \times (24 - \text{phase})}{24}$$

### Piece-Square Tables (PST)
Utilizes PeSTO-derived positional tables tuned separately for opening/middlegame and endgame across all 6 piece types.

### Positional & Tactical Evaluation Terms

1. **Pawn Structure**:
   - **Passed Pawns**: Evaluated with bonuses increasing quadratically by rank, with additional bonuses if passed pawns are protected.
   - **Isolated Pawns**: Penalties for pawns with no friendly pawns on adjacent files.
   - **Doubled Pawns**: Penalties for stacked pawns on the same file.
   - **Pawn Phalanx & Chains**: Bonuses for connected and defending pawn configurations.
2. **Piece Mobility & Placement**:
   - **Knight & Bishop Outposts**: Bonuses for minor pieces placed on protected central squares that cannot be driven away by enemy pawns.
   - **Bishop Pair**: Positional bonus for retaining both bishops.
   - **Rook on Open / Semi-Open Files**: Evaluation bonuses for rooks occupying files with no friendly or enemy pawns.
3. **King Safety & Defense**:
   - Pawn shield evaluations measuring pawn presence in front of the castled king.
   - Penalties for open and semi-open files directly adjacent to the king's square.
4. **Tempo**: Small bonus for the side having the move.

---

## Search Engine

### Negamax & Alpha-Beta Pruning
The search implements fail-soft Alpha-Beta pruning in a Negamax framework with check extensions and early mate detection.

### Iterative Deepening
Search starts at `depth = 1` and increments depth-by-depth up to the requested depth or time limit. The best move found at depth $D-1$ is prioritized at depth $D$, maximizing cutoffs early in search trees.

### Move Ordering Heuristics
Moves are scored and ordered using the following hierarchy:
1. **PV Move**: The best move from the previous iterative deepening iteration receives the highest score (`10000000`).
2. **Capturing Promotions**: Promoted captures (e.g. `exd8=Q`) scored with promotion value + victim value.
3. **Quiet Promotions**: Underpromotions and queen promotions (`200000 + promoValue`).
4. **MVV-LVA (Most Valuable Victim - Least Valuable Attacker)**:
   $$\text{Score} = 100000 + (\text{VictimValue} \times 10 - \text{AttackerValue})$$
5. **Killer Moves**: Up to 2 non-capture killer moves per ply that caused beta cutoffs in sibling branches (`90000` and `80000`).
6. **History Heuristic**: Quiet moves rewarded by `depth^2` when causing beta cutoffs, indexed by `[side][from][to]`.

### Quiescence Search & Delta Pruning
To avoid the horizon effect:
- Quiescence search evaluates noisy tactical moves (captures and promotions) until a stable position is reached.
- **Stand-Pat**: Uses static evaluation as a lower bound when not in check.
- **Delta Pruning**: Prunes captures that cannot raise $\alpha$ even with a generous safety margin ($975\text{ cp}$).
- **Quiet Move Fast Break**: Non-tactical moves are flagged and skipped immediately during selection sort when not in check.

### Search Safety & Timeout Protection
- **Atomic Iteration Commits**: `bestMove` is only updated when a depth iteration completes fully. If a timeout occurs mid-iteration, the engine discards the partial depth and plays the verified best move from the previous completed iteration.
- **Non-Destructive State Unwinding**: Board state is always unmade with `undoMove()` before returning on abort, preventing board corruption.
- **Periodic Polling**: Search checks elapsed time every 1024 nodes to maintain high NPS without timing overhead.

---

## Universal Chess Interface (UCI) & Time Management

ADAM supports the standard UCI protocol:

| Command | Action |
|---|---|
| `uci` | Engine identification (`ADAM`, author `Prajwal`) and option list output. |
| `isready` | Synchronizes engine readiness (`readyok`). |
| `ucinewgame` | Resets internal state, history, and search tables for a new game. |
| `position startpos [moves ...]` | Sets up standard board and plays move list. |
| `position fen <fen> [moves ...]` | Loads custom FEN string and plays move list. |
| `go [wtime N] [btime N] [winc N] [binc N] [movetime N] [depth N]` | Starts search with adaptive time controls or fixed limits. |
| `stop` | Forces immediate search abort and best move output. |
| `perft <depth>` | Runs perft verification at the specified depth. |
| `quit` | Terminates the engine process cleanly. |

### Adaptive Time Allocation
When playing clocked games, ADAM dynamically computes allocated move time based on remaining time, increment, and move overhead:
- **Classical / Long Rapid**: Allocates proportional time with safety ceilings.
- **Blitz / Rapid**: Targets ~1–3s per move with increment utilization.
- **Bullet / Time Scramble**: Fast cutoffs (down to 100–600ms) with hard safety bounds (`availableTime - 20ms`) to prevent flagging.

---

## How to Build, Start, and Operate ADAM

### Compilation

Compile ADAM with any standard C++17 or C++20 compiler (GCC / MinGW / Clang / MSVC).

#### Linux / macOS (GCC / Clang):
```bash
g++ -O3 -std=c++17 -IEngine \
    Engine/main.cpp \
    Engine/board/board.cpp \
    Engine/moves/move.cpp \
    Engine/moves/movegen.cpp \
    Engine/attack/attacks.cpp \
    Engine/attack/magic.cpp \
    Engine/attack/magicGen.cpp \
    Engine/attack/magic_instance.cpp \
    Engine/evaluation/eval.cpp \
    Engine/search/search.cpp \
    Engine/perft/perft.cpp \
    Engine/uci/uci.cpp \
    Engine/hash/zobrist.cpp \
    Engine/utils/bitboard_utilities.cpp \
    Engine/utils/tools.cpp \
    -o ADAM
```

#### Windows (MinGW / MSYS2 / PowerShell):
```powershell
g++ -O3 -std=c++17 -IEngine Engine/main.cpp Engine/board/board.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magicGen.cpp Engine/attack/magic_instance.cpp Engine/evaluation/eval.cpp Engine/search/search.cpp Engine/perft/perft.cpp Engine/uci/uci.cpp Engine/hash/zobrist.cpp Engine/utils/bitboard_utilities.cpp Engine/utils/tools.cpp -o ADAM.exe
```

---

### Running ADAM in Terminal

You can interact directly with ADAM using standard UCI commands:

```bash
./ADAM.exe
uci
isready
position startpos moves e2e4 e7e5
go movetime 1000
```

Sample Engine Output:
```text
info depth 1 score cp 65 nodes 20 nps 20 time 0 pv g1f3
info depth 2 score cp 0 nodes 80 nps 80 time 0 pv g1f3
info depth 3 score cp 62 nodes 650 nps 650 time 0 pv g1f3
info depth 4 score cp 0 nodes 2216 nps 2216 time 0 pv g1f3
info depth 5 score cp 43 nodes 18014 nps 4503500 time 4 pv g1f3
info depth 6 score cp 0 nodes 97334 nps 3244466 time 30 pv g1f3
bestmove g1f3
```

---

### Connecting to GUIs and Lichess-Bot

- **Chess GUIs**: ADAM works out of the box with any UCI-compliant GUI (Arena Chess GUI, Cutechess, BanksiaGUI, Fritz, Nibbler). Simply add `ADAM.exe` as a new UCI engine.
- **Lichess Bot**: In your `config.yml`:
  ```yaml
  engine:
    dir: "./engines/"
    name: "ADAM.exe"
    protocol: "uci"
  ```

---

## Validation & Test Suites

### Perft Validation

Perft (Performance Test) verifies move generation legality and accuracy against known Chess Programming Wiki node counts:

```bash
# In terminal inside UCI
position startpos
perft 5
```

Reference leaf nodes for standard startpos:
- Depth 1: 20
- Depth 2: 400
- Depth 3: 8,902
- Depth 4: 197,281
- Depth 5: 4,865,609
- Depth 6: 119,060,324

---

### Automated Unit Test Suites

Run individual test executables to validate core subsystems:

```bash
# Attack tables & non-sliding vectors
g++ -std=c++17 -O2 -IEngine Engine/test/test_attacks.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/utils/bitboard_utilities.cpp -o test_attacks
./test_attacks

# Magic bitboard mask invariance & ray collisions
g++ -std=c++17 -O2 -IEngine Engine/test/test_magic.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/utils/bitboard_utilities.cpp -o test_magic
./test_magic

# 32-bit move bit-field encoding & decoding
g++ -std=c++17 -O2 -IEngine Engine/test/moveTester.cpp Engine/moves/move.cpp Engine/utils/bitboard_utilities.cpp -o test_move
./test_move

# Strict legal move generator validation
g++ -std=c++17 -O2 -IEngine Engine/test/movegen.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/utils/bitboard_utilities.cpp Engine/hash/zobrist.cpp Engine/utils/tools.cpp -o test_movegen
./test_movegen

# MakeMove state transitions & bitboard synchronization
g++ -std=c++17 -O2 -IEngine Engine/test/makemove.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/utils/bitboard_utilities.cpp Engine/hash/zobrist.cpp Engine/utils/tools.cpp -o test_makemove
./test_makemove

# Make/Undo byte-for-byte round-trip state symmetry
g++ -std=c++17 -O2 -IEngine Engine/test/undoMoveTest.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/utils/bitboard_utilities.cpp Engine/hash/zobrist.cpp Engine/utils/tools.cpp -o test_undomove
./test_undomove
```

---

## Future Goals & Roadmap

The planned evolutionary milestones for ADAM include:

1. **Transposition Table (TT)**:
   - Fixed-size memory hash table with depth-preferred replacement schemes.
   - Exact, Alpha (Upper Bound), and Beta (Lower Bound) entry storage.
   - PV line extraction directly from TT hits.
2. **Advanced Search Pruning & Reductions**:
   - **Principal Variation Search (PVS)**: Zero-window scouting on non-PV nodes.
   - **Null Move Pruning (NMP)**: Dynamic adaptive $R$-value reductions for quick quiet cutoffs.
   - **Late Move Reductions (LMR)**: Depth reductions for quiet moves ordered late in the list.
   - **Reverse Futility Pruning (RFP) / Static Null Move Pruning**.
3. **Automated Parameter Tuning**:
   - Implementation of Texel's Tuning Algorithm or SPSA to optimize evaluation weights against grandmaster game datasets.
4. **Endgame Tablebase Probing**:
   - Syzygy 3-4-5-6 piece WDL and DTM probing integration.
5. **Multi-Threading**:
   - Lazy SMP (Shared Transposition Table) search parallelism.
6. **NNUE Architecture**:
   - Dual-perspective efficiently updatable neural network evaluation (`(768->N)x2 -> 1`).

---

## License

This project is created for educational and personal use.