#pragma once
#include "utils/type.h"
#include "board/board.h"
#include "perft/perft.h"
#include "moves/movegen.h"
#include "moves/move.h"
#include <string>
#include <sstream>
#include "evaluation/eval.h"
#include "search/search.h"

struct ParsedMove{
    Square from;
    Square to;
    Piece promotion = EMPTY;
};

class UCI{
public:
    UCI(Board& board, MoveGenerator &movegen, Evaluator &evaluator, Search& search);
    void loop();

private:
    Board& board;
    MoveGenerator& movegen;
    Perft perft;
    Evaluator evaluator;
    Search search;

    void parseCommand(const std::string &command);
    void handleUCI();
    void handleIsReady();
    void handleQuit();
    void newgame();
    void handlePosition(std::istringstream &iss);
    void handlePerft(std::istringstream &iss);
    void handleDivide(std::istringstream &iss);
    void handleEval();
    void handleGo(std::istringstream& iss);


    // helper
    bool parseUCIMove(const std::string& pos, ParsedMove& move);
    bool playMoves(std::istringstream &iss);
    inline Piece getPiece(char c,Color side);
    inline std::string moveToUCI(const Move& move);
    inline void handleDisplay();
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


inline std::string UCI::moveToUCI(const Move& move){
    if(move.getValue()==0) return "0000";

    std::string s = std::string(squareToStr[move.getFrom()]) + std::string(squareToStr[move.getTo()]);

    if(move.isPromotion()){
        Piece prom = move.getPromotion();
        if (prom == WN || prom == BN) s += 'n';
        else if (prom == WB || prom == BB) s += 'b';
        else if (prom == WR || prom == BR) s += 'r';
        else s += 'q';
    }

    return s;
}


inline void UCI::handleDisplay(){
    board.print();
}