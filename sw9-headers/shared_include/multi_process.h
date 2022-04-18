#define ROUND_UP(N,ALIGN)        (((N) + ((ALIGN)-1)) & ~((ALIGN)-1))
#define ROUND_UP_8KB(LEN) 	(((LEN) + ((8192) - 1))&~((8192) - 1))
#define ROUND_UP_16B(x)         ((((x) + 0xf) >> 4) << 4)       /* Alpha shared lock address, 16B aligned */
#define MAIN main(int argc, char *argv[])
#define MAX_NCGS 4
#define MAX_NCORES 4
/*
MCU_DEBUG0:0x981000004500
MCU_DEBUG1:0x981000004580
MCU_DEBUG2:0x981000004600
MCU_DEBUG3:0x981000004680
MCU_DEBUG4:0x981000004700
MCU_DEBUG5:0x981000004780
MCU_DEBUG6:0x981000004800
MCU_DEBUG7:0x981000004880
*/
/*
CPUID: 0x981000000900
*/


extern long cpuid;
extern int nprocs,jobid,scale,LOOP_CNT;
extern volatile int *syn_flags,*end_flags,*begin_flags,*success_flag;
//extern int *alloc_shared(int size);
extern int *alloc_shared(int argc,char **argv,int *size_len,int *size);

//rio will return load val from addr    
static inline unsigned long read_io(unsigned long addr)     
{
	register unsigned long __r0 __asm__("$0");     
	register unsigned long __r16 __asm__("$16") = 1;   
	register unsigned long  __r17 __asm__("$17") = addr;
	__asm__ __volatile__(     
			"sys_call %3 # "     
			: "=r"(__r16), "=r"(__r17),"=r"(__r0)     
			: "i"(0xB3), "0"(__r16), "1"(__r17)
			: "$1", "$22", "$23", "$24", "$25");     
	return __r0;     
}
//wio will store val to addr, then load addr, return its new val
static inline unsigned long write_io(unsigned long addr, unsigned long val) 
{
	register unsigned long __r0 __asm__("$0");     
	register unsigned long __r16 __asm__("$16") = 2;   
	register unsigned long __r17 __asm__("$17") = addr;
	register unsigned long __r18 __asm__("$18") = val; 
	__asm__ __volatile__(     
			"sys_call %4 # "     
			: "=r"(__r16), "=r"(__r17), "=r"(__r18),"=r"(__r0)
			: "i"(0xB3), "0"(__r16), "1"(__r17), "2"(__r18)
			: "$1", "$22", "$23", "$24", "$25");
	return __r0;
}
/*
#define __CALL_PAL_RW1(NAME, RTYPE, TYPE0)                      \
	static inline RTYPE NAME(TYPE0 arg0)                            \
{                                                               \
	register RTYPE __r0 __asm__("$0");                      \
	register TYPE0 __r16 __asm__("$16") = arg0;             \
	__asm__ __volatile__(                                   \
			"sys_call %2 # "#NAME                           \
			: "=r"(__r16), "=r"(__r0)                       \
			: \
			"i"(0xB6), "1"(__r16)                 \
			: "$1", "$22", "$23", "$24", "$25");   \         
		return __r0;                                            
}  

__CALL_PAL_RW1(setipl, unsigned long, unsigned long);
*/
/*
main()
{  
	setipl(7);//close  
	setipl(0);//open
} 
*/
