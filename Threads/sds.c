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
pthread_cond_t use;
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
    values.numReading = 0;
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
    for( ii = 1; ii <= values.numReaders; ii++)
    {
        pthread_create(&readers[ii], NULL, reader(), NULL);
    }
    // Create Writers
    for( ii = 1; ii <= values.numWriters; ii++)
    {
        pthread_create(&writers[ii], NULL, writer(), NULL);
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

void initMemory(int** dataBuffer, int** tracker, int** sharedData,
Values* values)
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&use, NULL);
    *dataBuffer = (int*)malloc(sizeof(int)*BUFFER_SIZE);
    *tracker = (int*)malloc(sizeof(int)*BUFFER_SIZE);
    *sharedData = (int*)malloc(sizeof(int)*SHARED_DATA_SIZE);
    values = (Values*)malloc(sizeof(Values));
}

void cleanMemory(void)
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&use);
    free(dataBuffer);
    free(tracker);
    free(sharedData);
    free(&values);
}