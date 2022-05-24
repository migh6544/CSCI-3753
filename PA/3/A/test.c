#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>



int singleTest()
{
    array my_array;

    if (array_init(&my_array) < 0)
    {
        return -1;
    }

    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        array_put(&my_array, "testing.com");
    }

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        char *test = NULL;
        array_get(&my_array, &test);

        if(strcmp(test, "testing.com") != 0)
        {
            return -1;
        }
    }

    array_free(&my_array);

    return 0;
}


typedef struct
{
    array arr;
    int count;
    sem_t mutex;
} threadData;

void *putThread(void *test)
{
    threadData *td = (threadData *)test;

    for (int i = 0; i < 1000; i++)
    {
        array_put(&td->arr, "testing.com");
    }

    return NULL;
}

void *getThread(void *test)
{
    threadData *td = (threadData *)test;

    while(1)
    {
        char *test = NULL;
        array_get(&td->arr, &test);

        sem_wait(&td->mutex);
            td->count++;
        sem_post(&td->mutex);

        if (strcmp(test, "testing.com") != 0)
        {
            printf("Error!\n");
        }
    }

    array_free(&td->arr);

    return NULL;
}

int multiTest()
{
    threadData test;

    array_init(&test.arr);
    sem_init(&test.mutex, 0, 1);
    test.count = 0;
    pthread_t putTIDs[50];
    pthread_t getTIDs[50];

    for(int i = 0; i < 50; i++)
    {
        pthread_create(&putTIDs[i], NULL, putThread, &test);
    }

    for (int i = 0; i < 50; i++)
    {
        pthread_create(&getTIDs[i], NULL, getThread, &test);
    }

    for (int i = 0; i < 50; i++)
    {
        pthread_join(putTIDs[i], NULL);
    }

    sleep(1);

    for (int i = 0; i < 50; i++)
    {
        pthread_kill(getTIDs[i], 0);
    }

    if (test.count != (50 * 1000))
    {
        return -1;
    }

    array_free(&test.arr);
    sem_destroy(&test.mutex);

    return 0;
}



int main()
{
    if(singleTest() == 0)
    {
        printf("Completed single thread test.\n");
    }
    else
    {
        printf("Failed single thread test!\n");
    }

    if (multiTest() == 0)
    {
        printf("Completed multi thread test.\n");
    }
    else
    {
        printf("Failed multi thread test!\n");
    }

    return 0;
}