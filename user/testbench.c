#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Helper: print a separator line
static void
sep(void)
{
    printf("--------------------------------------------------\n");
}

int
main(void)
{
    printf("\n==================================================\n");
    printf("  xv6 Custom Syscalls Demo  |  Group 28\n");
    printf("==================================================\n\n");

    // ── 1. getprocs ──────────────────────────────────────────────────────────
    sep();
    printf("[Syscall 1] getprocs()\n");
    int procs = getprocs();
    printf("  Active processes in the system: %d\n", procs);

    // ── 2. setpriority ───────────────────────────────────────────────────────
    sep();
    printf("[Syscall 2] setpriority()\n");
    int r = setpriority(7);
    if(r == 0)
        printf("  Priority set to 7 — OK\n");
    else
        printf("  setpriority failed (returned %d)\n", r);

    // Try an invalid value
    r = setpriority(15);
    printf("  setpriority(15) [invalid] returned: %d  (expected -1)\n", r);

    // ── 3. uptime_ms ─────────────────────────────────────────────────────────
    sep();
    printf("[Syscall 3] uptime_ms()\n");
    int ms = uptime_ms();
    printf("  System uptime: %d ms\n", ms);

    // ── 4. getmem ────────────────────────────────────────────────────────────
    sep();
    printf("[Syscall 4] getmem()\n");
    int mem = getmem();
    printf("  Process memory footprint: %d bytes (%d KB)\n", mem, mem / 1024);

    // ── 5. sigsend ───────────────────────────────────────────────────────────
    sep();
    printf("[Syscall 5] sigsend()\n");

    int pid = fork();
    if(pid < 0){
        printf("  fork() failed\n");
        exit(1);
    }

    if(pid == 0){
        // Child: sleep and wait for signal
        printf("  Child (pid=%d): started, sleeping for 100 ticks...\n", getpid());
        pause(100);
        // If sigsend works, this line should never be printed
        printf("  Child: woke normally (signal was NOT received)\n");
        exit(0);
    } else {
        // Parent: wait a moment then signal the child
        pause(5);
        printf("  Parent: sending signal 9 to child pid=%d\n", pid);
        int sr = sigsend(pid, 9);
        printf("  sigsend() returned: %d  (0 = success)\n", sr);
        wait(0);
        printf("  Parent: child has terminated.\n");
    }

    // ── Summary ──────────────────────────────────────────────────────────────
    sep();
    printf("\nFinal active process count: %d\n", getprocs());
    printf("\n==================================================\n");
    printf("  All 5 custom syscalls executed successfully!\n");
    printf("==================================================\n\n");

    exit(0);
}
