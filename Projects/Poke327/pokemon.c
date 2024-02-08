#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "queue.h"

//Individual map structure
typedef struct Map {
    char screen[21][80];
    int exitN;
    int exitS;
    int exitW;
    int exitE;
} map;

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
int currExitN;
int currExitS;
int currExitW;
int currExitE;

map *world[401][401];

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
    for(int j = h; j < 79; j++){
        screen[d][j]= EXIT;    
        }
   }

//probability calculation if pokecenters and marts will appear
double manhattanProb(){
    if(!(currWorldRow == 200 && currWorldCol == 200)){
        double d = abs(currWorldRow-200) + abs(currWorldCol-200);
        if(d > 200){
            return .05;
        }
        double equation = d * -45;
        equation /= 200.00;
        equation += 50;
        equation /= 100.00;
        return equation;
    }
    return 1.0;
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
//Set exits for a newly created map and line them up with existing maps
void setExits(){
    if(world[currWorldRow+1][currWorldCol] != NULL){
        currExitS = world[currWorldRow+1][currWorldCol]->exitS;
    }
    else{
        currExitS= rand() % 74 + 3;
    }
    if(world[currWorldRow-1][currWorldCol] != NULL){
        currExitN = world[currWorldRow-1][currWorldCol]->exitN;
    }
    else{
        currExitN= rand() % 74 + 3;
    }
    if(world[currWorldRow][currWorldCol+1] != NULL){
        currExitE = world[currWorldRow][currWorldCol+1]->exitW;
    }
    else{
        currExitE= rand() % 15 + 3;
    }
    if(world[currWorldRow][currWorldCol-1] != NULL){
        currExitW = world[currWorldRow][currWorldCol-1]->exitE;
    }
    else{
        currExitW= rand() % 15 + 3;
    }
}

void createMap(){

    map* newMap = malloc(sizeof(map));

    newMap->exitS = currExitN;
    newMap->exitN = currExitS;
    newMap->exitE = currExitE;
    newMap->exitW = currExitW;

    //create borders
    for(int l = 0; l < 80; l++){
        newMap->screen[0][l] = MOUNTAIN;
        newMap->screen[20][l] = MOUNTAIN;
        if(l == currExitS && currWorldRow != 400)
             newMap->screen[20][l] = EXIT;
        if(l == currExitN && currWorldRow != 0)
             newMap->screen[0][l] = EXIT;
    }
    for(int k = 0; k < 21; k++){
        newMap->screen[k][0] = MOUNTAIN;
        newMap->screen[k][79] = MOUNTAIN;
        if(k == currExitE && currWorldCol != 400)
             newMap->screen[k][79] = EXIT;
        if(k == currExitW && currWorldCol != 0)
             newMap->screen[k][0] = EXIT;
    }
    //create empty spaces using "-"
    for(int n = 1; n < 20; n++){
        for(int m = 1; m < 79; m++){
        newMap->screen[n][m] = '-';
        }
    }

    seeder(newMap->screen);
    roadPath(currExitN, currExitS, currExitW, currExitE, newMap->screen);
    double randomNum = ((double) rand()) / RAND_MAX;
    if(randomNum < manhattanProb()){
        martCenterHelper(newMap->screen);
    }
    world[currWorldRow][currWorldCol] = newMap;
}

void printMap(){
    for(int i = 0; i < 21; i++){
        for(int j = 0; j < 80; j++){
            printf("%c", world[currWorldRow][currWorldCol]->screen[i][j]);
        }
        printf("\n");
    }
    printf("%s %dx%d\n", "You are at coordinate: ", currWorldCol-200, currWorldRow-200);

}

void initMap(){
    currWorldRow = 200;
    currWorldCol = 200;
    setExits();
    createMap(currExitN,currExitS,currExitE,currExitW);
    printMap();
}

int move(char dir){
    if(dir == 'n'){
        if(currWorldRow != 0){
            currWorldRow--;
            if(world[currWorldRow][currWorldCol] == NULL){
                setExits();
                createMap(currExitN,currExitS,currExitE,currExitW);
                printMap();
            }
            else{
                printMap(); 
            }
        }
        else{
            printf("Can't move there. OUT OF BOUNDS!\n");
        } 
    }
    else if(dir == 's'){
        if(currWorldRow != 400){
            currWorldRow++;
            if(world[currWorldRow][currWorldCol] == NULL){
                setExits();
                createMap(currExitN,currExitS,currExitE,currExitW);
                printMap();
            }
            else{
                printMap(); 
            }
        }
        else{
            printf("Can't move there. OUT OF BOUNDS!\n");
        }
    }
    else if(dir == 'w'){
        if(currWorldCol != 0){
            currWorldCol--;
            if(world[currWorldRow][currWorldCol] == NULL){
                setExits();
                createMap(currExitN,currExitS,currExitE,currExitW);
                printMap();
            }
            else{
                printMap(); 
            }
        }
        else{
            printf("Can't move there. OUT OF BOUNDS!\n");
        }
    }
    else if(dir == 'e'){
        if(currWorldCol != 400){
            currWorldCol++;
            if(world[currWorldRow][currWorldCol] == NULL){
                setExits();
                createMap(currExitN,currExitS,currExitE,currExitW);
                printMap();
            }
            else{
                printMap(); 
            }
        }
        else{
            printf("Can't move there. OUT OF BOUNDS!\n");
        }
    }
    if(dir == 'f'){
        int x;
        int y;
        printf("Where to?\n");
        printf("Enter X Coordinate: ");
        scanf("%d",&x);
        printf("Enter Y Coordinate: ");
        scanf("%d",&y);
        if(x < 201 && y < 201){
            currWorldRow = abs(y+200);
            currWorldCol = abs(x+200);
            if(world[currWorldRow][currWorldCol] == NULL){
                setExits();
                createMap(currExitN,currExitS,currExitE,currExitW);
                printMap();
            }
            else{
                printMap(); 
            }
        }
        else{
            printf("Can't move there. OUT OF BOUNDS!\n");
        }
       
    }
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