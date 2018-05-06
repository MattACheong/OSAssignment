/*
 * **************************************************
 * Author: Matthew Cheong
 * File: sds.h
 * Created Date: Wednesday, May 2nd 2018, 8:46:24 pm
 * -----
 * Last Modified: Mon May 07 2018
 * Modified By: Matthew Cheong
 * -----
 * **************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define BUFFER_SIZE 20
#define SHARED_DATA "shared_data"
#define SHARED_DATA_SIZE 100

typedef struct
{
    int numReaders;
    int numWriters;
    int sleepRead;
    int sleepWrite;
    int writeNext;
} Values;

void validateArgs(int argc, char* argv[]);
void initMemory(int** dataBuffer, int** tracker, int** sharedData);
void cleanMemory(void);
void* writer();
void* reader();