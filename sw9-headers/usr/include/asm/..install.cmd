cmd_dest/include/asm/.install := /bin/sh scripts/headers_install.sh dest/include/asm ./arch/sw_64/include/uapi/asm a.out.h auxvec.h bitsperlong.h byteorder.h compiler.h console.h errno.h fcntl.h fpu.h gentrap.h ioctl.h ioctls.h ipcbuf.h kvm.h kvm_para.h mman.h msgbuf.h pal.h param.h poll.h posix_types.h ptrace.h reg.h regdef.h resource.h sembuf.h setup.h shmbuf.h sigcontext.h siginfo.h signal.h socket.h sockios.h stat.h statfs.h swab.h sysinfo.h termbits.h termios.h types.h unistd.h; /bin/sh scripts/headers_install.sh dest/include/asm ./arch/sw_64/include/asm elf.h page.h; /bin/sh scripts/headers_install.sh dest/include/asm ./arch/sw_64/include/generated/uapi/asm ; for F in ; do echo "\#include <asm-generic/$$F>" > dest/include/asm/$$F; done; touch dest/include/asm/.install