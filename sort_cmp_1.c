#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define KSORT_DEV "/dev/sort"

int main()
{
    int fd = open(KSORT_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        goto error;
    }
    
    FILE *f_sort_time = fopen("sort_time_1.dat", "w");
    if (f_sort_time == NULL) {
        printf("unable to open file\n");
        return 0;
    }

    FILE *f_ktime = fopen("ktime_1.dat", "w");
    if (f_ktime == NULL) {
        printf("unable to open file\n");
        return 0;
    }

    struct timespec start, end;

    int *set_sort = malloc(sizeof(int));
    if (!set_sort) {
        printf("set_sort malloc failed");
        goto error;
    }

    size_t set_res, k;

    for (size_t i = 3; i <= 10000; i++) {
        size_t n_elements = i;
        size_t size = n_elements * sizeof(uint64_t);
        uint64_t *inbuf_timsort = malloc(size);
        uint64_t *inbuf_linuxsort = malloc(size);
        uint64_t *inbuf_pdqsort = malloc(size);
        if (!inbuf_timsort)
            goto error;
        if (!inbuf_linuxsort)
            goto error;
        if (!inbuf_pdqsort)
            goto error;

        for (size_t j = 0; j < n_elements; j++) {
            inbuf_timsort[j] = n_elements - j;
            inbuf_linuxsort[j] = n_elements - j;
            inbuf_pdqsort[j] = n_elements - j;
        }

        // timsort measure
        *set_sort = 1;
        set_res = write(fd, set_sort, sizeof(int));
        if (set_res <= 0) {
            printf("set failed\n");
            free(set_sort);
            goto error;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        int r_time = read(fd, inbuf_timsort, size);
        clock_gettime(CLOCK_MONOTONIC, &end);

        int time_spend_timsort = (end.tv_sec - start.tv_sec) * 1000000000LL +
                               (end.tv_nsec - start.tv_nsec);
        printf("timsort time :  %d\n", time_spend_timsort);

       
        bool pass = true;

        for (k = 1; k < n_elements; k++) {
            if (inbuf_timsort[k] < inbuf_timsort[k - 1]) {
                pass = false;
                break;
            }
        }

        fprintf(f_sort_time, "%lu   %d   ", i, time_spend_timsort);
        fprintf(f_ktime, "%ld %d   ", i, r_time);

        free(inbuf_timsort);

        printf("timsort %s  %ld!\n", pass ? "succeeded" : "failed", i);

        // linuxsort measure
        *set_sort = 2;
        set_res = write(fd, set_sort, sizeof(int));
        if (set_res <= 0) {
            printf("set failed\n");
            free(set_sort);
            goto error;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        r_time = read(fd, inbuf_linuxsort, size);
        clock_gettime(CLOCK_MONOTONIC, &end);

        int time_spend_linuxsort = (end.tv_sec - start.tv_sec) * 1000000000LL +
                                   (end.tv_nsec - start.tv_nsec);
        printf("linuxsort time :  %d\n", time_spend_linuxsort);

      
        pass = true;

        for (k = 1; k < n_elements; k++) {
            if (inbuf_linuxsort[k] < inbuf_linuxsort[k - 1]) {
                pass = false;
                break;
            }
        }

        fprintf(f_sort_time, "%d   ", time_spend_linuxsort);
        fprintf(f_ktime, "%d   ", r_time);

        free(inbuf_linuxsort);

        printf("linuxsort %s  %ld!\n", pass ? "succeeded" : "failed", i);

        // pdqsort measure
        *set_sort = 3;
        set_res = write(fd, set_sort, sizeof(int));
        if (set_res <= 0) {
            printf("set failed\n");
            free(set_sort);
            goto error;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        r_time = read(fd, inbuf_pdqsort, size);
        clock_gettime(CLOCK_MONOTONIC, &end);

        int time_spend_pdqsort = (end.tv_sec - start.tv_sec) * 1000000000LL +
                                 (end.tv_nsec - start.tv_nsec);
        printf("pdqsort time :  %d\n", time_spend_pdqsort);

        pass = true;

        for (k = 1; k < n_elements; k++) {
            if (inbuf_pdqsort[k] < inbuf_pdqsort[k - 1]) {
                pass = false;
                break;
            }
        }

        fprintf(f_sort_time, "%d\n", time_spend_pdqsort);
        fprintf(f_ktime, "%d\n", r_time);

        free(inbuf_pdqsort);

        printf("pdqsort %s  %ld!\n", pass ? "succeeded" : "failed", i);
    }


    free(set_sort);

error:
    fclose(f_sort_time);
    fclose(f_ktime);
    if (fd > 0)
        close(fd);
    return 0;
}
