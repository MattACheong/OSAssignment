/*
 * **************************************************
 * Author: Matthew Cheong
 * File: sds.c
 * Created Date: Wednesday, May 2nd 2018, 8:46:27 pm
 * -----
 * Last Modified: Thu May 03 2018
 * Modified By: Matthew Cheong
 * -----
 * **************************************************
 */

#include "sds.h"

int main(int argc, char* argv[])
{
    validateArgs(argc, argv);
    
    // Read in arguments
    int numReaders = atoi(argv[1]);
    int numWriters = atoi(argv[2]);
    int sleepRead = atoi(argv[3]);
    int sleepWrite = atoi(argv[4]);

    // Variables
    int ii;
    int pid = 23546;

    // Shared Memory Declaration
    sem_t *semaphores;
    int writeNext, numReading, *data_buffer, *tracker;
    
    // File Descriptors
    int semaphoresFD, writeNextFD, numReadingFD, data_bufferFD, 
    trackerFD;

    initMemory(&semaphoresFD, &writeNextFD, &numReadingFD,
     &data_bufferFD, &trackerFD);

    mapMemory(&semaphoresFD, &writeNextFD, &numReadingFD,
     &data_bufferFD, &trackerFD,
     &semaphores, &writeNext, &numReading, &data_buffer, &tracker);

    // Readers
    for( ii = 1; ii <= numReaders; ii++)
    {
        if(pid > 0)
        {
            printf("Forking readers... %d\n", ii);
            pid = fork();
        }
    }
    if(pid == 0)
    {
        printf("R<%d>: I live!\n", getpid());
        reader();
    }

    // Writers
    for( ii = 1; ii <= numWriters; ii++)
    {
        if(pid > 0)
        {
            printf("Forking writers... %d\n", ii);
            pid = fork();
        }
    }
    if(pid == 0)
    {
        printf("W<%d>: I live!\n", getpid());        
        writer();
    }
    
    cleanMemory(&semaphoresFD, &writeNextFD, &numReadingFD,
     &data_bufferFD, &trackerFD,
     &semaphores, &writeNext, &numReading, &data_buffer, &tracker);
    
    return (0);
}

// Validates the command-line arguments
void validateArgs(int argc, char* argv[])
{
    // Ensure correct number of command line parameters
    if(argc != 5)
    {
        printf("Incorrect number of parameters.\n4 expected\n");
        exit(1);
    }
}

// Initializes the shared memory
void initMemory(int* semaphoresFD, int* writeNextFD, int* numReadingFD,
     int* data_bufferFD, int* trackerFD)
{
     *semaphoresFD = shm_open("semaphores", O_CREAT | O_RDWR, 0666);
     *writeNextFD = shm_open("writeNext", O_CREAT | O_RDWR, 0666);
     *numReadingFD = shm_open("numReading", O_CREAT | O_RDWR, 0666);
     *data_bufferFD = shm_open("data_buffer", O_CREAT | O_RDWR, 0666);
     *trackerFD = shm_open("tracker", O_CREAT | O_RDWR, 0666);

    // Check if created correctly
    if(*semaphoresFD == -1 || *writeNextFD == -1 || *numReadingFD == -1
        || *data_bufferFD == -1 || *trackerFD == -1)
    {
        fprintf( stderr, "Error creating shared memory blocks\n" );
        exit(1);
    }

    // Set size
    if(ftruncate(*semaphoresFD, sizeof(int)) == -1 )
    {
        fprintf( stderr, "Error setting size for semaphores\n" );
        exit(1);
    }
    if(ftruncate(*writeNextFD, sizeof(int)) == -1 )
    {
        fprintf( stderr, "Error setting size for writeNext\n" );
        exit(1);
    }
    if(ftruncate(*numReadingFD, sizeof(int)) == -1 )
    {
        fprintf( stderr, "Error setting size for numReading\n" );
        exit(1);
    }
    if(ftruncate(*data_bufferFD, sizeof(int)) == -1 )
    {
        fprintf( stderr, "Error setting size for data buffer\n" );
        exit(1);
    }
    if(ftruncate(*trackerFD, sizeof(int)) == -1 )
    {
        fprintf( stderr, "Error setting size for tracker\n" );
        exit(1);
    }
}

// Map shared memory to addresses
void mapMemory(int* semaphoresFD, int* writeNextFD, int* numReadingFD,
     int* data_bufferFD, int* trackerFD,
     sem_t** semaphores, int* writeNext, int* numReading,
     int** data_buffer, int** tracker)
{
    *semaphores = mmap(NULL, sizeof(sem_t) * 2, PROT_READ | PROT_WRITE,
        MAP_SHARED, *semaphoresFD, 0);
    *writeNext = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, *writeNextFD, 0);
    *numReading = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, *numReadingFD, 0);
    *data_buffer = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, *data_bufferFD, 0);
    *tracker = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
        MAP_SHARED, *trackerFD, 0);
}
void cleanMemory(int* semaphoresFD, int* writeNextFD, int* numReadingFD,
     int* data_bufferFD, int* trackerFD,
     sem_t** semaphores, int* writeNext, int* numReading,
     int** data_buffer, int** tracker)
{
    // Destroy semaphores
    sem_destroy(&((*semaphores)[0]));
    sem_destroy(&((*semaphores)[1]));

    // Clean up shared memory
    shm_unlink("semaphores");
    shm_unlink("writeNext");
    shm_unlink("numReading");
    shm_unlink("data_buffer");
    shm_unlink("tracker");

    // Close file descriptors
    close(semaphoresFD);
    close(writeNextFD);
    close(numReadingFD);
    close(data_bufferFD);
    close(trackerFD);

    // Unmap memory
    munmap(*semaphores, sizeof(sem_t) * 2);
    munmap(*writeNext, sizeof(int));
    munmap(*numReading, sizeof(int));
    munmap(*data_buffer, sizeof(int));
    munmap(*tracker, sizeof(int));   
}

void reader (void)
{
    exit(1);
}
void writer (void)
{
    exit(1);
}
