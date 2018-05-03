/*
 * **************************************************
 * Author: Matthew Cheong
 * File: sds.h
 * Created Date: Wednesday, May 2nd 2018, 8:46:24 pm
 * -----
 * Last Modified: Thu May 03 2018
 * Modified By: Matthew Cheong
 * -----
 * **************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

void validateArgs(int argc, char* argv[]);

void initMemory(int* semaphoresFD, int* writeNextFD, int* numReadingFD,
     int* data_bufferFD, int* trackerFD);

void mapMemory(int* semaphoresFD, int* writeNextFD, int* numReadingFD,
     int* data_bufferFD, int* trackerFD,
     sem_t** semaphores, int* writeNext, int* numReading,
     int** data_buffer, int** tracker);

void cleanMemory(int* semaphoresFD, int* writeNextFD, int* numReadingFD,
     int* data_bufferFD, int* trackerFD,
     sem_t** semaphores, int* writeNext, int* numReading,
     int** data_buffer, int** tracker);

void reader (void);
void writer (void);