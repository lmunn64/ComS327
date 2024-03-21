#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "queue.h"
#include "heap.h"
#include <unistd.h>
#include <string.h>
#include <menu.h>
#include <ncurses.h>

#define MAP_X 80
#define MAP_Y 21

#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]]);

#define INF 9999

#define NPCS numTrainers

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define CTRLD 	4

char* topper = "------Welcome trainers, to Pokemon for C!------";

int numTrainers = 6;

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


typedef struct heapNode{
    int weight;
    int mapx;
    int mapy;
    int totalDist;
    int id;
} heapNode;

//Individual map structure
typedef struct Map {
    terrain_type_t screen[21][80];
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

char *trainers[6];

typedef struct character {
  heap_node_t *hn;
  char symbol; 
  int dir;
  int next_turn;
  int sequence_num;
  int x;
  int y;
  int defeated;
} character_t;


int player;

character_t NPC[INF];

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

const int NPC_cost[4][11] = {{INF, INF, 10, 50, 50, 15, 10, 15, 15, INF, INF},{INF, INF, 10, 50, 50, 20, 10, INF, INF, INF, INF},{INF, INF, 10, 50, 50, 20, 10, 30, INF, INF, INF},{INF, INF, 10, 10, 10, 20, 10, INF, INF, INF, INF}};

char *NPC_moves[] = {"n","s","w","e","nw","se","ne","sw"};

//current exits for current screen
int currExitN;
int currExitS;
int currExitW;
int currExitE;

static int32_t path_cmp(const void *key, const void *with) {
  return ((path_t *) key)->cost - ((path_t *) with)->cost;
}

static int32_t turn_cmp(const void *key, const void *with) {
  return ((character_t *) key)->next_turn - ((character_t *) with)->next_turn;
}
//2d array of pointers to map structs
map *world[401][401];

distMap *paths[2];

void moveNPCs(character_t *t); 

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

void placeMartandCenter(int i, int a, terrain_type_t screen[21][80]){
    screen[i][a-1] = ter_mart;
    screen[i][a-2] = ter_mart;
    screen[i-1][a-1] = ter_mart;
    screen[i-1][a-2] = ter_mart;
    screen[i][a+1] = ter_center;
    screen[i][a+2] = ter_center;
    screen[i-1][a+1] = ter_center;
    screen[i-1][a+2] = ter_center;
}

void martCenterHelper(terrain_type_t screen[21][80]){
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

static void dijkstras_path(terrain_type_t screen[21][80], int character){ //0 for hiker, 1 for rival
  heap_t h;
  uint32_t x, y;  
  path_t path[MAP_Y][MAP_X];
  path_t *p;

    for (y = 0; y < MAP_Y; y++) {
        for (x = 0; x < MAP_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
        }
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
  
  for (y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) { 
        path[y][x].hn = heap_insert(&h, &path[y][x]);
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    int cost = pathFindCost(path[p->pos[dim_y] - 1][p->pos[dim_x]    ].terrain, character);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] - 1][p->pos[dim_x]    ] = ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y] - 1][p->pos[dim_x]    ]);
    }
   
    cost = pathFindCost(path[p->pos[dim_y]    ][p->pos[dim_x] - 1].terrain, character);
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y]][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y]][p->pos[dim_x] - 1] = ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y]    ][p->pos[dim_x] - 1]);
    }

    cost = pathFindCost(path[p->pos[dim_y]   ][p->pos[dim_x]  + 1].terrain, character);
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y]][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y]][p->pos[dim_x] + 1] =  ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y]    ][p->pos[dim_x] + 1]);
    }

    cost = pathFindCost(path[p->pos[dim_y] + 1][p->pos[dim_x]    ].terrain, character);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] + 1][p->pos[dim_x]    ] = ((p->cost + cost)); 
      heap_insert(&h, &path[p->pos[dim_y] + 1][p->pos[dim_x]    ]);
    }

    cost = pathFindCost(path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].terrain, character);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] - 1][p->pos[dim_x] - 1] = ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y] - 1][p->pos[dim_x] - 1]);
    }

    cost = pathFindCost(path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].terrain, character);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] + 1][p->pos[dim_x] + 1] = ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y] + 1][p->pos[dim_x] + 1]);
    }
    cost = pathFindCost(path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].terrain, character);
    if ((path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].hn) && (path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x] + 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] - 1][p->pos[dim_x] + 1] =  ((p->cost + cost));
      heap_insert(&h, &path[p->pos[dim_y] - 1][p->pos[dim_x] + 1]);
    }

    cost = pathFindCost(path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].terrain, character);
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].hn) && (path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost > ((p->cost + cost)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x] - 1].cost = ((p->cost + cost));
      paths[character]->screen[p->pos[dim_y] + 1][p->pos[dim_x] - 1] = ((p->cost + cost)); 
      heap_insert(&h, &path[p->pos[dim_y] + 1][p->pos[dim_x] - 1]);
    }
  }

  heap_delete(&h);
  return;
}


//TODO: implement bfs or dijkstras for this instead of dummy path finding
void roadPath(int a, int b, int c, int d, terrain_type_t screen[21][80]){    

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
int seeder(terrain_type_t screen[21][80]){
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
void placePlayer(terrain_type_t screen[21][80]){
    player = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    while(screen[player % 79][player / 79] != ter_path){
        player = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
    }
    character_t* pc = malloc(sizeof(character_t));
    pc->symbol = '@';
    pc->x = player / 79;
    pc->y = player % 79;
    pc->sequence_num = 0;
    pc->next_turn = 10;
    NPC[0] = *pc;
}

//Creates map
void createMap(){

    world[currWorldRow][currWorldCol] = malloc(sizeof(map));

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

    dijkstras_path(world[currWorldRow][currWorldCol]->screen, 0);
    dijkstras_path(world[currWorldRow][currWorldCol]->screen, 1);

}

void displayWarning(char* message){
     mvprintw(0, (MAP_X - strlen(message)) / 2, "%s", message);
     refresh();
     usleep(1500000);
     mvprintw(0, (MAP_X - strlen(topper)) / 2, "%s", topper);
     refresh();
}
//prints map to IO
void printMap(){
    int check;
    mvprintw(0, (MAP_X - strlen(topper)) / 2, "%s", topper);
    for(int i = 1; i < MAP_Y+1; i++){
        for(int j = 0; j < MAP_X; j++){
            check = 1;
            for(int k = 0; k < numTrainers; k++){
                if(NPC[k].y == i-1 && NPC[k].x == j){
                    mvprintw(i, j, "%c", NPC[k].symbol);
                    check = 0;
                    break;
                }
            }
            if(check == 1){
                switch (world[currWorldRow][currWorldCol]->screen[i-1][j]){
                    case ter_boulder:
                    case ter_mountain:
                        mvprintw(i, j, "%c", '%');
                        break;
                    case ter_tree:
                    case ter_forest:
                        mvprintw(i, j, "%c", '^');
                        break;
                    case ter_path:
                    case ter_gate:
                        mvprintw(i, j, "%c", '#');
                        break;
                    case ter_mart:
                        mvprintw(i, j, "%c", 'M');
                        break;
                    case ter_center:
                        mvprintw(i, j, "%c", 'C');
                        break;
                    case ter_grass:
                        mvprintw(i, j, "%c", ':');
                        break;
                    case ter_clearing:
                        mvprintw(i, j, "%c", '.');
                        break;
                    case ter_water:
                        mvprintw(i, j, "%c", '~');
                        break;
                    }
                }
            }
        // mvprintw(j, i, "%c" '/n')
    }

    // printw("%s %dx%d\n\n\n", "You are at coordinate: ", currWorldCol-200, currWorldRow-200);
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}

void fillTrainerArray(){
    int i;
    for(i = 0; i < numTrainers; i++){
        if(NPC[i + 1].symbol == 'r')
            trainers[i] = "Rival";
        if(NPC[i + 1].symbol == 'h')
            trainers[i] = "Hiker";
        if(NPC[i + 1].symbol == 's')
            trainers[i] = "Sentry";
        if(NPC[i + 1].symbol == 'e')
            trainers[i] = "Explorer";
        if(NPC[i + 1].symbol == 'p')
            trainers[i] = "Pacer";
        if(NPC[i + 1].symbol == 'w')
            trainers[i] = "Wanderer";
    }
    trainers[numTrainers] = (char *)NULL;
}

void displayMandCInterface(int display){
    WINDOW *win;
    refresh();
    int winWidth = 40;
    int winLen = 10;
    char *title;
    char c;
    
    win = newwin(10, 40, (MAP_Y - winLen) / 2, (MAP_X - winWidth) / 2);
    box(win,0,0);

    if(display == 0){
        mvwprintw(win, 0, (winWidth - strlen("Welcome to the PokeMart!")) / 2, "Welcome to the PokeMart!");
        mvwprintw(win, 2, (winWidth - 32) / 2, "You can buy various items here!");
    }
    else{
        mvwprintw(win, 0, (winWidth - strlen("Welcome to the PokeCenter!")) / 2, "Welcome to the PokeCenter!");
        mvwprintw(win, 2, (winWidth - 32) / 2, "You can heal your Pokemon here!");
    }

    wrefresh(win);
    refresh();

    while((c = wgetch(win)) != 27){       
        wrefresh(win);
    }
}

void displayBattle(){
    WINDOW *win;
    refresh();
    int winWidth = 40;
    int winLen = 10;
    char *title;
    char c;
 
    win = newwin(10, 40, (MAP_Y - winLen) / 2, (MAP_X - winWidth) / 2);
    box(win,0,0);

    mvwprintw(win, 0, (winWidth - strlen("A battle has commenced!")) / 2, "A battle has commenced!");
    mvwprintw(win, 2, (winWidth - 20) / 2, "You won the battle!");
    
    refresh();

    while((c = wgetch(win)) != 27){       
        wrefresh(win);
    }

    wclear(win);
    wrefresh(win);
    printMap();
}

void displayTrainerMenu(){
    ITEM **my_items;
	int c;				
	MENU *my_menu;
    WINDOW *my_menu_win;
    int n_choices, i;
    fillTrainerArray();
	/* Create items */
    n_choices = ARRAY_SIZE(trainers);

    char buffer[n_choices][50];

    my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
    for(i = 0; i < n_choices; ++i){
        int x = NPC[i+1].x - NPC[0].x;
        int y = NPC[i+1].y - NPC[0].y;
        if(NPC[i+1].y < NPC[0].y){
            if((NPC[i+1].x < NPC[0].x))
                sprintf(buffer[i], "%d north and %d west", abs(y), abs(x));
            else
                sprintf(buffer[i], "%d north and %d east", abs(y), abs(x));
        }
        else {
            if((NPC[i+1].x < NPC[0].x))
                sprintf(buffer[i], "%d south and %d west", abs(y), abs(x));
            else
                sprintf(buffer[i], "%d south and %d east", abs(y), abs(x));
        }        
        my_items[i] = new_item(trainers[i], buffer[i]);
    }

	/* Create menu */
	my_menu = new_menu((ITEM **)my_items);
    int winWidth = 40;
    int winLen = 10;
	/* Create the window to be associated with the menu */
    my_menu_win = newwin(10, 40, (MAP_Y - winLen) / 2, (MAP_X - winWidth) / 2);
    keypad(my_menu_win, TRUE);
     
	/* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 38, 3, 1));
	set_menu_format(my_menu, 5, 1);
			
	/* Set menu mark to the string " * " */
        set_menu_mark(my_menu, " * ");

	/* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0);
	print_in_middle(my_menu_win, 1, 0, 40, "Trainer List", COLOR_PAIR(1));
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
	mvwaddch(my_menu_win, 2, 39, ACS_RTEE);
        
	/* Post the menu */
	post_menu(my_menu);
	wrefresh(my_menu_win);
	
	refresh();

	while((c = wgetch(my_menu_win)) != 27){
     
       switch(c){
        case KEY_DOWN:
            menu_driver(my_menu, REQ_DOWN_ITEM);
            break;
        case KEY_UP:
            menu_driver(my_menu, REQ_UP_ITEM);
            break;
        case KEY_NPAGE:
            menu_driver(my_menu, REQ_SCR_DPAGE);
            break;
        case KEY_PPAGE:
            menu_driver(my_menu, REQ_SCR_UPAGE);
            break;
        }
        wrefresh(my_menu_win);
	}
    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    
    for(i = 0; i < n_choices; ++i)
        free_item(my_items[i]);
}

void moveFromDirection(character_t *t, int character){
    int direction = t->dir;
    int minX = t->x;
    int minY = t->y;

    if(direction == 0){
            t->y = t->y-1;
            t->x = t->x;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX], character);
        } 
    else if (direction == 2){
            t->y = t->y;
            t->x = t->x-1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX-1], character);
        }
    else if (direction == 4){
            t->y = t->y-1;
            t->x = t->x-1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX-1], character);
        }
    else if (direction == 1 ){
            t->y = t->y+1;
            t->x = t->x;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX], character);
        }
    else if (direction == 3){
            t->y = t->y;
            t->x = t->x+1;
             t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX+1], character);
        }
    else if (direction == 5){
            t->y = t->y+1;
            t->x = t->x+1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX+1], character);
        }
    else if (direction == 6){
            t->y = t->y-1;
            t->x = t->x+1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX+1], character);
        }
    else if (direction == 7){
            t->y = t->y+1;
            t->x = t->x-1;
            t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX-1], character);
        }
    else if (t->symbol != '@')
        mvprintw(22,0,"%s","cant move there");

}

void moveFollowers(character_t *t, int character){
    int minX = t->x;
    int minY = t->y;
    int min = INF;
    int a=0, b=0, c=0, d=0, e=0, f=0, g=0, h=0;
    for(int i = 0; i < numTrainers; i++){
        if(NPC[i].sequence_num != t->sequence_num){
            character_t neighbor = NPC[i];
            int neighborCoord = neighbor.x * 79 + neighbor.y;
            if(minX == neighbor.x && minY - 1 == neighbor.y){
                if(neighbor.symbol = '@' && t->dir == 0 && NPC[t->sequence_num].defeated == 0){
                    NPC[t->sequence_num].defeated = 1;
                    displayBattle();
                    
                }
                a = 1;
            }   
            else if(minX-1 == neighbor.x && minY == neighbor.y){
                if(neighbor.symbol = '@' && t->dir == 2 && NPC[t->sequence_num].defeated == 0){
                    NPC[t->sequence_num].defeated = 1;
                    displayBattle();
                }
                b = 1;
            }
            else if(minX-1 == neighbor.x && minY - 1 == neighbor.y){
                if(neighbor.symbol = '@'  && NPC[t->sequence_num].defeated == 0){
                    NPC[t->sequence_num].defeated = 1;
                    displayBattle();
                }
                c = 1;
                }
            else if(minX == neighbor.x && minY + 1 == neighbor.y){
                if(neighbor.symbol = '@' && t->dir == 1 && NPC[t->sequence_num].defeated == 0){
                    NPC[t->sequence_num].defeated = 1;
                    displayBattle();
                }
                d = 1;
            }
            else if(minX+1 == neighbor.x &&  minY == neighbor.y){
                if(neighbor.symbol = '@' && t->dir == 3 && NPC[t->sequence_num].defeated == 0){
                    NPC[t->sequence_num].defeated = 1;
                    displayBattle();
                } 
                e = 1;                        
            }
            else if(minX+1 == neighbor.x && minY + 1 == neighbor.y){
                if(neighbor.symbol = '@' && t->dir == 5 && NPC[t->sequence_num].defeated == 0){
                    NPC[t->sequence_num].defeated = 1;
                    displayBattle();
                } 
                f = 1;                
            }
            else if(minX+1 == neighbor.x && minY - 1 == neighbor.y){
                if(neighbor.symbol = '@' && t->dir == 6 && NPC[t->sequence_num].defeated == 0){
                    NPC[t->sequence_num].defeated = 1;
                    displayBattle();
                } 
                g = 1;                
            }
            else if(minX-1 == neighbor.x && minY + 1 == neighbor.y){
                if(neighbor.symbol = '@' && t->dir == 7 && NPC[t->sequence_num].defeated == 0){
                    NPC[t->sequence_num].defeated = 1;
                    displayBattle();
                } 
                h = 1;                
            }
        }
    }
    // printf("%d %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g, h);
    if(min > paths[character]->screen[minY-1][minX] && a == 0){
        min = paths[character]->screen[minY-1][minX];
        t->dir = 0;
        t->y = minY-1;
        t->x = minX;
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX], character);
    }
    if(min > paths[character]->screen[minY][minX-1] && b == 0){
        t->y = minY;
        t->dir = 2;
        t->x = minX-1;
        min = paths[character]->screen[minY][minX-1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX-1], character);
    }
    if (min > paths[character]->screen[minY-1][minX-1] && c == 0){
        t->y = minY-1;
        t->dir = 4;
        t->x = minX-1;
        min = paths[character]->screen[minY-1][minX-1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX-1], character);
    }
    if (min > paths[character]->screen[minY+1][minX] && d == 0){
        t->y = minY+1;
        t->dir = 1;
        t->x = minX;
        min = paths[character]->screen[minY+1][minX];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX], character);
    }
    if (min > paths[character]->screen[minY][minX+1] && e == 0){
        t->y = minY;
        t->dir = 3;
        t->x = minX+1;
        min = paths[character]->screen[minY][minX+1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX+1], character);
    }
    if (min > paths[character]->screen[minY+1][minX+1] && f == 0){
        t->y = minY+1;
        t->dir = 5;
        t->x = minX+1;
        min = paths[character]->screen[minY+1][minX+1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX+1], character);
    }
    if (min > paths[character]->screen[minY-1][minX+1] && g == 0){
        t->y = minY-1;
        t->dir = 6;
        t->x = minX+1;
        min = paths[character]->screen[minY-1][minX+1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX+1], character);
    }
    if (min > paths[character]->screen[minY+1][minX-1] && h == 0){
        t->y = minY+1;
        t->x = minX-1;
        t->dir = 7;
        t->next_turn = t->next_turn;
        min = paths[character]->screen[minY+1][minX-1];
        t->next_turn = t->next_turn + pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX-1], character);
    }
}

int getWandererDirection(character_t *t){
    int direction = rand() % 8;
    int c = 0;
    while(c == 0){
        direction = rand() % 8;
        if(direction == 0){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if(direction == 2){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 4){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 3){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 5){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                   c = 1;
            }
            else if (direction == 6){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
            else if (direction == 7){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == world[currWorldRow][currWorldCol]->screen[t->y][t->x])
                    c = 1;
            }
    }
    return direction;
}

int getExplorerDirection(character_t *t){
    int direction = rand() % 8;
    int c = 0;
    while(c == 0){
        direction = rand() % 8;
        if(direction == 0){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != ter_gate)
                c = 1;
        }
        else if(direction == 2){
            if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != ter_gate)
                c = 1;
        }
        else if (direction == 4){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != ter_gate)
                c = 1;
        }
        else if (direction == 1){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != ter_gate)
                c = 1;
        }
        else if (direction == 3){
            if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != ter_gate)
                c = 1;
        }
        else if (direction == 5){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != ter_gate)
                c = 1;
        }
        else if (direction == 6){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != ter_gate)
                c = 1;
        }
        else if (direction == 7){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != ter_water && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != ter_boulder && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != ter_forest && world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != ter_gate)
                c = 1;
        }
    }
    return direction;
}

void movePC(character_t *t, char ch){
    int minX = t->x;
    int minY = t->y;
    int a=0, b=0, c=0, d=0, e=0, f=0, g=0, h=0;
    
    for(int i = 0; i < numTrainers; i++){
        if(NPC[i].sequence_num != t->sequence_num){
            character_t neighbor = NPC[i];
            int neighborCoord = neighbor.x * 79 + neighbor.y;
            if(minX == neighbor.x && minY - 1 == neighbor.y){
                if(ch == 'k' || ch == '8'){
                    if(neighbor.defeated == 0){
                        NPC[neighbor.sequence_num].defeated = 1;
                        displayBattle();
                    }
                }
                a = 1;
            }
            if(minX-1 == neighbor.x && minY == neighbor.y){
                if(ch == 'h' || ch == '4'){
                    if(neighbor.defeated == 0){
                        NPC[neighbor.sequence_num].defeated = 1;
                        displayBattle();
                    }
                }
                b = 1;
            }
            if(minX-1 == neighbor.x && minY - 1 == neighbor.y){
                if(ch == 'y' || ch == '7'){
                    if(neighbor.defeated == 0){
                        NPC[neighbor.sequence_num].defeated = 1;
                        displayBattle();
                    }
                }
                c = 1;
            }
            if(minX == neighbor.x && minY + 1 == neighbor.y){
                if(ch == 'j' || ch == '2'){
                    if(neighbor.defeated == 0){
                        NPC[neighbor.sequence_num].defeated = 1;
                        displayBattle();
                    }
                }
                d = 1;
            }
            if(minX+1 == neighbor.x &&  minY == neighbor.y){
                if(ch == 'l' || ch == '6'){
                    if(neighbor.defeated == 0){
                        NPC[neighbor.sequence_num].defeated = 1;
                        displayBattle();
                    }
                }
                e = 1;                        
            }
            if(minX+1 == neighbor.x && minY + 1 == neighbor.y){
                if(ch == 'n' || ch == '3'){
                    if(neighbor.defeated == 0){
                        NPC[neighbor.sequence_num].defeated = 1;
                        displayBattle();
                    }
                }
                f = 1;                
            }
            if(minX+1 == neighbor.x && minY - 1 == neighbor.y){
                if(ch == 'u' || ch == '9'){
                    if(neighbor.defeated == 0){
                        NPC[neighbor.sequence_num].defeated = 1;
                        displayBattle();
                    }
                }
                g = 1;                
            }
            if(minX-1 == neighbor.x && minY + 1 == neighbor.y){
                if(ch == 'b' || ch == '1'){
                    if(neighbor.defeated == 0){
                        NPC[neighbor.sequence_num].defeated = 1;
                        displayBattle();
                    }
                }
                h = 1;                
            }
        }
    }
    if(ch == 'k' || ch == '8'){
        t->dir = 0;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX], 3) != INF && a == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'j' || ch == '2'){
        t->dir = 1;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX], 3) != INF && d == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'l' || ch == '6'){
        t->dir = 3;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX+1], 3) != INF && e == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'h' || ch == '4'){
        t->dir = 2;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY][minX-1], 3) != INF && b == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'y' || ch == '7'){
        t->dir = 4;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX-1], 3) != INF && c == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'n' || ch == '3'){
        t->dir = 5;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX+1], 3) != INF && f == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'u' || ch == '9'){
        t->dir = 6;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY-1][minX+1], 3) != INF && g == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 'b' || ch == '1'){
        t->dir = 7;
        if(pathFindCost(world[currWorldRow][currWorldCol]->screen[minY+1][minX-1], 3) != INF && h == 0)
            moveFromDirection(t,3);
    }
    else if(ch == 't'){
        displayTrainerMenu();
        t->next_turn = t->next_turn;
    }
    else if(ch == '5' | ch == 0x20 || ch == '.'){
        t->next_turn = t->next_turn + 10;
    }    
    else if(ch == '>' && (world[currWorldRow][currWorldCol]->screen[minY][minX] == ter_mart ||world[currWorldRow][currWorldCol]->screen[minY][minX] == ter_center)){
        if(world[currWorldRow][currWorldCol]->screen[minY][minX] == ter_mart)
            displayMandCInterface(0);
        else
            displayMandCInterface(1);
    }
    else{
        displayWarning("Incorrect Key! Check manual for inputs.");
        t->next_turn = t->next_turn;
    }

    if(minX != t->x || minY != t->y){
        player = (((t->x % 78) + 1) * 79) + ((t->y % 19) + 1);
        free(paths[0]);
        free(paths[1]);
        dijkstras_path(world[currWorldRow][currWorldCol]->screen, 0);
        dijkstras_path(world[currWorldRow][currWorldCol]->screen, 1);
    }
}

void moveNPCs(character_t *t){
    //if hiker
    if(t->symbol == 'h' && t->defeated == 0){
        moveFollowers(t, 0);
    }
    //if rival
    else if(t->symbol == 'r' && t->defeated == 0){
        moveFollowers(t, 1);
    }
    else{
        //if hit other npc, reverse 
        int minX = t->x;
        int minY = t->y;
        int a=0, b=0, c=0, d=0, e=0, f=0, g=0, h=0;

        for(int i = 0; i < numTrainers; i++){
            if(NPC[i].sequence_num != t->sequence_num){
                character_t neighbor = NPC[i];
                int neighborCoord = neighbor.x * 79 + neighbor.y;
                if(minX == neighbor.x && minY - 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 0 && NPC[t->sequence_num].defeated == 0){
                        NPC[t->sequence_num].defeated = 1;
                        displayBattle();
                    }
                    a = 1;
                }   
                else if(minX-1 == neighbor.x && minY == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 2 && NPC[t->sequence_num].defeated == 0){
                        NPC[t->sequence_num].defeated = 1;
                        displayBattle();
                    }
                    b = 1;
                }
                else if(minX-1 == neighbor.x && minY - 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 4 && NPC[t->sequence_num].defeated == 0){
                        NPC[t->sequence_num].defeated = 1;
                        displayBattle();
                    }
                    c = 1;
                    }
                else if(minX == neighbor.x && minY + 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 1 && NPC[t->sequence_num].defeated == 0){
                        NPC[t->sequence_num].defeated = 1;
                        displayBattle();
                    }
                    d = 1;
                }
                else if(minX+1 == neighbor.x &&  minY == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 3 && NPC[t->sequence_num].defeated == 0){
                        NPC[t->sequence_num].defeated = 1;
                        displayBattle();
                    } 
                    e = 1;                        
                }
                else if(minX+1 == neighbor.x && minY + 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 5 && NPC[t->sequence_num].defeated == 0){
                        NPC[t->sequence_num].defeated = 1;
                        displayBattle();
                    } 
                    f = 1;                
                }
                else if(minX+1 == neighbor.x && minY - 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 6 && NPC[t->sequence_num].defeated == 0){
                        NPC[t->sequence_num].defeated = 1;
                        displayBattle();
                    } 
                    g = 1;                
                }
                else if(minX-1 == neighbor.x && minY + 1 == neighbor.y){
                    if(neighbor.symbol == '@' && t->dir == 7 && NPC[t->sequence_num].defeated == 0){
                        NPC[t->sequence_num].defeated = 1;
                        displayBattle();
                    } 
                    h = 1;                
                }
            }
        }

        if(t->symbol == 'w'){ //wanderer
            int direction = t->dir; //if hit IMPASSIBLE terrain random 
            if(direction == 0 || a == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] != world[currWorldRow][currWorldCol]->screen[t->y][t->x] || a == 1)
                    t->dir = getWandererDirection(t);
            }
            else if(direction == 2 || b == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x] || b == 1)
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 4 || c == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x] || c == 1)
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 1 || d == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] != world[currWorldRow][currWorldCol]->screen[t->y][t->x] || d == 1)
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 3 || e == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x] || e == 1)
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 5 || f == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x] || f == 1)
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 6 || g == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x] || g == 1)
                    t->dir = getWandererDirection(t);
            }
            else if (direction == 7 || h == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] != world[currWorldRow][currWorldCol]->screen[t->y][t->x] || h == 1)
                    t->dir = getWandererDirection(t);
            }
            //now move from rand direction
            moveFromDirection(t, 2);
        }    
        else if(t->symbol == 'p'){ //pacers
            int  direction = t->dir;
            //if hit terrain reverse 
            if(direction == 0 || a == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_gate || a == 1)
                    t->dir = 1;
            }
            else if(direction == 2 || b == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_gate || b == 1)
                    t->dir = 3;
            }
            else if (direction == 4 || c == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_gate || c == 1)
                    t->dir = 5;
            }
            else if (direction == 1 || d == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_gate || d == 1)
                    t->dir = 0;
            }
            else if (direction == 3 || e == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_gate|| e == 1)
                    t->dir = 2;
            }
            else if (direction == 5 || f == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_gate || f == 1)
                    t->dir = 4;
            }
            else if (direction == 6 || g == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_boulder|| world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_gate || g == 1)
                    t->dir = 7;
            }
            else if (direction == 7 || h == 1){
                if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_mountain || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_gate || h == 1)
                    t->dir = 6;
            }
            moveFromDirection(t, 2);
    }
    else { //explorer or defeated hiker or rival
        int direction = t->dir;//if hit IMPASSIBLE terrain random 
        if(direction == 0 || a == 1){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x] == ter_gate || a == 1)
                t->dir = getExplorerDirection(t);
        }
        else if(direction == 2 || b == 1){
            if(world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y][t->x-1] == ter_gate || b == 1)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 4 || c == 1){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x-1] == ter_gate || c == 1)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 1 || d == 1){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x] == ter_gate || d == 1)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 3 || e == 1){
        if(world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y][t->x+1] == ter_gate || e == 1)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 5 || f == 1){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x+1] == ter_gate || f == 1)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 6 || g == 1){
            if(world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y-1][t->x+1] == ter_gate || g == 1)
                t->dir = getExplorerDirection(t);
        }
        else if (direction == 7 || h == 1){
            if(world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_water || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_boulder || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_forest || world[currWorldRow][currWorldCol]->screen[t->y+1][t->x-1] == ter_gate || h == 1)
                t->dir = getExplorerDirection(t);
        }
        //now move from rand direction
        moveFromDirection(t, 2);
    }
    }
}

//initialize npcs on single map
void traffic(){
    static character_t *c;
    heap_t h;
    char ch;
    heap_init(&h, turn_cmp, NULL);

    for(int i = 0; i < numTrainers; i++){
        NPC[i].hn = heap_insert(&h, &NPC[i]);
    }

    while ((c = heap_remove_min(&h))) {
        if(c->symbol == '@'){
            if((ch = getch()) != 'q')
                movePC(c, ch);
            else
                break;
        }
        else if(ch != 'q')
            moveNPCs(c); 
        if(c->sequence_num == 0){
            printMap();
            refresh();
        } 
        heap_insert(&h, &NPC[c->sequence_num]);
      }
    heap_delete(&h);
}

void spawnNPCs(int newNPC, int numT){
    if(newNPC == 0){ //Sentry
        character_t* sentry = malloc(sizeof(character_t));

        sentry->symbol = 's';
        int sentry_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[sentry_coord % 79][sentry_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[sentry_coord % 79][sentry_coord / 79] == ter_water || world[currWorldRow][currWorldCol]->screen[sentry_coord % 79][sentry_coord / 79] == ter_mountain){
            sentry_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        sentry->y = sentry_coord % 79;
        sentry->x = sentry_coord / 79;
        sentry->sequence_num = numTrainers - numT;
        sentry->defeated = 0;

        sentry->next_turn = INF;
    
        NPC[numTrainers - numT] = *sentry;
    }
    if(newNPC == 1){ //Wanderer
        character_t* wanderer = malloc(sizeof(character_t));

        wanderer->symbol = 'w';
        int wanderer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[wanderer_coord % 79][wanderer_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[wanderer_coord % 79][wanderer_coord / 79] == ter_forest || world[currWorldRow][currWorldCol]->screen[wanderer_coord % 79][wanderer_coord / 79] == ter_water || world[currWorldRow][currWorldCol]->screen[wanderer_coord % 79][wanderer_coord / 79] == ter_mountain){
            wanderer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        wanderer->y = wanderer_coord % 79;
        wanderer->x = wanderer_coord / 79;
        wanderer->sequence_num = numTrainers - numT;
        wanderer->dir = rand() % 8;
        wanderer->defeated = 0;

        wanderer->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[wanderer->y][wanderer->x], 1);
    
        NPC[numTrainers - numT] = *wanderer;
    }
    if(newNPC == 2){ //Pacer
        character_t* pacer = malloc(sizeof(character_t));

        pacer->symbol = 'p';
        int pacer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[pacer_coord % 79][pacer_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[pacer_coord % 79][pacer_coord / 79] == ter_forest || world[currWorldRow][currWorldCol]->screen[pacer_coord % 79][pacer_coord / 79] == ter_water || world[currWorldRow][currWorldCol]->screen[pacer_coord % 79][pacer_coord / 79] == ter_mountain){
            pacer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        pacer->y = pacer_coord % 79;
        pacer->x = pacer_coord / 79;
        pacer->sequence_num = numTrainers - numT;
        pacer->dir = 6;
        pacer->defeated = 0;


        pacer->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[pacer->y][pacer->x], 1);
    
        NPC[numTrainers - numT] = *pacer;
    }
    if(newNPC == 3){ //Explorers
        character_t* explorer = malloc(sizeof(character_t));

        explorer->symbol = 'e';
        int explorer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[explorer_coord % 79][explorer_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[explorer_coord % 79][explorer_coord / 79] == ter_water|| world[currWorldRow][currWorldCol]->screen[explorer_coord % 79][explorer_coord / 79] == ter_forest || world[currWorldRow][currWorldCol]->screen[explorer_coord % 79][explorer_coord / 79] == ter_mountain){
            explorer_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        explorer->y = explorer_coord % 79;
        explorer->x = explorer_coord / 79;
        explorer->sequence_num = numTrainers - numT;
        explorer->dir = rand() % 8;
        explorer->defeated = 0;
        
        

        explorer->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[explorer->y][explorer->x], 1);
    
        NPC[numTrainers - numT] = *explorer;
    }
}

void initNPCs(int numT){

    if(numT > 2){
        character_t* rival = malloc(sizeof(character_t));
        rival->symbol = 'r';
        int rival_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        while(world[currWorldRow][currWorldCol]->screen[rival_coord % 79][rival_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[rival_coord % 79][rival_coord / 79] == ter_forest || world[currWorldRow][currWorldCol]->screen[rival_coord % 79][rival_coord / 79] == ter_water || world[currWorldRow][currWorldCol]->screen[rival_coord % 79][rival_coord / 79] == ter_mountain){
            rival_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        rival->y = rival_coord % 79;
        rival->x = rival_coord / 79;
        rival->sequence_num = 2;
        rival->defeated = 0;

        rival->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[rival->y][rival->x], 1);
        
        character_t* hiker = malloc(sizeof(character_t));

        hiker->symbol = 'h';
        int hiker_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);

        while(world[currWorldRow][currWorldCol]->screen[hiker_coord % 79][hiker_coord / 79] == ter_boulder || world[currWorldRow][currWorldCol]->screen[hiker_coord % 79][hiker_coord / 79] == ter_water){
            hiker_coord = (((rand() % 78) + 1) * 79) + ((rand() % 19) + 1);
        }
        hiker->y = hiker_coord % 79;
        hiker->x = hiker_coord / 79;
        hiker->sequence_num = 1;
        hiker->defeated = 0;

        hiker->next_turn = pathFindCost(world[currWorldRow][currWorldCol]->screen[hiker->y][hiker->x], 0);


        NPC[1] = *hiker;
        NPC[2] = *rival;

        numT -= 3; //numT = 6
    }
    while(numT){
        int newNPC = rand() % 4;
        spawnNPCs(newNPC, numT);
        numT--;
    }

}

void initMap(){
    currWorldRow = 200;
    currWorldCol = 200;
    setExits();
    createMap(currExitN,currExitS,currExitE,currExitW);
    initNPCs(numTrainers);
    printMap();
    traffic();
}


int main(int argc, char *argv[]){
    
    srand(time(NULL));

    initscr();
    // raw();
    noecho();
    keypad(stdscr, TRUE);

    initMap();

    endwin();

    return 0;
}
