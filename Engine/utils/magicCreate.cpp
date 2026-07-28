#include <iostream>
#include "attack/magic.h"
#include "magicGen.h"

int main(){
    Magic magic;
    MagicGen::generateAll(magic);
}