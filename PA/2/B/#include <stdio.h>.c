#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>    

int main()
{

    int pid1 = fork();
    int pid2 = fork();

    if(pid1)
    {
        printf("Hello!\n");
    }
    else if(pid2)
    {
        printf("World!\n");
    }
    else
    {
        fork();
    }
    printf("World!\n");
}