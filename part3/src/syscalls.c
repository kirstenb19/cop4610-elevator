#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

// Declaration of the system call functions
asmlinkage int start_elevator(void);
asmlinkage int issue_request(int start_floor, int destination_floor, int type);
asmlinkage int stop_elevator(void);

// Function pointers for the syscall wrappers to point to
extern int (*STUB_start_elevator)(void) = NULL;
extern int (*STUB_issue_request)(int, int, int) = NULL;
extern int (*STUB_stop_elevator)(void) = NULL;

// Export the pointers so that the system call module can access them
EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_stop_elevator);

// Syscall wrapper for start_elevator
SYSCALL_DEFINE0(start_elevator) {
    printk(KERN_NOTICE "Inside %s system call\n", __FUNCTION__);
    if (STUB_start_elevator)
        return STUB_start_elevator();
    else
        return -ENOSYS; // No such system call if the module is not loaded
}

// Syscall wrapper for issue_request
SYSCALL_DEFINE3(issue_request, int, start_floor, int, destination_floor, int, type) {
    printk(KERN_NOTICE "Inside %s system call\n", __FUNCTION__);
    if (STUB_issue_request)
        return STUB_issue_request(start_floor, destination_floor, type);
    else
        return -EINVAL; // Invalid argument if parameters are out of range
}

// Syscall wrapper for stop_elevator
SYSCALL_DEFINE0(stop_elevator) {
    printk(KERN_NOTICE "Inside %s system call\n", __FUNCTION__);
    if (STUB_stop_elevator)
        return STUB_stop_elevator();
    else
        return -ENOSYS; // No such system call if the module is not loaded
}
