# Engine — Architecture & Logic Reference

This document covers the internal architecture, data structures, algorithms, state machine transitions, and design decisions behind Adam's C++20 engine code.

## Scope

This README is specifically for the Engine module of Adam.

- Project-level overview (Frontend, Backend, Engine): [../README.md](file:///c:/Users/sriva/OneDrive/Desktop/Adam/README.md)
- Engine language standard: C++20

---

## Quick Build and Run

From the Adam root directory:

```bash
g++ -std=c++20 -O2 -I Engine Engine/main.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/perft/perft.cpp Engine/uci/uci.cpp Engine/utils/bitboard_utilities.cpp Engine/utils/zobrist.cpp -o ADAM
./ADAM
```

## Quick Test Builds

```bash
# Magic Bitboard Test Suite
g++ -std=c++20 -O2 -I Engine Engine/test/test_magic.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/utils/bitboard_utilities.cpp -o test_magic
./test_magic

# Attack Table Test Suite
g++ -std=c++20 -O2 -I Engine Engine/test/test_attacks.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/utils/bitboard_utilities.cpp -o test_attacks
./test_attacks

# Move Bit-Encoding Test Suite
g++ -std=c++20 -O2 -I Engine Engine/test/moveTester.cpp Engine/moves/move.cpp Engine/utils/bitboard_utilities.cpp -o test_move
./test_move

# Strictly Legal Move Generator Test Suite
g++ -std=c++20 -O2 -I Engine Engine/test/movegen.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/utils/bitboard_utilities.cpp Engine/utils/zobrist.cpp -o test_movegen
./test_movegen

# Board makeMove State Machine Test Suite
g++ -std=c++20 -O2 -I Engine Engine/test/makemove.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/utils/bitboard_utilities.cpp Engine/utils/zobrist.cpp -o test_makemove
./test_makemove

# Make/Undo Round-Trip State Symmetry Test Suite
g++ -std=c++20 -O2 -I Engine Engine/test/undoMoveTest.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/attack/magic_instance.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/utils/bitboard_utilities.cpp Engine/utils/zobrist.cpp -o test_undomove
./test_undomove
```

---

## Table of Contents

- [Directory Layout](#directory-layout)
- [Utils — Types, Bitboards & Zobrist Hashing](#utils--types-bitboards--zobrist-hashing)
- [Board — State Representation, FEN Parsing & Move Execution](#board--state-representation-fen-parsing--move-execution)
- [Attack — Attack Table Generation & Magic Bitboards](#attack--attack-table-generation--magic-bitboards)
- [Moves — Move Encoding & Legal Move Generator](#moves--move-encoding--legal-move-generator)
- [Perft — Tree Validation & Speed Benchmarking](#perft--tree-validation--speed-benchmarking)
- [UCI — Protocol Handler](#uci--protocol-handler)
- [Test — Test Suites & Validation](#test--test-suites--validation)

---

## Directory Layout

```text
Engine/
├── main.cpp                      # CLI & UCI main loop entry point
├── utils/
│   ├── type.h                    # Core type aliases, enums, constants
│   ├── bitboard_utilities.h      # Bitboard operation declarations
│   ├── bitboard_utilities.cpp    # Bitboard operation implementations
│   ├── zobrist.h                 # Zobrist class declaration
│   ├── zobrist.cpp               # 64-bit Zobrist key initialization & hashing
│   ├── magic_numbers.h           # Precalculated 64-bit magic numbers
│   ├── magicGen.h / .cpp         # Magic candidate search algorithm
│   └── magicCreate.cpp           # Offline generator tool entry point
├── board/
│   ├── board.h                   # Board class declaration & inline attack lookups
│   └── board.cpp                 # State machine, makeMove, undoMove, FEN parser
├── attack/
│   ├── attacks.h                 # Attacks class declaration
│   ├── attacks.cpp               # Precomputed non-sliding attack tables
│   ├── magic.h                   # Magic bitboard class declaration
│   ├── magic.cpp                 # Magic table precomputation & attack lookups
│   ├── magic_instance.h          # Global magic instance header
│   └── magic_instance.cpp        # Global magic instance singleton definition
├── moves/
│   ├── move.h                    # 32-bit packed Move class & flag constants
│   ├── move.cpp                  # Move string conversion & helper implementations
│   ├── movegen.h                 # MoveGenerator class & CheckInfo declaration
│   ├── movegen.cpp               # Strictly legal move generation (pins, checks)
│   └── undomove.h                # UndoInfo state snapshot helper structure
├── perft/
│   ├── perft.h                   # Perft class declaration
│   └── perft.cpp                 # Perft tree traversal, divide tool, benchmark
├── uci/
│   ├── uci.h                     # UCI class declaration
│   └── uci.cpp                   # UCI command loop, parser, position & perft handler
└── test/
    ├── test_attacks.cpp          # Attack table validation test suite
    ├── test_magic.cpp            # Magic bitboard lookup test suite
    ├── moveTester.cpp            # 32-bit move encoding round-trip tests
    ├── movegen.cpp               # Strictly legal move generator test suite
    ├── makemove.cpp              # Board makeMove state transition tests
    └── undoMoveTest.cpp          # Make/Undo byte-for-byte round-trip symmetry tests
```

---

## Utils — Types, Bitboards & Zobrist Hashing

### `type.h`

Defines the foundational types, constants, and enumerations used throughout the engine:

| Definition | Type / Value | Description |
|---|---|---|
| `U64` | `uint64_t` | Primary 64-bit bitboard type (1 bit per square) |
| `U32` | `uint32_t` | Compact 32-bit integer for move encoding |
| `BOARD_SIZE` | `64` | Total squares on the chessboard |
| `RANK_SIZE` | `8` | Squares per rank / file |
| `NUM_PIECES` | `12` | 6 piece types × 2 colors |
| `NUM_COLORS` | `3` | `WHITE (0)`, `BLACK (1)`, `BOTH (2)` combined occupancy |
| `MAX_PLYS` | `2048` | Maximum game history depth |
| `MAX_MOVES` | `256` | Maximum move buffer allocation per position |

**Enums:**

- **`Color`**: `WHITE (0)`, `BLACK (1)`, `BOTH (2)`
- **`Piece`**: `EMPTY (-1)`, `WP (0)`, `WN (1)`, `WR (2)`, `WB (3)`, `WQ (4)`, `WK (5)`, `BP (6)`, `BN (7)`, `BR (8)`, `BB (9)`, `BQ (10)`, `BK (11)`
- **`Square`**: `NO_SQUARE (-1)`, `A1 (0)`, `B1 (1)`, ..., `H8 (63)` (Little-Endian Rank-File mapping)
- **`Castling`**: Bitmask flags: `CASTLE_WK = 1`, `CASTLE_WQ = 2`, `CASTLE_BK = 4`, `CASTLE_BQ = 8`

---

### `bitboard_utilities.h / .cpp`

Contains core bit manipulation routines residing in the `Bitboard` namespace:

| Function | Signature | Description |
|---|---|---|
| `getBit` | `bool (U64 bb, int sq)` | Checks if bit at square `sq` is set |
| `setBit` | `void (U64& bb, int sq)` | Sets bit at square `sq` to 1 |
| `clearBit` | `void (U64& bb, int sq)` | Clears bit at square `sq` to 0 |
| `popCount` | `int (U64 bb)` | Counts set bits via compiler builtin `__builtin_popcountll` |
| `lsb` | `int (U64 bb)` | Returns least-significant bit index via `__builtin_ctzll` (`-1` if `bb == 0`) |
| `popLSB` | `int (U64& bb)` | Returns LSB index and clears it in-place (destructive iterator) |
| `printBitboard` | `void (U64 bb)` | Prints ASCII 8×8 grid to stdout for visual debugging |

**Square Index Mapping (Little-Endian Rank-File):**

```text
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

---

### `zobrist.h / .cpp`

Provides fast 64-bit Zobrist hashing for position transpositions, draw detection, and search tree caching.

- **`pieceKeys[12][64]`**: Pseudo-random 64-bit keys for every piece type on every square.
- **`castleKeys[16]`**: 64-bit keys for all 16 castling rights bitmask combinations.
- **`enPassantKeys[8]`**: 64-bit keys for the en-passant target file (0 to 7).
- **`sideKey`**: 64-bit key toggled when it is Black's turn to move.

**Operations:**
- `Zobrist::init()`: Seeds and populates unique non-zero 64-bit keys.
- `Zobrist::generateHash(const Board& board)`: Full non-incremental 64-bit hash calculation from scratch.
- `makeMove` / `undoMove`: Incrementally update `zobristKey` via XOR operations ($\oplus$) for maximum execution speed.

---

## Board — State Representation, FEN Parsing & Move Execution

### Internal State Architecture

The [Board](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/board/board.h) class maintains dual board state representations and full historical undo snapshots:

```text
┌────────────────────────────────────────────────────────────────────────┐
│  bitboards[12]      — U64 bitboard per piece type                     │
│  board[64]          — Mailbox array (Piece enum per square)           │
│  occupancies[3]     — Occupancy bitboards (WHITE, BLACK, BOTH)        │
│  sideToMove         — Color (WHITE or BLACK)                          │
│  castlingRights     — int bitmask (WK=1, WQ=2, BK=4, BQ=8)            │
│  enPassant          — Square (NO_SQUARE if none)                      │
│  kingSquare[2]      — Square of White King [0] and Black King [1]     │
│  halfmoveClock      — int (50-move rule counter)                      │
│  fullmoveNumber     — int (increments after Black's move)              │
│  zobristKey         — U64 (incremental 64-bit Zobrist position hash)  │
│  history[MAX_PLYS]  — Stack of UndoInfo state snapshots               │
│  ply                — int current depth / move history pointer        │
└────────────────────────────────────────────────────────────────────────┘
```

The dual representation ensures **$O(1)$ square piece lookups** via `board[sq]` and **$O(1)$ set bit manipulations** via bitboards.

---

### FEN Parser — `loadFEN()`

Parses FEN (Forsyth-Edwards Notation) strings into board state:
1. **Piece Placement**: Fills `board[64]` and rebuilds all 12 piece bitboards.
2. **Side to Move**: Sets `sideToMove` (`w` $\rightarrow$ `WHITE`, `b` $\rightarrow$ `BLACK`).
3. **Castling Rights**: Parses `K`, `Q`, `k`, `q` or `-`.
4. **En Passant Target**: Parses square coordinates or `-`.
5. **Halfmove Clock & Fullmove Number**: Parses numeric counters.
6. **Atomic Update**: On any validation error, the board reverts atomically without corrupting state.
7. **Zobrist Initialization**: Recomputes `zobristKey` via `Zobrist::generateHash()`.

---

### Attack Lookup — `isSquareAttacked(square, bySide)`

Determines if `square` is under attack by any piece belonging to `bySide`:
- Checks Pawn attacks using precomputed pawn attack masks (`pawnAttacks[opp][square]`).
- Checks Knight attacks using `knightAttacks[square]`.
- Checks King attacks using `kingAttacks[square]`.
- Checks Bishop / Queen diagonal attacks using `g_magic.getBishopAttack(square, occupancies[BOTH])`.
- Checks Rook / Queen orthogonal attacks using `g_magic.getRookAttack(square, occupancies[BOTH])`.

---

### State Machine — `makeMove()` & `undoMove()`

#### `makeMove(const Move& move)`
1. **Snapshot**: Saves current `castlingRights`, `enPassant`, `halfmoveClock`, and `zobristKey` to `history[ply]`.
2. **Piece Removal / Relocation**:
   - Clears piece from `from` square in `board[from]` and `bitboards[movedPiece]`.
   - Handles capture: removes target piece from `board[to]` and `bitboards[capturedPiece]`, updates Zobrist key.
   - Places moved piece on `to` square in `board[to]` and `bitboards[movedPiece]`.
3. **Special Move Logic**:
   - **King Move**: Updates `kingSquare[sideToMove]` tracking array.
   - **Pawn Double Push**: Computes en-passant target square and XORs Zobrist en-passant key.
   - **En Passant Capture**: Removes captured pawn from square behind target square.
   - **Castling**: Relocates corresponding Rook (e.g. White O-O moves Rook from F1 to F1/H1 $\rightarrow$ F1/F1 etc.) and updates bitboards/mailbox/Zobrist.
   - **Promotion**: Replaces moved Pawn on target square with selected promotion piece (`WQ`, `WR`, `WB`, or `WN`).
4. **State Counters & Rights**:
   - Updates castling rights mask if King or Rooks move or are captured.
   - Resets `halfmoveClock` on pawn moves or captures; increments otherwise.
   - Increments `fullmoveNumber` when `sideToMove == BLACK`.
   - Toggles `sideToMove` (`WHITE` $\leftrightarrow$ `BLACK`) and updates Zobrist side key.
5. **Ply Increment**: Increments `ply`. Returns `true`.

#### `undoMove(const Move& move)`
1. **Ply Decrement**: Decrements `ply`.
2. **Restore State Snapshot**: Restores `castlingRights`, `enPassant`, `halfmoveClock`, and `zobristKey` from `history[ply]`.
3. **Reverse Move Execution**:
   - Restores moved piece to `from` square.
   - Restores captured piece (if any) to `to` square or en-passant capture square.
   - Restores castled Rook to original corner square.
   - Reverts King square tracking array if King was moved.
   - Reverts promotion piece back to a Pawn on `from` square.
4. **Rebuilding Occupancies**: Calls `updateOccupancies()` to sync `occupancies[WHITE]`, `occupancies[BLACK]`, and `occupancies[BOTH]`.

---

## Attack — Attack Table Generation & Magic Bitboards

### Non-Sliding Pieces (Precomputed at Construction)

Precomputed during [Attacks](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/attack/attacks.h) initialization for all 64 squares:
- **Knight**: 8 L-shaped offsets stored in `knightAttack[64]`.
- **King**: 8 directional compass offsets stored in `kingAttack[64]`.
- **Pawns**: Diagonal attack masks stored in `whitePawnAttack[64]` and `blackPawnAttack[64]`.

---

### Sliding Pieces — Magic Bitboards (`magic.h / magic.cpp`)

Sliding piece attack lookup (Bishops, Rooks, Queens) runs in $O(1)$ constant time using **Magic Bitboards**:

#### 1. Occupancy Ray Mask Calculation
- **Bishop Mask**: Truncates diagonal rays at board edges (ranks 0/7 and files 0/7) because edge blockers cannot block further rays.
- **Rook Mask**: Truncates orthogonal rays at outer board edges along ray directions.

#### 2. Perfect Magic Hashing Formula
For a given square $s$ and board occupancy $\text{occ}$:

$$\text{masked\_occ} = \text{occ} \ \& \ \text{mask}[s]$$

$$\text{hash} = \frac{\text{masked\_occ} \times \text{magic}[s]}{2^{64 - \text{shift}[s]}}$$

$$\text{attack} = \text{attacks}[s][\text{hash}]$$

#### 3. Global Magic Instance Singleton (`g_magic`)
A single initialized instance of `Magic` (`g_magic`) is accessible globally across the board and move generator modules for zero-overhead lookup table access.

#### 4. Queen Attacks
`getQueenAttack(square, occ)` is computed as:

$$\text{QueenAttacks}(s, \text{occ}) = \text{getBishopAttack}(s, \text{occ}) \mid \text{getRookAttack}(s, \text{occ})$$

---

## Moves — Move Encoding & Legal Move Generator

### Compact Move Encoding (`move.h / move.cpp`)

Each move is bit-packed into a single `U32` integer for minimal cache footprint:

```text
Bit Layout (32-bit Integer):
┌────────────┬────────────┬──────────────┬──────────────┬─────────────┬───────────────┬──────────┐
│  Bits 0-5  │  Bits 6-11 │  Bits 12-15  │  Bits 16-19  │ Bits 20-23  │  Bits 24-27   │ Bits 28+ │
│ From Square│  To Square │  Promotion   │   Move Flag  │ Moved Piece │ Captured Piece│ Reserved │
│   (0-63)   │   (0-63)   │  (Piece+1)   │ (MoveFlag)   │  (Piece+1)  │   (Piece+1)   │          │
└────────────┴────────────┴──────────────┴───────────---┴─────────────┴───────────────┴──────────┘
```

- **`PIECE_OFFSET = 1`**: Maps `EMPTY (-1)` to `0` to prevent negative integer encoding.
- **Move Flags**:
  - `quiet` (0), `capture` (1), `doublePawnPush` (2), `kingSideCastle` (3), `queenSideCastle` (4), `enPassant` (5), `promotion` (6), `promotion_capture` (7).

---

### Undo Snapshot Structure (`undomove.h`)

```cpp
struct UndoInfo {
    int castlingRights;
    Square enpassant;
    int halfMoveClock;
    U64 zobristKey;
};
```

---

### Strictly Legal Move Generator (`movegen.h / movegen.cpp`)

The [MoveGenerator](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/moves/movegen.h) class generates **strictly legal moves**, eliminating illegal moves before they are generated.

#### `CheckInfo` Structure
```cpp
struct CheckInfo {
    U64 checkers = 0;         // Bitmask of opponent pieces delivering check
    U64 checkMask = 0;        // Squares between checker and king (plus checker square)
    U64 pinnedPieces = 0;     // Bitmask of friendly pieces pinned to the king
    int checkerCount = 0;     // Number of checkers (0 = safe, 1 = single check, 2 = double check)
    U64 pinnedRay[64];        // Ray mask along which a pinned piece is allowed to move
};
```

#### Legal Move Generation Flow
1. **Attack Map & Check Detection (`computeAttackMapAndChecks`)**:
   - Computes `enemyAttackMap` by combining opponent piece attack bitboards.
   - Identifies checkers attacking `kingpos` and constructs `checkMask`.
2. **Pin Calculation (`computePins`)**:
   - Computes orthogonal pins (`computeOrthogonalPins`) and diagonal pins (`computeDiagonalPins`).
   - Pinned pieces have their destination target mask AND-ed with `pinnedRay[sq]`, constraining them to stay on the pin line.
3. **Double Check Handling**:
   - If `checkerCount >= 2`, **only King moves are generated**. All non-king moves are suppressed.
4. **Single Check Evasion**:
   - If `checkerCount == 1`, non-king piece moves are restricted to `checkMask` (either capturing the checking piece or blocking the ray).
5. **King Move Safety**:
   - King target squares are filtered with `~friendlyocc` and `~enemyAttackMap`.
   - King steps onto squares under sliding ray attacks are validated using temporary occupancy removal (`iskingattacked`).

---

## Perft — Tree Validation & Speed Benchmarking

The [Perft](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/perft/perft.h) class validates move generator correctness against standard Chess Programming Wiki reference benchmarks and measures move generation throughput.

### Methods

- `Perft::run(int depth)`: Recursively counts all leaf nodes at the specified depth using `makeMove` and `undoMove`.
- `Perft::divide(int depth)`: Prints per-move leaf node breakdown for debugging move generation bugs move-by-move.
- `Perft::benchmark(int depth)`: Measures execution wall-clock time and outputs **Nodes Per Second (NPS)**.

---

## UCI — Protocol Handler

Implemented in [uci.h](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/uci/uci.h) and [uci.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/uci/uci.cpp):

| Command | Engine Response / Action |
|---|---|
| `uci` | Responds with `id name ADAM`, `id author Prajwal`, `uciok` |
| `isready` | Responds with `readyok` |
| `ucinewgame` | Resets board to starting position (`setStartingPosition`) |
| `position startpos [moves ...]` | Loads starting position and executes move sequence |
| `position fen <fen> [moves ...]` | Loads custom FEN position and executes move sequence |
| `perft <depth>` | Runs perft benchmarking runner at depth `<depth>` |
| `quit` | Exits engine process cleanly |

---

## Test — Test Suites & Validation

The `Engine/test/` directory contains complete automated test suites:

1. **`test_magic.cpp`**:
   - Tests magic bitboard mask bit counts and attack table indexing.
   - Verifies 64,000+ random occupancy masks against an On-The-Fly raycaster.
   - Asserts submask invariance and symmetry properties.
2. **`test_attacks.cpp`**:
   - Validates Knight, King, Pawn, Bishop, Rook, and Queen attack masks across edge-cases.
3. **`moveTester.cpp`**:
   - Tests round-trip 32-bit `Move` bitfield encoding, decoding, and flag masks.
4. **`movegen.cpp`**:
   - Verifies strict legal move generation across starting position, kiwipete, en-passant, and pinned position test FENs.
5. **`makemove.cpp`**:
   - Unit tests `Board::makeMove()` state transitions, bitboard updates, capture handling, castling, en-passant, and king updates.
6. **`undoMoveTest.cpp`**:
   - Full byte-level state symmetry test: verifies that applying `makeMove()` followed by `undoMove()` produces a state 100% byte-for-byte identical to the original snapshot.
