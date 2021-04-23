/*
 * par_sumsq.c
 *
 * CS 446.646 Project 5 (Pthreads)
 *
 * Compile with --std=c99
 */

 /* Things that Need to be Done

 -- implement int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg)
    from manual thread is passed as an empty object to be filled by the function,
    from manual attr is a bunch of low level scope stuff that should just be passed as NULL;
    the routine should is a function pointer to the task that will be performed
    the args part is the args you want to pass to the thread


 This is all the objectives for now; will require A L O T more

 Completed:
 -- Change Main to accept the Correct number of Args
 -- Create a Linked List and Node Class for task tracking (C does not allow classes)
 -- Create a primitive to dequeue tasks
 -- Create a primitive to dequeue tasks




 */


#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct WorkerNode{
  struct WorkerNode *nextNode;
  long value;
} WorkerNode;

typedef struct WorkerQueue {
  WorkerNode *headNode;
} WorkerQueue;


// aggregate variables
long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;

// function prototypes
void calculate_square(long number);
void enqueueTask(volatile WorkerQueue *queue, long number);
void dequeueTask(volatile WorkerQueue *queue);
void workerFunction(volatile WorkerQueue *queue);


/*
 * update global aggregate variables given a number
 */
void calculate_square(long number)
{

  long the_square = number * number;

  sleep(number);

  sum += the_square;

  if (number % 2 == 1) {
    odd++;
  }

  if (number < min) {
    min = number;
  }

  if (number > max) {
    max = number;
  }
}


int main(int argc, char* argv[])
{
  // check and parse command line options
  if (argc != 3) {
    printf("Usage: par_sumsq <infile> <number of workers>\n" );
    exit(EXIT_FAILURE);
  }

  char *fn = argv[1];

  FILE* fin = fopen(fn, "r");
  char action;
  long num;

  volatile WorkerQueue *queue = (struct WorkerQueue *) malloc(sizeof(struct WorkerQueue)); // gotta love c's lack of new :))))
  queue->headNode = NULL;

  pthread_t worker;

  pthread_create(&worker, NULL, workerFunction, (void *)queue);

  while (fscanf(fin, "%c %ld\n", &action, &num) == 2) {
    if (action == 'p') {            // process, do some work
      calculate_square(num);
    } else if (action == 'w') {     // wait, nothing new happening
      sleep(num);
    } else {
      printf("ERROR: Unrecognized action: '%c'\n", action);
      exit(EXIT_FAILURE);
    }
  }
  fclose(fin);

  // print results
  printf("%ld %ld %ld %ld\n", sum, odd, min, max);

  // clean up and return
  return (EXIT_SUCCESS);
}

void enqueueTask(volatile WorkerQueue *queue, long number){

  WorkerNode *newNode = (WorkerNode *) malloc(sizeof(struct WorkerNode));
  newNode->value = number;
  newNode->nextNode = NULL;

  if (!queue->headNode){
    queue->headNode = newNode;
    return;
  }

  WorkerNode *curNode = queue->headNode;

  while(curNode->nextNode)
    curNode = curNode->nextNode;

  curNode->nextNode = newNode;
}

void dequeueTask(volatile WorkerQueue *queue){

  WorkerNode *curNode = queue->headNode;

  if (curNode)
    queue->headNode = curNode->nextNode;

}

void workerFunction(volatile WorkerQueue *queue){
  printf("%s\n", "we have successfully created a function pthread");
}
