#include <stdio.h>
#include <stdlib.h>

#include "stack.h"



int main(int argc, char *argv[]){
    stack_t s;
    int i;
    stack_init(&s);
    
    for(i = 0; i < 10; i++){
        stack_push(&s, i);
    }

    stack_top(&s, &i);

    printf("%d\n", stack_size(&s), i);

    while(!stack_is_empty){
        if(stack_pop(&s, &i)){
            //Runtime opens 3 files before starting main: stdin, stdout, stderr
            fprintf(stderr, "Attempt to pop an empty stack\n");
        }
        printf("%d\n", i);
    }

    printf("%d\n", stack_pop(&s, &i));

    stack_destroy(&s);

    return 0;
}