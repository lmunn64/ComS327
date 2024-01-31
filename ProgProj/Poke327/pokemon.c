#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char screen[21][80];
char MOUNTAIN = '%';
char LONGGRASS = ':';
char CLEARING = '.';
char terrain[] = {'%', ':', '.'};

void expandSeed(int x, int y, int num){
    if(num == 2){
        num = 0;
    }

    if(screen[x+1][y] == '-'){
        screen[x+1][y] = terrain[num];
        expandSeed(x+1, y, num++);
    }
    if(screen[x-1][y] == '-'){
        screen[x-1][y] = terrain[num];
        expandSeed(x-1, y, num++);
    }
    if(screen[x][y+1] == '-'){
        screen[x][y+1] = terrain[num];
        expandSeed(x, y+1, num++);
    }
    if(screen[x][y-1] == '-'){
        screen[x][y-1] = terrain[num];
        expandSeed(x, y-1, num++);
    }

}

void seeder(char screen[21][80]){
    
    //create borders
    for(int l = 0; l < 80; l++){
        screen[0][l] = MOUNTAIN;
        screen[20][l] = MOUNTAIN;
    }
    for(int k = 0; k < 21; k++){
        screen[k][0] = MOUNTAIN;
        screen[k][79] = MOUNTAIN;
    }
    //create empty spaces using "-"
    for(int n = 1; n < 20; n++){
        for(int m = 1; m < 79; m++){
        screen[n][m] = '-';
        }
    }
    //seed mountains, long grass and clearings
    
    int longGrassY = (rand() % 78) + 1;
    int longGrassX = (rand() % 19) + 1;

    int mountainY = (rand() % 78) + 1;
    int mountainX = (rand() % 19) + 1;

    int clearingsY = (rand() % 78) + 1;
    int clearingsX = (rand() % 19) + 1;

    screen[longGrassX][longGrassY] = LONGGRASS;
    screen[mountainX][mountainY] = MOUNTAIN;
    screen[clearingsX][clearingsY] = CLEARING;

    //Initial expansion of seed
    expandSeed(mountainX, mountainY, MOUNTAIN);
    expandSeed(longGrassX, longGrassY, LONGGRASS);
    expandSeed(clearingsX, clearingsY, CLEARING);
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