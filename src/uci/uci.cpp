#include "uci.h"
#include <cstdlib>
#include <iostream>

using namespace std;

void UCI::loop() {
    string command;

    while(getline(cin, command)){
        parseCommand(command);
    }
}

void UCI::parseCommand(const string& command){
    if(command=="uci"){
        handleUCI();
    }

    else if(command=="isready"){
        handleIsReady();
    }

    else if(command=="quit"){
        handleQuit();
    }
    else{
        cout<<"Unknow Command : "<<command<<"\n";
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