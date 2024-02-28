#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "queue.h"
#include "heap.h"
#include <unistd.h>

#define MAP_X 80
#define MAP_Y 21

#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]]);

#define INF 9999
#define numTrainers 9

//Individual map structure
typedef struct Map {
    char screen[21][80];
    int exitN;
    int exitS;
    int exitW;
    int exitE;
} map;

typedef struct distMap {
    int screen[21][80];
} distMap;

typedef struct path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
  char terrain;
} path_t;

typedef enum dim {
  dim_x,
  dim_y,
  num_dims
} dim_t;

typedef enum terrain_type {
  ter_debug,
  ter_boulder,
  ter_tree,
  ter_path,
  ter_mart,
  ter_center,
  ter_grass,
  ter_clearing,
  ter_mountain,
  ter_forest,
  ter_water,
  ter_gate,
  ter_player,
  ter_default
} terrain_type_t;

typedef struct character {
  heap_node_t *hn;
  char symbol;
  int dir;
  int next_turn;
  int sequence_num;
  int x;
  int y;
} character_t;



int player;

character_t NPC[numTrainers];

typedef int16_t pair_t[num_dims];

int currWorldRow, currWorldCol; //current world location

const char MOUNTAIN = '%';
const char LONGGRASS = ':';
const char CLEARING = '.';
const char FOREST = '^';
const char WATER = '~';
const char EXIT = '#';
const char CENTER = 'C';
const char PLAYER = '@';
const char MART = 'M';

const int NPC_cost[2][11] = {{INF, INF, 10, 50, 50, 15, 10, 15, 15, INF, INF},{INF, INF, 10, 50, 50, 20, 10, INF, INF, INF, INF}};

const char NPC_moves[8] = {'n','s','w','e','nw','se','ne','sw'}
//current exits for current screen
int currExitN;
int currExitS;
int currExitW;
int currExitE;

static int32_t path_cmp(const void *key, const void *with) {
  return ((path_t *) key)->cost - ((path_t *) with)->cost;
}

//2d array of pointers to map structs
map *world[401][401];

distMap *paths[2];

int pathFindCost(terrain_type_t t, int i){
    switch(t){
        case ter_boulder:
            return NPC_cost[i][0];
            break;
        case ter_tree:
            return NPC_cost[i][1];
            break;
        case ter_path:
            return NPC_cost[i][2];
            break;
        case ter_mart:
            return NPC_cost[i][3];
            break;
        case ter_center:
            return NPC_cost[i][4];
            break;
        case ter_grass:
            return NPC_cost[i][5];
            break;
        case ter_clearing:
            return NPC_cost[i][6];
            break;
        case ter_mountain:
            return NPC_cost[i][7];
            break;
        case ter_forest:
            return NPC_cost[i][8];
            break;
        case ter_water:
            return NPC_cost[i][9];
            break;
        case ter_gate:
            return NPC_cost[i][10];
            break;
        }
}
void placeMartandCenter(int i, int a, char screen[21][80]){
    screen[i][a-1] = ter_mart;
    screen[i][a-2] = ter_mart;
    screen[i-1][a-1] = ter_mart;
    screen[i-1][a-2] = ter_mart;
    screen[i][a+1] = ter_center;
    screen[i][a+2] = ter_center;
    screen[i-1][a+1] = ter_center;
    screen[i-1][a+2] = ter_center;
}

void martCenterHelper(char screen[21][80]){
    int mandc = rand() % 17 + 2; //pokecenter and mart between 2 and 18
    for(int i = 0; i < 79; i++){
        if (screen[mandc][i] == ter_path){
            //checks surrounding area to make sure there are no paths interfering with possible mart and center placement.
            if (screen[mandc][i+1] != ter_path && screen[mandc][i+2] != ter_path && screen[mandc-1][i+1] != ter_path && screen[mandc-1][i+2] != ter_path && screen[mandc-1][i-1] != ter_path && screen[mandc-1][i-2] != ter_path){
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

static void dijkstras_path(char screen[21][80], int player, int character) //0 for hiker, 1 for rival
{
  heap_t h;
  uint32_t x, y;  
  static path_t path[MAP_Y][MAP_X],  *p;
  static uint32_t initialized = 0;
 

  if (!initialized) {
    for (y = 0; y < MAP_Y; y++) {
      for (x = 0; x < MAP_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }

  paths[character] = malloc(sizeof(distMap));
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
        paths[character]->screen[y][x] = INF;
        path[y][x].cost = INF;
        path[y][x].terrain = screen[y][x];
    }
  }

  path[player % 79][player / 79].cost = 0;
  paths[character]->screen[player % 79][player / 79] = 0;

  heap_init(&h, path_cmp, NULL);
  
  for (y = 0; y < MAP_Y ; y++) {
    for (x = 0; x < MAP_X; x++) { 
        path[y][x].hn = heap_insert(&h, &path[y][x]);
    }
  }


  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    int cost = pathFindCost(path[p->pos[dim_y] - 1][p->pos[dim_x]    ].terrain, character);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] - 1][p->pos[dim_x]    ] = ((p->cost + cost));
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn);
    }
   
    cost = pathFindCost(path[p->pos[dim_y]    ][p->pos[dim_x] - 1].terrain, character);
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y]][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y]][p->pos[dim_x] - 1] = ((p->cost + cost));
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn);
    }

    cost = pathFindCost(path[p->pos[dim_y]   ][p->pos[dim_x]  + 1].terrain, character);
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y]][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y]][p->pos[dim_x] + 1] =  ((p->cost + cost));
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn);
    }

    cost = pathFindCost(path[p->pos[dim_y] + 1][p->pos[dim_x]    ].terrain, character);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] + 1][p->pos[dim_x]    ] = ((p->cost + cost)); 
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn);
    }

    cost = pathFindCost(path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].terrain, character);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] - 1][p->pos[dim_x] - 1] = ((p->cost + cost));
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn);
    }

    cost = pathFindCost(path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].terrain, character);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] + 1][p->pos[dim_x] + 1] = ((p->cost + cost));
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn);
    }
    cost = pathFindCost(path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].terrain, character);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] - 1][p->pos[dim_x] + 1] =  ((p->cost + cost));
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn);
    }

    cost = pathFindCost(path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].terrain, character);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] + 1][p->pos[dim_x] - 1] = ((p->cost + cost)); 
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn);
    }
  }

  heap_delete(&h);
      return;
}

//TODO: implement bfs or dijkstras for this instead of dummy path finding
void roadPath(int a, int b, int c, int d, char screen[21][80]){    

    int mandc = rand() % 16 + 3;
    if (mandc == 12){
        mandc = mandc + 1;
    }
    //source is exitN targer is exitS
    int i, h;
    
    //move down
    for(i = 1; i < 12; i++){
        screen[i][a] = ter_path;
    }

    //check which exit is lower and move horizontally left or right
    if(a < b){
        for(int k = a; k <= b; k++){
            screen[i][k]= ter_path;
        }
    }
    else if(a > b){
        for(int k = a; k >= b; k--){
            screen[i][k]= ter_path;    
        }
    }

    // move down until exit
    for(int j = i+1; j < 20; j++){
        screen[j][b]= ter_path; 
    }
        
    //source is exitW target is exitE


    // move right
    for(h = 1; h < 40; h++){
        screen[c][h] = ter_path;
    }
    
    //check which exit is lower and move vertically up or down
    if(c < d){
        for(int k = c; k <= d; k++){
            screen[k][h]= ter_path;
        }
    }
    else if (c > d){
        for(int k = c; k >= d; k--){
            screen[k][h]= ter_path;    
        }
    }
    
    //move right until exit
    for(int j = h; j < 79; j++){
        screen[d][j] = ter_path;    
    }
   }

//Probability calculation if pokecenters and marts will appear
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

//Seeds all of the terrain in a map
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


    screen[longGrassCoord1 % 79][longGrassCoord1 / 79] = ter_grass;
    screen[longGrassCoord2 % 79][longGrassCoord2 / 79] = ter_grass;
    screen[mountainsCoord % 79][mountainsCoord / 79] = ter_mountain;
    screen[clearingCoord1 % 79][clearingCoord1 / 79] = ter_clearing;
    screen[clearingCoord2 % 79][clearingCoord2 / 79] = ter_clearing;
    screen[forestCoord % 79][forestCoord / 79] = ter_forest;
    screen[waterCoord % 79][waterCoord / 79] = ter_water;
    
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
        if(screen[x][y] == ter_mountain)
            seed = ter_mountain;
        else if(screen[x][y] == ter_grass)
            seed = ter_grass;
        else if(screen[x][y] == ter_clearing)
            seed = ter_clearing;
        else if(screen[x][y] == ter_forest)
            seed = ter_forest;
        else if(screen[x][y] == ter_water)
            seed = ter_water;
        if(screen[x+1][y] == ter_default){
            screen[x+1][y] = seed;
            enqueue(queue, &tail, (y * 79 + (x+1)));
        }
        if(screen[x-1][y] == ter_default){
            screen[x-1][y] = seed;
            enqueue(queue, &tail, (y * 79 + (x-1)));
        }
        if(screen[x][y+1] == ter_default){
            screen[x][y+1] = seed;
            enqueue(queue, &tail, ((y+1) * 79 + x));
        }
        if(screen[x][y-1] == ter_default){
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

//places player character randomly on road
void placePlayer(char screen[21][80]){
    player = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    while(screen[player % 79][player / 79] != ter_path){
        player = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    }
    screen[player % 79][player / 79] = ter_player;
}

//Creates map
void createMap(){
    world[currWorldRow][currWorldCol]= malloc(sizeof(map));

    world[currWorldRow][currWorldCol]->exitS = currExitN;
    world[currWorldRow][currWorldCol]->exitN = currExitS;
    world[currWorldRow][currWorldCol]->exitE = currExitE;
    world[currWorldRow][currWorldCol]->exitW = currExitW;

    //create borders
    for(int l = 0; l < 80; l++){
        world[currWorldRow][currWorldCol]->screen[0][l] = ter_boulder;
        world[currWorldRow][currWorldCol]->screen[20][l] = ter_boulder;
        if(l == currExitS && currWorldRow != 400)
             world[currWorldRow][currWorldCol]->screen[20][l] = ter_gate;
        if(l == currExitN && currWorldRow != 0)
             world[currWorldRow][currWorldCol]->screen[0][l] = ter_gate;
    }
    for(int k = 0; k < 21; k++){
        world[currWorldRow][currWorldCol]->screen[k][0] = ter_boulder;
        world[currWorldRow][currWorldCol]->screen[k][79] = ter_boulder;
        if(k == currExitE && currWorldCol != 400)
             world[currWorldRow][currWorldCol]->screen[k][79] = ter_gate;
        if(k == currExitW && currWorldCol != 0)
             world[currWorldRow][currWorldCol]->screen[k][0] = ter_gate;
    }
    //create empty spaces using "-"
    for(int n = 1; n < 20; n++){
        for(int m = 1; m < 79; m++){
        world[currWorldRow][currWorldCol]->screen[n][m] = ter_default;
        }
    }

    seeder(world[currWorldRow][currWorldCol]->screen);
    roadPath(currExitN, currExitS, currExitW, currExitE, world[currWorldRow][currWorldCol]->screen);
    double randomNum = ((double) rand()) / RAND_MAX;
    if(randomNum < manhattanProb()){
        martCenterHelper(world[currWorldRow][currWorldCol]->screen);
    }
    placePlayer(world[currWorldRow][currWorldCol]->screen);

    dijkstras_path(world[currWorldRow][currWorldCol]->screen, player, 0);
    dijkstras_path(world[currWorldRow][currWorldCol]->screen, player, 1);

}

//prints map to IO
void printMap(){
    int check;
    for(int i = 0; i < 21; i++){
        for(int j = 0; j < 80; j++){
            check = 1;
            for(int k = 0; k < 2; k++){
                if(NPC[k].y == i && NPC[k].x == j){
                    putchar(NPC[k].symbol);
                    check = 0;
                    break;
                }
            }
            if(check == 1){
                switch (world[currWorldRow][currWorldCol]->screen[i][j]){
                    case ter_boulder:
                    case ter_mountain:
                        putchar('%');
                        break;
                    case ter_tree:
                    case ter_forest:
                        putchar('^');
                        break;
                    case ter_path:
                    case ter_gate:
                        putchar('#');
                        break;
                    case ter_mart:
                        putchar('M');
                        break;
                    case ter_center:
                        putchar('C');
                        break;
                    case ter_grass:
                        putchar(':');
                        break;
                    case ter_clearing:
                        putchar('.');
                        break;
                    case ter_water:
                        putchar('~');
                        break;
                    case ter_player:
                        putchar('@');
                        break;
                    }
                }
            }
        putchar('\n');
    }

    printf("%s %dx%d\n\n\n", "You are at coordinate: ", currWorldCol-200, currWorldRow-200);
}


void moveNPCs(character_t *t){
    //if hiker
    if(t->symbol == 'h'){
        int minX = t->x;
        int minY = t->y;
        int min = INF;

        if(min > paths[0]->screen[minY-1][minX] && paths[0]->screen[minY-1][minX] != 0){
            min = paths[0]->screen[minY-1][minX];
            t->y = minY-1;
            t->x = minX;
        }
        if(min > paths[0]->screen[minY][minX-1] && paths[0]->screen[minY][minX-1] != 0){
            t->y = minY;
            t->x = minX-1;
            min = paths[0]->screen[minY][minX-1];
        }
        if (min > paths[0]->screen[minY-1][minX-1] && paths[0]->screen[minY-1][minX-1] != 0){
            t->y = minY-1;
            t->x = minX-1;
            min = paths[0]->screen[minY-1][minX-1];
        }
        if (min > paths[0]->screen[minY+1][minX] && paths[0]->screen[minY+1][minX] != 0){
            t->y = minY+1;
            t->x = minX;
            min = paths[0]->screen[minY+1][minX];
        }
        if (min > paths[0]->screen[minY][minX+1] && paths[0]->screen[minY][minX+1] != 0){
            t->y = minY;
            t->x = minX+1;
            min = paths[0]->screen[minY][minX+1];
        }
        if (min > paths[0]->screen[minY+1][minX+1] && paths[0]->screen[minY+1][minX+1] != 0){
            t->y = minY+1;
            t->x = minX+1;
            min = paths[0]->screen[minY+1][minX+1];
        }
        if (min > paths[0]->screen[minY-1][minX+1] && paths[0]->screen[minY-1][minX+1] != 0){
            t->y = minY-1;
            t->x = minX+1;
            min = paths[0]->screen[minY-1][minX+1];
        }
        if (min > paths[0]->screen[minY+1][minX-1] && paths[0]->screen[minY+1][minX-1] != 0){
            t->y = minY+1;
            t->x = minX-1;
            min = paths[0]->screen[minY+1][minX-1];
        }
    }
    //if rival
    if(t->symbol == 'r'){
        int minX = t->x;
        int minY = t->y;
        int min = INF;

        if(min > paths[1]->screen[minY-1][minX] && paths[1]->screen[minY-1][minX] != 1){
            min = paths[1]->screen[minY-1][minX];
            t->y = minY-1;
            t->x = minX;
        }
        if(min > paths[1]->screen[minY][minX-1] && paths[1]->screen[minY][minX-1] != 1){
            t->y = minY;
            t->x = minX-1;
            min = paths[1]->screen[minY][minX-1];
        }
        if (min > paths[1]->screen[minY-1][minX-1] && paths[1]->screen[minY-1][minX-1] != 1){
            t->y = minY-1;
            t->x = minX-1;
            min = paths[1]->screen[minY-1][minX-1];
        }
        if (min > paths[1]->screen[minY+1][minX] && paths[1]->screen[minY+1][minX] != 1){
            t->y = minY+1;
            t->x = minX;
            min = paths[1]->screen[minY+1][minX];
        }
        if (min > paths[1]->screen[minY][minX+1] && paths[1]->screen[minY][minX+1] != 1){
            t->y = minY;
            t->x = minX+1;
            min = paths[1]->screen[minY][minX+1];
        }
        if (min > paths[1]->screen[minY+1][minX+1] && paths[1]->screen[minY+1][minX+1] != 1){
            t->y = minY+1;
            t->x = minX+1;
            min = paths[1]->screen[minY+1][minX+1];
        }
        if (min > paths[1]->screen[minY-1][minX+1] && paths[1]->screen[minY-1][minX+1] != 1){
            t->y = minY-1;
            t->x = minX+1;
            min = paths[1]->screen[minY-1][minX+1];
        }
        if (min > paths[1]->screen[minY+1][minX-1] && paths[1]->screen[minY+1][minX-1] != 1){
            t->y = minY+1;
            t->x = minX-1;
            min = paths[1]->screen[minY+1][minX-1];
        }
    }
    if(t->symbol = 'w'){ //wanderer
        char terr = world[currWorldRow][currWorldCol]->screen[minY-1][minX];
        if(){ //if hit NEW terrain random 
            t->dir = rand() % 8;
        }
        moveFromDirection(t);
    }
    if(t->symbol = 'e'){ //explorer
        char terr = world[currWorldRow][currWorldCol]->screen[minY-1][minX];
        if(){ //if hit IMPASSIBLE terrain random 
            t->dir = rand() % 8;
        }
        moveFromDirection(t);
    }
     if(t->symbol = 'p'){ //pacers
        char terr = world[currWorldRow][currWorldCol]->screen[minY-1][minX];
        if(){ //if hit terrain reverse 
            if(direction == 'n'){
                direction = 's';
            }
            else if(direction == 'w'){
                direction = 'e';
            }
            else if (direction == 'nw'){
                direction = 'se';
            }
            else if (direction == 's'){
                direction = 'n';
            }
            else if (direction == 'e'){
               direction = 'w';
            }
            else if (direction == 'se'){
                direction = 'nw';
            }
            else if (direction == 'ne'){
                direction = 'sw';
            }
            else if (direction == 'sw'){
                direction = 'ne';
            }
        }
        moveFromDirection(t);
    }
}

void moveFromDirection(character_t *t){
    char direction = NPC_move[t->dir];
    if(direction == 'n'){
            t->y = minY-1;
            t->x = minX;
        }
        if(direction == 'w'){
            t->y = minY;
            t->x = minX-1;
        }
        if (direction == 'nw'){
            t->y = minY-1;
            t->x = minX-1;
        }
        if (direction == 's'){
            t->y = minY+1;
            t->x = minX;
        }
        if (direction == 'e'){
            t->y = minY;
            t->x = minX+1;
        }
        if (direction == 'se'){
            t->y = minY+1;
            t->x = minX+1;
        }
        if (direction == 'ne'){
            t->y = minY-1;
            t->x = minX+1;
        }
        if (direction == 'sw'){
            t->y = minY+1;
            t->x = minX-1;
        }
}
//initialize npcs on single map
void traffic(){
    static character_t *p;
    heap_t h;
    
    heap_init(&h, path_cmp, NULL);

    for(int i = 0; i < 2; i++){
        heap_insert(&h, &NPC[i]);
    }

    while ((p = heap_remove_min(&h))) {
        p->hn = NULL;
        moveNPCs(p);
        printMap();
        heap_insert(&h, p);
        usleep(250000);
    }
}

void initNPCs(int numtrainers){

    character_t* rival = malloc(sizeof(character_t));

    rival->symbol = 'r';
    int rival_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    rival->y = rival_coord % 79;
    rival->x = rival_coord / 79;
    
    character_t* hiker = malloc(sizeof(character_t));

    hiker->symbol = 'h';
    int hiker_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    hiker->y = hiker_coord % 79;
    hiker->x = hiker_coord / 79;

    NPC[0] = *hiker;
    NPC[1] = *rival;
    numtrainers -= 2;

    while(numtrainers){
        
        numtrainers--;
    }

}
void initMap(){
    currWorldRow = 200;
    currWorldCol = 200;
    setExits();
    createMap(currExitN,currExitS,currExitE,currExitW);
    initNPCs(numTrainers);
    traffic();
}

int main(int argc, char *argv[]){
    char c;
    srand(time(NULL));

    initMap();

    // while((c = getc(stdin)) != 'q'){
    //     switch(c){
    //         case 'n':
    //             move('n');
    //             break;
    //         case 's':
    //             move('s');
    //             break;
    //         case 'w':
    //             move('w');
    //             break;
    //         case 'e':
    //             move('e');
    //             break;
    //         case 'f':
    //             move('f');
    //             break;
    //     }
    // }
    
    return 0;
}