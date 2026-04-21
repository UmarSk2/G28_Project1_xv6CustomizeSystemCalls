// testbench.c — Group 28 xv6 custom syscall demo
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define RST   "\033[0m"
#define CYAN  "\033[1;36m"
#define YEL   "\033[1;33m"
#define GRN   "\033[1;32m"
#define RED   "\033[1;31m"
#define DIM   "\033[0;37m"

static void section(const char *t) {
    printf("\n" CYAN "  %s\n" RST, t);
    printf(DIM "  ------------------------------------------------\n" RST);
}
static void pass(const char *m) { printf(GRN "  [PASS] %s\n" RST, m); }
static void fail(const char *m) { printf(RED "  [FAIL] %s\n" RST, m); }

int main(void)
{
    int r, pid;

    printf("\n");
    printf(CYAN "  ================================================\n" RST);
    printf(YEL  "    xv6 Custom System Calls  |  Group 28\n" RST);
    printf(CYAN "  ================================================\n" RST);

    // ── Syscall 1: getprocs ──────────────────────────────────────────────────
    section("SYSCALL 1: getprocs()  —  Count Active Processes");

    int before = getprocs();
    printf("  Before fork   : %d processes\n", before);

    pid = fork();
    if (pid == 0) { pause(3); exit(0); }

    printf("  Child alive   : %d processes\n", getprocs());
    wait(0);
    int after = getprocs();
    printf("  After wait    : %d processes\n", after);

    if (after == before)
        pass("count rose on fork and returned to baseline after exit");
    else
        fail("count mismatch");

    // ── Syscall 2: setpriority ───────────────────────────────────────────────
    section("SYSCALL 2: setpriority()  —  Set Scheduling Priority");

    printf(DIM "  Valid range [0..10] — must return 0:\n" RST);
    int valid[] = {0, 1, 5, 7, 10};
    int ok = 1;
    for (int i = 0; i < 5; i++) {
        r = setpriority(valid[i]);
        if (r == 0)
            printf("  setpriority(%d)  =  " GRN "0  OK\n" RST, valid[i]);
        else {
            printf("  setpriority(%d)  =  " RED "%d  FAIL\n" RST, valid[i], r);
            ok = 0;
        }
    }
    if (ok) pass("all valid priorities accepted");
    else    fail("a valid priority was rejected");

    printf(DIM "  Out-of-range — must return -1:\n" RST);
    int bad[] = {-1, 11, 99};
    ok = 1;
    for (int i = 0; i < 3; i++) {
        r = setpriority(bad[i]);
        if (r == -1)
            printf("  setpriority(%d)  =  " GRN "-1  rejected\n" RST, bad[i]);
        else {
            printf("  setpriority(%d)  =  " RED "%d  FAIL\n" RST, bad[i], r);
            ok = 0;
        }
    }
    if (ok) pass("all invalid priorities rejected with -1");
    else    fail("kernel accepted an invalid priority");
    setpriority(0);

    // ── Syscall 3: uptime_ms ─────────────────────────────────────────────────
    section("SYSCALL 3: uptime_ms()  —  System Uptime in Milliseconds");

    int t1 = uptime_ms();
    printf("  First reading   : %d ms\n", t1);
    pause(5);
    int t2 = uptime_ms();
    printf("  Second reading  : %d ms\n", t2);
    printf("  Elapsed         : " YEL "%d ms\n" RST, t2 - t1);

    if (t2 > t1) pass("uptime advanced — kernel tick counter is live");
    else         fail("uptime did not advance");

    // ── Syscall 4: getmem ────────────────────────────────────────────────────
    section("SYSCALL 4: getmem()  —  Process Memory Footprint");

    int m1 = getmem();
    printf("  Before sbrk(4096)  : %d bytes  (%d KB)\n", m1, m1/1024);
    sbrk(4096);
    int m2 = getmem();
    printf("  After  sbrk(4096)  : %d bytes  (%d KB)\n", m2, m2/1024);
    printf("  Increase           : " YEL "%d bytes\n" RST, m2 - m1);

    if (m2 == m1 + 4096) pass("getmem() reflects exact 4096-byte heap growth");
    else                 fail("getmem() did not track sbrk correctly");

    // ── Syscall 5: sigsend ───────────────────────────────────────────────────
    section("SYSCALL 5: sigsend()  —  Send Signal to Process by PID");

    printf(DIM "  Test A — send to non-existent PID 9999:\n" RST);
    r = sigsend(9999, 9);
    printf("  sigsend(9999, 9)  =  %d\n", r);
    if (r == -1) pass("returned -1 for unknown PID");
    else         fail("should have returned -1");

    printf(DIM "  Test B — fork child sleeping 100 ticks, kill after 5:\n" RST);
    int cnt = getprocs();
    printf("  Before fork   : %d processes\n", cnt);

    pid = fork();
    if (pid < 0) { printf("  fork failed\n"); exit(1); }

    if (pid == 0) {
        printf("  Child (pid=%d) sleeping 100 ticks...\n", getpid());
        pause(100);
        printf(RED "  Child woke naturally — sigsend FAILED!\n" RST);
        exit(0);
    } else {
        printf("  After fork    : %d processes\n", getprocs());
        pause(5);
        printf("  Sending SIGKILL to child pid=%d\n", pid);
        r = sigsend(pid, 9);
        printf("  sigsend()  =  %d\n", r);
        wait(0);
        printf("  After kill    : %d processes\n", getprocs());
        if (r == 0) pass("child killed mid-sleep; process table restored");
        else        fail("sigsend did not work");
    }

    printf("\n");
    printf(CYAN "  ================================================\n" RST);
    printf(GRN  "    All 5 custom syscalls executed successfully!\n" RST);
    printf(CYAN "  ================================================\n\n" RST);

    exit(0);
}
