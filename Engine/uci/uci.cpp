#include "uci.h"
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include "syzygy/syzygy.h"

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
        handleUCINewGame();
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

    else if(cmd=="setoption"){
        handleSetOption(iss);
    }

    else if(cmd=="stop"){
        handleStop();
    }

    else if(cmd=="ponderhit"){
        handlePonderHit();
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
    std::cout<<"option name Hash type spin default 64 min 1 max 1048576\n";
    std::cout<<"option name Threads type spin default 1 min 1 max 512\n";
    std::cout<<"option name Move Overhead type spin default 100 min 0 max 5000\n";
    std::cout<<"option name Clear Hash type button\n";
    std::cout<<"option name Ponder type check default false\n";
    std::cout<<"option name SyzygyPath type string default <empty>\n";
    std::cout<<"option name UCI_ShowWDL type check default false\n";
    std::cout<<"option name UCI_Chess960 type check default false\n";
    std::cout<<"uciok\n";
}

void UCI::handleIsReady(){
    std::cout<<"readyok\n";
}

void UCI::handleQuit(){
    exit(0);
}

void UCI::handleUCINewGame(){
    board.setStartingPosition();
    search.getTT().clear();
}


void UCI::handlePosition(istringstream &iss){
    string token;
    iss >> token;

    if(token == "startpos"){
        board.setStartingPosition();
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

    if(movetime > 0){
        search.setMoveTime(movetime);
    }
    else if(!infinite){
        Color movingSide = board.getMovingSide();
        long long myTime = (movingSide == WHITE ? wtime : btime);
        long long myInc = (movingSide == WHITE ? winc : binc);

        if(myTime > 0){
            long long allocatedTime = 0;
            long long availableTime = max(10LL, myTime - moveOverheadMs);

            if (movestogo > 0) {
                // Fixed moves to next time control
                int movesLeft = min(movestogo, 40);
                allocatedTime = (availableTime / movesLeft) + (myInc * 3 / 4);
            } else {
                // Dynamic time scaling based on remaining time
                int movesLeft = 40;
                if (availableTime >= 1200000) {       // 20+ min (Classical)
                    movesLeft = 90;
                } else if (availableTime >= 400000) { // 6.5 - 20 min (Rapid: 10m/15m)
                    movesLeft = 85;
                } else if (availableTime >= 80000) {  // 1.5 - 6.5 min (Standard Blitz: 3m/5m)
                    movesLeft = 40;
                } else if (availableTime >= 30000) {  // 30s - 80s (Bullet)
                    movesLeft = 50;
                } else if (availableTime >= 10000) {  // 10s - 30s (Low time)
                    movesLeft = 60;
                } else {                              // < 10s (Extreme scramble)
                    movesLeft = 100;
                }

                allocatedTime = (availableTime / movesLeft) + (myInc * 3 / 4);

                // Hard cap: never spend too much of remaining time on a single move
                if (availableTime < 10000) {
                    allocatedTime = min(allocatedTime, availableTime / 30);  // 3% cap in scramble
                } else if (availableTime < 80000) {
                    allocatedTime = min(allocatedTime, availableTime / 12);  // 8% cap in bullet
                } else if (availableTime < 300000) {
                    allocatedTime = min(allocatedTime, availableTime / 4);   // 25% cap in blitz
                }
            }

            // Hard safety bounds: never exceed remaining available time minus 20ms
            long long absoluteSafetyCap = max(10LL, availableTime - 20);
            if (allocatedTime > absoluteSafetyCap) {
                allocatedTime = absoluteSafetyCap;
            }
            if (allocatedTime < 10) {
                allocatedTime = 10;
            }

            // Tighter soft limit for bullet to prevent deep iterations from overshooting
            long long softLimit;
            if (availableTime < 10000) {
                softLimit = allocatedTime * 2 / 10;  // 20% soft limit for scramble
            } else if (availableTime < 80000) {
                softLimit = allocatedTime * 3 / 10;  // 30% soft limit for bullet
            } else {
                softLimit = allocatedTime * 6 / 10;  // 60% soft limit for blitz+
            }
            search.setMoveTime(allocatedTime, softLimit);
        } else {
            search.setMoveTime(15000, 9000); // 15-second default if no clock provided
        }
    }

    Move bestMove = search.findBestMove(depth);
    std::cout << "bestmove " << moveToUCI(bestMove) << "\n";
}


void UCI::handleSetOption(istringstream& iss) {
    string token, name, value;
    iss >> token; // consumes "name"
    
    while (iss >> token && token != "value") {
        if (!name.empty()) name += " ";
        name += token;
    }
    
    while (iss >> token) {
        if (!value.empty()) value += " ";
        value += token;
    }

    string lowerName = name;
    for (char& c : lowerName) c = tolower(c);

    if (lowerName == "hash" && !value.empty()) {
        hashSizeMb = stoi(value);
        search.getTT().init(hashSizeMb);
    }
    else if (lowerName == "clear hash"){
        search.getTT().clear();
    }
    else if (lowerName == "move overhead" && !value.empty()) {
        moveOverheadMs = stoi(value);
    }
    else if (lowerName == "threads" && !value.empty()) {
        numThreads = stoi(value);
    }
    else if (lowerName == "syzygypath"){
        Syzygy::init(value);
    }
}

void UCI::handleStop() {
    // Handled cleanly
}

void UCI::handlePonderHit() {
    // Handled cleanly
}

void UCI::handleDisplay() {
    board.print();
}

string UCI::moveToUCI(const Move& move) {
    if (move.getValue() == 0) return "0000";

    string s = string(squareToStr[move.getFrom()]) + string(squareToStr[move.getTo()]);

    if (move.isPromotion()) {
        Piece prom = move.getPromotion();
        if (prom == WN || prom == BN) s += 'n';
        else if (prom == WB || prom == BB) s += 'b';
        else if (prom == WR || prom == BR) s += 'r';
        else s += 'q';
    }

    return s;
}