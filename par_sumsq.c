/*
 * par_sumsq.c
 *
 * CS 446.646 Project 5 (Pthreads)
 *
 * Compile with --std=c99
 */

 /* Things that Need to be Done

 -- all taks perform the almost Correct amount of shit

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
 -- things are about to get A L O T worse with multiple threads lol

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
void enqueueTask(long number);
void dequeueTask();
long valueFromNode(WorkerNode *node);
void *workerFunction();

void printQueue(queue);

//global variables
volatile WorkerQueue *queue;

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

  queue = (struct WorkerQueue *)malloc(sizeof(struct WorkerQueue)); // gotta love c's lack of new :))))
  queue->headNode = NULL;

  int numberOfWorkers = atoi(argv[2]); // user specified amount of threads

  pthread_t *workers = (pthread_t *)malloc(sizeof(pthread_t)*numberOfWorkers); // allocating ample amount of memeory for all workers

  for (int i = 0; i < numberOfWorkers; i++) // creating amount of threads specified by user
      pthread_create(&workers[i], NULL, (void * (*)(void *))workerFunction, (void *)queue);

  while (fscanf(fin, "%c %ld\n", &action, &num) == 2) {
    if (action == 'p') {            // process, do some work
      pthread_mutex_lock(&queueProtector); //making sure threads cannot mutate the queue while adding new operations
      enqueueTask(num); // enqueuing taks with number: num
      pthread_mutex_unlock(&queueProtector); // allowing queue to be mutated by working threads

      pthread_cond_signal(&conditionInit); //allowing a worker to do work

      while (queue->headNode){ // allow all workers to work until the queue is empty
        pthread_cond_signal(&conditionInit);
      }

    } else if (action == 'w') {
      sleep(num);
    } else {
      printf("ERROR: Unrecognized action: '%c'\n", action);
      exit(EXIT_FAILURE);
    }
  }

  fclose(fin);

  //ensuring queue is actually empty before marking actions as complete
  while (queue->headNode);

  done = true;

  // allow all available threads to work
  pthread_cond_broadcast(&conditionInit);

  //killing threads
  for (int i = 0; i < numberOfWorkers; i++)
    pthread_join(workers[i], NULL);

  queue->headNode = NULL;

  printf("%ld %ld %ld %ld\n", sum, odd, min, max);

  return (EXIT_SUCCESS);
}

// add a task to the queue with the number specified by arg
void enqueueTask(long number){

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

// remove headNode of the worker queue
void dequeueTask(){
  WorkerNode *curNode = queue->headNode;

  if (curNode)
    queue->headNode = curNode->nextNode;
}

//get value from node so it can be copied to stack and not removed by dump collection
long valueFromNode(WorkerNode *node){

  long returnVal = 0;

  if (node)
    returnVal = node->value;

  return returnVal;
}

// pthread worker routine that will be used by all threads
void *workerFunction(){

  while(!done){
    pthread_mutex_lock(&queueProtector);

    while(pthread_cond_wait(&conditionInit, &queueProtector) != 0);

    if (done){
       pthread_mutex_unlock(&queueProtector);
       return;
    }

    long value = valueFromNode(queue->headNode);
    dequeueTask(queue);
    pthread_mutex_unlock(&queueProtector);
    calculate_square(value);
  }

  return;
}

// used for debugging to ensure proper functionality
void printQueue(){
  WorkerNode *curNode = queue->headNode;
  int i = 0;

  while(curNode){
    printf("%s:%d with value:%ld \n", "Node at index: ", i, curNode->value);
    i++;
    curNode = curNode->nextNode;
  }
}
