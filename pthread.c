#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ARRAY_SIZE 100
#define NUM_BUCKETS 10
#define NUM_THREADS 4

// Structure for passing parameters to thread function
typedef struct {
    int *array;
    int start;
    int end;
    int **buckets;
} ThreadData;

// Function to compare integers for qsort
int compareIntegers(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Function to insert elements into buckets
void *insertIntoBuckets(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int *array = data->array;
    int start = data->start;
    int end = data->end;
    int **buckets = data->buckets;

    for (int i = start; i < end; i++) {
        int bucket_index = array[i] / (ARRAY_SIZE / NUM_BUCKETS);
        int index = ++buckets[bucket_index][0];
        buckets[bucket_index][index] = array[i];
    }

    pthread_exit(NULL);
}

// Function to sort a single bucket
void sortBucket(int *bucket, int size) {
    qsort(bucket + 1, size, sizeof(int), compareIntegers);
}

// Function to merge buckets into a single array
void mergeBuckets(int *array, int **buckets) {
    int index = 0;
    for (int i = 0; i < NUM_BUCKETS; i++) {
        for (int j = 1; j <= buckets[i][0]; j++) {
            array[index++] = buckets[i][j];
        }
    }
}

int main() {
    int array[ARRAY_SIZE];
    int *buckets[NUM_BUCKETS];
    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    // Initialize array with random numbers
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % 100;
    }

    // Initialize buckets
    for (int i = 0; i < NUM_BUCKETS; i++) {
        buckets[i] = (int *)malloc((ARRAY_SIZE / NUM_BUCKETS + 1) * sizeof(int));
        buckets[i][0] = 0; // First element of each bucket stores the count of elements
    }

    // Scatter phase: Insert elements into buckets
    for (int i = 0; i < NUM_THREADS; i++) {
        threadData[i].array = array;
        threadData[i].start = i * (ARRAY_SIZE / NUM_THREADS);
        threadData[i].end = (i + 1) * (ARRAY_SIZE / NUM_THREADS);
        threadData[i].buckets = buckets;
        pthread_create(&threads[i], NULL, insertIntoBuckets, &threadData[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Sort each bucket using multiple threads
    for (int i = 0; i < NUM_BUCKETS; i++) {
        pthread_t sortThread;
        pthread_create(&sortThread, NULL, sortBucket, buckets[i]);
        pthread_join(sortThread, NULL);
    }

    // Merge sorted buckets back into the array
    mergeBuckets(array, buckets);

    // Print sorted array
    printf("Sorted Array: ");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    // Free memory
    for (int i = 0; i < NUM_BUCKETS; i++) {
        free(buckets[i]);
    }

    return 0;
}
