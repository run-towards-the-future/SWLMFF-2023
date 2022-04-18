/*Uncached addr to make sure master can see new value modifyed by slave-core*/
#define _DL_START_ADDR_RW 0x700000000000ULL
#define _DL_ReservedSpace   0x800
#define _DL_dlopen_hybrid_doit 0x20150131 //Special code for dlopen_hybrid
#define _DL_ATHREAD_STATE_BASE ((char*)(_DL_START_ADDR_RW+_DL_ReservedSpace)-64*4)
#define _DL_Athread_state(cg,pen) *(volatile char*)(_DL_ATHREAD_STATE_BASE+cg*64+pen)
#define _DL_OPEN_START ((unsigned long*)(_DL_ATHREAD_STATE_BASE)-1)
#define _DL_SPE_MASK ((unsigned long*)(_DL_OPEN_START)-1)
#define _DL_SLAVE_TLS_ADDR ((unsigned long*)(_DL_SPE_MASK)-64)
#define _DL_DLOPEN_HYBRID  ((unsigned long*)_DL_SLAVE_TLS_ADDR-1)
#define _DL_MALLOC_START  ((unsigned long*)_DL_DLOPEN_HYBRID-1)
#define _DL_MALLOC_SIZE  ((unsigned long*)_DL_MALLOC_START-1)
#define _DL_SPAWN_ARG  ((unsigned long*)_DL_MALLOC_SIZE-1)


#ifndef XIAOQ201905
/*For Test_multicore_space()*/
#define _IOC_NRBITS 8
#define _IOC_TYPEBITS   8
#define _IOC_SIZEBITS   13
#define _IOC_DIRBITS    3

#define _IOC_NRMASK ((1 << _IOC_NRBITS)-1)
#define _IOC_TYPEMASK   ((1 << _IOC_TYPEBITS)-1)
#define _IOC_SIZEMASK   ((1 << _IOC_SIZEBITS)-1)
#define _IOC_DIRMASK    ((1 << _IOC_DIRBITS)-1)

#define _IOC_NRSHIFT    0
#define _IOC_TYPESHIFT  (_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_SIZESHIFT  (_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_DIRSHIFT   (_IOC_SIZESHIFT+_IOC_SIZEBITS)

#define SLAVEIO 0xAA
#define _IOC_WRITE  4U
#define _IOC(dir,type,nr,size)          \
    ((unsigned int)             \
     (((dir)  << _IOC_DIRSHIFT) |       \
      ((type) << _IOC_TYPESHIFT) |      \
      ((nr)   << _IOC_NRSHIFT) |        \
      ((size) << _IOC_SIZESHIFT)))
#define _IOW(type,nr,size)  _IOC(_IOC_WRITE,(type),(nr),sizeof(size))
#define SLAVE_GET_VMAFLAG   _IOW(SLAVEIO,   0x0f, unsigned long)

#endif

#define _IS_DL_OPEN_HYBRID (Test_multicore_space()&&(*(unsigned long*)_DL_DLOPEN_HYBRID == _DL_dlopen_hybrid_doit))
