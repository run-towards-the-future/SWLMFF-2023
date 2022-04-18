#include <simd.h>
#ifdef __sw_host__
typedef int256 intvec;
#endif
#ifdef __sw_slave__
#ifdef __sw_ocn__
typedef int512 intvec;
#else
typedef int256 intvec;
#endif
#endif

#ifdef __sw_ocn__
#ifdef __sw_host__
__attribute__((always_inline)) inline int sys_m_cgid(){
    unsigned long cid; 
    asm volatile(
            "rcid %0\n"
            "sll  %0, 61, %0\n"
            "srl  %0, 61, %0\n"
            :"=r"(cid)
            :
            :"memory");
    return (int) cid;
}
// #define CPE_LDM_ADDR(cgid, cpe_id, ldm_addr) (0x8002000000L | (cgid) << 36L | (cpe_id) << 16L | (long)(ldm_addr))
#define LDM_HW_BASE   (0x1ULL<<47ULL | 0x2ULL<<22ULL )
#define CPE_LDM_ADDR(cgid,speid,disp)\
            (unsigned long *)(LDM_HW_BASE + ( (unsigned long)cgid<<40) + ((unsigned long)speid <<24) + (unsigned long)disp)
#endif
#endif
#ifdef __sw_thl__
#define CPE_LDM_ADDR(cgid, cpe_id, ldm_addr) (0x8002000000L | (cgid) << 36L | (cpe_id) << 16L | (long)(ldm_addr))
#endif
