#ifndef  _SLAVE_H_
#define  _SLAVE_H_

#ifdef __sw_host__
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/hardirq.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/cpumask.h>
#include <linux/spinlock.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/bug.h>
#include <linux/mm.h>
#include <linux/mmu_notifier.h>
#include <linux/preempt.h>
#include <linux/msi.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/ratelimit.h>
#include <linux/err.h>
#include <linux/bitmap.h>
#include <asm/signal.h>
#endif

//#include <athread.h>
#define NR_SCORE_ARRAY_NUM         6
#define CG_NUM NR_SCORE_ARRAY_NUM
#define NR_SC_NUM_PER_ARRAY 	   16
#define NR_SCORE_NUM_PER_ARRAY     64
#define MAX_CORES NR_SCORE_NUM_PER_ARRAY

#define SLAVE_API_VERSION 1

#define SLAVEIO 0xAA

/* no-cache flags */
#define PRIVATE_NC			(0x1UL)
#define SHARED_RO_NC		(0x1UL << 1)
#define SHARED_RW_NC		(0x1UL << 2)
#define CROSS_NC			(0x1UL << 3)

#ifndef current_array_id
#define current_array_id athread_get_arrayid
static inline int athread_get_arrayid(void)
{
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
#endif


struct scoremask {
    /*  Input Arguments */
    unsigned long           coremask[NR_SCORE_ARRAY_NUM];
};


/* struct for user space apis */
struct slave_probe_score {
    /*  Output Arguments */
    struct  scoremask       scoremask_access;
    struct  scoremask       scoremask_fault;
    struct  scoremask       scoremask_busy;
    struct  scoremask       scoremask_occupied;
    struct  scoremask       scoremask_idle;
};

struct slave_get_cgstat {
	/* output arguments */
	unsigned long 		cgstat;
};


struct sjob_alloc_crossm {
    /*  Input Arguments */
    unsigned long   size;

    /*  Output Arguments */
    unsigned long   vaddr;
    unsigned long   vaddr_nc;
};

struct stask_alloc_memory {
    /*  Input Arguments */
    unsigned long           private_size;                   /*  private memory size for a *single* slave core */
    unsigned long           shared_ro_size;                 /*  shared ro memory size for *all* slave cores of this task */
    unsigned long           shared_rw_size;                 /*  shared rw memory size for *all* slave cores of this task */
};


struct stask_map_private {
    /*  Input Arguments */
    int                     arrayid;
    int                     coreid;
    /*  Output Arguments */
    unsigned long           score_private_vaddr;
};


struct stask_map_shared {
    /*  Output Arguments */
    unsigned long           shared_ro_vaddr;
    unsigned long           shared_rw_vaddr;
    unsigned long           shared_ro_vaddr_nc;
    unsigned long           shared_rw_vaddr_nc;
};

struct stask_run_score {
    /*  Input Arguments */
    struct scoremask        scoremask;
    unsigned long           pc;
};

struct stask_config_score {
    /*  Input Arguments */
    struct scoremask        scoremask;
    int			ldm_dcache_size;	/* SCORE_DCACHE_0KB | SCORE_DCACHE_32KB | SCORE_DCACHE_128KB */
	int			ldm_shared_size;	/* SCORE_LDM_SHARED_0 | 4 | 8 | 16 | 32 | 64 | 128 KB */	
	int			ldm_shared_mode;	/* SCORE_LDM_SHARED_SINGLE | DOUBLE | QUAD | ALL_CLUSTER | ALL_ROW */
};

struct stask_stat_ldm {
    /*  Input Arguments */
    int         arrayid;
    int			coreid;

	/*  Output Arguments */
    int			ldm_dcache_size;	/* SCORE_DCACHE_0KB | SCORE_DCACHE_32KB | SCORE_DCACHE_128KB */
	int			ldm_shared_size;	/* SCORE_LDM_SHARED_0 | 4 | 8 | 16 | 32 | 64 | 128 KB */	
	int			ldm_shared_mode;	/* SCORE_LDM_SHARED_SINGLE | DOUBLE | QUAD | ALL_CLUSTER | ALL_ROW */
};

struct stask_rdwr_ldm {
	/* Input Aruguments */
    int             arrayid;
    int			    coreid;
	unsigned long	ldm_offset;
	int 			flag;			/* SCORE_LDM_READ_4/8B | SCORE_LDM_WRITE_4/8B | SCORE_LDM_BROADCAST_WRITE_4/8B */
	/* Input & Output Arguments */
	unsigned long 	value;			/* 8B value */
};

struct stask_vaddr_to_paddr {
	/* Input Aruguments */
	unsigned long 		vaddr;
    unsigned long       pid;    /* pid of stask belongs to the same sjob */
	/* Output Arguments */
	unsigned long 		paddr;
};

struct stask_stat_sarray_stable {
	/* Input Aruguments */
    int             arrayid;
    int             flag;           /* SARRAY_SET_STABLE | SARRAY_GET_STABLE */
	/* Output Arguments */
    int             is_stable;
};

struct slave_va2pa {
    /* Input Arguments */
    unsigned long va;
    unsigned long pid;
    /* Outpu Arguments */
    unsigned long pa;
};

struct expt_inf {
    unsigned long expt_vector;          // 异常向量
    unsigned long expt_pc;              // 异常pc
    unsigned long dma_expt_type;        // 保存dma异常类型
    unsigned long paiu_slb_excpspot;    // sdlb异常此位有效
    unsigned long io_addr;              // 访问io异常，保留cio_io_stat现场
};

struct  slave_get_expt_info {
	/* Input Aruguments */
    int expt_unit;                      // 部件类型
    int cgid;
    int scoreid;
	/* Output Arguments */
    struct expt_inf ei;                 // 异常现场
}; 

struct slave_alloc_mem {
	/* Input Aruguments */
    int nid;
    unsigned long size;
	/* Output Arguments */
    unsigned long paddr;
    unsigned long vaddr;
};

struct slave_test_stable {
    int pid;
};

struct stask_athread_reopen {
    /* Input Arguments */
    unsigned long pc;
    /* Output Arguments */
    unsigned long vm_start;
    unsigned long vm_end;
    int fd;
};

struct sarray_expt_scene {
        unsigned long PA_SINT;
        unsigned long SPC_ERR[NR_SC_NUM_PER_ARRAY];
        unsigned long SPE_ERR[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_IBXIO_ERR[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_IBXIO_GPC[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_IBXIO_STAT[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_EBXIO_EXCP[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_EBXIO_PCRC[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_EBXIO_DESC_0SRCL[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_EBXIO_DESC_0SRCH[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_EBXIO_DESC_1SRC[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_EBXIO_DESC_2SRC[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_EBXIO_FPCRL[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_EBXIO_FPCRH[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_MBXIO_EXCP[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_MBXIO_DA_INT[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_MBXIO_DA_MATCH0[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_MBXIO_DA_MATCH1[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_SBOX_ERR[NR_SCORE_NUM_PER_ARRAY];
        unsigned long SPE_IOBXIO_IOERR[NR_SCORE_NUM_PER_ARRAY];

        unsigned long TBOX_ERR_SUM[NR_SC_NUM_PER_ARRAY];
        unsigned long TBOX_EXCP[NR_SC_NUM_PER_ARRAY];
        unsigned long RBX_ERR[NR_SC_NUM_PER_ARRAY];
        unsigned long SCMT_ERR[NR_SC_NUM_PER_ARRAY];
        unsigned long PAIU_ERR[4];
        unsigned long RIU_IOERR[2];
        unsigned long SRI_IOERR[2];
#ifdef CONFIG_SW_AI
        unsigned long SRI_SLB_ERR[2];
#endif
        unsigned long PAMU_ERR;
};

struct expt_scene {
        int arrayid; /* Input */
        struct sarray_expt_scene *expscene;
};

#define SLAVE_EXPT_PARSE_K  1
#define SLAVE_EXPT_PARSE_U  2

/*
 * ioctls for /dev/slave fd:
 */
#define SLAVE_GET_API_VERSION                 _IO(SLAVEIO,    0x00)
#define SLAVE_PROBE_SCORE                     _IOW(SLAVEIO,   0x01, struct slave_probe_score) 
#define SLAVE_RESET_SCORE                     _IOW(SLAVEIO,   0x02, struct scoremask) 
#define SLAVE_CREATE_SJOB                     _IOW(SLAVEIO,   0x04, unsigned long) /*  arg: jobid, return a sjob fd */
#define SLAVE_MAP_SPE_IO                      _IO(SLAVEIO,    0x05) 
#define SLAVE_MAP_SPE_LDM                     _IO(SLAVEIO,    0x06) 
#define SLAVE_HALT_SCORE                      _IOW(SLAVEIO,   0x07, struct scoremask)
#define SLAVE_GET_CGSTAT                      _IOW(SLAVEIO,   0x08, struct slave_get_cgstat)
#define SLAVE_GET_EXPT_INFO                   _IOW(SLAVEIO,   0x09, struct slave_get_expt_info)
#define SLAVE_STAT_SJOB_CROSSM                _IOW(SLAVEIO,   0x0a, unsigned long)  /* arg: jobid */
#define SLAVE_ALLOC_MEM                       _IOW(SLAVEIO,   0x0b, struct slave_alloc_mem)
#define SLAVE_TEST_STABLE                     _IOW(SLAVEIO,   0x0c, struct slave_test_stable)
#define SLAVE_EXPT_MODE_CONTROL               _IOW(SLAVEIO,   0x0d, unsigned long) /* arg: SLAVE_EXPT_PARSE_K or SLAVE_EXPT_PARSE_U */
#define SLAVE_EXPT_CLEAR                      _IOW(SLAVEIO,   0x0e, unsigned long) /* arg: vector */
#define SLAVE_GET_VMAFLAG                     _IOW(SLAVEIO,   0x0f, unsigned long) /* arg: vaddr */
#define SLAVE_COMPRESS                        _IOW(SLAVEIO,   0x16, unsigned long) /* arg: vaddr */
#define SLAVE_FENTRY                          _IOW(SLAVEIO,   0x17, struct excpinfo)
#define SLAVE_GET_EXPT_SCENE                  _IOW(SLAVEIO,   0x18, struct expt_scene) /* arg: vaddr */

#define SLAVE_DEEPSLEEP_SARRAY                _IOW(SLAVEIO,   0x40, unsigned long) /* arg: sarray id */
#define SLAVE_WAKEUP_SARRAY                   _IOW(SLAVEIO,   0x41, unsigned long) /* arg: sarray id */
#define SLAVE_VA2PA                   _IOW(SLAVEIO,   0x42, struct slave_va2pa)
/*
 * ioctls for sjob fd:
 */
#define SJOB_CREATE_STASK                     _IOW(SLAVEIO,   0x10, unsigned long)
#define SJOB_ALLOC_CROSSM                     _IOW(SLAVEIO,   0x11, struct sjob_alloc_crossm) 
#define SJOB_FREE_CROSSM                      _IO(SLAVEIO,    0x12)

/* 
 * ioctls for slave task fd:
 */
#define STASK_ALLOC_SCORE             _IOW(SLAVEIO,   0x20, struct scoremask) 
#define STASK_ALLOC_MEMORY            _IOW(SLAVEIO,   0x21, struct stask_alloc_memory)
#define STASK_MAP_PRIVATE             _IOW(SLAVEIO,   0x22, struct stask_map_private)
#define STASK_MAP_SHARED              _IOW(SLAVEIO,   0x23, struct stask_map_shared)
#define STASK_RUN                     _IOW(SLAVEIO,   0x30, unsigned long) /*  arg: slave score pc */
#define STASK_RUN_SCORE               _IOW(SLAVEIO,   0x31, unsigned long) /*  arg: slave score pc */
#define STASK_HALT_SCORE              _IOW(SLAVEIO,   0x32, struct scoremask)
#define STASK_RESET_SORE              _IOW(SLAVEIO,   0x33, struct scoremask)
#define STASK_VADDR_TO_PADDR 		  _IOW(SLAVEIO,   0x34, struct stask_vaddr_to_paddr)
#define STASK_CONFIG_SCORE 		      _IOW(SLAVEIO,   0x35, struct stask_config_score)
#define STASK_RDWR_LDM	 		      _IOW(SLAVEIO,   0x36, struct stask_rdwr_ldm)
#define STASK_STAT_LDM	 		      _IOW(SLAVEIO,   0x37, struct stask_stat_ldm)
#define STASK_STAT_SARRAY_STABLE      _IOW(SLAVEIO,   0x38, struct stask_stat_sarray_stable)
#define STASK_AKERNEL_REMAP           _IOW(SLAVEIO,   0x39, unsigned long)
#define STASK_AKERNEL_REOPEN          _IOW(SLAVEIO,   0x40, struct stask_athread_reopen)


#define for_each_score_array(arrayid) \
    for(arrayid = 0; arrayid < NR_SCORE_ARRAY_NUM; arrayid++) 

#define for_each_score_in_array(coreid) \
    for(coreid = 0; coreid < NR_SCORE_NUM_PER_ARRAY; coreid++) 

#define for_each_spc_in_array(spcid) \
    for(spcid = 0; spcid < NR_SC_NUM_PER_ARRAY; spcid++) 

#define is_score_set(arrayid, coreid, scoremaskp)  \
    test_bit(coreid, &(  ((struct scoremask *)(scoremaskp))->coremask[arrayid]))

#define for_each_score_scoremask(aid, cid, scoremaskp)  \
    for_each_score_array(aid)  \
        for_each_score_in_array(cid) \
            if(is_score_set(aid, cid, scoremaskp))



#endif
#endif   /* ----- #ifndef _SLAVE_H_  ----- */
