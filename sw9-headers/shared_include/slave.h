#ifndef _SLAVE_H
#define _SLAVE_H
#ifdef __sw_slave__
#include <signal.h>
//#include <bits/types.h>
//#include "share.h"
#include "slave_intc.h"
#include "slave_sig.h"

#ifndef WUWH20160808
typedef unsigned char           uint8_t;
typedef unsigned int            uint32_t;
typedef unsigned short          uint16_t;
typedef unsigned long           uint64_t;
# ifndef __int8_t_defined
# define __int8_t_defined
typedef char                    int8_t;
typedef int                     int32_t;
typedef short                   int16_t;
typedef long                    int64_t;
#endif 
#endif
//#ifndef __cplusplus
typedef _Float16 float16;
//#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __sw_slave_ioproxy__
extern int my_printf(const char* ss,...);
#define printf my_printf
#endif
#ifdef __cplusplus
}
#endif

#ifndef GML20200508
#define LDMS_0KB        6
#define LDMS_4KB        0
#define LDMS_8KB        1
#define LDMS_16KB       2
#define LDMS_32KB       3
#define LDMS_64KB       4
#define LDMS_128KB      5
#define LDMS_SINGLE     0
#define LDMS_DOUBLE     1
#define LDMS_QUAD       2
#define LDMS_ALL_CLUSTER 4
#define LDMS_ALL_ROW     5
#define CACHE_0KB       3
#define CACHE_32KB      0       
#define CACHE_128KB     1
#endif

#if 0
/*XIAOQ201810 New way to support ldm*/
#ifdef _SW_COMPILER_VERSION
#define  __thread  __attribute__ ((section (".tdata_private")))          
#define  __thread_local  __attribute__ ((section (".tdata_local"))) 
#define  __thread_local_fix  __attribute__ ((section (".tdata_local_fix")))
#define  __thread_kernel(kname)  __attribute__ ((section ( ".tdata_kernel_"kname))) 
#else
#define  __thread_local __thread  __attribute__ ((section (".tdata_local")))
#define  __thread_local_fix __thread  __attribute__ ((section (".tdata_local_fix")))
#endif

#define  __thread_mix  __attribute__ ((section (".tdata_private_mix"))) 

#ifndef WUW20170320_LDMREUSE
#define __thread_kernel(kname,...)\
__thread __attribute__ ((section (tdata_kernel_str(.tdata_kernel_##kname##_ReUsELdM_##__VA_ARGS__))))
#define tdata_kernel_str(a) #a

#else
#ifndef WUW20170111_LDMREUSE
#define __thread_kernel(kname,...) __thread __attribute__ ((section (tdata_kernel_str(.tdata_kernel_##kname##_##__VA_ARGS__))))
#define tdata_kernel_str(a) #a
#endif
#define  __thread_local_share  __thread __attribute__ ((section (".tdata_local_share"))) 

#endif


#else

#ifdef _SLAVE_LLVM
/*ZHOUWH20190311_SWLLVM_SPE_LDM*/
#define __ldm_fix __ldm __attribute__ ((section (".ldm_fix")))
#define __ldm_kernel(kname,...)\
    __ldm __attribute__ ((section (tdata_kernel_str(.ldm_reuse_##kname##_ReUsELdM_##__VA_ARGS__))))
#define __thread_kernel(kname,...)\
    __ldm __attribute__ ((section (tdata_kernel_str(.ldm_reuse_##kname##_ReUsELdM_##__VA_ARGS__))))
#define tdata_kernel_str(a) #a
#define __thread_local_fix __ldm_fix
#define __thread_local __ldm
#define  __ldm_share __ldm  __attribute((ldm_model("share")))
#define __thread_local_share __ldm_share
#else

/*XIAOQ201810 New way to support ldm*/
#define __ldm_fix __attribute__ ((section (".ldm_fix")))
#define __ldm __attribute__ ((section (".ldm")))
#define __ldm_kernel(kname,...)\
__attribute__ ((section (tdata_kernel_str(.ldm_reuse_##kname##_ReUsELdM_##__VA_ARGS__))))
#define __thread_kernel(kname,...)\
__attribute__ ((section (tdata_kernel_str(.ldm_reuse_##kname##_ReUsELdM_##__VA_ARGS__))))
#define tdata_kernel_str(a) #a
#define __thread_local_fix __ldm_fix
#define __thread_local __ldm
#define  __thread_local_share  __attribute__ ((section (".ldm_share")))
#endif
#endif

#define  __atomic __uncached 
#define  __atomic_thread __uncached __thread
#define  __cross __cross 
#ifdef XIAOQ20180131
#define  __uncached __attribute__ ((section (".uncached_rw"))) 
#define  __uncached_thread __thread __attribute__ ((section (".uncached_tdata_private"))) 
#define  __uncached_cross __thread __attribute__ ((section (".uncached_cross")))
#define  __atomic_cross __thread __attribute__ ((section (".uncached_cross")))
#define  __cross __thread __attribute__ ((section (".cross_rw"))) 
#endif



#define sw_slave_evictd(x) __builtin_sw_slave_evictd(x)
#define sw_slave_flushd(x) __builtin_sw_slave_flushd(x)
#define sw_slave_write_hint(x) __builtin_sw_slave_cachesc(0,x,0)
//ZHUQ20170321 adjust the sequence of first two prameters of cachesc
#define sw_slave_private_section0(start,end) __builtin_sw_slave_cachesc(end,start,0x10)
#define sw_slave_private_section1(start,end) __builtin_sw_slave_cachesc(end,start,0x11)
#define sw_slave_private_section2(start,end) __builtin_sw_slave_cachesc(end,start,0x12)
#define sw_slave_private_section3(start,end) __builtin_sw_slave_cachesc(end,start,0x13)

#include "crts2ath.h"
#define PRINT(x,y)\
{\
        asm volatile ("bis	%0,%1,$31\n\t"\
                        ::"r"(x),"r"(y):"memory"\
                        );\
}

#define RPCC(x)\
{asm volatile("memb\n\trcsr %0,4":"=r"(x));}

#define RTC()\
({unsigned long __time__; asm volatile("rcsr	%0,4":"=r"(__time__)::"memory");__time__;})

#define RFPCR0(x) {asm volatile("rfpcr $63\n\tvextf $63,0,%0":"=r"(x)::);}
#define RFPCR1(x) {asm volatile("rfpcr $63\n\tvextf $63,1,%0":"=r"(x)::);}
#define RFPCR(x,y) {asm volatile("rfpcr $63\n\tvextf $63,0,%0\n\tvextf $63,1,%1":"=r"(x),"=r"(y)::);}
#define WFPCR(x,y) {asm volatile("vinsf %0,$63,0,$63\n\tvinsf %1,$63,1,$63\n\twfpcr $63"::"r"(x),"r"(y):"memory");}

//QIANH20181120 add for reading and writing LOG3R truth registers
#define RLOG3R0(x) {asm volatile("rcsr %0,0x80":"=r"(x)::);}
#define RLOG3R1(x) {asm volatile("rcsr %0,0x81":"=r"(x)::);}
#define RLOG3R2(x) {asm volatile("rcsr %0,0x82":"=r"(x)::);}
#define RLOG3R3(x) {asm volatile("rcsr %0,0x83":"=r"(x)::);}
#define RLOG3R4(x) {asm volatile("rcsr %0,0x84":"=r"(x)::);}
#define RLOG3R5(x) {asm volatile("rcsr %0,0x85":"=r"(x)::);}
#define RLOG3R6(x) {asm volatile("rcsr %0,0x86":"=r"(x)::);}
#define RLOG3R7(x) {asm volatile("rcsr %0,0x87":"=r"(x)::);}
#define RLOG3R8(x) {asm volatile("rcsr %0,0x88":"=r"(x)::);}
#define RLOG3R9(x) {asm volatile("rcsr %0,0x89":"=r"(x)::);}
#define RLOG3R10(x) {asm volatile("rcsr %0,0x8a":"=r"(x)::);}
#define RLOG3R11(x) {asm volatile("rcsr %0,0x8b":"=r"(x)::);}
#define RLOG3R12(x) {asm volatile("rcsr %0,0x8c":"=r"(x)::);}
#define RLOG3R13(x) {asm volatile("rcsr %0,0x8d":"=r"(x)::);}
#define RLOG3R14(x) {asm volatile("rcsr %0,0x8e":"=r"(x)::);}
#define RLOG3R15(x) {asm volatile("rcsr %0,0x8f":"=r"(x)::);}
#define WLOG3R0(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x80"::"r"(truth_tmp):);}
#define WLOG3R1(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x81"::"r"(truth_tmp):);}
#define WLOG3R2(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x82"::"r"(truth_tmp):);}
#define WLOG3R3(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x83"::"r"(truth_tmp):);}
#define WLOG3R4(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x84"::"r"(truth_tmp):);}
#define WLOG3R5(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x85"::"r"(truth_tmp):);}
#define WLOG3R6(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x86"::"r"(truth_tmp):);}
#define WLOG3R7(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x87"::"r"(truth_tmp):);}
#define WLOG3R8(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x88"::"r"(truth_tmp):);}
#define WLOG3R9(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x89"::"r"(truth_tmp):);}
#define WLOG3R10(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x8a"::"r"(truth_tmp):);}
#define WLOG3R11(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x8b"::"r"(truth_tmp):);}
#define WLOG3R12(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x8c"::"r"(truth_tmp):);}
#define WLOG3R13(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x8d"::"r"(truth_tmp):);}
#define WLOG3R14(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x8e"::"r"(truth_tmp):);}
#define WLOG3R15(x) {unsigned int truth_tmp=x; asm volatile("wcsr %0,0x8f"::"r"(truth_tmp):);}

#ifndef GML20180917
#define SET_round_mode(x)\
{\
        unsigned long val0, val1, val2;\
	RFPCR(val1,val0);\
        switch(x)\
        {\
                case 0:\
                {\
                        val2 = val1 & 0xF3FFFFFFFFFFFFFF;\
                        break;\
                }\
                case 1:\
                {\
                        val2 = val1 & 0xF7FFFFFFFFFFFFFF;\
                        val2 = val2 | 0x0400000000000000;\
                        break;\
                }\
                case 2:\
                {\
                        val2 = val1 & 0xFBFFFFFFFFFFFFFF;\
                        val2 = val2 | 0x0800000000000000;\
                        break;\
                }\
                case 3:\
                {\
                        val2 = val1 | 0x0c00000000000000;\
                        break;\
                }\
                default: \
                        printf("Error: incorrect round mode!!!!!\n");\
        }\
        WFPCR(val2,val0);\
}
#endif

#define updt_addw(_n_, _addr_) \
{			       \
            asm volatile(                   "vinsf %0,$31,0x1,$63\n\t"  \
                                            "ldi    $63, 4($63)\n\t"                  \
                                            "updt   $63, 0(%1)\n\t"                  \
                                            ::"r"(_n_),"r"(_addr_):"memory");        \
}
#define updt_subw(_n_, _addr_) \
{                              \
            asm volatile(                   "vinsf %0, $31, 0x1, $63\n\t"  \
                                            "ldi    $63, 6($63)\n\t"                  \
                                            "updt   $63, 0(%1)\n\t"                  \
                                            ::"r"(_n_),"r"(_addr_):"memory");        \
}

#define updt_addw_i	updt_addw
#define updt_subw_i	updt_subw

#ifndef SHENL20130117
#define atomic_inc1_w(addr)\
        ({unsigned int __res__;asm volatile ("faaw %0, 0(%1)":"=r"(__res__):"r"(addr):"memory");__res__;})

#define atomic_inc1_l(addr)\
        ({unsigned long __res__;asm volatile ("faal %0, 0(%1)":"=r"(__res__):"r"(addr):"memory");__res__;})

#endif


typedef enum {
        PE_MODE,
        COL_MODE=2
} dma_mode;

typedef enum {
        DMA_PUT,
        DMA_GET,
        DMA_PUT_P,
        DMA_GET_P,
        DMA_BARRIER = 5,
        ALL_BARRIER = 6
} DMA_OP;

typedef enum {
	ROW_SCOPE,
	COL_SCOPE,
	ARRAY_SCOPE,
	PP_SCOPE,
}scope;


//extern int athread_get_core( int id);
//extern int athread_get_id(int core);
extern int athread_syn(scope scp,int mask);

extern __thread_local_fix  char  _CGN,_ROW,_COL,_PEN;
extern __thread_local_fix volatile void (*_PC)();
extern __thread_local_fix volatile void (*_ARG)();
extern __thread_local_fix  int   _MYID;
extern __thread_local_fix  int _cache_size, _ldm_share_mode, _ldm_share_size, _ldm_share_size_3bit;

#ifdef __cplusplus
extern "C" {
#endif
long get_slave_cache_size();
long get_slave_ldmshare_size();
void evict_slave_cache_cont(void* start, void* end);
void flush_slave_cache();
int set_cache_size(int cache_size);
int set_ldm_mode_size(int ldm_share_mode, int ldm_share_size);
void set_ptr_uncached(void **addr);
void set_ptr_cached(void **addr);

int ordered_printf(const char * __restrict format, ...);

//void* ldm_malloc(size_t size);
//void ldm_free(void *addr, size_t size);

static __inline float16 __attribute__((__always_inline__))
sqrth (float16 __x)
{
        return (float16) __builtin_sw_slave_sqrth(__x);
}

extern __thread_local unsigned short _divhfm;
extern __thread_local unsigned short _divhfone;
extern __thread_local unsigned int _divvhfone;
static inline void set_fp16_mode()
{
	_divhfm=0x7798;
	_divhfone=0x3c00;
	_divvhfone=0x3c003c00;
	unsigned long fpcr0,fpcr1;
	RFPCR(fpcr0,fpcr1);
	fpcr0 &= (~(1ULL<<43));
	WFPCR(fpcr0,fpcr1);
}

static inline void set_bf16_mode()
{
	_divhfm=0x7ef3;
	_divhfone=0x3f80;
	_divvhfone=0x3f803f80;
	unsigned long fpcr0,fpcr1;
	RFPCR(fpcr0,fpcr1);
	fpcr0 |= (1ULL<<43);
	WFPCR(fpcr0,fpcr1);
}

static inline float16 fcvthh(float16 __a)
{
	return __builtin_sw_slave_fcvthh(__a);
}

static inline void convert_float16(float16 *dest, float16 *src, int num)
{
	int i;
	for(i=0;i<num;i++)
		dest[i]=fcvthh(src[i]);
}
#ifdef __cplusplus
}
#endif
#endif //__sw_slave__
#endif //_SLAVE_H
