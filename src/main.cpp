#include "uci/uci.h"
#include "board/board.h"
#include "bitboard/bitboard_utilities.h"
#include "attack/attacks.h"
#include <iostream>

using namespace std;

int main() {
    Attacks attacks;

    Bitboard::printBitboard(attacks.getKnightAttack(H8));
    Bitboard::printBitboard(attacks.getKnightAttack(A1));

    Bitboard::printBitboard(attacks.getKingAttack(A1));
    Bitboard::printBitboard(attacks.getKingAttack(H8));

    Bitboard::printBitboard(attacks.getWhitePawnAttack(A2));
    Bitboard::printBitboard(attacks.getWhitePawnAttack(H2));

    Bitboard::printBitboard(attacks.getBlackPawnAttack(A7));
    Bitboard::printBitboard(attacks.getBlackPawnAttack(H7));
}