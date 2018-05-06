/*
 * **************************************************
 * Author: Matthew Cheong
 * File: sds.c
 * Created Date: Wednesday, May 2nd 2018, 8:46:27 pm
 * -----
 * Last Modified: Mon May 07 2018
 * Modified By: Matthew Cheong
 * -----
 * **************************************************
 */

#include "sds.h"

//Global Variables
pthread_mutex_t mutex;
pthread_cond_t empty;
pthread_cond_t full;
Values values;
int *dataBuffer, *tracker, *sharedData;

int main(int argc, char* argv[])
{
    validateArgs(argc, argv);

    // Variables
    int ii;

    // Initialize Data
    for (ii = 0; ii <= BUFFER_SIZE; ii++)
    {
        tracker[ii] = 0;
        dataBuffer[ii] = 0;
    }
    values.writeNext = 0;

    // Read in arguments
    char* fileName = SHARED_DATA;
    values.numReaders = atoi(argv[1]);
    values.numWriters = atoi(argv[2]);
    values.sleepRead = atoi(argv[3]);
    values.sleepWrite = atoi(argv[4]);

    // Create threads
    pthread_t* writers = (pthread_t*)malloc(sizeof(pthread_t) *
    values.numWriters);
    pthread_t* readers = (pthread_t*)malloc(sizeof(pthread_t) *
    values.numReaders);

    initMemory(&dataBuffer, &tracker, &sharedData, &values);

    // Read in file
    FILE* f = fopen(fileName, "r");
    if(f == NULL)
    {
        fprintf(stderr, "Error opening file!\n");
        cleanMemory();
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
        cleanMemory();
        exit(1);
    }
    fclose(f);

    // Create Writers
    for( ii = 1; ii <= values.numWriters; ii++)
    {
        pthread_create(&writers[ii], NULL, writer(), NULL);
        pthread_detach(writers[ii]);
    }
    // Create Readers
    for( ii = 1; ii <= values.numReaders; ii++)
    {
        pthread_create(&readers[ii], NULL, reader(), NULL);
        pthread_detach(readers[ii]);
    }

    // Ensures children have time to exit
    for(ii = 0; ii < (values.numReaders+values.numWriters); ii++)
    {
        wait(NULL);
    }

    cleanMemory();

    return 0;
}// End main

// **************************************************

// Validates the command-line arguments
void validateArgs(int argc, char* argv[])
{
    // Ensure correct number of command line parameters
    if(argc != 5)
    {
        fprintf(stderr, "Incorrect number of parameters.\n4 expected\n");
        exit(1);
    }
    // Ensure at least one reader and writer
    if(atoi(argv[1]) < 1 || atoi(argv[2]) < 1)
    {
        fprintf(stderr, "Must have at least 1 reader and 1 writer\n");
        exit(1);
    }
    // Ensure sleep time is positive
    if(atoi(argv[3]) < 0 || atoi(argv[4]) < 0)
    {
        fprintf(stderr, "Wait times must be positive\n");
        exit(1);
    }
}

// Initializes memory
void initMemory(int** dataBuffer, int** tracker, int** sharedData,
Values* values)
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&empty, NULL);
    pthread_cond_init(&full, NULL);
    *dataBuffer = (int*)malloc(sizeof(int)*BUFFER_SIZE);
    *tracker = (int*)malloc(sizeof(int)*BUFFER_SIZE);
    *sharedData = (int*)malloc(sizeof(int)*SHARED_DATA_SIZE);
    values = (Values*)malloc(sizeof(Values));
}

void cleanMemory(void)
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&empty);
    pthread_cond_destroy(&full);
    free(dataBuffer);
    free(tracker);
    free(sharedData);
    free(&values);
}

// Writer
void* writer(void)
{
    int ii, writeCount = 0;

    pid_t tid = gettid();

    printf("W <%d>: I live!\n", tid);

    while(values.writeNext < SHARED_DATA_SIZE)
    {
        // Locks mutex
        pthread_mutex_lock(&mutex);
        if(tracker[values.writeNext] == 0 &&
        values.writeNext < SHARED_DATA_SIZE)
        {
            // Waits for signal before continuing
            pthread_cond_wait(&empty, &mutex);
            dataBuffer[values.writeNext % BUFFER_SIZE] =
            sharedData[values.writeNext];
            tracker[values.writeNext] = values.numReaders;
            printf("W <%d>: I wrote [%d] to data buffer[%d]!\n", tid,
            sharedData[values.writeNext], values.writeNext % BUFFER_SIZE);
            printf("W <%d>: Tracker: {", tid);
            for(ii = 0; ii < BUFFER_SIZE; ii++)
            {
                printf("%d ", tracker[ii]);
            }
            printf("}\n");
            printf("W <%d>: dataBuffer: {", tid);
            for(ii = 0; ii < BUFFER_SIZE; ii++)
            {
                printf("%d ", dataBuffer[ii]);
            }
            printf("}\n");
            values.writeNext++;
            writeCount++;
            
        }
        // Sends full signal and unlocks mutex
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
        sleep(values.sleepWrite);
    }
    printf("W <%d>: Signing off!\n", getpid());
    
    FILE* f = fopen("sim_out", "a");

    fprintf(f, "Writer %d has finished writing %d pieces of data to the data buffer\n", tid, writeCount);

    fclose(f);

    pthread_exit(0);
}

// Reader
void* reader(void)
{
    int ii, readCount = 0;

    pid_t tid = gettid();

    while(readCount < SHARED_DATA_SIZE)
    {
        pthread_mutex_lock(&mutex);
        while(readCount < values.writeNext)
        {
            pthread_cond_wait(&full, &mutex);
            printf("R <%d>: I read [%d] from data buffer[%d]!\n",
            tid, dataBuffer[readCount % BUFFER_SIZE], readCount % BUFFER_SIZE);
            tracker[readCount % BUFFER_SIZE]--;
            readCount++;
            pthread_cond_signal(&empty);
        }
        pthread_mutex_unlock(&mutex);
        
        sleep(values.sleepRead);
    }

    FILE* f = fopen("sim_out", "a");

    fprintf(f, "Reader %d has finished reading %d pieces of data from the data buffer\n", tid, readCount);

    fclose(f);

    // Decrements numReaders when they exit. Prevents deadlock on tracker
    values.numReaders--;

    pthread_exit(0);
}