#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "queue.h"
#include "heap.h"

#define MAP_X 80
#define MAP_Y 21

#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]]);

//Individual map structure
typedef struct Map {
    char screen[21][80];
    int exitN;
    int exitS;
    int exitW;
    int exitE;
} map;

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

typedef struct player {
  int x;
  int y;
} player_t;

int player;

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

int rivalPaths[21][80];

int hikerPaths[21][80];

int hikerCost(terrain_type_t t){
    if(t == ter_mountain){
        return 15;
    }
    if(t == ter_boulder){
        return SHRT_MAX;
    }
    if(t == ter_gate){
        return SHRT_MAX;
    }
    if(t == ter_grass){
        return 15;
    }
    if(t == ter_clearing){
        return 10;
    }
    if(t == ter_forest){
        return 15;
    }
    if(t == ter_water){
        return SHRT_MAX;
    }
    if(t == ter_mart){
        return 50;
    }
    if(t == ter_center){
        return 50;
    }
    if(t == ter_path){
        return 10;
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

static void hiker_path(char screen[21][80], int player)
{
  static path_t path[MAP_Y][MAP_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t x, y;

  if (!initialized) {
    for (y = 0; y < MAP_Y; y++) {
      for (x = 0; x < MAP_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }
  
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      path[y][x].cost = SHRT_MAX;
      path[y][x].terrain = screen[y][x];
    }
  }

  path[player % 79][player / 79].cost = 0;

  heap_init(&h, path_cmp, NULL);
  
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      path[y][x].hn = heap_insert(&h, &path[y][x]);
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    // if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
    //   for (x = to[dim_x], y = to[dim_y];
    //        (x != from[dim_x]) || (y != from[dim_y]);
    //        p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
    //     // // Don't overwrite the gate
    //     // if (x != to[dim_x] || y != to[dim_y]) {
    //     //   mapxy(x, y) = ter_path;
    //     //   heightxy(x, y) = 0;
    //     // }
    //   }
    //   heap_delete(&h);
    //   return;
    // }

    int cost = hikerCost(path[p->pos[dim_y] - 1][p->pos[dim_x]    ].terrain);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost = ((p->cost + cost));
      hikerPaths[p->pos[dim_y] - 1][p->pos[dim_x]    ] = path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost;
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn);
    }
    cost = hikerCost(path[p->pos[dim_y]    ][p->pos[dim_x] - 1].terrain);
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y]][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      hikerPaths[p->pos[dim_y]][p->pos[dim_x] - 1] = path[p->pos[dim_y]][p->pos[dim_x] - 1].cost;
    //   printf("%d\n", hikerPaths[p->pos[dim_y]][p->pos[dim_x] - 1]  );
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn);
    }
    cost = hikerCost(path[p->pos[dim_y]   ][p->pos[dim_x]  + 1].terrain);
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y]][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      hikerPaths[p->pos[dim_y]][p->pos[dim_x] + 1] =  path[p->pos[dim_y]][p->pos[dim_x] + 1].cost;
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn);
    }
    cost = hikerCost(path[p->pos[dim_y] + 1][p->pos[dim_x]    ].terrain);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost = ((p->cost + cost));
      hikerPaths[p->pos[dim_y] + 1][p->pos[dim_x]    ] = path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost; 
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn);
    }
    cost = hikerCost(path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].terrain);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      hikerPaths[p->pos[dim_y] - 1][p->pos[dim_x] - 1] = path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost;
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn);
    }
    cost = hikerCost(path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].terrain);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      hikerPaths[p->pos[dim_y] + 1][p->pos[dim_x] + 1] = path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost;
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn);
    }
    cost = hikerCost(path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].terrain);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      hikerPaths[p->pos[dim_y] - 1][p->pos[dim_x] + 1] =  path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost;
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn);
    }
    cost = hikerCost(path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].terrain);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      hikerPaths[p->pos[dim_y] + 1][p->pos[dim_x] - 1] = path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost; 
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

    map* newMap = malloc(sizeof(map));

    // pair_t player = malloc(sizeof(pair_t));

    newMap->exitS = currExitN;
    newMap->exitN = currExitS;
    newMap->exitE = currExitE;
    newMap->exitW = currExitW;

    //create borders
    for(int l = 0; l < 80; l++){
        newMap->screen[0][l] = ter_boulder;
        newMap->screen[20][l] = ter_boulder;
        if(l == currExitS && currWorldRow != 400)
             newMap->screen[20][l] = ter_gate;
        if(l == currExitN && currWorldRow != 0)
             newMap->screen[0][l] = ter_gate;
    }
    for(int k = 0; k < 21; k++){
        newMap->screen[k][0] = ter_boulder;
        newMap->screen[k][79] = ter_boulder;
        if(k == currExitE && currWorldCol != 400)
             newMap->screen[k][79] = ter_gate;
        if(k == currExitW && currWorldCol != 0)
             newMap->screen[k][0] = ter_gate;
    }
    //create empty spaces using "-"
    for(int n = 1; n < 20; n++){
        for(int m = 1; m < 79; m++){
        newMap->screen[n][m] = ter_default;
        }
    }

    seeder(newMap->screen);
    roadPath(currExitN, currExitS, currExitW, currExitE, newMap->screen);
    double randomNum = ((double) rand()) / RAND_MAX;
    if(randomNum < manhattanProb()){
        martCenterHelper(newMap->screen);
    }
    placePlayer(newMap->screen);
    world[currWorldRow][currWorldCol] = newMap;
    hiker_path(newMap->screen, player);
}

void printMap(){
    for(int i = 0; i < 21; i++){
        for(int j = 0; j < 80; j++){
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
        putchar('\n');
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

    for(int i = 0; i < 21; i++){
        for(int j = 0; j < 80; j++){
            if(hikerPaths[i][j] == 0){
                printf("   ");
            }
            else
                printf("%d ", hikerPaths[i][j]);
        }
        printf("\n");
    }

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