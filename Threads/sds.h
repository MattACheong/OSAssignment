/*
 * **************************************************
 * Author: Matthew Cheong
 * File: sds.h
 * Created Date: Wednesday, May 2nd 2018, 8:46:24 pm
 * -----
 * Last Modified: Sun May 06 2018
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
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 20
#define SHARED_DATA "shared_data"
#define SHARED_DATA_SIZE 100

typedef struct
{
    int numReaders;
    int numWriters;
    int sleepRead;
    int sleepWrite;
    int numReading;
    int writeNext;
} Values;

typedef struct
{
    sem_t mutex;
    sem_t wrt;
    sem_t empty;
    sem_t full;
} Semaphores;

void validateArgs(int argc, char* argv[]);

void initMemory(int* semaphoresFD, int* valuesFD,
int* data_bufferFD, int* trackerFD, int* sharedDataFD);

void mapMemory(int* semaphoresFD, int* valuesFD,
int* data_bufferFD, int* trackerFD, int* sharedDataFD,
Semaphores** semaphores, Values** values,
int** data_buffer, int** tracker, int** sharedData);

void initSemaphores(Semaphores* semaphores);

void cleanMemory(int* semaphoresFD, int* valuesFD,
int* data_bufferFD, int* trackerFD, int* sharedDataFD,
Semaphores** semaphores, Values** values,
int** data_buffer, int** tracker, int** sharedData);

void reader (Semaphores* semaphores, Values* values,
int** data_buffer, int** tracker);

void writer (Semaphores* semaphores, Values* values,
int** data_buffer, int** tracker, int** sharedData);