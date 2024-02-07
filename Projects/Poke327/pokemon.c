#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "queue.h"


int currWorldRow, currWorldCol; //current world location

const char MOUNTAIN = '%';
const char LONGGRASS = ':';
const char CLEARING = '.';
const char FOREST = '^';
const char WATER = '~';
const char EXIT = '#';
const char CENTER = 'C';
const char MART = 'M';

//current exits for current screen
int exitN;
int exitS;
int exitW;
int exitE;

char *world[401][401];

void placeMartandCenter(int i, int a, char screen[21][80]){
    screen[i][a-1] = MART;
    screen[i][a-2] = MART;
    screen[i-1][a-1] = MART;
    screen[i-1][a-2] = MART;
    screen[i][a+1] = CENTER;
    screen[i][a+2] = CENTER;
    screen[i-1][a+1] = CENTER;
    screen[i-1][a+2] = CENTER;
}

void martCenterHelper(char screen[21][80]){
    int mandc = rand() % 17 + 2; //pokecenter and mart between 2 and 18
    for(int i = 0; i < 79; i++){
        if (screen[mandc][i] == EXIT){
            //checks surrounding area to make sure there are no paths interfering with possible mart and center placement.
            if (screen[mandc][i+1] != EXIT && screen[mandc][i+2] != EXIT && screen[mandc-1][i+1] != EXIT && screen[mandc-1][i+2] != EXIT && screen[mandc-1][i-1] != EXIT && screen[mandc-1][i-2] != EXIT){
                placeMartandCenter(mandc, i, screen);
                return;
            } 
            else {
                martCenterHelper(screen);
                return;
            }  
        }
    }
}

//TODO: implement bfs or dijkstras for this instead of dummy path finding
//Ugly and gross
//Adds pokecenters and marts as well
void roadPath(int a, int b, int c, int d, char screen[21][80]){    
    //Pokecenter and mart location
    int mandc = rand() % 16 + 3;
    if (mandc == 12){
        mandc = mandc + 1;
    }
    //source is exitN targer is exitS
    int i;
    
    //move down
    for(i = 1; i < 12; i++){
        screen[i][a] = EXIT;
    }

    //check which exit is lower and move horizontally left or right
    if(a < b){
        for(int k = a; k <= b; k++){
            screen[i][k]= EXIT;
        }
    }
    else if(a > b){
        for(int k = a; k >= b; k--){
            screen[i][k]= EXIT;    
        }
    }

    // move down until exit
    for(int j = i+1; j < 20; j++){
        screen[j][b]= EXIT; 
    }
        
    //source is exitW target is exitE
    int h;

    // move right
    for(h = 1; h < 40; h++){
        screen[c][h] = EXIT;
    }
    
    //check which exit is lower and move vertically up or down
    if(c < d){
        for(int k = c; k <= d; k++){
            screen[k][h]= EXIT;
        }
    }
    else if (c > d){
        for(int k = c; k >= d; k--){
            screen[k][h]= EXIT;    
        }
    }

    // move right until exit
    for(int j = h; j < 80; j++){
        screen[d][j]= EXIT;    
        }
   }

int seeder(char screen[21][80]){
    //queue's size
    const int SIZE = 1580; 

    //initialize queue
    int head, tail;
    int queue[1580];
    initQueue(&head,&tail);
    
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

void createMap(int n, int s, int w, int e){
    
    //is this the first map or not
    if(n == 0){
        exitS = rand() % 74 + 3;
        exitN = rand() % 74 + 3;
        exitE = rand() % 15 + 3;
        exitW = rand() % 15 + 3;
    }
    else{
        exitS = s;
        exitN = n;
        exitE = e;
        exitW = w; 
        }
    
    char (*newScreen)[80] = malloc(sizeof(char[21][80]));

    //create borders
    for(int l = 0; l < 80; l++){
        newScreen[0][l] = MOUNTAIN;
        newScreen[20][l] = MOUNTAIN;
        if(l == exitS)
             newScreen[20][l] = EXIT;
        if(l == exitN)
             newScreen[0][l] = EXIT;
    }
    for(int k = 0; k < 21; k++){
        newScreen[k][0] = MOUNTAIN;
        newScreen[k][79] = MOUNTAIN;
        if(k == exitE)
             newScreen[k][79] = EXIT;
        if(k == exitW)
             newScreen[k][0] = EXIT;
        
    }
    //create empty spaces using "-"
    for(int n = 1; n < 20; n++){
        for(int m = 1; m < 79; m++){
        newScreen[n][m] = '-';
        }
    }

    seeder(newScreen);
    roadPath(exitN, exitS, exitW, exitE, newScreen);
    martCenterHelper(newScreen);
    
    world[currWorldRow][currWorldCol] = (char*)malloc(sizeof(newScreen));

    world[currWorldRow][currWorldCol] = *newScreen;
  
}

void printMap(int row, int col){
    for(int i = 0; i < 21; i++){
        for(int j = 0; j < 80; j++){
            printf("%c", world[row][col][i*80+j]);

    }
        printf("\n");
    }
    printf("%s %dx%d\n", "We are at coord", currWorldRow, currWorldCol);

}

void initMap(){
    currWorldRow = 200;
    currWorldCol = 200;
    createMap(0,0,0,0);
    printMap(currWorldRow, currWorldCol);
}

int move(char dir){
    if(dir == 'n'){
        currWorldRow--;
        if(world[currWorldRow][currWorldCol] == NULL){
            createMap(0,0,0,0);
            printMap(currWorldRow, currWorldCol);  
        }
        else{
            printMap(currWorldRow, currWorldCol); 
        }
    }
    else if(dir == 's'){
        currWorldRow++;
        if(world[currWorldRow][currWorldCol] == NULL){
           createMap(0,0,0,0);
           printMap(currWorldRow, currWorldCol);
        }
        else{
            printMap(currWorldRow, currWorldCol); 
        }
    }
    else if(dir == 'w'){
        currWorldCol--;
        if(world[currWorldRow][currWorldCol] == NULL){
            createMap(0,0,0,0);
            printMap(currWorldRow, currWorldCol);
        }
        else{
            printMap(currWorldRow, currWorldCol); 
        }
    }
    else if(dir == 'e'){
        currWorldCol++;
        if(world[currWorldRow][currWorldCol] == NULL){
            createMap(0,0,0,0);
            printMap(currWorldRow, currWorldCol);
        }
        else{
            printMap(currWorldRow, currWorldCol); 
        }
    }
    // if(dir == 'f'){
    //     printf("%s\n", "Where to?");
    //     int x = getc(stdin);
    //     int y = getc(stdin);
    //     if(world[x][y] == NULL){
    //         createMap(0,0,0,0);
    //         printMap(x, y);
    //     }
    //     else{
    //         printMap(x, y); 
    //     }
    // }
}

int main(int argc, char *argv[]){
    char c;
    srand(time(NULL));

    initMap();

    //allows multiple chars FIX
    while((c = getc(stdin)) != 'q'){
        switch(c){
            case 'n':
                move('n');
                break;
            case 's':
                move('s');
                break;
            case 'w':
                move('w');
                break;
            case 'e':
                move('e');
                break;
            case 'f':
                move('f');
                break;
        }
    }
    return 0;
}