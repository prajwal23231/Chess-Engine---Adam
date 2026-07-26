#pragma once
#include "utils/type.h"
#include <string>
#include "board/board.h"

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
    void handlePosition(istringstream &iss);
};