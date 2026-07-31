#include "bitboard_utilities.h"
#include <iostream>

using namespace std;

namespace Bitboard {

    void printBitboard(U64 bb) {
        for (int i = RANK_SIZE; i > 0; i--) {
            cout << i << "  ";

            for (int j = 0; j < RANK_SIZE; j++) {
                Square square = static_cast<Square>((i - 1) * RANK_SIZE + j);
                cout << getBit(bb, square) << " ";
            }

            cout << "\n";
        }

        cout << "\n   ";

        for (char c = 'a'; c <= 'h'; c++) {
            cout << c << " ";
        }

        cout << "\n";
    }
}