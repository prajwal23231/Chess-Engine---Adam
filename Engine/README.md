# Engine — Architecture & Logic Reference

This document covers the internal architecture, data structures, algorithms, and design decisions behind Adam's engine code.

## Scope

This README is specifically for the Engine module of Adam.

- Project-level overview (Frontend, Backend, Engine): `../README.md`
- Engine language standard: C++20

## Quick Build and Run

From the Adam root directory:

```bash
g++ -std=c++20 -O2 -I Engine Engine/main.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/moves/move.cpp Engine/moves/moveGenerator.cpp Engine/uci/uci.cpp Engine/utils/bitboard_utilities.cpp -o adam
./adam
```

## Quick Test Build

Examples:

```bash
g++ -std=c++20 -O2 -I Engine Engine/test/test_attacks.cpp Engine/attack/attacks.cpp Engine/utils/bitboard_utilities.cpp -o test_attacks
./test_attacks
```

---

## Table of Contents

- [Directory Layout](#directory-layout)
- [Utils — Type Definitions & Bitboard Utilities](#utils--type-definitions--bitboard-utilities)
- [Board — State Representation & FEN Parsing](#board--state-representation--fen-parsing)
- [Attack — Attack Table Generation](#attack--attack-table-generation)
- [Moves — Move Encoding & Generation](#moves--move-encoding--generation)
- [UCI — Protocol Handler](#uci--protocol-handler)
- [Test — Test Suites](#test--test-suites)

---

## Directory Layout

```
Engine/
├── main.cpp
├── utils/
│   ├── type.h                    # Core type aliases, enums, constants
│   ├── bitboard_utilities.h      # Bitboard operation declarations
│   └── bitboard_utilities.cpp    # Bitboard operation implementations
├── board/
│   ├── board.h                   # Board class declaration
│   └── board.cpp                 # Board state management & FEN parser
├── attack/
│   ├── attacks.h                 # Attacks class declaration
│   └── attacks.cpp               # Attack table init & sliding piece logic
├── moves/
│   ├── move.h                    # Move class & encoding constants
│   ├── move.cpp                  # Move encoding/decoding implementation
│   ├── moveGenerator.h           # MoveGenerator class declaration
│   └── moveGenerator.cpp         # Pseudo-legal move generation for all pieces
├── uci/
│   ├── uci.h                     # UCI class declaration
│   └── uci.cpp                   # UCI command loop & parsing
└── test/
    ├── test_attacks.cpp           # Attack generation test suite
    ├── moveTester.cpp             # Move encoding round-trip tests
    └── moveGeneratorTester.cpp    # Move generator correctness tests
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

**Parsing order:**

1. **Piece placement** — Walks the FEN rank-by-rank (8th rank first in FEN, mapped to rank 8 internally). Validates rank length, character validity, and total square count.
2. **Side to move** — `'w'` → `WHITE`, `'b'` → `BLACK`.
3. **Castling rights** — Parses `KQkq` characters into a bitmask. Handles `-` for no castling. Detects duplicates.
4. **En passant square** — Parses file letter + rank digit. Validates that en passant squares are only on rank 3 (for Black's move) or rank 6 (for White's move).
5. **Halfmove clock** — Parses multi-digit integer.
6. **Fullmove number** — Parses multi-digit integer, must be ≥ 1.

After all fields validate, the board state is committed and `rebuildBitboards()` + `updateOccupancies()` are called.

### Key Methods

| Method | Description |
|---|---|
| `clear()` | Zeroes all state, sets defaults (White to move, move 1) |
| `setStartingPosition()` | Loads the standard starting FEN |
| `rebuildBitboards()` | Reconstructs all 12 bitboards from the mailbox array |
| `updateOccupancies()` | ORs piece bitboards into WHITE/BLACK/BOTH occupancies |
| `print()` | Outputs an ASCII board with rank/file labels |
| `loadFEN(fen)` | Full FEN parser with validation, returns `bool` |

### Accessor Methods

| Method | Returns |
|---|---|
| `getMovingSide()` | Current `Color` to move |
| `getEnPassant()` | En passant target `Square` |
| `getBitboard(Piece)` | `U64` bitboard for a specific piece type |
| `getOccupancy(Color)` | `U64` occupancy for WHITE, BLACK, or BOTH |
| `getPieceBoard(Square)` | `Piece` on a given square (mailbox lookup) |
| `getCastlingRights()` | Castling bitmask `int` |

---

## Attack — Attack Table Generation

### Non-Sliding Pieces (Precomputed at Construction)

The `Attacks` constructor precomputes attack tables for all 64 squares for each non-sliding piece type. The approach is direction-offset iteration with bounds checking:

**Knight:**
- 8 possible L-shaped offsets: `{±2, ±1}` and `{±1, ±2}` in (rank, file)
- Stored in `knightAttack[64]`
- Corner squares yield 2 attacks, edge squares 3–4, center squares 8

**King:**
- 8 directional offsets: all combinations of `{-1, 0, +1}` in (rank, file) excluding `(0,0)`
- Stored in `kingAttack[64]`
- Corner squares yield 3 attacks, edges 5, center 8

**Pawns:**
- White: `{+1, -1}` and `{+1, +1}` (diagonal captures forward)
- Black: `{-1, -1}` and `{-1, +1}` (diagonal captures forward from Black's perspective)
- Stored in `whitePawnAttack[64]` and `blackPawnAttack[64]`
- Only capture squares, not push squares (pushes handled in move generator)
- Edge files produce 1 attack instead of 2

### Sliding Pieces (Computed On-the-Fly)

Sliding piece attacks are calculated at runtime, taking an occupancy bitboard as input to determine blocker positions:

**Bishop — `getBishopAttack(square, occupancy)`:**
- Rays along all 4 diagonals: NE `(+1,+1)`, NW `(+1,-1)`, SE `(-1,+1)`, SW `(-1,-1)`
- Each ray walks outward, setting attack bits, stopping **on** the first occupied square (includes that square — it's either a capturable enemy or a friendly blocker filtered later)

**Rook — `getRookAttack(square, occupancy)`:**
- Rays along all 4 orthogonals: North `(+1,0)`, South `(-1,0)`, East `(0,+1)`, West `(0,-1)`
- Same walk-and-block logic as bishop

**Queen — `getQueenAttack(square, occupancy)`:**
- Simply `getBishopAttack() | getRookAttack()` — combines diagonal and orthogonal rays

---

## Moves — Move Encoding & Generation

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

- **PIECE_OFFSET = 1** — All piece values are stored as `(piece + 1)` so that `EMPTY (-1)` maps to `0`, avoiding negative values in unsigned fields.
- **SquareMask = 0x3F** (6 bits) — Masks from/to squares.
- **pieceMask = 0xF** (4 bits) — Masks piece fields.
- **flagMask = 0xF** (4 bits) — Masks the move flag.

**Move Flags (`MoveFlag` enum):**

| Flag | Value | Description |
|---|---|---|
| `quiet` | 0 | Normal non-capture move |
| `capture` | 1 | Standard capture |
| `doublePawnPush` | 2 | Pawn advances two squares from starting rank |
| `kingSideCastle` | 3 | O-O |
| `queenSideCastle` | 4 | O-O-O |
| `enPassant` | 5 | En passant capture |
| `promotion` | 6 | Pawn promotes (no capture) |
| `promotion_capture` | 7 | Pawn promotes while capturing |

**Query Helpers:**

| Method | Logic |
|---|---|
| `isCapture()` | Flag is `capture`, `promotion_capture`, or `enPassant` |
| `isPromotion()` | Flag is `promotion` or `promotion_capture` |
| `isCastle()` | Flag is `kingSideCastle` or `queenSideCastle` |
| `isEnPassant()` | Flag is `enPassant` |

### Move Generator (`moveGenerator.h / moveGenerator.cpp`)

The `MoveGenerator` takes a `const Board&` and `const Attacks&` reference and generates all **pseudo-legal moves** (legality filtering for check is not yet implemented).

`generateMoves()` calls each piece-specific generator and collects results into a single `vector<Move>`.

#### Piece-by-Piece Generation Logic:

**Knight Moves:**
1. Get the bitboard for the active side's knights.
2. For each knight (via `popLSB` iteration), get its precomputed attack table.
3. Mask out friendly pieces (`& ~movingSideOcc`).
4. Each remaining set bit is a valid destination — check mailbox for capture vs quiet.

**King Moves:**
1. Same pattern as knight — get attack table, mask friendlies.
2. **Castling** is handled separately:
   - Checks `castlingRights` bitmask for availability.
   - Verifies the path between king and rook is empty (mailbox lookup).
   - King-side: F1/F8 and G1/G8 must be empty.
   - Queen-side: B1/B8, C1/C8, and D1/D8 must be empty.
   - **Note:** Does not yet check if squares are attacked (legal move filtering is planned).

**Pawn Moves:**
1. **Captures:** Get pawn attack table, mask out friendly pieces. For each attack target:
   - If destination is the en passant square and target is empty → `enPassant` flag, captured piece set to enemy pawn.
   - If destination is empty and not en passant → skip (pawns can't move diagonally without capturing).
   - If on the final rank → generate 4 promotion-capture moves (Q, R, B, N).
   - Otherwise → normal `capture`.
2. **Single push:** Compute one square forward. If occupied → skip (and skip double push too).
   - If on the final rank → generate 4 promotion moves.
   - Otherwise → `quiet` move.
3. **Double push:** Only from the starting rank (rank 2 for White, rank 7 for Black). Destination must also be empty. Flagged as `doublePawnPush`.

**Bishop / Rook / Queen Moves:**
1. Get piece bitboard, iterate each piece.
2. Call the corresponding attack function with `BOTH` occupancy.
3. Mask out friendly pieces.
4. Each remaining bit → check mailbox for capture vs quiet.
5. Queen simply uses `getQueenAttack()` which combines bishop + rook rays.

---

## UCI — Protocol Handler

### `UCI` Class

Implements a basic UCI communication loop:

| Command | Handler | Behavior |
|---|---|---|
| `uci` | `handleUCI()` | Responds with `id name ADAM`, `id author Prajwal`, `uciok` |
| `isready` | `handleIsReady()` | Responds with `readyok` |
| `quit` | `handleQuit()` | Calls `exit(0)` |
| `ucinewgame` | `newgame()` | Loads the standard starting position |
| `position startpos` | `handlePosition()` | Sets board to starting position; `moves` token is parsed but move application is not yet implemented |
| `position fen <fen>` | `handlePosition()` | Parses all 6 FEN tokens and calls `loadFEN()` |
| (unknown) | — | Prints `"Unknon Command : <cmd>"` |

**Not yet implemented:** `go`, `bestmove`, `stop`, `setoption`, move application from `position startpos moves ...`.

---

## Test — Test Suites

### `test_attacks.cpp` — Attack Generation Tests

Uses a custom `TestStats` class and assertion helpers (`assertBitboardEqual`, `assertBitCount`, `assertSquareInBitboard`, `assertSquareNotInBitboard`).

**Test suites:**

| Suite | What It Validates |
|---|---|
| Knight Attacks | Center (8 attacks), corners (2 attacks), edges (4 attacks), symmetry across all 64 squares |
| King Attacks | Center (8), corners (3), edges (5), symmetry verification |
| Pawn Attacks | White/Black from center (2 each), edge files (1 each) |
| Bishop Attacks | Empty board (13 from center, 7 from corner), single-direction blocking, all-direction blocking, long diagonal |
| Rook Attacks | Empty board (14 from any square), single blocking, all-direction blocking, edge squares |
| Queen Attacks | Decomposition property `queen == bishop | rook` verified for all 64 squares with empty and occupied boards |
| Edge Cases | All four corners, every board edge, multiple blockers on same ray, adjacent blockers |
| Consistency | No piece attacks its own square, sliding pieces don't attack themselves |

### `moveTester.cpp` — Move Encoding Tests

Tests round-trip encoding/decoding for all move fields:

- All 12 piece types as moved piece and captured piece
- All promotion piece options
- All move flags
- Full 64×64 from/to sweep (4096 combinations)
- Bit field isolation (ensures fields don't bleed into each other)
- Copy constructor and value equality
- Game sequence simulation (Italian Game opening)

### `moveGeneratorTester.cpp` — Move Generator Tests

Tests actual move generation from specific FEN positions:

| Test | Position | Validates |
|---|---|---|
| Starting position | Standard | 20 moves (16 pawn + 4 knight), 8 double pushes, 0 captures |
| Isolated knight | Center knight + king | 8 knight moves, 5 king moves |
| Corner knight | A1 knight | 2 moves (B3, C2) |
| En passant (White) | Pawn on E5, EP square D6 | EP move generated with correct from/to/captured |
| En passant (Black) | Pawn on D4, EP square E3 | Mirror case for Black |
| Blocked double push | Pawn on E2, enemy on E4 | Single push yes, double push no |
| Non-starting rank | Pawn on E3 | No double push generated |
| Promotion matrix | Pawn on B7, enemies on A8/C8 | 4 quiet promotions + 8 capture-promotions |
| Castling (both clear) | Rooks + King, KQ rights | Both castle moves present |
| Castling (king-side blocked) | Bishop on F1 | Only queen-side castle |
| Castling (no rights) | Same position, `-` rights | Zero castle moves |
| Castling (queen-side blocked) | Knight on B1 | Only king-side castle |
| Sliding piece blocking | Rook with own/enemy blockers | Can't pass own piece, can capture enemy, can't pass enemy |
| Queen open board | Queen on D4 | Exactly 27 moves |
| Kings only | Just two kings | 5 moves for the side to move |
| Side to move | Black to move | Only BK moves generated |
| Known-issue checks | Promotion-capture & EP | Validates `isCapture()`/`isPromotion()` for combined flags |
