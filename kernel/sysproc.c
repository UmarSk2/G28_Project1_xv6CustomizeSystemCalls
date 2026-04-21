// Group 28 — custom syscall implementations (appended to kernel/sysproc.c)
// All xv6 kernel headers are already included by the original sysproc.c.

extern struct proc proc[];

// Syscall 22: count non-UNUSED entries in the global proc[] table.
uint64
sys_getprocs(void)
{
  struct proc *p;
  int count = 0;
  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->state != UNUSED)
      count++;
    release(&p->lock);
  }
  return count;
}

// Syscall 23: set scheduling priority of the calling process (range 0–10).
uint64
sys_setpriority(void)
{
  int prio;
  argint(0, &prio);
  if (prio < 0 || prio > 10)
    return -1;
  myproc()->priority = prio;
  return 0;
}

// Syscall 24: send a signal to the process with the given PID.
// Sets killed=1 and wakes the target if sleeping (SIGKILL semantics).
uint64
sys_sigsend(void)
{
  int pid, signum;
  argint(0, &pid);
  argint(1, &signum);
  (void)signum;
  struct proc *p;
  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->pid == pid) {
      p->killed = 1;
      if (p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

// Syscall 25: return system uptime in milliseconds (ticks * 100 ms/tick).
uint64
sys_uptime_ms(void)
{
  uint xticks;
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return (uint64)xticks * 100;
}

// Syscall 26: return the calling process's virtual address space size in bytes.
uint64
sys_getmem(void)
{
  return (uint64)myproc()->sz;
}
