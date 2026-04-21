#!/usr/bin/env python3
"""Applies Group 28 custom syscall patches to the cloned xv6-riscv source."""

def read(p):
    with open(p) as f: return f.read()

def write(p, c):
    with open(p, "w") as f: f.write(c)

def append_file(dst, src):
    with open(src) as sf:
        data = sf.read()
    with open(dst, "a") as df:
        df.write("\n" + data)

# 1. kernel/syscall.h — add syscall numbers 22-26
content = read("kernel/syscall.h")
if "SYS_getprocs" not in content:
    append_file("kernel/syscall.h", "/tmp/g28_syscall.h")
    print("[1/9] kernel/syscall.h patched")

# 2. kernel/proc.h — add priority field to struct proc
content = read("kernel/proc.h")
if "int priority;" not in content:
    write("kernel/proc.h", content.replace(
        "  int killed;",
        "  int killed;\n  int priority;"))
    print("[2/9] kernel/proc.h patched")

# 3. kernel/proc.c — initialise priority in allocproc()
content = read("kernel/proc.c")
if "p->priority = 0;" not in content:
    write("kernel/proc.c", content.replace(
        "  p->killed = 0;",
        "  p->killed = 0;\n  p->priority = 0;"))
    print("[3/9] kernel/proc.c patched")

# 4. kernel/syscall.c — add extern declarations and dispatch table entries
content = read("kernel/syscall.c")
if "sys_getprocs" not in content:
    EXTERNS = (
        "extern uint64 sys_getprocs(void);\n"
        "extern uint64 sys_setpriority(void);\n"
        "extern uint64 sys_sigsend(void);\n"
        "extern uint64 sys_uptime_ms(void);\n"
        "extern uint64 sys_getmem(void);\n"
    )
    TABLE = (
        "[SYS_getprocs]    sys_getprocs,\n"
        "[SYS_setpriority] sys_setpriority,\n"
        "[SYS_sigsend]     sys_sigsend,\n"
        "[SYS_uptime_ms]   sys_uptime_ms,\n"
        "[SYS_getmem]      sys_getmem,\n"
    )
    content = content.replace(
        "extern uint64 sys_close(void);",
        "extern uint64 sys_close(void);\n" + EXTERNS)
    content = content.replace(
        "[SYS_close]   sys_close,",
        "[SYS_close]   sys_close,\n" + TABLE)
    write("kernel/syscall.c", content)
    print("[4/9] kernel/syscall.c patched")

# 5. kernel/sysproc.c — append implementations
content = read("kernel/sysproc.c")
if "sys_getprocs" not in content:
    append_file("kernel/sysproc.c", "/tmp/g28_sysproc.c")
    print("[5/9] kernel/sysproc.c patched")

# 6. user/user.h — append prototypes
content = read("user/user.h")
if "getprocs" not in content:
    append_file("user/user.h", "/tmp/g28_user.h")
    print("[6/9] user/user.h patched")

# 7. user/usys.pl — append entry() calls
content = read("user/usys.pl")
if "getprocs" not in content:
    append_file("user/usys.pl", "/tmp/g28_usys.pl")
    print("[7/9] user/usys.pl patched")

# 8. user/testbench.c — copied by Dockerfile
print("[8/9] user/testbench.c copied by Dockerfile")

# 9. Makefile — add testbench to UPROGS
content = read("Makefile")
if "_testbench" not in content:
    write("Makefile", content.replace(
        "$U/_zombie\\",
        "$U/_zombie\\\n\t$U/_testbench\\"))
    print("[9/9] Makefile patched")

print("\nAll patches applied.")
