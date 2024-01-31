#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "queue.h"


char screen[21][80];
const char MOUNTAIN = '%';
const char LONGGRASS = ':';
const char CLEARING = '.';
const char FOREST = '^';
const char WATER = '~';
const char EXIT = '#';

void seeder(char screen[21][80]){
    //queue's size
    const int SIZE = 1580; 

    //initialize queue
    int head, tail;
    int queue[1580];
    initQueue(&head,&tail);

    int exitS = rand() % 78 + 1;
    int exitN = rand() % 78 + 1;
    int exitE = rand() % 19 + 1;
    int exitW = rand() % 19 + 1;
    
    //create borders
    for(int l = 0; l < 80; l++){
        screen[0][l] = MOUNTAIN;
        screen[20][l] = MOUNTAIN;
        if(l == exitS)
             screen[20][l] = EXIT;
        if(l == exitN)
             screen[0][l] = EXIT;
    }
    for(int k = 0; k < 21; k++){
        screen[k][0] = MOUNTAIN;
        screen[k][79] = MOUNTAIN;
        if(k == exitE)
             screen[k][79] = EXIT;
        if(k == exitW)
             screen[k][0] = EXIT;
        
    }
    //create empty spaces using "-"
    for(int n = 1; n < 20; n++){
        for(int m = 1; m < 79; m++){
        screen[n][m] = '-';
        }
    }
    //seed mountains, long grass and clearings
    int mountainsCoord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);

    int forestCoord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);

    int waterCoord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);

    int longGrassCoord1 = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);

    int longGrassCoord2 = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);

    int clearingCoord1 = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);

    int clearingCoord2 = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);


    screen[longGrassCoord1 % 79][longGrassCoord1 / 79] = LONGGRASS;
    screen[longGrassCoord2 % 79][longGrassCoord2 / 79] = LONGGRASS;

    screen[mountainsCoord % 79][mountainsCoord / 79] = MOUNTAIN;

    screen[clearingCoord1 % 79][clearingCoord1 / 79] = CLEARING;
    screen[clearingCoord2 % 79][clearingCoord2 / 79] = CLEARING;

    screen[forestCoord % 79][forestCoord / 79] = FOREST;

    screen[waterCoord % 79][waterCoord / 79] = WATER;
    
    enqueue(queue, &tail, longGrassCoord1);
    enqueue(queue, &tail, longGrassCoord2);
    enqueue(queue, &tail, forestCoord);
    enqueue(queue, &tail, waterCoord);
    enqueue(queue, &tail, mountainsCoord);
    enqueue(queue, &tail, clearingCoord1);
    enqueue(queue, &tail, clearingCoord2);


    while(!empty(head, tail)){
        int coord = dequeue(queue,&head);
        int x =  coord % 79;
        int y = coord / 79;
        char seed;
        //Check which biome and set seed to it
        if(screen[x][y] == MOUNTAIN)
            seed = MOUNTAIN;
        else if(screen[x][y] == LONGGRASS)
            seed = LONGGRASS;
        else if(screen[x][y] == CLEARING)
            seed = CLEARING;
        else if(screen[x][y] == FOREST)
            seed = FOREST;
        else if(screen[x][y] == WATER)
            seed = WATER;
        if(screen[x+1][y] == '-'){
            screen[x+1][y] = seed;
            enqueue(queue, &tail, (y * 79 + (x+1)));
        }
        if(screen[x-1][y] == '-'){
            screen[x-1][y] = seed;
            enqueue(queue, &tail, (y * 79 + (x-1)));
        }
        if(screen[x][y+1] == '-'){
            screen[x][y+1] = seed;
            enqueue(queue, &tail, ((y+1) * 79 + x));
        }
        if(screen[x][y-1] == '-'){
            screen[x][y-1] = seed;
            enqueue(queue, &tail, ((y-1) * 79 + x));
        }
    }

    
}


int main(int argc, char *argv[]){
    srand(time(NULL));
    // printf("%s", 100);
    seeder(screen);
    for(int i = 0; i < 21; i++){
        for(int j = 0; j < 80; j++){
        printf("%c", screen[i][j]);
    }
    printf("\n");
    }
    return 0;
}