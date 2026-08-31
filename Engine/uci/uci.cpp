#include "uci.h"
#include <cstdlib>
#include <iostream>
#include <algorithm>

using namespace std;


UCI::UCI(Board& board, MoveGenerator& movegen, Evaluator& evaluator, Search& search) : movegen(movegen), board(board), perft(board,movegen), evaluator(evaluator), search(search){
}


void UCI::parseCommand(const string& command){
    istringstream iss(command);

    string cmd;
    iss>>cmd;

    if(cmd=="uci"){
        handleUCI();
    }

    else if(cmd=="isready"){
        handleIsReady();
    }

    else if(cmd=="quit"){
        handleQuit();
    }

    else if(cmd=="ucinewgame"){
        newgame();
    }

    else if(cmd=="position"){
        handlePosition(iss);
    }

    else if(cmd=="perft"){
        handlePerft(iss);
    }

    else if(cmd=="move"){
        if(!playMoves(iss)){
            std::cout<<"Invalid move\n";
        }
    }

    else if(cmd=="divide"){
        handleDivide(iss);
    }

    else if(cmd=="eval"){
        handleEval();
    }

    else if(cmd=="go"){
        handleGo(iss);
    }

    else if(cmd=="d"){
        handleDisplay();
    }

    else{
        std::cout<<"Unknown Command : "<<cmd<<"\n";
    }
}



bool UCI::playMoves(istringstream &iss){
    string pos;
    Move moves[MAX_MOVES];
    
    while(iss>>pos){
        ParsedMove move;
        if(!parseUCIMove(pos, move)) return false;

        int legal = movegen.generateLegalMoves(moves);
        bool found = false;

        for(int i=0; i<legal; i++){
            if(moves[i].getFrom() == move.from && moves[i].getTo() == move.to
            && moves[i].getPromotion() == move.promotion){
                board.makeMove(moves[i]);
                found = true;
                break;
            }
        }

        if(!found) return false;
    }

    return true;
}


bool UCI::parseUCIMove(const string &pos, ParsedMove &move){
    if(pos.size()<4 || pos.size()>5) return false;

    Square from = charToSquare(pos[0], pos[1]);
    Square to = charToSquare(pos[2], pos[3]);
    Piece promotion = EMPTY;

    if(from == NO_SQUARE || to == NO_SQUARE) return false;

    if(pos.size() == 5){
        promotion = getPiece(pos[4], board.getMovingSide());

        if(promotion == EMPTY) return false;
    }

    move.from = from;
    move.to = to;
    move.promotion = promotion;

    return true;
}


void UCI::loop() {
    string command;

    while(getline(cin, command)){
        parseCommand(command);
    }
}


void UCI::handleUCI(){
    std::cout<<"id name ADAM\n";
    std::cout<<"id author Prajwal\n";
    std::cout<<"uciok\n";
}

void UCI::handleIsReady(){
    std::cout<<"readyok\n";
}

void UCI::handleQuit(){
    exit(0);
}

void UCI::newgame(){
    board.setStartingPosition();
}


void UCI::handlePosition(istringstream &iss){
    string token;
    iss >> token;

    if(token == "startpos"){
        newgame();
    }

    else if(token == "fen"){
        string placement, tomove, castling, enpassant, halfmove, fullmove;

        iss >> placement;
        iss >> tomove;
        iss >> castling;
        iss >> enpassant;
        iss >> halfmove;
        iss >> fullmove;

        string fen = placement + " " + tomove + " " + castling + " " + enpassant + " " + halfmove + " " + fullmove;

        if(!board.loadFEN(fen)){
            std::cout<<"Unknown position Command\n";
            return ;
        }
    }

    else {
        std::cout<<"Unknown position Command\n";
    }


    // parsing moves
    string word;

    if(iss>>word){
        if(word == "moves"){
            if(!playMoves(iss)){
                std::cout<<"Invalid move\n";
                return ;
            }
        }

        else{
            std::cout<<"Invalid command\n";
            return ;
        }
    }
}


void UCI::handlePerft(istringstream &iss){
    int depth;
    
    if (!(iss >> depth)) {
        std::cout << "Invalid perft depth\n";
        return;
    }

    string word;

    if ((iss >> word)) {
        std::cout << "Invalid perft command\n";
        return;
    }

    perft.benchmark(depth);
}


void UCI::handleDivide(istringstream& iss){
    int depth;
    
    if (!(iss >> depth)) {
        std::cout << "Invalid perft depth\n";
        return;
    }

    string word;

    if ((iss >> word)) {
        std::cout << "Invalid perft command\n";
        return;
    }

    perft.divide(depth);
}


void UCI::handleEval(){
    int score = evaluator.evaluate(board);
    std::cout<<"Eval: "<<score<<" cp\n";
}



void UCI::handleGo(istringstream& iss){
    string token;
    int depth = 64;
    long long wtime = 0, btime = 0, winc = 0, binc = 0;
    long long movetime = 0;
    int movestogo = 0;
    bool infinite = false;

    while(iss>>token){
        if(token == "depth"){
            iss>>depth;
        }
        else if(token=="movetime"){
            iss>>movetime;
        }
        else if (token == "wtime") {
            iss >> wtime;
        }
        else if (token == "btime") {
            iss >> btime;
        }
        else if (token == "winc") {
            iss >> winc;
        }
        else if (token == "binc") {
            iss >> binc;
        }
        else if (token == "movestogo") {
            iss >> movestogo;
        }
        else if(token=="infinite"){
            infinite = true;
            depth = 64;
        }
    }

    if(movetime>0){
        search.setMoveTime(movetime);
    }

    else if(!infinite){
        Color movingSide = board.getMovingSide();
        long long myTime = (movingSide == WHITE ? wtime : btime);
        long long myInc = (movingSide == WHITE ? winc : binc);

        if(myTime > 0){
            int movesLeft = (movestogo>0) ? min(movestogo,50) : 30;

            long long allocatedTime = (myTime/movesLeft) + (myInc * 3/4);

            if(allocatedTime > myTime - 50){
                allocatedTime = max(10LL, myTime-50);
            }

            if(allocatedTime < 10){
                allocatedTime = 10;
            }

            search.setMoveTime(allocatedTime);
        }
    }

    Move bestMove = search.findBestMove(depth);
    std::cout << "bestmove " << moveToUCI(bestMove) << "\n";

    search.resetMoveTime();
}