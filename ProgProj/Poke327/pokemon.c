#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char screen[21][80];

void seeder(char screen[21][80]){
    int longGrass;
    int mountains;
    int clearings;

    //create borders
    for(int l = 0; l < 80; l++){
        screen[0][l] = '%';
        screen[20][l] = '%';
    }
    for(int k = 0; k < 21; k++){
        screen[k][0] = '%';
        screen[k][79] = '%';
    }
    //seed mountains, long grass and clearings
    int i = 0;
    while(i < 3){
        int y = rand() % 79;
        int x = rand() % 20;

        if(i == 0) // longGrass
            screen[x][y] = ':';
        if(i == 1) //mountains
            screen[x][y] = '%';
        if(i == 2) //clearings
            screen[x][y] = '.';
        i++;
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