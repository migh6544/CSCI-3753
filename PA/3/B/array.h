// Defining header file (Library)
#ifndef ARRAY_H
#define ARRAY_H

// Including header files
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

// Defining macros
#define ARRAY_SIZE 8                            // Stack of strings size
#define MAX_NAME_LENGTH 64                      // Each string size

// Defining the stack struct
typedef struct
{
    char *address[ARRAY_SIZE];                  // Size and location of strings storage
    sem_t space;                                // Producer semaphore
    sem_t item;                                 // Consumer semaphore
    sem_t mutex;                                // Critical section mutex
    int top;                                    // Stack counter
} array;

// Defining member functions
int array_init(array *s);                       // Initializes the stack struct and allocate memory
int array_put(array *s, char *hostname);        // Place a string on the stack struct
int array_get(array *s, char **hostname);       // Collect a string from the stack struct
void array_free(array *s);                      // Deallocate memory and destroy semaphores of the stack struct

#endif