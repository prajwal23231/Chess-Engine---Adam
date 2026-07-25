#include "uci/uci.h"
#include "board/board.h"
#include "utils/bitboard_utilities.h"
#include "utils/type.h"
#include "attack/attacks.h"
#include <iostream>

using namespace std;

int main() {
    Attacks attacks;

    U64 occ = 0;

    occ |= 1ULL << E5; // North
    occ |= 1ULL << G4; // East
    occ |= 1ULL << E3; // South
    occ |= 1ULL << B4; // West

    occ |= 1ULL << G6; // NE
    occ |= 1ULL << C6; // NW
    occ |= 1ULL << G2; // SE
    occ |= 1ULL << C2; // SW

    Bitboard::printBitboard(occ);

    Bitboard::printBitboard(attacks.getBishopAttack(E4, occ));
    Bitboard::printBitboard(attacks.getRookAttack(E4, occ));
    Bitboard::printBitboard(attacks.getQueenAttack(E4, occ));
}