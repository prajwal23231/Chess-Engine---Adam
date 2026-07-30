#pragma once
#include "utils/type.h"
#include "board/board.h"
#include "perft/perft.h"
#include <string>
#include <sstream>


struct ParsedMove{
    Square from;
    Square to;
    Piece promotion = EMPTY;
};

class UCI{
public:
    UCI() = default;
    void loop();

private:
    Board board;

    void parseCommand(const std::string &command);
    void handleUCI();
    void handleIsReady();
    void handleQuit();
    void newgame();
    void handlePosition(std::istringstream &iss);
    void handlePerft(std::istringstream &iss);

    // helper
    bool parseUCIMove(const std::string& pos, ParsedMove& move);
    void playMoves(std::istringstream &iss);
};