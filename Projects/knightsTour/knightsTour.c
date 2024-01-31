#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define N 5


bool canMove();
// int num = 0;

void tour(int grid[N][N], int x, int y, int visited){
    int move = 0;
    grid[x][y] = visited;
    if(visited == N*N){
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                printf("%d", grid[i][j]);
                if(i + j != 2*(N-1))
                    printf(",");
            }
        }
        printf("\n");
        // num++;

        //Backtrack
        grid[x][y]=-1;
    }

    int DirX[] = {2, 1, -1, -2, -2, -1, 1, 2};
    int DirY[] = {1, 2, 2, 1, -1, -2, -2, -1};

    for(int i=0; i<8; i++) {
        int newX = x + DirX[i];
        int newY = y + DirY[i];

        if(canMove(newX, newY, grid)){
            tour(grid, newX, newY, visited + 1);
        }
    }

    grid[x][y] = -1;
} 

bool canMove(int i, int j, int grid[N][N]){
    if (i >= 0 && i < N && j >= 0 && j < N && grid[i][j] == -1){
        return true;
    }  
    return false;
}
void startTour(int x, int y){
    int grid[N][N];

    int i, j;
    for(i = 0; i < N; i++) {
        for(j = 0; j < N; j++) {
        grid[i][j] = -1;
        }
    }

    tour(grid, x, y, 1);
}
int main(int argc, char *argv[]){
    for(int x = 0; x < N; x++){
        for(int y = 0; y < N; y++){
            startTour(x, y);
        }
    }
    // printf("%d", num);
    return 0;
}