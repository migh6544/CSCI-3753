// Including header files
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

#include "multi-lookup.h"
#include "array.h"
#include "util.h"

// Defining the requestor data struct
typedef struct
{
	int nextFile;								// Count indicator to the next file
	int inCount;								// Counter of input files
	FILE *inLog;								// Pointer to the requestor log file
	char *inFiles[100];							// Pointer to input files array
	array *sharBuff;							// Pointer to shared buffer array struct
	pthread_mutex_t lockInFile;					// Thread mutex for inout files
	pthread_mutex_t lockInLog;					// Thread mutex for requestor log file
} request;

// Defining the resolver data struct
typedef struct
{
	pthread_mutex_t lockOutFile;				// Thread mutex for output files
	array *sharBuff;							// Pointer to shared buffer array struct
	FILE *outLog;								// Pointer to the resolver log file
	sem_t signal;								// Producer consumer signal semaphore
} resolve;

// Defining the arguments cleaner data struct
typedef struct
{
	pthread_mutex_t *lockOutFile;				// Pointer to the thread mutex for output files
	int IPcount;								// Counter of IP addresses resolved
} CleanupArgs;


// Defining the requestor function
void *req(void *data)
{
	request *reqData = (request *)data;			// Creating an instance of request struct type
	long nameCount = 0;							// Defining and intializing a counter for hostnames handeled
	int fileCount = 0;							// Defining and intializing a counter for files handeled

	while (1)
	{
		char *ptrFile = NULL;

		pthread_mutex_lock(&reqData->lockInFile);					// Locking critical section
			if (reqData->nextFile < reqData->inCount)				// Checking if there are files to service
			{
				ptrFile = reqData->inFiles[reqData->nextFile];		// Seting a pointer to the next file on the list
				reqData->nextFile++;								// Incrementing the next file counter
				fileCount++;										// Incrementing the count of total files handeled
			}
		pthread_mutex_unlock(&reqData->lockInFile);					// Unlock critical section

		if (ptrFile == NULL)										// Checking if there are no files to service
		{
			break;													// Stop if there is no file to point to
		}

		FILE *newFile = fopen(ptrFile, "r");						// Opening the next file and setting a pointer to the file
		char strPtr[1024];											// Defining and intializing an array for the hostname string pointers

		while (fgets(strPtr, 1023, newFile) != NULL)				// Checking for hostname strings to service
		{
			pthread_mutex_lock(&reqData->lockInLog);				// Locking critical section
				fputs(strPtr, reqData->inLog);						// Adding string to the requestor log file
			pthread_mutex_unlock(&reqData->lockInLog);				// Unlocking critical section

			strPtr[strlen(strPtr) - 1] = 0;							// Accounting for null terminator
			array_put(reqData->sharBuff, strPtr);					// Placing the hostname string to on the shared buffer

			nameCount++;											// incrementing the counter of hostname strings services
		}

		fclose(newFile);											// Closing the input file
	}

	pthread_mutex_lock(&reqData->lockInLog);																		// Locking critical section
		printf("thread %08x serviced %d files\n", (unsigned int)syscall(SYS_gettid), fileCount);					// Printing progress of the input thread 
	pthread_mutex_unlock(&reqData->lockInLog);																		// Unlocking critical section

	pthread_exit((void *) nameCount);																				// Terminating the input thread
}

// Defining a funtion that frees arguments allocated memory
void getterCleanup(void *args)
{
	CleanupArgs *cArgs = (CleanupArgs *)args;																		// Creating an instance of CleanupArgs struct type 

	pthread_mutex_lock(cArgs->lockOutFile);																			// Locking critical section
		printf("thread %08x resolved %d hostnames\n", (unsigned int)syscall(SYS_gettid), cArgs->IPcount);			// Printing progress of the output thread 
	pthread_mutex_unlock(cArgs->lockOutFile);																		// locking critical section

	free(cArgs);																									// Freeing allocated memory used by output thread
}

// Defining the resolver function
void *res(void *data)
{
	resolve *resData = (resolve *)data;											// Creating an instance of resolve struct type
	char *ptrName = NULL;														// Declaring and intializing a pointer for hostnames
	char ptrIP[1024];															// Defining and intializing an array for the IP addresse strings

	CleanupArgs *cArgs = (CleanupArgs *)malloc(sizeof(CleanupArgs));			// Allocating memory to be used by the output threads
	cArgs->IPcount = 0;															// Setting intial count of IP addresses serviced to 0
	cArgs->lockOutFile = &resData->lockOutFile;									// Setting pointer to the output mutex from the the CleanupArgs struct to the resolve struct
	pthread_cleanup_push(getterCleanup, cArgs);									// Setting how to handel the cleanup of ouput threads

	while (1)
	{
		array_get(resData->sharBuff, &ptrName);									// Retriving the hostname to service
		if (dnslookup(ptrName, ptrIP, 1023) == UTIL_FAILURE)					// Checking if there is no IP address listed for the hostname
		{
			pthread_mutex_lock(&resData->lockOutFile);							// Locking critical section
			fprintf(resData->outLog, "%s, NOT_RESOLVED\n", ptrName);			// Printing to the requestor log file that the hostname was not serviced
			pthread_mutex_unlock(&resData->lockOutFile);						// Unlocking critical section
		}
		else
		{
			pthread_mutex_lock(&resData->lockOutFile);							// Locking critical section
			fprintf(resData->outLog, "%s, %s\n", ptrName, ptrIP);				// Printing to the requestor log file that the hostname serviced and assigned IP address
			pthread_mutex_unlock(&resData->lockOutFile);						// Unlocking critical section
		}

		cArgs->IPcount++;														// Incrementing the count of hostnames serviced with IP addresse lookup

		free(ptrName);															// Freeing allocated memory used by the ppointer to hostnames
		sem_post(&resData->signal);												// Incrementing the producer consumer signal semaphore 
	}

	pthread_cleanup_pop(1);														// Calling of the output thread cleanup mechanisim for the output thread
}


// Defining our main section for running the program
int main(int argc, char *argv[])
{
	struct timeval start, stop;													// Instantiating a timer struct
	gettimeofday(&start, NULL);													// Using standard function to capture starting time

	if(argc < 6)																// Checking if the number of arguments are less than needed and printing a message accordingly
    {
		fprintf(stderr, "Arguments Error! Required: ./multi-lookup <# requester> <# resolver> <requester log> <resolver log> [<data file> ...]\n");
		return -1;
	}
    else if(argc > (5 + MAX_INPUT_FILES))										// Checking if the number of arguments are more than needed and printing a message accordingly
    {
        fprintf(stderr, "Arguments Error! Too many input files.\n");
        return -1;	
	}

	int reqNum = atoi(argv[1]);													// Converting the input of number of requestors to int type
	if(reqNum < 1)																// Checking for the minimum number of requestors needed and printing a message accordingly
    {
		fprintf(stderr, "Need 1 requestor thread atleast.\n");
		return -1;
	}
    else if (reqNum > MAX_REQUESTER_THREADS)									// Checking for the maximum number of requestors allowed and printing a message accordingly
    {
        fprintf(stderr, "Need less  than %d requester threads.\n", MAX_REQUESTER_THREADS);
        return -1;
	}

	int resNum = atoi(argv[2]);													// Converting the input of number of resolvers to int type
	if (resNum < 1)																// Checking for the minimum number of resolvers needed and printing a message accordingly
	{
        fprintf(stderr, "Need 1 resolver thread atleast.\n");
        return -1;
	}
    else if (resNum > MAX_RESOLVER_THREADS)										// Checking for the maximum number of resolvers allowed and printing a message accordingly
    {
        fprintf(stderr, "Need less  than %d resolver threads.\n", MAX_RESOLVER_THREADS);
        return -1;
	}

	array buff;																	// Creating an instance of array struct type
	array_init(&buff);															// Intializing the array struct type

	request dataReq;															// Creating a request struct type
	dataReq.sharBuff = &buff;													// Setting the shared buffer pointer of the requestor struct to the newly declared array struct
	dataReq.inCount = 0;														// Intializing the input files counter for the requestor struct to 0
	dataReq.nextFile = 0;														// Intializing the input files next file counter for the requestor struct to 0
	pthread_mutex_init(&dataReq.lockInFile, NULL);								// intializing the input thread input file mutex
	pthread_mutex_init(&dataReq.lockInLog, NULL);								// intializing the input thread requestor log file mutex

	dataReq.inLog = fopen(argv[3], "w+");										// Opening and setting the request struct requestor log file pointer to appropriate command line argument

	for(int i = 5; i < argc; i++)
    {
		if(access(argv[i], F_OK) == -1)											// Checking the provided paths validatey for the input files
        {
            fprintf(stderr, "Bad input filepath: %s\n", argv[i]);				// Printing a messae if paths(s) are invalid
        }
        else
        {
			dataReq.inFiles[dataReq.inCount] = malloc(sizeof(char) * 256);		// Allocating memory for the request struct input files
            strncpy(dataReq.inFiles[dataReq.inCount], argv[i], 255);			// Copying the hostname string to the requestor struct input files
			dataReq.inCount++;													// Incrementing the counter of input files for the request struct 
        }
	}

	resolve dataRes;															// Creating a resolve struct type
	dataRes.sharBuff = &buff;													// Setting the shared buffer pointer of the resolve struct to the newly declared array struct
	dataRes.outLog = fopen(argv[4], "w+");										// Opening and setting the resolve struct resolver log file pointer to appropriate command line argument
	pthread_mutex_init(&dataRes.lockOutFile, NULL);								// Initializing the output file mutex for the resolve struct
	sem_init(&dataRes.signal, 0, 0);											// Intializing the producer consumer counting semaphore

	pthread_t reqTID[reqNum];													// Intializing the request input threads and their TIDs
	pthread_t resTID[resNum];													// Intializing the resolve output threads and their TIDs

	for (int i = 0; i < reqNum; i++)
	{
		pthread_create(&reqTID[i], NULL, req, &dataReq);						// Creating the input requestor threads
	}

	for (int i = 0; i < resNum; i++)
	{
		pthread_create(&resTID[i], NULL, res, &dataRes);						// Creating the output resolver threads
	}

	int hostSum = 0;															// Declaring and intializing a counter for the hostnames serviced

	for (int i = 0; i < reqNum; i++)
	{
		void *hostCount = NULL;													// Declaring and intializing a pointer to the hostname based on count
		pthread_join(reqTID[i], &hostCount);									// Collecting and terminating the input requestor threads
		hostSum = hostSum + (long)hostCount;									// Calculating the total number of ostnames serviced
	}

	while (hostSum > 0)															// Checking for hostnames to service
	{
		sem_wait(&dataRes.signal);												// Decrementing the producer consumer counting semaphore
		hostSum--;																// Decrementing the total sum of hostnames serviced
	}

	for (int i = 0; i < resNum; i++)
	{
		pthread_cancel(resTID[i]);												// Requesting cancelation of output resolver threads
		pthread_join(resTID[i], NULL);											// Collecting and terminating the output resolver threads
	}

	fclose(dataReq.inLog);														// Closing the requestore log file
	fclose(dataRes.outLog);														// Closing the resolver log file

	array_free(&buff);															// Freeing memory allocated to the shared buffer pointer array struct
	for (int i = 0; i < dataReq.inCount; i++)
	{
		free(dataReq.inFiles[i]);												// Freeing memory to the request struct input files array pointer
	}

	pthread_mutex_destroy(&dataReq.lockInFile);									// Destroying the input files requestor struct mutex
	pthread_mutex_destroy(&dataReq.lockInLog);									// Destroying the requestor log file mutex
	pthread_mutex_destroy(&dataRes.lockOutFile);								// Destroying the output files resolver struct mutex
	sem_destroy(&dataRes.signal);												// Destroying the producer consumer counting semaphore

	gettimeofday(&stop, NULL);													// Using standard function to capture stoping time

	// Calculating and printing the program runtime in seconds up to six decimals
	printf("%s: total time is %0.6f seconds\n", argv[0], ((stop.tv_sec * 1000000 + stop.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)) / (float)1000000);
	return 0;
}