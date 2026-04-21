#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    printf("Total active processes: %d\n", getprocs());
    
    int pid = fork();
    if(pid == 0) {
        printf("Child: Waiting for signal...\n");
        sleep(10); // Wait for parent
        exit(0);
    } else {
        printf("Parent: Sending signal to child %d\n", pid);
        sigsend(pid, 9); // Custom signal
        wait(0);
    }
    
    printf("Final process count: %d\n", getprocs());
    exit(0);
}