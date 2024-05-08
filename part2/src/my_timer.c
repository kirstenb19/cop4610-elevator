#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>

#define PROC_ENTRY_NAME "timer"

static struct proc_dir_entry *proc_entry;
static ktime_t last_time;

//referenced from stack overflow: "how to print time difference in accuracy of milliseconds and nanoseconds"
//and example from web.mit.edu "netsed.c"
static int timer_read(char *page, char **start, off_t off, int count, int *eof, void *data) {
    ktime_t current_time = ktime_get_real_ts64();
    int len = 0;

    len += snprintf(page + len, count - len, "current time: %lld.%09ld\n", current_time.tv64 / NSEC_PER_SEC, current_time.tv64 % NSEC_PER_SEC);

    if (last_time.tv64 != 0) {
        ktime_t elapsed = ktime_sub(current_time, last_time);
        len += snprintf(page + len, count - len, "elapsed time: %lld.%09ld\n", elapsed.tv64 / NSEC_PER_SEC, elapsed.tv64 % NSEC_PER_SEC);
    }

    last_time = current_time;

    return len;
}

static const struct file_operations proc_fops = {
    .read = timer_read,
};

//rdrr.io: proc running time of R 
//emblogic.com: about proc, proc_create, proc_entry 
//stackoverdlow: remove_proc_entry 

static int __init timer_init(void) {
    proc_entry = proc_create(PROC_ENTRY_NAME, 0444, NULL, &proc_fops);
    if (!proc_entry) {
        printk(KERN_ERR "Failed to create /proc/%s\n", PROC_ENTRY_NAME);
        return -ENOMEM;
    }
    return 0;
}

static void __exit timer_exit(void) {
    remove_proc_entry(PROC_ENTRY_NAME, NULL);
}

module_init(timer_init);
module_exit(timer_exit);
