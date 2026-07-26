#include "uci.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace std;

void UCI::loop() {
    string command;

    while(getline(cin, command)){
        parseCommand(command);
    }
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

    else{
        cout<<"Unknon Command : "<<cmd<<"\n";
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
        string word;

        if(iss >> word){
            if(word == "moves"){
                // left for later
            }
        }
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
        }
    }

    else {
        cout<<"Unknown position Command\n";
    }
}