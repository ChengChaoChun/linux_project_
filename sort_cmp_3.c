#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define KSORT_DEV "/dev/sort"

#define MAX_BYTES_PER_READ 8
static unsigned char rx[MAX_BYTES_PER_READ]; /* Receive buffer from the LKM */

void zero_rx(void)
{
    for (int b_idx = 0; b_idx < MAX_BYTES_PER_READ; b_idx++) {
        rx[b_idx] = 0;
    }
}


int main()
{
    int fd_sort = open(KSORT_DEV, O_RDWR);
    if (fd_sort < 0) {
        perror("Failed to open character device");
        goto error;
    }

    int fd_xoro = open("/dev/xoro", O_RDONLY);
    if (0 > fd_xoro) {
        perror("Failed to open the device.");
        goto error;
    }

    FILE *f_sort_time = fopen("sort_time_3.dat", "w");
    if (f_sort_time == NULL) {
        printf("unable to open file\n");
        return 0;
    }

    FILE *f_ktime = fopen("ktime_3.dat", "w");
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

    for (size_t i = 1; i <= 10000; i++) {
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

        for (size_t i = 0; i < n_elements; i++) {
            /* Clear/zero the buffer before copying in read data. */
            zero_rx();

            /* Read the response from the LKM. */
            int n_bytes = rand() % 2 + 1;
            ssize_t n_bytes_read = read(fd_xoro, rx, n_bytes);

            if (0 > n_bytes_read) {
                perror("Failed to read all bytes.");
                goto error;
            }

            uint64_t value_ = 0;
            for (int b_idx = 0; b_idx < n_bytes_read; b_idx++) {
                unsigned char b = rx[b_idx];
                value_ |= ((uint64_t) b << (8 * b_idx));
            }
            inbuf_timsort[i] = value_;
            inbuf_linuxsort[i] = value_;
            inbuf_pdqsort[i] = value_;
            //printf("n_bytes=%d n_bytes_read=%ld value=%016lx\n", n_bytes,
            //        n_bytes_read, value_);
        }


        			
        //timsort measure	
        *set_sort = 1;
        set_res = write(fd_sort, set_sort, sizeof(int));
        if(set_res <= 0){
            printf("set failed\n");
            free(set_sort);
            goto error;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        int r_time = read(fd_sort, inbuf_timsort, size);
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

        fprintf(f_sort_time, "%ld   %d   ", i, time_spend_timsort);
	fprintf(f_ktime, "%ld   %d   ", i, r_time);
        free(inbuf_timsort);

        printf("timsort %s  %ld!\n", pass ? "succeeded" : "failed", i);
    

        // linuxsort measure
        *set_sort = 2;
        set_res = write(fd_sort, set_sort, sizeof(int));
        if (set_res <= 0) {
            printf("set failed\n");
            free(set_sort);
            goto error;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        r_time = read(fd_sort, inbuf_linuxsort, size);
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
        set_res = write(fd_sort, set_sort, sizeof(int));
        if (set_res <= 0) {
            printf("set failed\n");
            free(set_sort);
            goto error;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        r_time = read(fd_sort, inbuf_pdqsort, size);
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
    if (fd_sort > 0)
        close(fd_sort);
    if (fd_xoro > 0)
        close(fd_xoro);

    return 0;
}
