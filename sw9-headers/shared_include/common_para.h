#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <slave_kapi.h>

#define SIZEOFLDM  15840 //  16K-544
#define ABS(x) ((x) >= 0 ? (x) : -(x))

#define KB 1024
#define MB (1024*1024)
#define MAX_PENUM_CG       64
#define MAX_PENUM      	384 
#define MAX_CGNUM	6
#define MAX_ROW_NUM 8
#define MAX_COL_NUM 8
#define ALIGN(x,y) (((unsigned long)x+y-1)&~(y-1))
#define ROUND_UP(N,M)        (((N) + ((M)-1)) & ~((M)-1))
#define SYN_COUNT 1

#define LDM_SIZE (64*1024)//slave.h repeat
#ifdef USE_MPI
#ifdef FULL
#define SHARED_SIZE (512*1024*1024)
#define CROSS_SIZE (256*1024*1024)     //cross shared size of every cg
#else
#define SHARED_SIZE (64*1024*1024)
#define CROSS_SIZE (4*1024*1024)     //cross shared size of every cg
#endif
#else
#ifdef FULL
#define SHARED_SIZE (512*1024*1024)/*512->64*/
#define CROSS_SIZE (256*1024*1024)     //cross shared size of every cg /*64->256*/
#else
#define SHARED_SIZE (64*1024*1024)
#define CROSS_SIZE (32*1024*1024)     //cross shared size of every cg
#endif
#endif

#define ALL_SHARED_SIZE (CROSS_SIZE*MAX_CGNUM*8)
#define SYN_SHARED_SIZE (SYN_COUNT*MAX_PENUM)
#define SYN_M_SIZE (SYN_COUNT*MAX_CGNUM)
#define SYN_MS_SIZE (SYN_COUNT*(MAX_CGNUM+MAX_PENUM))

#define DISP 0x30

typedef struct {
	volatile unsigned long lock;
} arch_spinlock_t;

#define __ARCH_SPIN_LOCK_UNLOCKED       { 0 }

typedef struct {
	arch_spinlock_t lock;
	volatile int counter;
} arch_rwlock_t;
enum LOCK_MEMBER
{
	M_LOCK=0,
	M_COUNTER
};

int slave_fd,sjob_fd,stask_fd;//之前线程库定义了，现在需要用户自己定义
double theorytime,delaytime;
unsigned long phy_cpuid,phy_cgid,phy_coreid,log_cpuid,log_cgid,log_coreid;
unsigned int master_core_id,master_core_log_id;
#ifndef FPGA_TEST
__thread int id[MAX_PENUM_CG];
pthread_t thread[MAX_PENUM_CG];
#endif
unsigned int penum,cg_num;
volatile unsigned int m_id,m_num;

unsigned long long ALL_SHARED_ADDR;
void **ADDR_CROSS;
volatile unsigned long cross_size_used,cross_size_used_by_me;
#ifdef MCG
volatile __cross int *pe_check;
volatile __cross int *pe_over;
volatile __cross arch_spinlock_t *addr_master_lock,*addr_ms_lock,*addr_ms_lock1,*addr_ms_lock_tmp;
volatile __cross arch_spinlock_t *addr_m_lock,*addr_s_lock;
volatile __cross unsigned long LOCK_COUNT=0;
volatile __cross unsigned long LOCK_ADDR=0;
volatile __cross unsigned int *SYN_SHARED_BEGIN;
volatile __cross unsigned int *SYN_SHARED_BEGIN_SLAVE;
volatile __cross unsigned int *SYN_M_BEGIN;
volatile __cross unsigned int *SYN_MS_BEGIN;
volatile __cross unsigned int *SYN_MS_BEGIN_SLAVE;
volatile __cross unsigned long *ALL_SHARED_BEGIN_PHY[MAX_CGNUM];
volatile __cross unsigned long *ALL_SHARED_BEGIN[MAX_CGNUM];
#else
volatile int *pe_check;
volatile int *pe_over;
volatile arch_spinlock_t *addr_master_lock,*addr_ms_lock,*addr_ms_lock1,*addr_ms_lock_tmp;
volatile arch_spinlock_t *addr_m_lock,*addr_s_lock;
volatile unsigned long LOCK_COUNT=0;
volatile unsigned long LOCK_ADDR=0;
volatile unsigned int *SYN_SHARED_BEGIN;
volatile unsigned int *SYN_SHARED_BEGIN_SLAVE;
volatile unsigned int *SYN_M_BEGIN;
volatile unsigned int *SYN_MS_BEGIN;
volatile unsigned int *SYN_MS_BEGIN_SLAVE;
volatile unsigned long *ALL_SHARED_BEGIN_PHY[MAX_CGNUM];
volatile unsigned long *ALL_SHARED_BEGIN[MAX_CGNUM];
#endif
volatile int s_check,s_check_sum;
volatile int m_check,m_check_sum;
volatile int *ADDR_PE_CHECK[MAX_PENUM];
volatile int *ADDR_PE_OVER[MAX_PENUM];
volatile unsigned long *ADDR_MASTER_LOCK[1][2];
volatile unsigned long *ADDR_M_LOCK[1][2],*ADDR_S_LOCK[1][2];
volatile arch_rwlock_t *addr_master_rwlock;
volatile unsigned long *ADDR_MASTER_RWLOCK[1][2];
volatile arch_rwlock_t *addr_slave_rwlock;
volatile unsigned long *ADDR_SLAVE_RWLOCK[1][2];


//volatile unsigned long *LOCK_COUNT;
volatile unsigned int *BEGIN_FLAG;
volatile unsigned int *ADDR_BEGIN_FLAG;
volatile unsigned int *ADDR_SYN_SHARED_BEGIN[SYN_SHARED_SIZE];
volatile unsigned int *ADDR_SYN_SHARED_BEGIN_SLAVE[SYN_SHARED_SIZE];
volatile unsigned int *ADDR_SYN_M_BEGIN[SYN_M_SIZE];
volatile unsigned int *ADDR_SYN_MS_BEGIN[SYN_MS_SIZE];
volatile unsigned int *ADDR_SYN_MS_BEGIN_SLAVE[SYN_MS_SIZE];

volatile unsigned long *SHARED_BEGIN_PHY[MAX_CGNUM];

volatile unsigned long *SHARED_BEGIN[MAX_CGNUM];
volatile unsigned long *ldm_addr[MAX_CGNUM][MAX_ROW_NUM*MAX_COL_NUM];
volatile unsigned int  synp_core_num[MAX_PENUM];


typedef struct {
        unsigned int   row_id;
        unsigned int   col_id;
        unsigned int   local_id;
        unsigned int   global_id;
        unsigned int   phy_id;
        unsigned int   random_id;
        unsigned int   valid;
} corelist_t;

typedef struct {
        unsigned int   phy_id;
        unsigned int   log_id;
        unsigned int   valid;
} masterlist_t;

typedef struct
{
  char *para;
  int *myid;
} mypara;

enum CORELIST_MEMBER
{
	ROW_ID=0,
	COL_ID,
	LOCAL_ID,
	GLOBAL_ID,
	PHY_ID,
	RANDOM_ID,
	VALID	
};
enum MASTERLIST_MEMBER
{
	M_PHY_ID=0,
	M_LOG_ID,
	M_VALID
};

volatile corelist_t  *corelist[MAX_CGNUM];
volatile unsigned int  *ADDR_CORELIST[MAX_CGNUM][MAX_ROW_NUM*MAX_COL_NUM][7];
volatile masterlist_t  *masterlist;
volatile unsigned int  *ADDR_MASTERLIST[MAX_CGNUM][3];

unsigned long *coremask;
unsigned long *ADDR_COREMASK[MAX_CGNUM];
unsigned long rowmask[MAX_CGNUM],colmask[MAX_CGNUM];
unsigned long fullrowmask[MAX_CGNUM],fullcolmask[MAX_CGNUM];
unsigned long mask_in_row[MAX_CGNUM][MAX_ROW_NUM];
unsigned long mask_in_col[MAX_CGNUM][MAX_COL_NUM];


int exception_num,exception_cnt,s_error_cnt;
unsigned long g_coremask_original;
unsigned long g_coremask_adjust;
unsigned long thresholdtime;

