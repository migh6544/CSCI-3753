#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>                     

#define BUFFER_SIZE 1024


int main(int argc, char * argv[]) 
{
	bool run = true;
	char option;
    FILE * fd;
    fd = fopen(argv[1], "r+");

    if(fd == NULL) 
    {
		printf("Filename doesn't exist, or isn't readable/writeable!\n");
		return -1;
	}

    while(run == true) 
    {
        int statEOF;
        char * statsEOF;
        int readSize;
        char * readPtr;
        char strngArry[BUFFER_SIZE];
        int adjust;
        int offset;
        int whence;

        printf("\n");
        printf("Option (r for read, w for write, s for seek):\n");
        statEOF = scanf(" %c", &option);

        if (statEOF == EOF)
        {
            run = false;
            exit(0);
        }
        getchar();

        switch (option)
        {
            case 'r': 
                printf("Enter the number of bytes you want to read:\n");
                statEOF = scanf("%d", &readSize);

                if (statEOF == EOF)
                {
                    run = false;
                    exit(0);
                }

                readPtr = malloc(readSize);
                fread(readPtr, sizeof(char), readSize, fd);

                for(int i = 0; i < readSize; i++) 
                {
                    printf("%c", * (readPtr + i));
                }
                printf("\n");
                free(readPtr);
                break;

            case 'w': 
                printf("Enter the string you want to write:\n");
                statsEOF = fgets(strngArry, BUFFER_SIZE, stdin);

                if (statsEOF == NULL)
                {
                    run = false;
                    exit(0);
                }

                if (strngArry[strlen(strngArry) - 1] == '\n')
                {
                    adjust = fwrite(strngArry, sizeof(char), strlen(strngArry) - 1, fd);
                }
                else
                {
                    adjust = fwrite(strngArry, sizeof(char), strlen(strngArry), fd);
                }
                break;

            case 's':
                printf("Enter an offset value:\n");
                statEOF = scanf("%d", &offset);
                if (statEOF == EOF)
                {
                    run = false;
                    exit(0);
                }

                printf("Enter a value for whence (0 for SEEK_SET, 1 for SEEK_CUR, 2 for SEEK_END):\n");
                statEOF = scanf("%d", &whence);
                if (statEOF == EOF)
                {
                    run = false;
                    exit(0);
                }
                
                fseek(fd, offset, whence);
                break;

            default:
                break;
        }
    }

	fclose(fd);
	return 0;
}