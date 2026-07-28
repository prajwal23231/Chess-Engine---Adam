# Adam — Full-Stack Chess Platform

Adam is a modern, high-performance chess platform built from scratch across three distinct system layers:

- **Engine**: A high-performance, UCI-compatible chess engine written in C++20 featuring dual Bitboard + Mailbox board representations, Magic Bitboards for sliding piece attacks, pseudo-legal move generation, FEN parsing, and automated unit testing suites.
- **Backend** *(Planned)*: Service and API orchestration layer responsible for managing sessions, real-time multiplayer, game flow, and engine RPC execution.
- **Frontend** *(Planned)*: Responsive player interface for interactive board rendering, move input, real-time visual feedback, and engine controls.

**Author**: Prajwal

---

## High-Level Architecture

```text
┌────────────────┐          ┌────────────────┐          ┌────────────────┐
│    Frontend    │  ◄─────► │    Backend     │  ◄─────► │  Engine (UCI)  │
│  (UI / WebApp) │          │  (API / Game)  │          │    (C++20)     │
└────────────────┘          └────────────────┘          └────────────────┘
```

- **Frontend**: Renders vector board UI (using SVG piece sets in `assests/`), receives user input, and manages UI state.
- **Backend**: Validates moves, handles player auth/state, and communicates with the engine over standard I/O (UCI).
- **Engine**: Executes board state updates, computes attack tables, generates pseudo-legal/legal moves, and evaluates positions.

---

## Current Component Status

| Component | Technology | Description | Status |
|---|---|---|---|
| **Engine Core** | C++20 | Dual bitboard/mailbox board representation, FEN parser, bitwise utilities | **Complete** |
| **Attack Tables** | C++20 | Precomputed non-sliding (Knight, King, Pawn) + Magic Bitboards (Bishop, Rook, Queen) | **Complete** |
| **Move Generator** | C++20 | Pseudo-legal move generation for all pieces, castling, promotions, en passant | **Complete** |
| **Move Encoding** | C++20 | 32-bit compact bit-packed move structures (`Move` class) | **Complete** |
| **UCI Protocol** | C++20 | `uci`, `isready`, `ucinewgame`, `position fen`, `quit` command handlers | **In Progress** |
| **Test Suites** | C++20 | Unit test runners for attack masks, magic bitboards, move encoding, and generation | **Complete** |
| **Asset Library** | SVG Vector | High-quality SVG vector chess piece assets (`wb`, `wk`, `wn`, `wp`, `wq`, `wr`, `bb`, `bk`, `bn`, `bp`, `bq`, `br`) | **Complete** |
| **Backend Service** | Go / Node.js | Game session API and engine process manager | **Planned** |
| **Frontend WebApp** | TypeScript | Interactive chessboard UI and game interface | **Planned** |

---

## Repository Structure

```text
Adam/
├── README.md                 # Project-level overview and architecture reference
├── assests/                  # SVG vector graphics for chess pieces
│   ├── wp.svg, wn.svg, wb.svg, wr.svg, wq.svg, wk.svg   # White piece assets
│   └── bp.svg, bn.svg, bb.svg, br.svg, bq.svg, bk.svg   # Black piece assets
└── Engine/                   # C++20 Chess Engine Module
    ├── README.md             # Detailed engine technical & logic specification
    ├── main.cpp              # Engine CLI entry point
    ├── attack/               # Precomputed & Magic bitboard attack tables
    │   ├── attacks.h / .cpp  # Consolidated attack table interfaces
    │   └── magic.h / .cpp    # Magic bitboard lookup table generator & engine
    ├── board/                # Board state representation & FEN parser
    │   └── board.h / .cpp    # Board class (12 bitboards + 64-square mailbox)
    ├── moves/                # Move encoding & pseudo-legal move generator
    │   ├── move.h / .cpp     # 32-bit packed Move structure & flags
    │   ├── movegen.h / .cpp  # MoveGenerator class implementation
    │   └── undomove.h        # Move undo helper definitions
    ├── uci/                  # Universal Chess Interface protocol handler
    │   └── uci.h / .cpp      # UCI loop parser and dispatcher
    ├── utils/                # Bitboard helpers, type definitions, magic generator
    │   ├── type.h            # Core types (U64, U32), enums (Piece, Square, Color)
    │   ├── bitboard_utilities.h / .cpp  # Bit manipulation helpers (popCount, popLSB, etc.)
    │   ├── magic_numbers.h   # Precalculated 64-bit magic numbers for all squares
    │   ├── magicGen.h / .cpp # Monte Carlo magic number search algorithm
    │   └── magicCreate.cpp   # Offline utility entry point to rebuild magic_numbers.h
    └── test/                 # Engine unit test suites
        ├── test_attacks.cpp  # Test suite for non-sliding & sliding piece attacks
        ├── test_magic.cpp    # Test suite for magic bitboards & mask invariance
        ├── moveTester.cpp    # Test suite for 32-bit move bit-field encoding
        └── movegen.cpp       # Move generator test runner
```

---

## Build and Run Instructions

### 1. Build & Run the Main Engine Executable

From the root `Adam/` directory:

```bash
g++ -std=c++20 -O2 -I Engine Engine/main.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/moves/move.cpp Engine/moves/movegen.cpp Engine/uci/uci.cpp Engine/utils/bitboard_utilities.cpp -o adam
./adam
```

### 2. Run Engine Unit Test Suites

- **Magic Bitboard Test Suite**:
  ```bash
  g++ -std=c++20 -O2 -I Engine Engine/test/test_magic.cpp Engine/attack/magic.cpp Engine/utils/bitboard_utilities.cpp -o test_magic
  ./test_magic
  ```

- **Attack Table Test Suite**:
  ```bash
  g++ -std=c++20 -O2 -I Engine Engine/test/test_attacks.cpp Engine/attack/attacks.cpp Engine/attack/magic.cpp Engine/utils/bitboard_utilities.cpp -o test_attacks
  ./test_attacks
  ```

- **Move Bit-Encoding Test Suite**:
  ```bash
  g++ -std=c++20 -O2 -I Engine Engine/test/moveTester.cpp Engine/moves/move.cpp Engine/utils/bitboard_utilities.cpp -o test_move
  ./test_move
  ```

---

## Further Documentation

For deep technical details regarding bitboard encodings, magic bitboard hashing algorithms, FEN validation, move bit-packing layouts, and UCI command handling, read [Engine/README.md](file:///c:/Users/sriva/OneDrive/Desktop/Adam/Engine/README.md).

---

## License

This project is created for educational and personal use.