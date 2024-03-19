#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define max_threads 80

struct bucket {
    int n_elem;
    int idx; 
    int start; 
};

struct thread_data {
    int thread_id;
    int size;
    int num_buckets;
    int limit;
    int *arrA;
    int *arrB;
    struct bucket *buckets;
};

int cmpfunc(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

void *bucket_sort(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int thread_id = data->thread_id;
    int size = data->size;
    int num_buckets = data->num_buckets;
    int limit = data->limit;
    int *arrA = data->arrA;
    int *arrB = data->arrB;
    struct bucket *buckets = data->buckets;

    int i, j, k, local_idx, real_bucket_idx, workload, prev_idx;
    int w = limit / num_buckets;

    workload = size / num_buckets;
    int my_start = thread_id * workload;
    int my_end = my_start + workload;

    for (i = my_start; i < my_end; i++) {
        local_idx = arrA[i] / w;
        if (local_idx > num_buckets - 1)
            local_idx = num_buckets - 1;
        real_bucket_idx = local_idx + thread_id * num_buckets;
        buckets[real_bucket_idx].n_elem++;
    }

    pthread_barrier_wait(&barrier);

    if (thread_id == 0) {
        int global_n_elem[num_buckets]; 
        int global_start_position[num_buckets]; 
        memset(global_n_elem, 0, sizeof(int) * num_buckets);
        memset(global_start_position, 0, sizeof(int) * num_buckets);

        for (i = 0; i < num_buckets * max_threads; i++) {
            int bucket_id = i % num_buckets;
            global_n_elem[bucket_id] += buckets[i].n_elem;
        }

        global_start_position[0] = 0;
        for (i = 1; i < num_buckets; i++) {
            global_start_position[i] = global_start_position[i - 1] + global_n_elem[i - 1];
        }

        for (i = 0; i < num_buckets * max_threads; i++) {
            int bucket_id = i % num_buckets;
            buckets[i].start = global_start_position[bucket_id] + buckets[i].n_elem;
            buckets[i].idx = buckets[i].start;
        }
    }

    pthread_barrier_wait(&barrier);

    for (i = my_start; i < my_end; i++) {
        j = arrA[i] / w;
        if (j > num_buckets - 1)
            j = num_buckets - 1;
        k = j + thread_id * num_buckets;
        int b_index = buckets[k].idx++;
        arrB[b_index] = arrA[i];
    }

    pthread_barrier_wait(&barrier);

    for (i = 0; i < num_buckets; i++) {
        qsort(arrB + buckets[thread_id * num_buckets + i].start, buckets[thread_id * num_buckets + i].n_elem, sizeof(int), cmpfunc);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int size, num_buckets, i;
    double start_time, total_time;
    pthread_t threads[max_threads];
    struct thread_data td[max_threads];
    struct bucket *buckets;

    printf("Give length of array to sort\n");
    if (scanf("%d", &size) != 1) {
        printf("Error\n");
        return -1;
    }
    printf("Give number of buckets\n");
    if (scanf("%d", &num_buckets) != 1) {
        printf("Error\n");
        return -1;
    }

    int limit = 100000;
    int *arrA = (int *)malloc(sizeof(int) * size);
    int *arrB = (int *)malloc(sizeof(int) * size);

    for (i = 0; i < size; i++) {
        arrA[i] = random() % limit;
    }

    if (size <= 40) {
        printf("Unsorted data\n");
        for (i = 0; i < size; i++) {
            printf("%d ", arrA[i]);
        }
        printf("\n");
    }

    buckets = (struct bucket *)calloc(num_buckets * max_threads, sizeof(struct bucket));

    pthread_barrier_init(&barrier, NULL, max_threads);

    start_time = omp_get_wtime();

    for (i = 0; i < max_threads; i++) {
        td[i].thread_id = i;
        td[i].size = size;
        td[i].num_buckets = num_buckets;
        td[i].limit = limit;
        td[i].arrA = arrA;
        td[i].arrB = arrB;
        td[i].buckets = buckets;
        pthread_create(&threads[i], NULL, bucket_sort, (void *)&td[i]);
    }

    for (i = 0; i < max_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    total_time = omp_get_wtime() - start_time;

    printf("Sorted Array:\n");
    for (i = 0; i < size; i++) {
        printf("%d ", arrB[i]);
    }
    printf("\n");

    printf("Sorting %d elements took %f seconds\n", size, total_time);

    free(arrA);
    free(arrB);
    free(buckets);
    pthread_barrier_destroy(&barrier);

    return 0;
}
