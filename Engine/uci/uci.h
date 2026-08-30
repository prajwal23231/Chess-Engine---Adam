#pragma once
#include "utils/type.h"
#include "board/board.h"
#include "perft/perft.h"
#include "moves/movegen.h"
#include "moves/move.h"
#include <string>
#include <sstream>
#include "evaluation/eval.h"

struct ParsedMove{
    Square from;
    Square to;
    Piece promotion = EMPTY;
};

class UCI{
public:
    UCI(Board& board, MoveGenerator &movegen, Evaluator &evaluator);
    void loop();

private:
    Board& board;
    MoveGenerator& movegen;
    Perft perft;
    Evaluator evaluator;

    void parseCommand(const std::string &command);
    void handleUCI();
    void handleIsReady();
    void handleQuit();
    void newgame();
    void handlePosition(std::istringstream &iss);
    void handlePerft(std::istringstream &iss);
    void handleDivide(std::istringstream &iss);
    void handleEval();

    // helper
    bool parseUCIMove(const std::string& pos, ParsedMove& move);
    bool playMoves(std::istringstream &iss);
    inline Piece getPiece(char c,Color side);
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