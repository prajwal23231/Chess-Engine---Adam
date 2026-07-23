#include "board.h"
#include <iostream>
using namespace std;

Board::Board(){
    clear();
}

void Board::clear(){
    bitboards.fill(0);
    occupancies.fill(0);
    board.fill(EMPTY);

    sideToMove = WHITE;
    castlingRights = 0;
    enPassant = NO_SQUARE;
    
    halfmoveClock = 0;
    fullmoveNumber = 0;
}

void Board::setStartingPosition(){
    
}