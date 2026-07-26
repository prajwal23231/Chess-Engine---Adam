# Adam

Adam is a full-stack chess platform built from scratch with three core layers:

- Frontend: player-facing interface (board rendering, move input, game controls, UX)
- Backend: service layer that will manage sessions, game flow, and engine communication
- Engine: a UCI-compatible chess engine written in C++20

Author: Prajwal

## Vision

Adam is not only a chess engine project. It is designed as a complete chess product where:

- the frontend handles interaction and presentation,
- the backend coordinates game logic and system integration,
- the engine provides chess computation and move generation.

## Current Status

| Component | Role | Status |
|---|---|---|
| Engine | Core chess logic and UCI integration | In progress |
| Backend | API and orchestration layer between UI and engine | Planned |
| Frontend | User interface and gameplay experience | Planned |

## High-Level Architecture

```text
Frontend <-> Backend <-> Engine
```

- Frontend sends game actions to backend.
- Backend validates and routes requests.
- Engine computes moves/positions and responds through UCI-compatible commands.

## Project Structure

```text
Adam/
|- README.md
|- Engine/
|  |- README.md
|  |- main.cpp
|  |- attack/
|  |- board/
|  |- moves/
|  |- test/
|  |- uci/
|  `- utils/
```

## Technology

- Language: C++20 (engine)
- Engine protocol: UCI
- Board representation: bitboards + mailbox

## Build and Run (Engine)

From the Adam root directory:

```bash
g++ -std=c++20 -O2 -I Engine Engine/main.cpp Engine/board/board.cpp Engine/attack/attacks.cpp Engine/moves/move.cpp Engine/moves/moveGenerator.cpp Engine/uci/uci.cpp Engine/utils/bitboard_utilities.cpp -o adam
./adam
```

For deeper implementation details, see `Engine/README.md`.

## License

This project is for educational and personal use.