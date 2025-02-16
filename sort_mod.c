#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/ktime.h>

#include "sort.h"

#include <linux/sort.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Concurrent sorting driver");
MODULE_VERSION("0.1");

#define DEVICE_NAME "sort"

static dev_t dev = -1;
static struct cdev cdev;
static struct class *class;
static int sort_selector = 0;

struct workqueue_struct *workqueue;

static int sort_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int sort_release(struct inode *inode, struct file *file)
{
    return 0;
}

static int num_compare(const void *a, const void *b)
{
    return (*(uint64_t *) a - *(uint64_t *) b);
}

static int cmpuint64(const void *a, const void *b)
{
    return *(uint64_t *) a < *(uint64_t *) b;
}

int tim_compare(void *priv, const struct list_head *a, const struct list_head *b)
{
    if (a == b)
        return 0;

    int res = list_entry(a, element_t, list)->val -
              list_entry(b, element_t, list)->val;

    if (priv)
        *((int *) priv) += 1;

    return res;
}

void timsort_init(struct timsort_struct *t,
                  void *priv_,
                  struct list_head *h_,
                  list_cmp_func_t cmp_)
{
    INIT_WORK(&t->w, timsort);
    t->priv = priv_;
    t->h = h_;
    t->cmp = cmp_;
    t->p = current->pid;
}


static ssize_t sort_read(struct file *file,
                         char *buf,
                         size_t size,
                         loff_t *offset)
{
    char _comm[TASK_COMM_LEN];
    get_task_comm(_comm, current);
    printk("%s (pid=%d, comm=%s, state=%d)\n", __func__, current->pid, _comm, current->__state);

    unsigned long len;
    size_t es;

    uint64_t *sort_buffer = kmalloc(size, GFP_KERNEL);
    if (!sort_buffer)
        return 0;

    /* FIXME: Requiring users to manually input data into a buffer for read
     * operations is not ideal, even if it is only for testing purposes.
     */
    len = copy_from_user(sort_buffer, buf, size);
    if (len != 0)
        return 0;

    /* TODO: While currently designed to handle integer arrays, there is an
     * intention to expand the sorting object's capability to accommodate
     * various types in the future.
     */
    es = sizeof(uint64_t);
    ktime_t start_time , time_cost;

    if(sort_selector == 4){
         start_time = ktime_get();
         sort_main(sort_buffer, size / es, es, num_compare);
         time_cost = ktime_sub(ktime_get(), start_time);
         printk("qsort time_cost  %lld ns\n", ktime_to_ns(time_cost));
         printk("sorted by qsort\n");
    }else if(sort_selector == 3){
        start_time = ktime_get();

        sort_pdqsort(sort_buffer, size / es, es, cmpuint64, 0);

        time_cost = ktime_sub(ktime_get(), start_time);


        printk("pdqsort time_cost  %lld ns\n", ktime_to_ns(time_cost));
        printk("sorted by pdqsort\n");
    }else if(sort_selector == 2){
        start_time = ktime_get();

        sort(sort_buffer, size / es, es , num_compare, NULL);

        time_cost = ktime_sub(ktime_get(), start_time);


        printk("linux sort time_cost  %lld ns\n", ktime_to_ns(time_cost));
        printk("sorted by linux sort\n");
    }else if(sort_selector == 1){//timsort
	struct list_head head;
        INIT_LIST_HEAD(&head);
	int ele_num = size / es;
        for (int i = 0; i < ele_num; i++) {
            element_t *elem = kmalloc(sizeof(element_t), GFP_KERNEL);
            elem->val = *(sort_buffer + i);
            list_add_tail(&elem->list, &head);
        }

	int count = 0;
        struct timsort_struct tim_;
        timsort_init(&tim_, &count, &head, tim_compare);	


	start_time = ktime_get();
    	//timsort(&count, &head, tim_compare);
	queue_work(workqueue, &tim_.w);
	drain_workqueue(workqueue);
	time_cost = ktime_sub(ktime_get(), start_time);
	
	struct list_head *pos;
        uint64_t *buf_p = sort_buffer;
        element_t *entry;
        list_for_each (pos, &head) {
            entry = list_entry(pos, element_t, list);
            *(buf_p) = entry->val;
            buf_p++;
        }

        /* Free list */
        element_t *node, *safe;
        list_for_each_entry_safe (node, safe, &head, list) {
            list_del(&node->list);
            kfree(node);
        }

	printk("timsort time_cost  %lld ns\n", ktime_to_ns(time_cost));
        printk("sorted by timsort\n");
    }else {
        printk("please select sort type first\n");
    }


    len = copy_to_user(buf, sort_buffer, size);
    if (len != 0)
        return 0;

    kfree(sort_buffer);
    return time_cost;
}

static ssize_t sort_write(struct file *file,
                          const char *buf,
                          size_t size,
                          loff_t *offset)
{
    void *set_sort_buf = kmalloc(size, GFP_KERNEL);
    if (!set_sort_buf)
        return 0;

    unsigned long len;
    len = copy_from_user(set_sort_buf, buf, size);
    if (len != 0)
        return 0;

    int set_sort = *(int *)set_sort_buf;
    if(set_sort > 0 && set_sort < 5){
        sort_selector = set_sort;
        printk("sort_selector : %d", set_sort);
        return set_sort;
    }else{
        printk("sort_selector error");
        return 0;
    }
}

static const struct file_operations fops = {
    .read = sort_read,
    .write = sort_write,
    .open = sort_open,
    .release = sort_release,
    .owner = THIS_MODULE,
};

static int __init sort_init(void)
{
    struct device *device;

    printk(KERN_INFO DEVICE_NAME ": loaded\n");

    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0)
        return -1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
    class = class_create(THIS_MODULE, DEVICE_NAME);
#else
    class = class_create(DEVICE_NAME);
#endif
    if (IS_ERR(class)) {
        goto error_unregister_chrdev_region;
    }

    device = device_create(class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(device)) {
        goto error_class_destroy;
    }

    cdev_init(&cdev, &fops);
    if (cdev_add(&cdev, dev, 1) < 0)
        goto error_device_destroy;

    workqueue = alloc_workqueue("sortq", 0, WQ_MAX_ACTIVE);
    if (!workqueue)
        goto error_cdev_del;

    return 0;

error_cdev_del:
    cdev_del(&cdev);
error_device_destroy:
    device_destroy(class, dev);
error_class_destroy:
    class_destroy(class);
error_unregister_chrdev_region:
    unregister_chrdev_region(dev, 1);

    return -1;
}

static void __exit sort_exit(void)
{
    /* Given that drain_workqueue will be executed, there is no need for an
     * explicit flush action.
     */
    destroy_workqueue(workqueue);

    cdev_del(&cdev);
    device_destroy(class, dev);
    class_destroy(class);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO DEVICE_NAME ": unloaded\n");
}

module_init(sort_init);
module_exit(sort_exit);
