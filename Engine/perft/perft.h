#include "moves/movegen.h"
#include "board/board.h"
#include "utils/type.h"
#include "moves/move.h"

class Perft{
public:
    Perft(Board& board, MoveGenerator& moveGen);
    
    U64 run(int depth);
    void divide(int depth);
    void benchmark(int depth);

private:
    Board& board;
    MoveGenerator& moveGen;
};