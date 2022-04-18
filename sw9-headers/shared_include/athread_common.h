#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#define CG_NUM 6
#define MAX_CORES 64
#define BitGet(mask, num) (((unsigned long)mask >> num) & 1ULL)

#define BitSet(mask, num) (mask |= (1ULL << num))

#define BitClr(mask, num) (mask &= ~(1ULL << num))

#define BitCount(mask)                                                         \
	({                                                                     \
		int __count__;                                                 \
		asm volatile("ctpop    %1,%0\n\t"                              \
			     : "=&r"(__count__)                                \
			     : "r"(mask)                                       \
			     : "memory");                                      \
		__count__;                                                     \
	})

#define FmtAssert(Cond, ParmList)                                              \
	(Cond ? (int)1 : (printf("AHTREAD-LIB:Assertion Failed at file:%s "    \
				 "line:%d \n::",                               \
				 __FILE__, __LINE__),                          \
			  printf ParmList, printf("\n"), exit(0)))

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned long uint64_t;

extern int slave_fd, stask_fd, sjob_fd;

typedef struct s_thread_info {
	char valid; // the thread info is valid or not
	int thread_id;
	int core_num;
	volatile int state_flag; // run :1 ; finish:0
	void *pc;
	void *arg;
	char fini_sig; // finish type
	long gid;
	int team_size;
	int flush_slave_cache; // flush slave cache or not
} thread_info_t;

//=================================
//      Define the .tdata_* section addr
//===================================
extern long *_tdata_local_start __attribute__((weak));
extern long *_tdata_local_end __attribute__((weak));
extern long *_tdata_private_start __attribute__((weak));
extern long *_tdata_private_end __attribute__((weak));
extern long *_tdata_local_fix_end __attribute__((weak));
