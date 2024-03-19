#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 100
#define NUM_BUCKETS 10

// Function to compare integers for qsort
int compareIntegers(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int main() {
    int array[ARRAY_SIZE];
    int *buckets[NUM_BUCKETS];
    int i, j;

    // Initialize array with random numbers
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % 100;
    }

    // Initialize buckets
    #pragma omp parallel for
    for (i = 0; i < NUM_BUCKETS; i++) {
        buckets[i] = (int *)malloc((ARRAY_SIZE / NUM_BUCKETS + 1) * sizeof(int));
        buckets[i][0] = 0; // First element of each bucket stores the count of elements
    }

    // Scatter phase: Insert elements into buckets
    #pragma omp parallel for private(i, j)
    for (i = 0; i < ARRAY_SIZE; i++) {
        int bucket_index = array[i] / (ARRAY_SIZE / NUM_BUCKETS);
        int index = ++buckets[bucket_index][0];
        buckets[bucket_index][index] = array[i];
    }

    // Sort buckets individually
    #pragma omp parallel for private(i)
    for (i = 0; i < NUM_BUCKETS; i++) {
        qsort(buckets[i] + 1, buckets[i][0], sizeof(int), compareIntegers);
    }

    // Gather sorted elements from buckets
    int index = 0;
    #pragma omp parallel for private(i, j) shared(index)
    for (i = 0; i < NUM_BUCKETS; i++) {
        for (j = 1; j <= buckets[i][0]; j++) {
            array[index++] = buckets[i][j];
        }
    }

    // Print sorted array
    printf("Sorted Array: ");
    for (i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    // Free memory
    #pragma omp parallel for
    for (i = 0; i < NUM_BUCKETS; i++) {
        free(buckets[i]);
    }

    return 0;
}
