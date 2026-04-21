#!/usr/bin/env python3
"""
Patches the cloned xv6-riscv source to add 5 custom syscalls:
  22  getprocs    - count active processes
  23  setpriority - set process scheduling priority
  24  sigsend     - send a signal to another process by PID
  25  uptime_ms   - return system uptime in milliseconds
  26  getmem      - return calling process memory footprint
"""

# ── 1. kernel/syscall.h ─────────────────────────────────────────────────────
with open("kernel/syscall.h", "a") as f:
    f.write("\n// Custom syscalls (Group 28)\n")
    f.write("#define SYS_getprocs    22\n")
    f.write("#define SYS_setpriority 23\n")
    f.write("#define SYS_sigsend     24\n")
    f.write("#define SYS_uptime_ms   25\n")
    f.write("#define SYS_getmem      26\n")
print("[1/7] kernel/syscall.h patched")

# ── 2. kernel/proc.h ── add priority field to struct proc ───────────────────
with open("kernel/proc.h", "r") as f:
    content = f.read()

if "int priority;" not in content:
    content = content.replace(
        "  int killed;",
        "  int killed;\n  int priority;          // scheduling priority (0 = default)"
    )
    with open("kernel/proc.h", "w") as f:
        f.write(content)
print("[2/7] kernel/proc.h patched")

# ── 3. kernel/proc.c ── initialise priority in allocproc() ──────────────────
with open("kernel/proc.c", "r") as f:
    content = f.read()

if "p->priority = 0;" not in content:
    content = content.replace(
        "  p->killed = 0;",
        "  p->killed = 0;\n  p->priority = 0;"
    )
    with open("kernel/proc.c", "w") as f:
        f.write(content)
print("[3/7] kernel/proc.c patched")

# ── 4. kernel/syscall.c ── extern declarations + dispatch table entries ──────
with open("kernel/syscall.c", "r") as f:
    content = f.read()

EXTERNS = (
    "extern uint64 sys_getprocs(void);\n"
    "extern uint64 sys_setpriority(void);\n"
    "extern uint64 sys_sigsend(void);\n"
    "extern uint64 sys_uptime_ms(void);\n"
    "extern uint64 sys_getmem(void);\n"
)
TABLE_ENTRIES = (
    "[SYS_getprocs]    sys_getprocs,\n"
    "[SYS_setpriority] sys_setpriority,\n"
    "[SYS_sigsend]     sys_sigsend,\n"
    "[SYS_uptime_ms]   sys_uptime_ms,\n"
    "[SYS_getmem]      sys_getmem,\n"
)

if "sys_getprocs" not in content:
    content = content.replace(
        "extern uint64 sys_close(void);",
        "extern uint64 sys_close(void);\n" + EXTERNS
    )
    content = content.replace(
        "[SYS_close]   sys_close,",
        "[SYS_close]   sys_close,\n" + TABLE_ENTRIES
    )
    with open("kernel/syscall.c", "w") as f:
        f.write(content)
print("[4/7] kernel/syscall.c patched")

# ── 5. kernel/sysproc.c ── implementations ──────────────────────────────────
IMPL = r"""
// ── Custom syscalls (Group 28) ──────────────────────────────────────────────
// proc[] lives in proc.c; declare it extern so sysproc.c can iterate it.
extern struct proc proc[];

// Returns the number of currently active (non-UNUSED) processes.
uint64
sys_getprocs(void)
{
  struct proc *p;
  int count = 0;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->state != UNUSED)
      count++;
    release(&p->lock);
  }
  return count;
}

// Sets the calling process's scheduling priority (0–10).
uint64
sys_setpriority(void)
{
  int prio;
  argint(0, &prio);
  if(prio < 0 || prio > 10)
    return -1;
  myproc()->priority = prio;
  return 0;
}

// Sends a signal to the process identified by pid.
// Currently implements SIGKILL semantics (sets killed flag).
uint64
sys_sigsend(void)
{
  int pid, signum;
  argint(0, &pid);
  argint(1, &signum);
  struct proc *p;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      p->killed = 1;
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

// Returns system uptime in milliseconds.
// xv6 timer fires every ~100 ms (1,000,000 cycles at 10 MHz QEMU clock).
uint64
sys_uptime_ms(void)
{
  uint xticks;
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return (uint64)xticks * 100;
}

// Returns the memory footprint (in bytes) of the calling process.
uint64
sys_getmem(void)
{
  return (uint64)myproc()->sz;
}
"""

with open("kernel/sysproc.c", "r") as f:
    existing = f.read()

if "sys_getprocs" not in existing:
    with open("kernel/sysproc.c", "a") as f:
        f.write(IMPL)
print("[5/7] kernel/sysproc.c patched")

# ── 6. user/user.h ── append user-space prototypes ──────────────────────────
with open("user/user.h", "r") as f:
    existing = f.read()

if "getprocs" not in existing:
    with open("user/user.h", "a") as f:
        f.write("\n// Custom syscall prototypes (Group 28)\n")
        f.write("int getprocs(void);\n")
        f.write("int setpriority(int);\n")
        f.write("int sigsend(int, int);\n")
        f.write("int uptime_ms(void);\n")
        f.write("int getmem(void);\n")
print("[6/7] user/user.h patched")

# ── 7. user/usys.pl ── append assembly stub entries ─────────────────────────
with open("user/usys.pl", "r") as f:
    existing = f.read()

if "getprocs" not in existing:
    with open("user/usys.pl", "a") as f:
        f.write('entry("getprocs");\n')
        f.write('entry("setpriority");\n')
        f.write('entry("sigsend");\n')
        f.write('entry("uptime_ms");\n')
        f.write('entry("getmem");\n')
print("[7/7] user/usys.pl patched")

# ── 8. Makefile ── add testbench to UPROGS ───────────────────────────────────
with open("Makefile", "r") as f:
    content = f.read()

if "_testbench" not in content:
    content = content.replace(
        "$U/_zombie\\",
        "$U/_zombie\\\n\t$U/_testbench\\"
    )
    with open("Makefile", "w") as f:
        f.write(content)
print("[8/8] Makefile patched")

print("\nAll patches applied successfully!")
