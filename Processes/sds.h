/*
 * **************************************************
 * Author: Matthew Cheong
 * File: sds.h
 * Created Date: Wednesday, May 2nd 2018, 8:46:24 pm
 * -----
 * Last Modified: Sat May 05 2018
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

void validateArgs(int argc, char* argv[]);

void initMemory(int* semaphoresFD, int* writeNextFD, int* numReadingFD,
int* data_bufferFD, int* trackerFD);

void mapMemory(int* semaphoresFD, int* writeNextFD, int* numReadingFD,
int* data_bufferFD, int* trackerFD, sem_t** semaphores, int* writeNext,
int* numReading, int** data_buffer, int** tracker);

void cleanMemory(int* semaphoresFD, int* writeNextFD, int* numReadingFD,
int* data_bufferFD, int* trackerFD, sem_t** semaphores, int* writeNext,
int* numReading, int** data_buffer, int** tracker);

void reader (sem_t** semaphores, int* writeNext, int* numReading,
int** data_buffer, int** tracker);

void writer (sem_t** semaphores, int* writeNext, int* numReading,
int** data_buffer, int** tracker);