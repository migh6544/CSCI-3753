#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define ARRAY_SIZE 8
#define MAX_NAME_LENGTH 32

typedef struct
{
    char *address[ARRAY_SIZE];
    sem_t space;
    sem_t item;
    sem_t mutex;
    int top;
} array;

int array_init(array *s);
int array_put(array *s, char *hostname);
int array_get(array *s, char **hostname);
void array_free(array *s);

#endif