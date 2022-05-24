#include "array.h"
#include <string.h>


int array_init(array *s)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        s->address[i] = malloc(sizeof(char) * MAX_NAME_LENGTH);
    }

    sem_init(&s->mutex, 0, 1);
    sem_init(&s->space, 0, ARRAY_SIZE);
    sem_init(&s->item, 0, 0);

    s->top = 0;

    return 0;
}

int array_put(array *s, char *hostname)
{
    sem_wait(&s->space);
        sem_wait(&s->mutex);
            strncpy(s->address[s->top], hostname, MAX_NAME_LENGTH);
            s->top++;
        sem_post(&s->mutex);
    sem_post(&s->item);

    return 0;
}

int array_get(array *s, char **hostname)
{
    sem_wait(&s->item);
        sem_wait(&s->mutex);
            s->top--;
            *hostname = malloc(sizeof(char) * MAX_NAME_LENGTH);
            strncpy(*hostname, s->address[s->top], MAX_NAME_LENGTH);
        sem_post(&s->mutex);
    sem_post(&s->space);

    return 0;
}

void array_free(array *s)
{
    for (int i = ARRAY_SIZE - 1; i >= 0; i--)
    {
        free(s->address[i]);
    }

    sem_destroy(&s->mutex);
    sem_destroy(&s->space);
    sem_destroy(&s->item);
}