#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
//QUEUE PROVIDED BY LearnC (https://www.learnc.net/c-data-structures/c-queue/)

/*
    initialize queue pointers
*/
void initQueue(int *head, int *tail)
{
    *head = *tail = 0;
}

/*
   enqueue an element
   precondition: the queue is not full
*/
void enqueue(int *q,int *tail, int element)
{
    q[(*tail)++] = element;
}

/*
    dequeue an element
    precondition: queue is not empty
*/
int dequeue(int *q,int *head)
{
    return q[(*head)++];
}

/*
    return 1 if queue is full, otherwise return 0
*/
int full(int tail,const int size)
{
    return tail == size;
}

/*
  return 1 if the queue is empty, otherwise return 0
*/
int empty(int head, int tail)
{
    return tail == head;
}
/*
  display queue content
*/
void display(int *q,int head,int tail)
{
    int i = tail - 1;
    while(i >= head)
        printf("%d ",q[i--]);
    printf("\n");
}