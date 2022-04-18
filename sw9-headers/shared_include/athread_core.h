#include "athread_common.h"


#define START_ADDR_ALLSHARE 0x600000000000LL
#define START_ADDR 0x500000000000LL

#define SLAVE_FD *((int *)(START_ADDR + 0x1000 - 28))
#define SJOB_FD  *((int *)(START_ADDR + 0x1000 - 32))
#define STASK_FD *((int *)(START_ADDR + 0x1000 - 36))

#define SPEIO_REG_ADDR(cgid,speid,reg)\
          (unsigned long *)(SPEIO_HW_BASE + (reg) + ( (unsigned long)cgid<<40) + ((unsigned long)speid <<24) );
#define SPEIO_READ_REG(cgid,speid,reg)\
            *(unsigned long *)(SPEIO_HW_BASE + (reg) + ( (unsigned long)cgid<<40) + ((unsigned long)speid <<24) );
#define SPEIO_WRITE_REG(cgid,speid,reg,val)               \
            *(unsigned long *)(SPEIO_HW_BASE + (reg) + ( (unsigned long)cgid<<40) + ((unsigned long)speid <<24) ) = val;\
        asm("memb");
