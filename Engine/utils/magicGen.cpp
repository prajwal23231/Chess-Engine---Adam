#include <iostream>
#include "magicGen.h"
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <stdexcept>

using namespace std;
using namespace Bitboard;

void MagicGen::generateAll(const Magic& magic){
    U64 bishopMagics[BOARD_SIZE];
    U64 rookMagics[BOARD_SIZE];

    for(int sq=0; sq<BOARD_SIZE; sq++){
        Square s = static_cast<Square>(sq);

        bishopMagics[s] = MagicGen::findMagic(s, true, magic);
        cout<<"bishop done,"<<flush;

        rookMagics[s] = MagicGen::findMagic(s, false, magic);
        cout<<"rook done\n"<<flush;
    }

    std::ofstream out("magic_numbers.h");

    out << "#pragma once\n";
    out << "#include \"type.h\"\n\n";

    out << "constexpr U64 bishopMagics[BOARD_SIZE] = {\n";

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        out << "    0x"
            << std::hex
            << std::uppercase
            << bishopMagics[i]
            << "ULL";

        if (i != BOARD_SIZE - 1)
            out << ",";

        out << "\n";
    }

    out << "};\n";

    out << "constexpr U64 rookMagics[BOARD_SIZE] = {\n";

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        out << "    0x"
            << std::hex
            << std::uppercase
            << rookMagics[i]
            << "ULL";

        if (i != BOARD_SIZE-1)
            out << ",";

        out << "\n";
    }

    out << "};\n";

    out.close();
}


U64 MagicGen::findMagic(Square square, bool bishop, const Magic &magic){
    U64 mask = (bishop ? magic.getBishopMask(square) : magic.getRookMask(square));

    int relevantBits = popCount(mask);
    int occupancyCount = 1 << relevantBits;

    vector<U64> occupancies(occupancyCount);
    vector<U64> attacks(occupancyCount);
    vector<U64> used(occupancyCount);
    vector<bool> filled(occupancyCount);

    for(int index=0; index<occupancyCount; ++index){
        occupancies[index] = magic.setOccupancy(index, relevantBits, mask);

        if(bishop){
            attacks[index] = magic.getBishopAttackOTF(square, occupancies[index]);
        }

        else{
            attacks[index] = magic.getRookAttackOTF(square, occupancies[index]);
        }
    }


    for (int attempts = 0; attempts < 10000000; ++attempts){
        U64 candidate = randomMagicCandidate();

        fill(used.begin(), used.end(), 0);
        fill(filled.begin(), filled.end(), false);
        bool found = true;

        for(int index=0; index<occupancyCount; index++){
            size_t hash = (occupancies[index] * candidate) >> (64 - relevantBits);

            if(!filled[hash]){
                filled[hash] = true;
                used[hash] = attacks[index];
            }

            else if(used[hash] != attacks[index]){
                found = false;
                break;
            }
        }

        if(found) return candidate;

        if (attempts % 1000000 == 0)
        cout << attempts << '\n';
    }

    throw runtime_error("Failed");
}



U64 MagicGen::randomU64(){
    static mt19937_64 rng(std::random_device{}());
    return rng();
}



U64 MagicGen::randomMagicCandidate(){
    U64 a = randomU64();
    U64 b = randomU64();
    U64 c = randomU64();

    return (a&b&c);
}