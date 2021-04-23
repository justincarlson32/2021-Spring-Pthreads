/*
 * par_sumsq.c
 *
 * CS 446.646 Project 5 (Pthreads)
 *
 * Compile with --std=c99
 */

 /* Things that Need to be Done

 -- Create a primitive to dequeue tasks
 -- do Pthread nonsense (probs create and a bunch of low level pthread types)

 This is all the objectives for now; will require A L O T more

 Completed:
 -- Change Main to accept the Correct number of Args
 -- Create a Linked List and Node Class for task tracking (C does not allow classes)
 -- Create a primitive to dequeue tasks




 */


#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

  enqueueTask(queue, 15);
  printf("enqueued: %ld \n", queue->headNode->value);

  enqueueTask(queue, 12);
  printf("enqueued: %ld \n", queue->headNode->nextNode->value);

  dequeueTask(queue);

  printf("dequueed and head is now: %ld \n", queue->headNode->value);

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
