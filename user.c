/*
 * parameter   1 : timsort , 2 : linuxsort , 3 : pdqsort , 4 : qosrt  
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define KSORT_DEV "/dev/sort"

int main(int argc, char *argv[])
{
    int fd = open(KSORT_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        goto error;
    }

    size_t n_elements = 1000;
    size_t size = n_elements * sizeof(uint64_t);
    uint64_t *inbuf = malloc(size);
    if (!inbuf)
        goto error;

    for (size_t i = 0; i < n_elements; i++)
        inbuf[i] = rand() % n_elements;


    int *set_sort = malloc(sizeof(int));
    *set_sort = atoi(argv[1]);
    if (*set_sort == 4) {
        printf("sort by qsort\n");
    }else if (*set_sort == 3) {
        printf("sort by pdqsort\n");
    } else if (*set_sort == 2) {
        printf("sort by linux lib sort\n");
    } else {
        printf("sort by timsort\n");
    }
    ssize_t set_res = write(fd, set_sort, sizeof(int));
    if (set_res > 0) {
        printf("set seccuess  %zd\n", set_res);
    } else {
        printf("set failed\n");
    }
    free(set_sort);


    ssize_t r_time = read(fd, inbuf, size);
    if (r_time <= 0) {
        perror("Failed to read character device");
        goto error;
    }
    printf("sorting time : %ld\n", r_time);


    bool pass = true;
    int ret = 0;
    /* Verify the result of sorting */
    for (size_t i = 1; i < n_elements; i++) {
        if (inbuf[i] < inbuf[i - 1]) {
            pass = false;
            break;
        }
    }

    printf("Sorting %s!\n", pass ? "succeeded" : "failed");

error:
    free(inbuf);
    if (fd > 0)
        close(fd);
    return ret;
}
