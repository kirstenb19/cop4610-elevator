#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

//from example
MODULE_LICENSE("GPL");
MODULE_AUTHOR("cop4610t");
MODULE_DESCRIPTION("Proc for elevator");

#define ENTRY_NAME "elevator"
#define PERMS 0644
#define PARENT NULL
#define BUF_LEN 10000


#define MAX_PASSENGERS 5
#define MAX_WEIGHT 7
#define FLOOR_MIN 1
#define FLOOR_MAX 5

// Passenger types
#define PART_TIME 0
#define LAWYER 1
#define BOSS 2
#define VISITOR 3

//proc prototypes  
static ssize_t elevator_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos);

static const struct proc_ops elevator_fops = {
    .proc_read = elevator_read,
};

static struct proc_dir_entry *elevator_entry;


//geeksforgeeks.org: C structures 
struct passenger {
    int type;
    int destination;
    struct list_head list;
};

struct floor {
    int floor_num;
    int passengers_waiting;
    struct list_head passenger_list;
    struct mutex lock;
};

static int passenger_weight(int type) {
    int weight = 0;

    switch (type) {
        case PART_TIME:
            weight = 1;
            break;
        case LAWYER:
            weight = 1.5;
            break;
        case BOSS:
            weight = 2;
            break;
        case VISITOR:
            weight = 0.5;
            break;
        default:
            printk(KERN_ERR "Invalid Passenger type");
            weight = 0;
            break;
    }

    return weight;
}

char passenger_id(int type) {
    switch (type) {
        case PART_TIME:
		return 'P';
        case LAWYER:    
		return 'L';
        case BOSS:     
		return 'B';
        case VISITOR:  
		return 'V';
        default:      
		return ' ';
    }
}

static struct floor floors[FLOOR_MAX];
static struct task_struct *elevator_thread;
static int current_floor = FLOOR_MIN;
static int elevator_load = 0;
static bool elevator_active = false;
static struct mutex elevator_lock;
static int pass_serv = 0;



extern int (*STUB_issue_request)(int, int, int);
//add a passenger to a floor's waiting list
//referencing litux.nl: kernel development, specifically for gfp flags and alerts
int add_passenger_to_floor(int floor_num, int type, int destination) {
    struct passenger *new_passenger = kmalloc(sizeof(struct passenger), GFP_KERNEL);
    if (!new_passenger) {
        printk(KERN_ALERT "Failed to allocate memory for passenger\n");
        return -ENOMEM;
    }
    new_passenger->type = type;
    new_passenger->destination = destination;

    mutex_lock(&floors[floor_num - 1].lock);
    list_add_tail(&new_passenger->list, &floors[floor_num - 1].passenger_list);
    floors[floor_num - 1].passengers_waiting++;
    mutex_unlock(&floors[floor_num - 1].lock);

    return 0;
}

//load passengers into the elevator
//linux entries such as list_for_each_entry_safe and kthread 
//are referenced from kernel.org 
//and github user mathemandy / list_examples.c

static void load_passengers(void) {
    struct passenger *temp_passenger, *next;
    list_for_each_entry_safe(temp_passenger, next, &floors[current_floor - 1].passenger_list, list) {
        if (elevator_load + passenger_weight(temp_passenger->type) <= MAX_WEIGHT) {
            list_del(&temp_passenger->list);
            elevator_load += passenger_weight(temp_passenger->type);
            pass_serv++;
            kfree(temp_passenger);
            floors[current_floor - 1].passengers_waiting--;
        }
    }
}


//elevator thread function
static int elevator_func(void *data) {
    while (!kthread_should_stop()) {
        if (elevator_active) {
            if (current_floor < FLOOR_MAX) {
                //moving between floors
                msleep(2000); 
                current_floor++;
            } else {
                //moving between floors
                msleep(2000);
                current_floor--;
            }
            if (floors[current_floor - 1].passengers_waiting > 0) {
                mutex_lock(&elevator_lock);
                load_passengers();
                mutex_unlock(&elevator_lock);
                //loading/unloading passengers
                msleep(1000);
            }
        } else {
            //idle state
            msleep(1000);
        }
    }
    return 0;
}

extern int (*STUB_start_elevator)(void);
//start elevator system call
static int start_elevator(void) {
    if (!elevator_active) {
        elevator_thread = kthread_run(elevator_func, NULL, "elevator_thread");
        if (IS_ERR(elevator_thread)) {
            printk(KERN_ALERT "Failed to start elevator\n");
            return -ENOMEM;
        }
        elevator_active = true;
        printk(KERN_INFO "Elevator started\n");
        return 0;
    }
    printk(KERN_INFO "Elevator already active\n");
    return 1;
}


static ssize_t elevator_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    char buf[BUF_LEN];//from example code 
    int len = 0;
    //determine the elevator state 
    const char *state;
    if (!elevator_active)
        state = "OFFLINE";//set the state to offline if not active bool 
    else if (current_floor == FLOOR_MIN && elevator_load == 0)
        state = "IDLE";
    else if (floors[current_floor - 1].passengers_waiting > 0 || elevator_load > 0)//elevator is stopped on a floor to load and unload passengers
        state = "LOADING";
    else if (current_floor < FLOOR_MAX)//elevator is moving from a lower floor to a higher floor
        state = "UP";
    else if (current_floor > FLOOR_MIN)// elevator is moving from a higher floor to a lower floor
        state = "DOWN";
    else
        state = " ";

    //print elevator info 
    len += snprintf(buf + len, BUF_LEN - len, "Elevator state: %s\n", state);
    len += snprintf(buf + len, BUF_LEN - len, "Current floor: %d\n", current_floor);
    len += snprintf(buf + len, BUF_LEN - len, "Current load: %d lbs\n", elevator_load);
    len += snprintf(buf + len, BUF_LEN - len, "Elevator status: ");
    struct passenger *passenger;
    list_for_each_entry(passenger, &floors[current_floor - 1].passenger_list, list) {
        char passenger_type = passenger_id(passenger->type);//using the passenger id thing 
        len += snprintf(buf + len, BUF_LEN - len, "%c%d ", passenger_type, passenger->destination);
    }
    len += snprintf(buf + len, BUF_LEN - len, "\n");

    //floor 
    for (int i = 0; i < FLOOR_MAX; i++) {
        char floor_indicator;
        if (current_floor == i + 1) {
            floor_indicator = '*';
        } else {
            floor_indicator = ' ';
        }
        len += snprintf(buf + len, BUF_LEN - len, "[%c] Floor %d: ", floor_indicator, i + 1);
        list_for_each_entry(passenger, &floors[i].passenger_list, list) {
            char passenger_type = passenger_id(passenger->type);
            len += snprintf(buf + len, BUF_LEN - len, "%c%d ", passenger_type, passenger->destination);
        }
        len += snprintf(buf + len, BUF_LEN - len, "\n");
    }


    //waiting and serviced passengers 
    int total_waiting = 0;
    for (int i = 0; i < FLOOR_MAX; i++) {
        total_waiting += floors[i].passengers_waiting;
    }
    //print passenger info 
    len += snprintf(buf + len, BUF_LEN - len, "Number of passengers: %d\n", elevator_load);
    len += snprintf(buf + len, BUF_LEN - len, "Number of passengers waiting: %d\n", total_waiting);
    len += snprintf(buf + len, BUF_LEN - len, "Number of passengers serviced: %d\n", pass_serv);

    // Check buffer and copy to user
    if (*ppos > 0 || count < len)
        return 0;
    if (copy_to_user(ubuf, buf, len))
        return -EFAULT;
    *ppos = len;
    return len;
}

extern int (*STUB_stop_elevator)(void);
//stop elevator system call
static int stop_elevator(void) {
    if (elevator_active) {
        kthread_stop(elevator_thread);
        elevator_active = false;
        current_floor = FLOOR_MIN;
        elevator_load = 0;
        printk(KERN_INFO "Elevator stopped\n");
        return 0;
    }
    printk(KERN_INFO "Elevator already stopped\n");
    return 1;
}


//initialize floors
static int __init elevator_init(void) {
    
    STUB_start_elevator = start_elevator;
	STUB_issue_request = add_passenger_to_floor;
	STUB_stop_elevator = stop_elevator;
    
    elevator_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &elevator_fops);
    if (!elevator_entry) {
        return -ENOMEM;
    }
    int i;
    for (i = 0; i < FLOOR_MAX; i++) {
        INIT_LIST_HEAD(&floors[i].passenger_list);
        mutex_init(&floors[i].lock);
    }
    mutex_init(&elevator_lock);
    return 0;
}

//cleanup floors and stop elevator
static void __exit elevator_exit(void) {
    int i;
    for (i = 0; i < FLOOR_MAX; i++) {
        struct passenger *temp_passenger, *next;
        mutex_lock(&floors[i].lock);
        list_for_each_entry_safe(temp_passenger, next, &floors[i].passenger_list, list) {
            list_del(&temp_passenger->list);
            kfree(temp_passenger);
        }
        mutex_unlock(&floors[i].lock);
        mutex_destroy(&floors[i].lock);
    }
    mutex_destroy(&elevator_lock);
    if (elevator_active)
        stop_elevator();

    proc_remove(elevator_entry);
    //the system call sets to null now 
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;


}

module_init(elevator_init);
module_exit(elevator_exit);
