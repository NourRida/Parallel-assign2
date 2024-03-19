#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define max_threads 80

struct bucket {
    int n_elem;
    int idx; 
    int start; 
};

int cmpfunc(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int main(int argc, char *argv[]) {
    int *arrA, *arrB, *temp;
    int size, num_buckets, i, w, limit, num_threads, workload, b_index;
    struct bucket *buckets;
    double start_time; 
    float total_time; 

    printf("Give length of array to sort \n");
    if (scanf("%d", &size) != 1) {
        printf("error\n");
        return -1;
    }
    printf("Give number of buckets \n");
    if (scanf("%d", &num_buckets) != 1) {
        printf("error\n");
        return -1;
    }

    int global_elem_count[num_buckets];
    int global_start_position[num_buckets]; 
    memset(global_elem_count, 0, sizeof(int) * num_buckets);
    memset(global_start_position, 0, sizeof(int) * num_buckets);

    num_threads = num_buckets;
    omp_set_num_threads(num_threads);

    limit = 100000;
    w = limit / num_buckets;
    arrA = (int *)malloc(sizeof(int) * size);
    arrB = (int *)malloc(sizeof(int) * size);

    for (i = 0; i < size; i++) {
        arrA[i] = random() % limit;
    }

    if (size <= 40) {
        printf("Unsorted data \n");
        for (i = 0; i < size; i++) {
            printf("%d ", arrA[i]);
        }
        printf("\n");
    }

    buckets = (struct bucket *)calloc(num_buckets * num_threads, sizeof(struct bucket));

    start_time = omp_get_wtime();
#pragma omp parallel
    {
        num_threads = omp_get_num_threads();

        int j, k;
        int local_idx; 
        int real_bucket_idx; 
        int my_id = omp_get_thread_num();
        workload = size / num_threads;
        int prev_idx;

#pragma omp for private(i, local_idx)
        for (i = 0; i < size; i++) {
            local_idx = arrA[i] / w;
            if (local_idx > num_buckets - 1)
                local_idx = num_buckets - 1;
            real_bucket_idx = local_idx + my_id * num_buckets;
            buckets[real_bucket_idx].n_elem++;
        }

        int local_sum = 0;
        for (j = my_id; j < num_buckets * num_threads; j = j + num_threads) {
            local_sum += buckets[j].n_elem;
        }
        global_elem_count[my_id] = local_sum;

#pragma omp barrier

#pragma omp master
        {
            for (j = 1; j < num_buckets; j++) {
                global_start_position[j] = global_start_position[j - 1] + global_elem_count[j - 1];
                buckets[j].start = buckets[j - 1].start + global_elem_count[j - 1];
                buckets[j].idx = buckets[j - 1].idx + global_elem_count[j - 1];
            }
        }

#pragma omp barrier
        for (j = my_id + num_buckets; j < num_buckets * num_threads; j = j + num_threads) {
            prev_idx = j - num_buckets;
            buckets[j].start = buckets[prev_idx].start + buckets[prev_idx].n_elem;
            buckets[j].idx = buckets[prev_idx].idx + buckets[prev_idx].n_elem;
        }
#pragma omp barrier

#pragma omp for private(i, b_index)
        for (i = 0; i < size; i++) {
            j = arrA[i] / w;
            if (j > num_buckets - 1)
                j = num_buckets - 1;
            k = j + my_id * num_buckets;
            b_index = buckets[k].idx++;
            arrB[b_index] = arrA[i];
        }

#pragma omp for private(i)
        for (i = 0; i < num_buckets; i++)
            qsort(arrB + global_start_position[i], global_elem_count[i], sizeof(int), cmpfunc);
    }
    total_time = omp_get_wtime() - start_time;
    temp = arrA;
    arrA = arrB;
    arrB = temp;

    if (size <= 40) {
        printf("A \n");
        for (i = 0; i < size; i++) {
            printf("%d ", arrA[i]);
        }
        printf("\n");
    }
    printf("Sorting %d elements took %f seconds\n", size, total_time);

    int sorted = 1;
    for (i = 0; i < size - 1; i++) {
        if (arrA[i] > arrA[i + 1])
            sorted = 0;
    }
    if (!sorted)
        printf("not sorted");

    return 0;
}
