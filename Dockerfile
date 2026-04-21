FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install RISC-V toolchain, QEMU, and build tools
RUN apt-get update && apt-get install -y \
    git \
    gcc \
    gcc-riscv64-unknown-elf \
    binutils-riscv64-unknown-elf \
    qemu-system-misc \
    make \
    python3 \
    bc \
    expect \
    && rm -rf /var/lib/apt/lists/*

# Clone the official xv6-riscv source
RUN git clone https://github.com/mit-pdos/xv6-riscv.git /xv6-riscv

WORKDIR /xv6-riscv

# Apply kernel + user patches (syscall.h, proc.h, proc.c, syscall.c,
# sysproc.c, user.h, usys.pl, Makefile) via a Python script
COPY docker/patch.py /tmp/patch.py
COPY docker/run_testbench.exp /xv6-riscv/run_testbench.exp
RUN python3 /tmp/patch.py

# Copy the test program that exercises all 5 custom syscalls
COPY user/testbench.c user/testbench.c

# Build the kernel and filesystem image (single CPU)
RUN make CPUS=1 kernel/kernel fs.img

# Interactive mode:   docker run -it xv6-custom
# Automated demo:     docker run --rm xv6-custom expect run_testbench.exp
CMD ["make", "qemu", "CPUS=1"]
