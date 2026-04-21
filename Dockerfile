FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git gcc gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf \
    qemu-system-misc make python3 bc expect \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/mit-pdos/xv6-riscv.git /xv6-riscv
WORKDIR /xv6-riscv

COPY kernel/syscall.h  /tmp/g28_syscall.h
COPY kernel/sysproc.c  /tmp/g28_sysproc.c
COPY user/user.h       /tmp/g28_user.h
COPY user/usys.pl      /tmp/g28_usys.pl
COPY user/testbench.c  user/testbench.c
COPY patch.py          /tmp/patch.py
RUN python3 /tmp/patch.py

RUN make CPUS=1 kernel/kernel fs.img

COPY run_testbench.exp run_testbench.exp

CMD ["make", "qemu", "CPUS=1"]
