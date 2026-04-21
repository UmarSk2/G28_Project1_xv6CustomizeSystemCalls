// Example: Implementation of getprocs
uint64
sys_getprocs(void) {
  struct proc *p;
  int count = 0;
  // xv6 uses a global proc table
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state != UNUSED) {
      count++;
    }
    release(&p->lock);
  }
  return count;
}