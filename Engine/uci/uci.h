#pragma once
#include "utils/type.h"
#include "board/board.h"
#include "perft/perft.h"
#include "moves/movegen.h"
#include "moves/move.h"
#include "evaluation/eval.h"
#include "search/search.h"
#include <string>
#include <sstream>

struct ParsedMove {
    Square from;
    Square to;
    Piece promotion = EMPTY;
};

class UCI {
public:
    UCI(Board& board, MoveGenerator& movegen, Evaluator& evaluator, Search& search);
    void loop();

private:
    Board& board;
    MoveGenerator& movegen;
    Perft perft;
    Evaluator evaluator;
    Search& search;

    // UCI Options
    int hashSizeMb = 16;
    int moveOverheadMs = 10;
    int numThreads = 1;

    // Core UCI Protocol Handlers
    void parseCommand(const std::string& command);
    void handleUCI();
    void handleIsReady();
    void handleSetOption(std::istringstream& iss);
    void handleUCINewGame();
    void handlePosition(std::istringstream& iss);
    void handleGo(std::istringstream& iss);
    void handleStop();
    void handlePonderHit();
    void handleQuit();

    // Custom Developer & Debug Commands
    void handleDisplay();
    void handleEval();
    void handlePerft(std::istringstream& iss);
    void handleDivide(std::istringstream& iss);

    // Helpers
    bool parseUCIMove(const std::string& pos, ParsedMove& move);
    bool playMoves(std::istringstream& iss);
    inline Piece getPiece(char c, Color side);
    std::string moveToUCI(const Move& move);
};

inline Piece UCI::getPiece(char c, Color side) {
    switch (c) {
        case 'q': return (side == WHITE) ? WQ : BQ;
        case 'r': return (side == WHITE) ? WR : BR;
        case 'b': return (side == WHITE) ? WB : BB;
        case 'n': return (side == WHITE) ? WN : BN;
        default:  return EMPTY;
    }
}