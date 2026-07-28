# Engine — Architecture & Logic Reference

This document covers the internal architecture, data structures, algorithms, and design decisions behind Adam's C++20 engine code.

## Scope

This README is specifically for the Engine module of Adam.

- Project-level overview (Frontend, Backend, Engine): [../README.md](file:///c:/Users/sriva/OneDrive/Desktop/Adam/README.md)
- Engine language standard: C++20

---

## Quick Build and Run

From the Adam root directory:

```bash
g++ -std=c++20 -O2 -I Engine Engine/main.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/uci/uci.cpp Engine/utils/bitboard_utilities.cpp -o adam
./adam
```

## Quick Test Builds

```bash
# Magic Bitboard Test Suite
g++ -std=c++20 -O2 -I Engine Engine/test/test_magic.cpp Engine/attack/magic.cpp Engine/utils/bitboard_utilities.cpp -o test_magic
./test_magic

# Attack Table Test Suite
g++ -std=c++20 -O2 -I Engine Engine/test/test_attacks.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/utils/bitboard_utilities.cpp -o test_attacks
./test_attacks

# Move Encoding Test Suite
g++ -std=c++20 -O2 -I Engine Engine/test/moveTester.cpp Engine/moves/move.cpp Engine/utils/bitboard_utilities.cpp -o test_move
./test_move
```

---

## Table of Contents

- [Directory Layout](#directory-layout)
- [Utils — Type Definitions & Bitboard Utilities](#utils--type-definitions--bitboard-utilities)
- [Board — State Representation & FEN Parsing](#board--state-representation--fen-parsing)
- [Attack — Attack Table Generation & Magic Bitboards](#attack--attack-table-generation--magic-bitboards)
- [Moves — Move Encoding & Pseudo-Legal Generator](#moves--move-encoding--pseudo-legal-generator)
- [UCI — Protocol Handler](#uci--protocol-handler)
- [Test — Test Suites & Validation](#test--test-suites--validation)

---

## Directory Layout

```
Engine/
├── main.cpp                      # CLI & main loop entry point
├── utils/
│   ├── type.h                    # Core type aliases, enums, constants
│   ├── bitboard_utilities.h      # Bitboard operation declarations
│   ├── bitboard_utilities.cpp    # Bitboard operation implementations
│   ├── magic_numbers.h           # Precalculated 64-bit magic numbers
│   ├── magicGen.h / .cpp         # Magic candidate search algorithm
│   └── magicCreate.cpp           # Generator tool entry point
├── board/
│   ├── board.h                   # Board class declaration
│   └── board.cpp                 # Board state management & FEN parser
├── attack/
│   ├── attacks.h                 # Attacks class declaration
│   ├── attacks.cpp               # Attack table init & sliding piece delegation
│   ├── magic.h                   # Magic bitboard class declaration
│   └── magic.cpp                 # Magic table precomputation & attack lookups
├── moves/
│   ├── move.h                    # Move class & encoding constants
│   ├── move.cpp                  # Move encoding/decoding implementation
│   ├── movegen.h                 # MoveGenerator class declaration
│   ├── movegen.cpp               # Pseudo-legal move generation logic
│   └── undomove.h                # Move undo structure helper
├── uci/
│   ├── uci.h                     # UCI class declaration
│   └── uci.cpp                   # UCI command loop & parser
└── test/
    ├── test_attacks.cpp          # Attack generation test suite
    ├── test_magic.cpp            # Magic bitboard test suite
    ├── moveTester.cpp            # Move encoding round-trip tests
    └── movegen.cpp               # Move generator test suite
```

---

## Utils — Type Definitions & Bitboard Utilities

### `type.h`

Defines the foundational types and enums used across the entire engine.

| Definition | Description |
|---|---|
| `U64` (`uint64_t`) | Primary bitboard type — each bit maps to one board square |
| `U32` (`uint32_t`) | Used for compact move encoding |
| `BOARD_SIZE = 64` | Total squares on the board |
| `RANK_SIZE = 8` | Squares per rank/file |
| `NUM_PIECES = 12` | 6 piece types × 2 colors |
| `NUM_COLORS = 3` | `WHITE`, `BLACK`, `BOTH` (combined occupancy) |

**Enums:**

- **`Color`** — `WHITE (0)`, `BLACK (1)`, `BOTH (2)`
- **`Piece`** — `EMPTY (-1)`, then `WP=0, WN=1, WR=2, WB=3, WQ=4, WK=5, BP=6, BN=7, BR=8, BB=9, BQ=10, BK=11`
- **`Square`** — `NO_SQUARE (-1)`, then `A1=0, B1=1, ... H8=63` (rank-major, A1 = bit 0, H8 = bit 63)
- **`Castling`** — Bitmask flags: `CASTLE_WK=1, CASTLE_WQ=2, CASTLE_BK=4, CASTLE_BQ=8`

### `bitboard_utilities.h / .cpp`

All operations live in the `Bitboard` namespace:

| Function | Signature | Description |
|---|---|---|
| `getBit` | `bool (U64 bb, int sq)` | Returns whether bit `sq` is set |
| `setBit` | `void (U64& bb, int sq)` | Sets bit `sq` to 1 |
| `clearBit` | `void (U64& bb, int sq)` | Clears bit `sq` to 0 |
| `popCount` | `int (U64 bb)` | Population count via `__builtin_popcountll` |
| `lsb` | `int (U64 bb)` | Index of least-significant set bit via `__builtin_ctzll`, returns `-1` if empty |
| `popLSB` | `int (U64& bb)` | Returns and clears the LSB (destructive iteration helper) |
| `printBitboard` | `void (U64 bb)` | Prints an 8×8 grid representation to stdout for debugging |

**Square Mapping Convention:**

```
Bit 0  = A1 (bottom-left)
Bit 7  = H1 (bottom-right)
Bit 56 = A8 (top-left)
Bit 63 = H8 (top-right)

Rank = square / 8
File = square % 8
```

---

## Board — State Representation & FEN Parsing

### Internal State

The `Board` class holds the complete game state:

```
┌─────────────────────────────────────────────────────┐
│  bitboards[12]    — one U64 per piece type           │
│  board[64]        — mailbox array (Piece per square)  │
│  occupancies[3]   — WHITE / BLACK / BOTH occupancy    │
│  sideToMove       — Color (WHITE or BLACK)            │
│  castlingRights   — int bitmask (K=1, Q=2, k=4, q=8) │
│  enPassant        — Square (NO_SQUARE if none)        │
│  halfmoveClock    — int (50-move rule counter)        │
│  fullmoveNumber   — int (increments after Black's move)│
└─────────────────────────────────────────────────────┘
```

The engine maintains **dual representation** — both a bitboard array (for fast bitwise move generation) and a mailbox array (for fast square-to-piece lookup). They are always kept in sync.

### FEN Parser — `loadFEN()`

Parses all six FEN fields with strict validation. On any validation failure, the board state is **not modified** (atomic update pattern using a temp board).

---

## Attack — Attack Table Generation & Magic Bitboards

### Non-Sliding Pieces (Precomputed at Construction)

The `Attacks` constructor precomputes attack tables for all 64 squares for non-sliding piece types:

- **Knight**: 8 L-shaped offsets `{±2, ±1}` and `{±1, ±2}` stored in `knightAttack[64]`.
- **King**: 8 directional offsets stored in `kingAttack[64]`.
- **Pawns**: Diagonal capture masks stored in `whitePawnAttack[64]` and `blackPawnAttack[64]`.

### Sliding Pieces — Magic Bitboards (`magic.h / magic.cpp`)

Sliding piece attacks (Bishops and Rooks) use **Magic Bitboards** for $O(1)$ constant time lookup:

#### 1. Mask Calculation (`bishopMask`, `rookMask`)
- **Bishop Mask**: Truncates diagonal rays at rank/file edges (`0` and `7`) because board border pieces cannot block further sliding moves beyond the board.
- **Rook Mask**: Truncates orthogonal rays at rank/file edges along ray direction.

#### 2. Perfect Magic Hashing
For a square $s$ and occupancy $\text{occ}$:
$$\text{masked\_occ} = \text{occ} \ \& \ \text{mask}[s]$$
$$\text{hash} = \frac{\text{masked\_occ} \times \text{magic}[s]}{2^{64 - \text{shift}[s]}}$$
$$\text{attack} = \text{attacks}[s][\text{hash}]$$

#### 3. Validation (`validate()`)
Upon constructor execution, the `Magic` class validates every entry of all 64 squares against reference On-The-Fly raycasters to guarantee 100% collision-free lookup correctness.

#### 4. Queen Attacks
`getQueenAttack(square, occupancy)` simply returns `getBishopAttack(square, occupancy) | getRookAttack(square, occupancy)`.

---

## Moves — Move Encoding & Pseudo-Legal Generator

### Move Encoding (`move.h / move.cpp`)

Each move is packed into a **single `U32` (32-bit integer)** for memory efficiency:

```
Bit Layout:
┌────────────┬────────────┬──────────────┬──────────┬─────────────┬───────────────┬──────────┐
│  Bits 0-5  │  Bits 6-11 │  Bits 12-15  │ Bits 16-19│ Bits 20-23 │  Bits 24-27   │ Bits 28+ │
│   From     │    To      │  Promotion   │   Flag    │ Moved Piece│ Captured Piece│ Reserved │
│  (Square)  │  (Square)  │  (Piece+1)   │ (MoveFlag)│  (Piece+1) │   (Piece+1)   │          │
└────────────┴────────────┴──────────────┴──────────┴─────────────┴───────────────┴──────────┘
```

- **PIECE_OFFSET = 1** — `EMPTY (-1)` maps to `0` to avoid negative values in unsigned fields.
- **Move Flags**: `quiet` (0), `capture` (1), `doublePawnPush` (2), `kingSideCastle` (3), `queenSideCastle` (4), `enPassant` (5), `promotion` (6), `promotion_capture` (7).

### Move Generator (`movegen.h / movegen.cpp`)

Generates pseudo-legal moves for the side to move:
- **Knight & King**: Iterates bitboard pieces, fetches attack table, filters out friendly occupied squares. Handles O-O and O-O-O castling availability.
- **Pawns**: Generates single pushes, double pushes from starting rank, diagonal captures, en passant, and promotion choices (Queen, Rook, Bishop, Knight).
- **Sliders**: Calls `getBishopAttack`, `getRookAttack`, or `getQueenAttack` with combined occupancy bitboards and filters out friendly pieces.

---

## UCI — Protocol Handler

Implemented in [uci.h](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/uci/uci.h) / [uci.cpp](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/uci/uci.cpp):
- `uci` $\rightarrow$ `id name ADAM`, `id author Prajwal`, `uciok`
- `isready` $\rightarrow$ `readyok`
- `ucinewgame` $\rightarrow$ resets board state
- `position fen <fen>` $\rightarrow$ parses FEN string and loads board
- `quit` $\rightarrow$ exits application

---

## Test — Test Suites & Validation

The `Engine/test/` directory contains unit test suites for verifying core engine subsystems:

1. **`test_magic.cpp`**: Comprehensive Magic Bitboard verification (mask bit counts, attack lookups, submask invariance, 64,000+ random occupancy comparisons vs OTF, symmetry, self-attack checks).
2. **`test_attacks.cpp`**: Non-sliding and sliding piece attack table validation.
3. **`moveTester.cpp`**: 32-bit move encoding round-trip bitfield integrity tests.
4. **`movegen.cpp`**: Pseudo-legal move generator test runner.
