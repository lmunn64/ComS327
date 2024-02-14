#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// compare is a pointer to a function that takes 2 void * params and returns int
// int (*compare) (const void *v1, const void *v2)
int compare_int_reverse(const int *v1, const int *v2){
    return *((int *) v2) - *((int *) v1);
}

void insertion_sort(void *v, int s, int n, int (*compare)(const void *, const void *)){
    int i, j;
    void *t = malloc(s);
    char *a = v;

    for(i = 1; i < n; i++){
       for(memcpy(t, a + (s*i), s), j = i - 1; j > -1 && compare(a + (s*j), t) > 0; j--){
            memcpy(a + (s * (j + 1)), a + (s * j), s);
        } 
        memcpy(a + (s * (j + 1)), t, s);
    }
    free(t);
}


void insertion_sort_int(int *a, int n){
    int i, j, t;

    for(i = 1; i < n; i++){
       for(t = a[i], j = i - 1; j > -1 && a[j] > t; j--){
            a[j+1] = a[j];
        } 
        a[j+1] = t;
    }
}
int main(int argx, char *argv[]){
    int a[] = {9,8,7,6,5,4,3,2,1,0};
    int i;

    for(i = 0; i < sizeof (a) / sizeof (a[0]); i++){
        printf("%d\t",a[i]);
    }
    printf("\n");

    insertion_sort_int(a, 10);

    for(i = 0; i < sizeof (a) / sizeof (a[0]); i++){
        printf("%d\t",a[i]);
    }
    printf("\n");

    insertion_sort(a, sizeof(a[0]), sizeof (a) / sizeof (a[0]), compare_int_reverse);


    for(i = 0; i < sizeof (a) / sizeof (a[0]); i++){
        printf("%d\t",a[i]);
    }
    printf("\n");

    return 0;
}