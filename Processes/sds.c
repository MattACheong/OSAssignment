/*
 * **************************************************
 * Author: Matthew Cheong
 * File: sds.c
 * Created Date: Wednesday, May 2nd 2018, 8:46:27 pm
 * -----
 * Last Modified: Sun May 06 2018
 * Modified By: Matthew Cheong
 * -----
 * **************************************************
 */

#include "sds.h"

int main(int argc, char* argv[])
{
    validateArgs(argc, argv);

    // Variables
    int ii;
    int pid = 23546;        //Set parent pid

    // Shared Memory Declaration
    Semaphores* semaphores;
    Values* values;
    int *dataBuffer, *tracker, *sharedData;

    // File Descriptors
    int semaphoresFD, valuesFD, dataBufferFD, trackerFD, sharedDataFD;

    initMemory(&semaphoresFD, &valuesFD, &dataBufferFD, &trackerFD,
        &sharedDataFD);

    mapMemory(&semaphoresFD, &valuesFD, &dataBufferFD, &trackerFD,
    &sharedDataFD, &semaphores, &values, &dataBuffer, &tracker, &sharedData);

    initSemaphores(semaphores);

    // Initialize Data
    for (ii = 0; ii <= BUFFER_SIZE; ii++)
    {
        tracker[ii] = 0;
        dataBuffer[ii] = 0;
    }
    values->numReading = 0;
    values->writeNext = 0;

    // Read in arguments
    char* fileName = argv[1];
    values->numReaders = atoi(argv[2]);
    values->numWriters = atoi(argv[3]);
    values->sleepRead = atoi(argv[4]);
    values->sleepWrite = atoi(argv[5]);

    // Read in file
    FILE* f = fopen(fileName, "r");
    if(f == NULL)
    {
        fprintf(stderr, "Error opening file!\n");
        exit(1);
    }
    for(ii = 0; ii < SHARED_DATA_SIZE; ii++)
    {
        fscanf(f, "%d ", &(sharedData[ii]));
    }
    fclose(f);

    // Refreshes output file
    f = fopen("sim_out", "w");
    if(f == NULL)
    {
        fprintf(stderr, "Error opening file!\n");
        exit(1);
    }
    fclose(f);

    // Create Readers
    for( ii = 1; ii <= values->numReaders; ii++)
    {
        if(pid > 0)
        {
            printf("Forking %d reader(s)... \n", ii);
            pid = fork();
        }
    }
    if(pid == 0)
    {
        reader(semaphores, values, &dataBuffer, &tracker);
    }

    // Create writers
    for( ii = 1; ii <= values->numWriters; ii++)
    {
        if(pid > 0)
        {
            printf("Forking %d writer(s)...\n", ii);
            pid = fork();
        }
    }
    if(pid == 0)
    {
        writer(semaphores, values, &dataBuffer, &tracker, &sharedData);
        printf("Write finished!\n");
    }
    for(ii = 0; ii < (values->numReaders+values->numWriters); ii++)
    {
        wait(NULL);
    }
    cleanMemory(&semaphoresFD, &valuesFD, &dataBufferFD, &trackerFD,
    &sharedDataFD, &semaphores, &values, &dataBuffer, &tracker, &sharedData);

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
    if(atoi(argv[2]) < 1 || atoi(argv[3]) < 1)
    {
        fprintf(stderr, "Must have at least 1 reader and 1 writer\n");
        exit(1);
    }
    // Ensure sleep time is positive
    if(atoi(argv[4]) < 0 || atoi(argv[5]) < 0)
    {
        fprintf(stderr, "Wait times must be positive\n");
        exit(1);
    }
}

// Initializes the shared memory
void initMemory(int* semaphoresFD, int* valuesFD,
int* dataBufferFD, int* trackerFD, int* sharedDataFD)
{
     *semaphoresFD = shm_open("semaphores", O_CREAT | O_RDWR, 0666);
     *valuesFD = shm_open("values", O_CREAT | O_RDWR, 0666);
     *dataBufferFD = shm_open("dataBuffer", O_CREAT | O_RDWR, 0666);
     *trackerFD = shm_open("tracker", O_CREAT | O_RDWR, 0666);
     *sharedDataFD = shm_open("sharedData", O_CREAT | O_RDWR, 0666);

    // Check if created correctly
    if(*semaphoresFD == -1 || *valuesFD == -1 || *dataBufferFD == -1 ||
        *trackerFD == -1 || *sharedDataFD == -1)
    {
        fprintf( stderr, "Error creating shared memory\n" );
        exit(1);
    }

    // Set size
    if(ftruncate(*semaphoresFD, sizeof(Semaphores)) == -1 )
    {
        fprintf( stderr, "Error setting size for semaphores\n" );
        exit(1);
    }
    if(ftruncate(*valuesFD, sizeof(Values)) == -1 )
    {
        fprintf( stderr, "Error setting size for values\n" );
        exit(1);
    }
    if(ftruncate(*dataBufferFD, sizeof(int) * BUFFER_SIZE) == -1 )
    {
        fprintf( stderr, "Error setting size for data buffer\n" );
        exit(1);
    }
    if(ftruncate(*trackerFD, sizeof(int) * BUFFER_SIZE) == -1 )
    {
        fprintf( stderr, "Error setting size for tracker\n" );
        exit(1);
    }
    if(ftruncate(*sharedDataFD, sizeof(int) * SHARED_DATA_SIZE) == -1 )
    {
        fprintf( stderr, "Error setting size for shared data\n" );
        exit(1);
    }
}

// Map shared memory to addresses
void mapMemory(int* semaphoresFD, int* valuesFD,
int* dataBufferFD, int* trackerFD, int* sharedDataFD,
Semaphores** semaphores, Values** values,
int** dataBuffer, int** tracker, int** sharedData)
{
    *semaphores = (Semaphores*) mmap(NULL, sizeof(Semaphores), PROT_READ |
        PROT_WRITE, MAP_SHARED, *semaphoresFD, 0);
    *values = (Values*) mmap(NULL, sizeof(Values), PROT_READ | PROT_WRITE,
        MAP_SHARED, *valuesFD, 0);
    *dataBuffer = (int*) mmap(NULL, sizeof(int) * BUFFER_SIZE, PROT_READ |
        PROT_WRITE, MAP_SHARED, *dataBufferFD, 0);
    *tracker = (int*) mmap(NULL, sizeof(int) * BUFFER_SIZE, PROT_READ |
        PROT_WRITE, MAP_SHARED, *trackerFD, 0);
    *sharedData = (int*) mmap(NULL, sizeof(int) * SHARED_DATA_SIZE, PROT_READ |
        PROT_WRITE, MAP_SHARED, *sharedDataFD, 0);
}

// Initializes semaphores
void initSemaphores(Semaphores* semaphores)
{
    int errCheck = 0;
    errCheck += sem_init(&semaphores->mutex, 1, 1);
    errCheck += sem_init(&semaphores->wrt, 1, 1);
    errCheck += sem_init(&semaphores->empty, 1, 1);
    errCheck += sem_init(&semaphores->full, 1, 1);
    if (errCheck != 0)
    {
        fprintf(stderr, "Could not initialize semaphore\n");
        exit(1);
    }
}

// Clears shared memory
void cleanMemory(int* semaphoresFD, int* valuesFD,
int* dataBufferFD, int* trackerFD, int* sharedDataFD,
Semaphores** semaphores, Values** values,
int** dataBuffer, int** tracker, int** sharedData)
{
    // Destroy semaphores
    sem_destroy(&(*semaphores)->mutex);
    sem_destroy(&(*semaphores)->wrt);
    sem_destroy(&(*semaphores)->empty);
    sem_destroy(&(*semaphores)->full);

    // Clean up shared memory
    shm_unlink("semaphores");
    shm_unlink("values");
    shm_unlink("dataBuffer");
    shm_unlink("tracker");
    shm_unlink("sharedData");

    // Close file descriptors
    close(*semaphoresFD);
    close(*valuesFD);
    close(*dataBufferFD);
    close(*trackerFD);
    close(*sharedDataFD);

    // Unmap memory
    munmap(*semaphores, sizeof(Semaphores));
    munmap(*values, sizeof(Values));
    munmap(*dataBuffer, sizeof(int) * BUFFER_SIZE);
    munmap(*tracker, sizeof(int) * BUFFER_SIZE);
    munmap(*sharedData, sizeof(int) * SHARED_DATA_SIZE);
}

// Reader
void reader (Semaphores* semaphores, Values* values,
int** dataBuffer, int** tracker)
{
    int ii, readCount = 0;

    printf("R <%d>: I live!\n", getpid());

    while (readCount < SHARED_DATA_SIZE)
    {
        // Locks if nothing to read
        if(readCount == values->writeNext)
            sem_wait(&semaphores->full);

        // Locks when reading also unlocks any full writers
        sem_wait(&semaphores->mutex);
        printf("R <%d> set mutex\n", getpid());
        values->numReading++;
        printf("NumReading: %d\n",values->numReading);
        if(values->numReading == 1)
        {
            sem_wait(&semaphores->wrt);
            sem_post(&semaphores->empty);
            printf("R <%d> set wrt\n", getpid());
        }
        sem_post(&semaphores->mutex);
        printf("R <%d> release mutex\n", getpid());

        // Read
        printf("%d and %d\n", readCount, values->writeNext);
        while(readCount < values->writeNext)
        {
            printf( "R <%d>: I read [%d] from data buffer[%d]!\n",
                getpid(), (*dataBuffer)[readCount%BUFFER_SIZE, readCount%BUFFER_SIZE]);
            ((*tracker)[readCount%BUFFER_SIZE])--;
            readCount++;
        }

        // Unlock
        sem_wait(&semaphores->mutex);
        printf("R <%d> set mutex\n", getpid());
        (values->numReading)--;
        if((values->numReading) == 0)
        {
            sem_post(&semaphores->wrt);
            printf("R <%d> release wrt\n", getpid());
        }
        sem_post(&semaphores->mutex);
        printf("R <%d> release mutex\n", getpid());

        sleep(values->sleepRead);
    }
    printf("R <%d>: Signing off!\n", getpid());

    // Write to file
    FILE* f = fopen("sim_out", "a");

    fprintf(f, "Reader %d has finished reading %d pieces of data from the data buffer\n", getpid(), readCount);
    fclose(f);

    // Decrements numReaders when they exit. Prevents deadlock on tracker
    (values->numReaders)--;

    exit(1);
}

// Writer
void writer (Semaphores* semaphores, Values* values,
int** dataBuffer, int** tracker, int** sharedData)
{
    int ii, writeCount = 0;

    printf("W <%d>: I live!\n", getpid());

    while (values->writeNext < SHARED_DATA_SIZE)
    {
        // Locks if nothing to write
        if((*tracker)[values->writeNext%BUFFER_SIZE] != 0)
            sem_wait(&semaphores->empty);

        // Locks when writing, unlocks any empty readers
        printf("W <%d> set wrt\n", getpid());
        sem_wait(&semaphores->wrt);
        sem_post(&semaphores->full);

        // Write
        if((*tracker)[values->writeNext%BUFFER_SIZE] == 0 &&
            values->writeNext < SHARED_DATA_SIZE)
        {
            (*dataBuffer)[values->writeNext%BUFFER_SIZE] =
                (*sharedData)[values->writeNext];
            (*tracker)[values->writeNext%BUFFER_SIZE] = values->numReaders;
            printf("W <%d>: I wrote [%d] to data buffer[%d]!\n", getpid(), (*sharedData)[values->writeNext], values->writeNext%BUFFER_SIZE);
            printf("W <%d>: Tracker: {", getpid());
            for(ii = 0; ii < BUFFER_SIZE; ii++)
            {
                printf("%d ", (*tracker)[ii]);
            }
            printf("}\n");
            printf("W <%d>: dataBuffer: {", getpid());
            for(ii = 0; ii < BUFFER_SIZE; ii++)
            {
                printf("%d ", (*dataBuffer)[ii]);
            }
            printf("}\n");
            (values->writeNext)++;
            writeCount++;
        }

        // Unlock
        sem_post(&semaphores->wrt);
        printf("W <%d> release wrt\n", getpid());

        sleep(values->sleepWrite);
    }

    printf("W <%d>: Signing off!\n", getpid());

    // Write to file
    FILE* f = fopen("sim_out", "a");

    fprintf(f, "Writer %d has finished writing %d pieces of data to the data buffer\n", getpid(), writeCount);
    fclose(f);

    // Ensures that no readers are left when writers have finished.
    sem_post(&semaphores->empty);

    exit(1);
}
