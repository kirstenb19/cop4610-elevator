#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <time.h>

int main(){

fork();

struct timespec ts; 
clock_gettime(CLOCK_REALTIME, &ts);

getcpu();


mkdir("testing");

chdir("testing");

return 0; 
}
