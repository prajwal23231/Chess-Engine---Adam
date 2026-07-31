#include "uci.h"
#include <cstdlib>
#include <iostream>

using namespace std;


UCI::UCI(Board& board, MoveGenerator& movegen) : movegen(movegen), board(board), perft(board,movegen){
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
        if(playMoves(iss)) board.print();

        else{
            cout<<"Invalid move\n";
        }
    }

    else if(cmd=="divide"){
        handleDivide(iss);
    }

    else{
        cout<<"Unknown Command : "<<cmd<<"\n";
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
    cout<<"id name ADAM\n";
    cout<<"id author Prajwal\n";
    cout<<"uciok\n";
}

void UCI::handleIsReady(){
    cout<<"readyok\n";
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
            cout<<"Unknown position Command\n";
            return ;
        }
    }

    else {
        cout<<"Unknown position Command\n";
    }


    // parsing moves
    string word;

    if(iss>>word){
        if(word == "moves"){
            if(!playMoves(iss)){
                cout<<"Invalid move\n";
                return ;
            }
        }

        else{
            cout<<"Invalid command\n";
            return ;
        }
    }

    board.print();
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