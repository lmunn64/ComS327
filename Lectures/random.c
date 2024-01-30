#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char *argv[]){

    srand(time(NULL));  //seed set for random (time is seconds since the Epoch)
                        // This should be done exactly once in your program! Probably in main.

    printf("%d\n", rand()); // Range is [0, RAND_MAX]
    printf("%d\n", rand()); // RAND_MAX is 2^31 -1

    printf("%d\n", rand() % 10); // rand % gives [0, x-1]

    printf("%d\n", (rand() % 10) + 15); // Added shifts range, [15,24]

    printf("%f\n", ((double) rand()) / RAND_MAX); //[0.0, 1.0]

    return 0;
}
/*
    STAGES OF COMPILER
===========================
- Pre-processor: Finds #, fetches files of include tags, and also handles macro replacements
    Can run pre-proccessor only with gcc -E
- Compiler: Translate from c to assembly code
    Can stop after compiler using gcc -S
- Assembling
- Linking: Putting objects together and libraries into an executable




*/