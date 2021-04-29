/*
 * par_sumsq.c
 *
 * CS 446.646 Project 5 (Pthreads)
 *
 * Compile with --std=c99
 */

 /* Things that Need to be Done

 -- things are about to get A L O T worse with multiple threads lol


 This is all the objectives for now; will require A L O T more

 Completed:
 -- Change Main to accept the Correct number of Args
 -- Create a Linked List and Node Class for task tracking (C does not allow classes)
 -- Create a primitive to enqueue tasks
 -- Create a primitive to dequeue tasks

 -- implement int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg)
    from manual thread is passed as an empty object to be filled by the function,
    from manual attr is a bunch of low level scope stuff that should just be passed as NULL;
    the routine should is a function pointer to the task that will be performed
    the args part is the args you want to pass to the thread

 -- pthread_mutex_lock(pthread_mutex_t *mutex) need one of these for task queue and aggregate



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


// thread nonsese variables

pthread_mutex_t aggregateProtector, queueProtector;
pthread_cond_t conditionInit;

// aggregate variables
long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
volatile bool done = false;

// function prototypes
void calculate_square(long number);
void enqueueTask(volatile WorkerQueue *queue, long number);
long dequeueTask(volatile WorkerQueue *queue);
void *workerFunction(void *queue);

void printQueue(volatile WorkerQueue *queue);


/*
 * update global aggregate variables given a number
 */
void calculate_square(long number)
{
  long the_square = number * number;

  sleep(number);

  pthread_mutex_lock(&aggregateProtector);

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

  pthread_mutex_unlock(&aggregateProtector);

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

  pthread_cond_init(&conditionInit, NULL);
  pthread_mutex_init(&queueProtector, NULL);
  pthread_mutex_init(&aggregateProtector, NULL);

  volatile WorkerQueue *queue = (struct WorkerQueue *) malloc(sizeof(struct WorkerQueue)); // gotta love c's lack of new :))))
  queue->headNode = NULL;

  int numberOfWorkers = atoi(argv[2]);

  pthread_t *workers = (pthread_t *)malloc(sizeof(pthread_t)*numberOfWorkers);

  for (int i = 0; i < numberOfWorkers; i++{
      pthread_create(&workers[i], NULL, (void * (*)(void *))workerFunction, (void *)queue);

  while (fscanf(fin, "%c %ld\n", &action, &num) == 2) {
    if (action == 'p') {            // process, do some work
      pthread_mutex_lock(&queueProtector);
      enqueueTask(queue, num);
      pthread_cond_signal(&conditionInit);
      pthread_mutex_unlock(&queueProtector);
    } else if (action == 'w') {     // wait, nothing new happening
      sleep(num);
    } else {
      printf("ERROR: Unrecognized action: '%c'\n", action);
      exit(EXIT_FAILURE);
    }
  }

  fclose(fin);

  while (queue->headNode){}

  done = true;
  pthread_mutex_lock(&queueProtector);
  pthread_cond_broadcast(&conditionInit);
  pthread_mutex_unlock(&queueProtector);

  for (int i = 0; i < numberOfWorkers; i++)
    pthread_join(workers[i], NULL);

  queue->headNode = NULL;

  printf("%ld %ld %ld %ld\n", sum, odd, min, max);

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

long dequeueTask(volatile WorkerQueue *queue){
  WorkerNode *curNode = queue->headNode;

  if (curNode)
    queue->headNode = curNode->nextNode;
  else
    return 0;

  long returnVal = curNode->value;

  return returnVal;
}

void *workerFunction(void *queueArg){

  WorkerQueue *queue = (WorkerQueue *)queueArg;
  while(!done){
    pthread_mutex_lock(&queueProtector);

    while(!done && !(queue->headNode)){
      pthread_cond_wait(&conditionInit, &queueProtector);
    }

    if (done){
       pthread_mutex_unlock(&queueProtector);
       break;
    }

    long value = dequeueTask(queue);
    pthread_mutex_unlock(&queueProtector);
    calculate_square(value);
  }

  return EXIT_SUCCESS;
}

void printQueue(volatile WorkerQueue *queue){
  WorkerNode *curNode = queue->headNode;
  int i = 0;

  while(curNode){
    printf("%s:%d with value:%ld \n", "Node at index: ", i, curNode->value);
    i++;
    curNode = curNode->nextNode;
  }
}
