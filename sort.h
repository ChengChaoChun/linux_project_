#ifndef KSORT_H
#define KSORT_H

#include <linux/types.h>
#include <linux/workqueue.h>

typedef int cmp_t(const void *, const void *);

extern struct workqueue_struct *workqueue;

void sort_main(void *sort_buffer, size_t size, size_t es, cmp_t cmp);

//pdqsort
typedef void (*swap_func_t)(void *a, void *b, int size);

typedef int (*cmp_r_func_t)(const void *a, const void *b, const void *priv);
typedef int (*cmp_func_t)(const void *a, const void *b);

void sort_pdqsort(void *base, size_t num, size_t size, cmp_func_t cmp_func, swap_func_t swap_func);

//timsort
struct list_head;

typedef struct {
    struct list_head list;
    int val;
    int seq;
} element_t;

typedef int (*list_cmp_func_t)(void *,
                               const struct list_head *,
                               const struct list_head *);

struct timsort_struct {
    struct work_struct w;
    void *priv;
    struct list_head *h;
    list_cmp_func_t cmp;
    pid_t p;    
};

void timsort(struct work_struct *w);

//void timsort(void *priv, struct list_head *head, list_cmp_func_t cmp);


#endif
