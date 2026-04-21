extern uint64 sys_getprocs(void);
extern uint64 sys_sigsend(void);

static uint64 (*syscalls[])(void) = {
  // ... existing calls
  [SYS_getprocs]  sys_getprocs,
  [SYS_sigsend]   sys_sigsend,
};