/*
 * **************************************************
 * Author: Matthew Cheong
 * File: sds.c
 * Created Date: Wednesday, May 2nd 2018, 8:46:27 pm
 * -----
 * Last Modified: Sat May 05 2018
 * Modified By: Matthew Cheong
 * -----
 * **************************************************
 */

#include "sds.h"

int main(int argc, char* argv[])
{
    validateArgs(argc, argv);
    
    // Read in arguments
    char* fileName = argv[1];
    int numReaders = atoi(argv[2]);
    int numWriters = atoi(argv[3]);
    int sleepRead = atoi(argv[4]);
    int sleepWrite = atoi(argv[5]);

    // Variables
    int ii;
    int pid = 23546;        //Set parent pid

    // Shared Memory Declaration
    Semaphores semaphores;
    Values values;
    int *data_buffer, *tracker;
    
    // File Descriptors
    int semaphoresFD, valuesFD, data_bufferFD, trackerFD;

    initMemory(&semaphoresFD, &valuesFD, &data_bufferFD, &trackerFD);

    mapMemory(&semaphoresFD, &valuesFD, &data_bufferFD, &trackerFD,
    &semaphores, &values, &data_buffer, &tracker);
    
    // Initialize Data
    for (ii = 0; ii <= BUFFER_SIZE; ii++)
    {
        tracker[ii] = 0;
    }

    FILE* f = fopen(fileName, "r");
    if(f = NULL)
    {
        fprintf(stderr, "Error opening file!\n");
        exit(1);
    }

    // Create Readers
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
        reader(&semaphores, &values, &data_buffer, &tracker);
    }

    // Create writers
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
        writer(&semaphores, &values, &data_buffer, &tracker);
    }
    
    cleanMemory(&semaphoresFD, &valuesFD, &data_bufferFD, &trackerFD,
    &semaphores, &values, &data_buffer, &tracker);
    
    return (0);
}

// Validates the command-line arguments
void validateArgs(int argc, char* argv[])
{
    // Ensure correct number of command line parameters
    if(argc != 6)
    {
        fprintf(stderr, "Incorrect number of parameters.\n5 expected\n");
        exit(1);
    }
    // Ensure at least one reader and writer
    if(argv[2] < 1 || argv[3] < 1)
    {
        fprintf(stderr, "Must have at least 1 reader and 1 writer\n");
        exit(1);
    }
    // Ensure sleep time is positive
    if(argv[4] < 0 || argv[5] < 0)
    {
        fprintf(stderr, "Wait times must be positive\n");
        exit(1);
    }
}

// Initializes the shared memory
void initMemory(int* semaphoresFD, int* valuesFD,
int* data_bufferFD, int* trackerFD)
{
     *semaphoresFD = shm_open("semaphores", O_CREAT | O_RDWR, 0666);
     *valuesFD = shm_open("values", O_CREAT | O_RDWR, 0666);
     *data_bufferFD = shm_open("data_buffer", O_CREAT | O_RDWR, 0666);
     *trackerFD = shm_open("tracker", O_CREAT | O_RDWR, 0666);

    // Check if created correctly
    if(*semaphoresFD == -1 || *valuesFD == -1 ||
    data_bufferFD == -1 || *trackerFD == -1)
    {
        fprintf( stderr, "Error creating shared memory bsemaphores\n" );
        exit(1);
    }

    // Set size
    if(ftruncate(*semaphoresFD, sizeof(int)) == -1 )
    {
        fprintf( stderr, "Error setting size for semaphores\n" );
        exit(1);
    }
    if(ftruncate(*valuesFD, sizeof(int)) == -1 )
    {
        fprintf( stderr, "Error setting size for values\n" );
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
void mapMemory(int* semaphoresFD, int* valuesFD,
int* data_bufferFD, int* trackerFD,
Semaphores* semaphores, Values* values,
int** data_buffer, int** tracker)
{
    semaphores = mmap(NULL, sizeof(Semaphores), PROT_READ |
    PROT_WRITE, MAP_SHARED, *semaphoresFD, 0);
    values = mmap(NULL, sizeof(Values), PROT_READ | PROT_WRITE,
    MAP_SHARED, *valuesFD, 0);
    *data_buffer = (int*) mmap(NULL, sizeof(int) * BUFFER_SIZE, PROT_READ |
    PROT_WRITE, MAP_SHARED, *data_bufferFD, 0);
    *tracker = (int*) mmap(NULL, sizeof(int) * BUFFER_SIZE, PROT_READ |
    PROT_WRITE, MAP_SHARED, *trackerFD, 0);
}

// Initializes semaphores
void initSemaphores(Semaphores* semaphores)
{
    int errCheck;
    errCheck += sem_init( &semaphores->mutex, -1, 1 );
    errCheck += sem_init( &semaphores->wrt, -1, 0 );
    errCheck += sem_init( &semaphores->empty, -1, 1 );
    if (errCheck != 0)
    {
        fprintf(stderr, "Could not initialize semaphore\n");
        exit(1);
    }
}

// Clears shared memory
void cleanMemory(int* semaphoresFD, int* valuesFD,
int* data_bufferFD, int* trackerFD,
Semaphores* semaphores, Values* values,
int** data_buffer, int** tracker)
{
    // Destroy semaphores
    sem_destroy(&semaphores->mutex);
    sem_destroy(&semaphores->wrt);
    sem_destroy(&semaphores->empty);

    // Clean up shared memory
    shm_unlink("semaphores");
    shm_unlink("values");
    shm_unlink("data_buffer");
    shm_unlink("tracker");

    // Close file descriptors
    close(*semaphoresFD);
    close(*valuesFD);
    close(*data_bufferFD);
    close(*trackerFD);

    // Unmap memory
    munmap(semaphores, sizeof(Semaphores));
    munmap(values, sizeof(Values));
    munmap(*data_buffer, sizeof(int) * BUFFER_SIZE);
    munmap(*tracker, sizeof(int) * BUFFER_SIZE);   
}

void reader (Semaphores* semaphores, Values* values,
int** data_buffer, int** tracker)
{
    int readCount = 0;

    while (readCount < SHARED_DATA_SIZE)
    {
        // Lock
        sem_wait(&semaphores->mutex);
        (values->numReading)++;
        if(values->numReading == 1)
            sem_wait(&semaphores->wrt);
        sem_post(&semaphores->mutex);
        
        // Read
        
        readCount++;
        
        // Unlock
        sem_wait(&semaphores->mutex);
        (values->numReading)--;
        if((values->numReading) == 0)
            sem_post(&semaphores->wrt);
        sem_post(&semaphores->mutex);
    }
    exit(1);
}
void writer (Semaphores* semaphores, Values* values,
int** data_buffer, int** tracker)
{
    while (values->writeNext < SHARED_DATA_SIZE)
    {
        sem_wait(&semaphores->wrt);

        // Write
        if(*tracker[values->writeNext] == 0)
        {
            // data_buffer[values->writeNext] = 
        }

        // Unlock
        sem_post(&(semaphores->wrt));

    }

    exit(1);
}