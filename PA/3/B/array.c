// Including header files
#include "array.h"
#include <string.h>


// Function to initialize the stack struct and allocate memory
int array_init(array *s)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        s->address[i] = malloc(sizeof(char) * MAX_NAME_LENGTH);             // Allocating memory for the contents of the stack struct (strings) 
    }

    sem_init(&s->mutex, 0, 1);                                              // Initializing the semaphore to act as a mutext and set to 1 (Unlocked)
    sem_init(&s->space, 0, ARRAY_SIZE);                                     // Initializing the producer count semaphore (Full)
    sem_init(&s->item, 0, 0);                                               // Initializing the consumer count semaphore (Empty)

    s->top = 0;                                                             // Stack count is set to 0

    return 0;
}

// Function to place a string on the stack struct
int array_put(array *s, char *hostname)
{
    sem_wait(&s->space);                                                     // Decrement count of the producer semaphore (Space available) 
        sem_wait(&s->mutex);                                                 // Lock the critical section
            strncpy(s->address[s->top], hostname, MAX_NAME_LENGTH);          // Place string onto the stack struct from pointer
            s->top++;                                                        // Increment stack count
        sem_post(&s->mutex);                                                 // Unlock the critical section
    sem_post(&s->item);                                                      // Increment count of the consumer semaphore (Item(s) available)

    return 0;
}

// Function to collect a string from the stack struct
int array_get(array *s, char **hostname)
{
    sem_wait(&s->item);                                                      // Decrement count of the consumer semaphore (Item(s) available)
        sem_wait(&s->mutex);                                                 // Lock the critical section
            s->top--;                                                        // Decrement stack count
            *hostname = malloc(sizeof(char) * MAX_NAME_LENGTH);              // Allocate memory to copy a string to and set a pointer to it
            strncpy(*hostname, s->address[s->top], MAX_NAME_LENGTH);         // Copy a string onto the allocated memory
        sem_post(&s->mutex);                                                 // Unlock the critical section
    sem_post(&s->space);                                                     // Increment count of the producer semaphore (Space available)

    return 0;
}

// Deallocate meomry and destroy semaphores of the stack struct
void array_free(array *s)
{
    for (int i = ARRAY_SIZE - 1; i >= 0; i--)
    {
        free(s->address[i]);                                                 // Free the allocated memory of the contents of the stack struct 
    }

    sem_destroy(&s->mutex);                                                  // Destroy the critical section mutex
    sem_destroy(&s->space);                                                  // Destroy producer semaphore
    sem_destroy(&s->item);                                                   // Destroy consumer semaphore
}