/*
 * par_sumsq.c
 *
 * CS 446.646 Project 5 (Pthreads)
 *
 * Compile with --std=c99
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



/* Things that Need to be Done

-- Change Main to accept the Correct number of Args
-- Create a Linked List and Node Class for task tracking (C does not allow classes)
-- Create a primitive to enqueue tasks
-- Create a primitive to dequeue tasks

This is all the objectives for now; will require A L O T more


*/


// aggregate variables
long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;

// function prototypes
void calculate_square(long number);

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
  if (argc != 2) {
    printf("Usage: sumsq <infile>\n");
    exit(EXIT_FAILURE);
  }
  char *fn = argv[1];

  // load numbers and add them to the queue
  FILE* fin = fopen(fn, "r");
  char action;
  long num;

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
