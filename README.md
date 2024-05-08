Documentation for Project 2: 
Divsion of Labor: (After)
Project 2: Division of Labor
Operating Systems
Team Members:
1. Alyssa Evans
2. Kirsten Blair

Part 1: System Call Tracing - Kirsten Blair

Part 2: Timer Kernel Module - Alyssa Evans

Part 3a: Adding System Calls - Kirsten Blair 

Part 3b: Kernel Compilation - Kirsten Blair

Part 3c: Threads - Alyssa Evans

Part 3d: Linked List - Alyssa Evans

Part 3e: Mutexes - Alyssa Evans

Part 3f: Scheduling Algorithm - Alyssa Evans 


part 1/ 

src/

empty.c

empty.trace

part1.c

part1.trace

Makefile


part 2/

src/
Makefile

my_timer.c


part 3/

src/
Makefile

syscalls.c

-Makefile

elevator.c


README.md

FOR ELEVATOR.C PART 3 STEPS 4-6 RUNNING:
Makefile included is for the file elevator.c. It creates an executable, elevator.ko. Run the elevator by: 
1. using "make"
2. load the compiled module into the kernel using the command "sudo insmod elevator.ko"
3. check if the module is loaded by using the command "lsmod | grep elevator"
4. test elevator with rest of the system
      gcc producer.c -o producer
      gcc consumer.c -o consumer
      ./consumer --start
      ./producer [number of passengers]
   monitor elevator status using proc file:
       cat /proc/elevator
   after testing, you can stop the elevator using "./consumer --stop"
6. when done, unload the module from the kernel using the command "sudo rmmod elevator"

To check the kernel logs to see if there are any error messages use: dmesg 


NOTE *** our TA had trouble compiling our code when downloaded from git clone for an unknown cause for the first project.. if this occurs again please let us know. Everything compiles on our end perfectly. 
