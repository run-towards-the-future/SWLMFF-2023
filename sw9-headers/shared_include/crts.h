/**********************************************************
 *  Created by QJX, 2016-0707
 *
 *  Modified by QP, 2016-1109, host prototypes and slave
 *    prototypes written in the same header file
 **********************************************************/

#ifndef __CRTS_H
#define __CRTS_H

#include <stdint.h>
#include <stdlib.h>

//#include "crts_common.h"
#define SIG_OUTPUT 46
#define SIG_IO 47
#define SIG_CUSTOM 45
//extern int __crts_print_init;
#define CRTS_MAX_MPE_NUM                6
#define CRTS_MAX_SPE_NUM                64
#define CRTS_SMAK_ALL 0x3f
#define CRTS_PRINT_BUF_SIZE 1024
#define CRTS_CUSTOM_NODE_LOCK_NUM       8

#define CRTS_CORSS_START_ADDR           0x780000000000ULL

typedef struct CRTS_Proc_info
{
    int32_t rank;
    int32_t size;
    int32_t node_rank;
    int32_t node_size;
    uint8_t mpe_map;
    //int32_t node_id;
    int32_t jobid;
    int32_t pid;
} CRTS_Proc_info;

typedef struct CRTS_Thrd_info
{
    CRTS_Proc_info  pinfo;
    int32_t  trank;
    int32_t  tsize;
    int16_t  node_trank;
    int16_t  node_tsize;
    uint64_t spe_map;
    uint8_t  row_map;
    uint8_t  col_map;
    uint8_t  array_trank;
    uint8_t  array_tsize;
    int16_t  node_spc_trank;
    uint8_t  array_spc_trank;

    uint16_t cache_size;
    uint16_t ldm_stack_size;
    uint16_t free_ldm_addr;
    uint16_t free_ldm_size;
    uint16_t shared_mode;
    uint16_t free_shared_ldm_addr;
    uint16_t free_shared_ldm_size;
} CRTS_Thrd_info;

typedef struct CRTS_Cross_info
{
    uint8_t  avail;
    uint64_t total;
    uint8_t  crts_rsv_avail;
    uint64_t crts_rsv_addr;
    uint8_t  msg_rsv_avail;
    uint64_t msg_rsv_addr;
    uint64_t free_addr;
    uint64_t free_size;
} CRTS_Cross_info;

typedef struct CRTS_Custom_lock
{
    int64_t lock;
    int32_t used;
} CRTS_Custom_lock;

typedef struct CRTS_Node_cross
{
    volatile int64_t handup[CRTS_MAX_MPE_NUM];
    volatile int64_t sync[CRTS_MAX_MPE_NUM];
    //int64_t lock;
    int64_t slave_sync;
    int64_t slave_lock;
    uint64_t host_lock;
    CRTS_Custom_lock custom_lock[CRTS_CUSTOM_NODE_LOCK_NUM];
    CRTS_Custom_lock custom_slave_lock[CRTS_CUSTOM_NODE_LOCK_NUM];
    int32_t  custom_lockid;
    int32_t  custom_slave_lockid;
} CRTS_Node_cross;

typedef struct CRTS_SIGNAL_INFO
{
  uint64_t src; 
  uint64_t size;
  uint64_t dest;
  uint64_t type;
} CRTS_SIGNAL_INFO;
extern CRTS_SIGNAL_INFO crts_signal_info[CRTS_MAX_SPE_NUM];
extern CRTS_Proc_info crts_proc_info;
extern CRTS_Thrd_info crts_thrd_info_h;

extern volatile CRTS_Node_cross *crts_node_cross;
extern int crts_sldm_size_h, crts_sldm_mode_h, crts_sldm_scap_h;
extern unsigned long crts_sldm_len_h;
typedef void*(*CRTS_FUNCPTR)(void*);
void CRTS_sig_user_init (CRTS_FUNCPTR funp);
#ifdef __sw_host__
/* ####  1. Host prototypes: */
#include <athread.h>
//#include <common.h>

typedef volatile long crts_rply_t;

#define CRTS_rank           (crts_proc_info.rank)
#define CRTS_size           (crts_proc_info.size)
#define CRTS_node_rank      (crts_proc_info.node_rank)
#define CRTS_node_size      (crts_proc_info.node_size)
#define CRTS_jobid          (crts_proc_info.jobid)

#define CRTS_array_tsize        (crts_thrd_info_h.array_tsize)
#define CRTS_node_tsize         (crts_thrd_info_h.node_tsize)

#define CRTS_athread_spawn              athread_spawn
#define CRTS_athread_spawn_noflush      athread_spawn_noflush
#define CRTS_athread_create  athread_create 
#define CRTS_athread_wait athread_wait
#define CRTS_athread_end athread_end
#define CRTS_athread_join               athread_join
#define CRTS_athread_halt               athread_halt
#define CRTS_athread_get_max_threads    athread_get_max_threads
#define CRTS_time_cycle()                                   \
({                                                          \
    unsigned long tmp;                                      \
    asm volatile ("rtc %0\n":"=&r"(tmp) ::"memory"); tmp;   \
})

extern int __crts_cgn;
extern thread_info_t __v_core_info[CRTS_MAX_MPE_NUM][CRTS_MAX_SPE_NUM];
#define athread_get_id_crts(i) (__v_core_info[(current_array_id())][i].thread_id)
#define CRTS_athread_get_id_h athread_get_id_crts


int  CRTS_init(void);
int  CRTS_init_all(void);
#ifndef ZHUQ20200910_CRTS_for_Frotran  
int CRTS_init_all_(void);
int crts_init_all_(void);
#endif
void CRTS_sync_node(void);
void CRTS_sync_master_spe(int tid);
void CRTS_sync_master_array(void);
void CRTS_sync_master_spe_share (int mask, int tid);//the array as the mask will sync with the master together
void CRTS_sync_master_array_share(int mask);//the same as spe sync
int CRTS_mutex_lock_node (void);
int CRTS_mutex_trylock_node (void);
int CRTS_mutex_unlock_node (void);
int CRTS_mutex_lock_node_all (void);
void CRTS_mutex_unlock_node_all (void);
int CRTS_mutex_lock_master_array(void);
int CRTS_mutex_unlock_master_array(void);
/*
int CRTS_mutex_init_node_custom (void);
int CRTS_mutex_lock_node_custom (int lockid);
int CRTS_mutex_trylock_node_custom (int lockid);
int CRTS_mutex_unlock_node_custom (int lockid);
int CRTS_mutex_free_node_custom (int lockid);
*/
int CRTS_mutex_lock(volatile int64_t *lock);
int CRTS_mutex_trylock(volatile int64_t *lock);
int CRTS_mutex_unlock(volatile int64_t *lock);
void* CRTS_get_phy_addr( void* );

#define athread_get_id(core) (__v_core_info[(current_array_id())][core].thread_id)
extern int CRTS_mng_get_ldm_allocatable();
#elif defined __sw_slave__    /* SLAVE */
/* ####  2. Slave prototypes: */
#include <slave.h>
#include <athread_core.h>
#define CRTS_int       0x0
#define CRTS_uint      0x1
#define CRTS_long      0x2
#define CRTS_ulong     0x3
#define CRTS_float     0x4
#define CRTS_double    0x5
#define CRTS_intv8     0x6
#define CRTS_uintv8    0x7
#define CRTS_int256    0x8
#define CRTS_uint256   0x9
#define CRTS_floatv4   0xa
#define CRTS_doublev4  0xb
#define CRTS_intv16    0xc
#define CRTS_uintv16   0xd
#define CRTS_int512    0xe
#define CRTS_uint512   0xf
#define CRTS_floatv8   0x10
#define CRTS_doublev8  0x11

#define OP_add        0x1
#define OP_and        0x2
#define OP_or         0x3
#define OP_xor        0x4
#define OP_eqv        0x5
#define OP_min        0x6
#define OP_max        0x7
extern __uncached thread_info_t __v_core_info[CRTS_MAX_MPE_NUM][CRTS_MAX_SPE_NUM];
extern volatile __uncached __cross CRTS_Node_cross   crts_node_cross_info;
#define athread_get_id_crts(i) (__v_core_info[_CGN][i].thread_id)
#define CRTS_athread_get_id athread_get_id_crts
typedef volatile int crts_rply_t;

extern __thread_local_fix CRTS_Thrd_info crts_thrd_info;
extern __thread_local_fix uint64_t __crts_reg_tmp[2];
extern __thread_local_fix uint64_t __crts_RB;
extern __thread_local_fix uint64_t __crts_RC;
extern __thread_local_fix volatile crts_rply_t __crts_rply;
extern __thread_local_fix volatile crts_rply_t __crts_rma_rply;
extern __thread_local_fix int crts_sldm_id, crts_sldm_size;
extern __thread_local_fix volatile unsigned long __ldm_start_addr;
extern __thread_local_fix unsigned long crts_ldm_malloc_end;
extern __thread_local_fix int CRTS_row_size, CRTS_col_size;


#define CRTS_rank           (crts_thrd_info.pinfo.rank)
#define CRTS_size           (crts_thrd_info.pinfo.size)
#define CRTS_node_rank      (crts_thrd_info.pinfo.node_rank)
#define CRTS_node_size      (crts_thrd_info.pinfo.node_size)
#define CRTS_jobid          (crts_thrd_info.pinfo.jobid)


#define CRTS_stime_cycle()                                   \
({                                                           \
    unsigned long tmp;                                       \
    asm volatile ("rcsr %0,4\n":"=&r"(tmp) ::"memory"); tmp; \
})


#define CRTS_smng_cgn()                                   \
({                                                           \
    char tmp;                                       \
    asm volatile ("rcsr %0,3\n":"=&r"(tmp) ::"memory"); tmp; \
})

#define athread_get_id(core) (__v_core_info[(_CGN)][core==-1?_PEN:core].thread_id)
#define CRTS_athread_get_id_h athread_get_id
//slave management
#define CRTS_array_trank        (crts_thrd_info.array_trank)
#define CRTS_array_tsize        (crts_thrd_info.array_tsize)
#define CRTS_node_trank         (crts_thrd_info.node_trank)
#define CRTS_node_tsize         (crts_thrd_info.node_tsize)

#define CRTS_tid    (_PEN)
#define CRTS_rid    (_ROW)
#define CRTS_cid    (_COL)
#define CRTS_cgn    (_CGN)
#define CRTS_CGN    (_CGN)
#define CRTS_pen    CRTS_array_tsize

#define CRTS_smng_get_tid()     CRTS_tid
#define CRTS_smng_get_rid()     CRTS_rid
#define CRTS_smng_get_cid()     CRTS_cid
#define CRTS_smng_get_cgn()     CRTS_cgn

/* spe ID transform: CRTS_tid --> tid in order of spe clusters */
//#define CRTS_spc_tid    ((_PEN&0x31) | ((_PEN&0x6)<<1) | ((_PEN&0x8)>>2))
#define CRTS_spc_tid    (crts_thrd_info.array_spc_trank)
#define CRTS_spcn       (CRTS_spc_tid>>2)

#define CRTS_smng_get_tid_from_spc(_stid_) \
            ((_stid_)&0x31) | (((_stid_)&0x2)<<2) | (((_stid_)&0xc)>>1)

#define CRTS_smng_get_spc_tid()     CRTS_spc_tid
#define CRTS_smng_get_spcn()        CRTS_spcn
extern __thread_local_fix int crts_sldm_id;
#define CRTS_smng_get_sldm_id()     crts_sldm_id

#define CRTS_MAX_LDM_SIZE               (256<<10)
#define CRTS_pldm_get_free_size     get_allocatable_size
#define CRTS_pldm_malloc            ldm_malloc
#define CRTS_pldm_free              ldm_free
extern void ldm_free_all();
extern void* ldm_malloc_max(size_t *size);


#define _crts_faa_long(addr)\
({\
    long tmp = 1;\
    asm volatile ("faal %0, 0(%1) \n"\
                  "memb\n"\
                  :"=&r"(tmp)\
                  :"r"(addr));\
    tmp;\
})

#define _crts_count_ones(value)\
({\
    int rv;\
    asm volatile ("ctpop %1, %0\n" : "=r"(rv) : "r"(value));\
    rv;\
})
extern int CRTS_scoll_alltoall(void *src_addr, void *dest_addr, int units_size);
extern void CRTS_scoll_alltoall_inplace(void *src_addr, int units_size, void *buf, int buf_item);
extern int
CRTS_scoll_redurt(
	void *src_addr,     /* src_addr data address */
	void *dest_addr,    /* target data address */
	int  units,         /* units of base element */
	int  dtype,         /* data type, such as int, long, float, double, etc */
	int  optype,        /* operation type, such as add, and, or, xor, eqv, min,max, etc */
	void *redu_buf,
	int buf_item);
//sync
#define CRTS_ssync_16spe()\
({\
    asm volatile ("rcsr      $1, 0            \n"\
                  "xor       $1, 0x8, $1      \n"\
                  "ldi       $2, 0xff         \n"\
                  "synr      $2               \n"\
                  "synp      $1               \n"\
                  :::"memory", "$1", "$2");\
    0;\
})

/* 4 spes in 1 cluster: 2spes x 2 */
#define CRTS_ssync_1spc()\
({\
    asm volatile ("rcsr      $1, 0            \n"\
                  "xor       $1, 0x1, $2      \n"\
                  "xor       $1, 0x8, $1      \n"\
                  "synp      $2               \n"\
                  "synp      $1               \n"\
                  :::"memory", "$1", "$2");\
    0;\
})

/* 8 spes in 2 clusters: 4spes x 2 */
#define CRTS_ssync_2spc()\
({\
    asm volatile ("rcsr      $1, 0            \n"\
                  "ldi       $2, 0x0f         \n"\
                  "ldi       $3, 0xf0         \n"\
                  "and       $1, 0x4, $4      \n"\
                  "xor       $1, 0x8, $1      \n"\
                  "seleq     $4, $2, $3, $2   \n"\
                  "synr      $2               \n"\
                  "synp      $1               \n"\
                  :::"memory", "$1", "$2", "$3", "$4");\
    0;\
})

#define CRTS_ssync_2spe()\
({\
    asm volatile ("rcsr      $1, 0            \n"\
                  "xor       $1, 0x1, $2      \n"\
                  "synp      $2               \n"\
                  :::"memory", "$1", "$2");\
    0;\
})

#define CRTS_ssync_32spe()\
({\
    asm volatile ("rcsr      $1, 1            \n"\
                  "ldi       $2, 0x0f         \n"\
                  "ldi       $3, 0xf0         \n"\
                  "and       $1, 0x4, $1      \n"\
                  "seleq     $1, $2, $3,  $2  \n"\
                  "ldi       $1, 0xff         \n"\
                  "sync      $2               \n"\
                  "synr      $1               \n"\
                  :::"memory", "$1", "$2", "$3");\
    0;\
})

/* same as CRTS_ssync_16spe() */
#define CRTS_ssync_4spc()\
({\
    asm volatile ("rcsr      $1, 0            \n"\
                  "xor       $1, 0x8, $1      \n"\
                  "ldi       $2, 0xff         \n"\
                  "synr      $2               \n"\
                  "synp      $1               \n"\
                  :::"memory", "$1", "$2");\
    0;\
})

/* 4 spes in 2 clusters: 4spes x 1 (half of row) */
#define CRTS_ssync_4spe()\
({\
    asm volatile ("rcsr      $1, 0            \n"\
                  "xor       $1, 0x1, $2      \n"\
                  "xor       $1, 0x2, $1      \n"\
                  "synp      $2               \n"\
                  "synp      $1               \n"\
                  :::"memory", "$1", "$2");\
    0;\
})

/* same as CRTS_ssync_32spe() */
#define CRTS_ssync_8spc()\
({\
    asm volatile ("rcsr      $1, 1            \n"\
                  "ldi       $2, 0x0f         \n"\
                  "ldi       $3, 0xf0         \n"\
                  "and       $1, 0x4, $1      \n"\
                  "seleq     $1, $2, $3,  $2  \n"\
                  "ldi       $1, 0xff         \n"\
                  "sync      $2               \n"\
                  "synr      $1               \n"\
                  :::"memory", "$1", "$2", "$3");\
    0;\
})

/* 8 spes in 4 clusters: 8spes x 1 (the same row) */
#define CRTS_ssync_8spe()\
({\
    asm volatile ("ldi       $1,  0xff        \n"\
                  "synr      $1               \n"\
                  :::"memory", "$1");\
    0;\
})

#define CRTS_ssync_array()\
({\
	 asm volatile("ldi  $1,  0xff\n"\
			 "synr $1       \n"\
			 "sync $1       \n"\
			 :::"memory", "$1");\
	 0;\
})
/* Modified by QP, 2017-1010 */
/*
#define CRTS_ssync_array()\
({\
    if (crts_thrd_info.array_tsize == CRTS_MAX_SPE_NUM)\
    {\
        asm volatile("ldi  $1,  0xff\n"\
                     "synr $1       \n"\
                     "sync $1       \n"\
                     :::"memory", "$1");\
    }\
    else\
    {\
        asm volatile("synr %0          \n"\
                     "sync %1          \n"\
                     :\
                     :"r"(crts_thrd_info.row_map),\
                      "r"(crts_thrd_info.col_map));\
    }\
    0;\
})
*/

#define CRTS_ssync_sldm() \
({\
   int _ret = 0;\
   switch(_ldm_share_mode)\
   {\
   case 0:\
   CRTS_ssync_1spc();  break;\
   case 1:\
   CRTS_ssync_2spc();  break;\
   case 2:\
   CRTS_ssync_4spc();  break;\
   case 4:\
   CRTS_ssync_array(); break;\
   case 5:\
   CRTS_ssync_array(); break;\
   default:\
   CRTS_ssync_array();\
   }\
   _ret;\
})

#define CRTS_ssync_col()\
({\
    asm volatile("ldi  $1, 0xff \n"\
                 "sync $1       \n"\
                 :::"memory", "$1");\
    0;\
})

#define CRTS_ssync_col_left()\
({\
    asm volatile("ldi  $1, 0x0f \n"\
                 "sync $1       \n"\
                 :::"memory", "$1");\
    0;\
})

#define CRTS_ssync_col_right()\
({\
    asm volatile("ldi  $1, 0xf0 \n"\
                 "sync $1       \n"\
                 :::"memory", "$1");\
    0;\
})

extern __atomic volatile int64_t __crts_master_spe_sync[CRTS_MAX_SPE_NUM];
extern __atomic volatile int64_t __crts_master_array_sync;
extern __atomic volatile int64_t __crts_master_spe_sync_all[CRTS_MAX_MPE_NUM][CRTS_MAX_SPE_NUM];
extern __atomic volatile int64_t __crts_master_array_sync_all[CRTS_MAX_MPE_NUM];
#define CRTS_ssync_master_spe() \
{\
  __crts_master_spe_sync[_PEN]++;\
  while((__crts_master_spe_sync[_PEN] % 2) != 0); \
}
#define CRTS_ssync_master_spe_share() \
{\
  __crts_master_spe_sync_all[_CGN][_PEN]++;\
  while((__crts_master_spe_sync_all[_CGN][_PEN] % 2) != 0); \
}

/* only SPEs in the same node, MPEs not included */
#define CRTS_ssync_node() \
({\
   uint64_t rv, ok_cnt, node_size;\
   CRTS_ssync_array ();\
   if (CRTS_array_trank == 0)\
   {\
   node_size = CRTS_node_tsize / CRTS_array_tsize;\
   rv = _crts_faa_long ( &(crts_node_cross_info.slave_sync) );\
   ok_cnt = rv/node_size*node_size + node_size;\
   while ( crts_node_cross_info.slave_sync < ok_cnt);\
   }\
   CRTS_ssync_array ();\
   0;\
})

#define CRTS_ssync_master_array() \
{\
  CRTS_ssync_array (); \
  if(_PEN == 0)\
  {\
	__crts_master_array_sync++;\
	while((__crts_master_array_sync % 2) != 0); \
  }\
  CRTS_ssync_array();\
}
#define CRTS_ssync_master_array_share() \
{\
  CRTS_ssync_array (); \
  if(_PEN == 0)\
  {\
	__crts_master_array_sync_all[_CGN]++;\
	while((__crts_master_array_sync_all[_CGN] % 2) != 0); \
  }\
  CRTS_ssync_array();\
}



#define CRTS_ssync_peer(tid)\
  ({\
   asm volatile ("synp  %0\n"::"r" (tid):"memory");\
   })

#define CRTS_ssync_row()\
  ({\
   asm volatile("ldi  $1, 0xff \n"\
	 "synr $1       \n"\
	 :::"memory", "$1");\
   0;\
   })


//rma
//#include "crts_rma.h"
/*NOTE:
    row RMA bcast: bcast data to SPEs on the same row
    col RMA bcast: bcast data to SPEs on the same column
    row RMA mcast: bcast data to SPEs on the rows specified by `mask`
    col RMA mcast: bcast data to SPEs on the columns specified by `mask`

  WARNING:
    RMA bcast: the SPE which lauches RMA DOES receive data.
    RMA mcast: the SPE which lauches RMA does NOT receive data.
*/

extern __thread_local_fix uint64_t __crts_reg_tmp[2];
extern __thread_local_fix uint64_t __crts_RB;
extern __thread_local_fix uint64_t __crts_RC;
extern __thread_local_fix crts_rply_t __crts_rply;
extern __thread_local_fix crts_rply_t __crts_rma_rply;

#define RMA_MODE_SINGLE_CORE                    0ULL
#define RMA_MODE_ROW_BCAST                      1ULL
#define RMA_MODE_COL_BCAST                      2ULL
#define RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA   5ULL
#define RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA   6ULL

#define RMA_OPCODE_PUT          0ULL
#define RMA_OPCODE_GET          1ULL
#define RMA_OPCODE_BARRIER      5ULL
#define RMA_OPCODE_ALL_BARRIER  6ULL

#define __macro_crts_rma(R_tid,Laddr,Raddr,size,Lrply,Rrply,mode,opcode,mask) \
({\
    __crts_reg_tmp[1]=(uint64_t)(((((uint64_t)(Rrply))&0x3ffffUL)<<32)  \
                      |	(((uint64_t)(mask))<<24)                        \
                      | (((uint64_t)(Lrply))&0x3ffff));                 \
    __crts_reg_tmp[0]=(uint64_t)((((uint64_t)(mode))<<60)               \
                      | (((uint64_t)(opcode))<<56) | (uint64_t)(size)); \
    __crts_RB = ((uint64_t)(R_tid)<<20)                                 \
                | (((uint64_t)(Raddr))&0x3ffffUL);                      \
    __crts_RC = ((uint64_t)(Laddr))&0x3ffffUL;                          \
    __asm__("vldd $32,0(%0)\n"                                          \
            "rma  $32,%1,%2 \n"                                         \
            :                                                           \
            :"r"(__crts_reg_tmp),"r"(__crts_RB),"r"(__crts_RC)          \
            :"memory","$32");                                           \
    0;\
})

#define CRTS_rma_all_barrier()\
({\
    uint64_t tmp_ld;\
    __crts_reg_tmp[0] = RMA_OPCODE_ALL_BARRIER << 56;\
    __asm__("vldd $31,0(%1)\n"\
            "rma  $31,%2,%3\n"\
            :"=&r"(tmp_ld)\
            :"r"(__crts_reg_tmp), "r"(__crts_RB), "r"(__crts_RC)\
            :"memory", "$0");\
    0;\
})

#define CRTS_rma_barrier()\
({\
    uint64_t tmp_ld;\
    __crts_reg_tmp[0] = RMA_OPCODE_BARRIER << 56;\
    __asm__("vldd $31,0(%1)\n"\
            "rma  $31,%2,%3\n"\
            :"=&r"(tmp_ld)\
            :"r"(__crts_reg_tmp), "r"(__crts_RB), "r"(__crts_RC)\
            :"memory", "$0");\
    0;\
})

#define CRTS_rma_bcast(dst, src, len, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply, r_rply,\
                     RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 0xff);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_bcast_coll(dst, src, len, root)\
({\
    __crts_rply = 0;\
    __crts_rma_rply = 0;\
    CRTS_ssync_array ();\
    if (CRTS_tid == (root)) {\
        __macro_crts_rma(root, src, dst, len, &__crts_rply,\
                         &__crts_rma_rply,\
                         RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                         RMA_OPCODE_PUT, 0xff);\
    }\
    while (__crts_rma_rply == 0);\
    CRTS_ssync_array ();\
    0;\
})

#define CRTS_rma_col_bcast(dst, src, len, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply,\
                     r_rply, RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 1<<CRTS_cid);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_col_bcast_coll(dst, src, len, col_root)\
({\
    __crts_rply = 0;\
    __crts_rma_rply = 0;\
    CRTS_ssync_col ();\
    if (CRTS_rid == (col_root))\
    {\
        __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply,\
                         &__crts_rma_rply,\
                         RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                         RMA_OPCODE_PUT, 1<<CRTS_cid);\
    }\
    while (__crts_rma_rply == 0);\
    CRTS_ssync_col ();\
    0;\
})

#define CRTS_rma_col_ibcast(dst, src, len, l_rply, r_rply)\
({\
    __macro_crts_rma(CRTS_tid, src, dst, len, l_rply, r_rply,\
                     RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 1<<CRTS_cid);\
    0;\
})

#define CRTS_rma_col_imcast(dst, src, len, l_rply,\
                            r_cols_mask, r_rply)\
({\
    __macro_crts_rma(CRTS_tid, src, dst, len, l_rply, r_rply,\
                     RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, r_cols_mask);\
    0;\
})

#define CRTS_rma_col_mcast(dst, src, len, r_cols_mask, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply, r_rply,\
                     RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, r_cols_mask);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_get(l_addr, len, r_tid, r_addr, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma((r_tid), (l_addr), (r_addr), (len), &__crts_rply,\
                     (r_rply), RMA_MODE_SINGLE_CORE, RMA_OPCODE_GET,\
                     0);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_ibcast(dst, src, l_rply, len, r_rply)\
({\
    __macro_crts_rma(CRTS_tid, src, dst, len, l_rply, r_rply,\
                     RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 0xff);\
    0;\
})

#define CRTS_rma_bcast_other(dst, src, len, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply, r_rply,\
                     RMA_MODE_COL_BCAST,\
                     RMA_OPCODE_PUT, 0xff);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_bcast_coll_other(dst, src, len, root)\
({\
    __crts_rply = 0;\
    __crts_rma_rply = 0;\
    CRTS_ssync_array ();\
    if (CRTS_tid == (root)) {\
        __macro_crts_rma(root, src, dst, len, &__crts_rply,\
                         &__crts_rma_rply,\
                         RMA_MODE_COL_BCAST,\
                         RMA_OPCODE_PUT, 0xff);\
    }\
    while (__crts_rma_rply == 0);\
    CRTS_ssync_array ();\
    0;\
})

#define CRTS_rma_col_bcast_other(dst, src, len, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply,\
                     r_rply, RMA_MODE_COL_BCAST,\
                     RMA_OPCODE_PUT, 1<<CRTS_cid);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_col_bcast_coll_other(dst, src, len, col_root)\
({\
    __crts_rply = 0;\
    __crts_rma_rply = 0;\
    CRTS_ssync_col ();\
    if (CRTS_rid == (col_root))\
    {\
        __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply,\
                         &__crts_rma_rply,\
                         RMA_MODE_COL_BCAST,\
                         RMA_OPCODE_PUT, 1<<CRTS_cid);\
    }\
    while (__crts_rma_rply == 0);\
    CRTS_ssync_col ();\
    0;\
})

#define CRTS_rma_col_ibcast_other(dst, src, len, l_rply, r_rply)\
({\
    __macro_crts_rma(CRTS_tid, src, dst, len, l_rply, r_rply,\
                     RMA_MODE_COL_BCAST,\
                     RMA_OPCODE_PUT, 1<<CRTS_cid);\
    0;\
})

#define CRTS_rma_col_imcast_other(dst, src, len, l_rply,\
                            r_cols_mask, r_rply)\
({\
    __macro_crts_rma(CRTS_tid, src, dst, len, l_rply, r_rply,\
                     RMA_MODE_COL_BCAST,\
                     RMA_OPCODE_PUT, r_cols_mask);\
    0;\
})

#define CRTS_rma_col_mcast_other(dst, src, len, r_cols_mask, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply, r_rply,\
                     RMA_MODE_COL_BCAST,\
                     RMA_OPCODE_PUT, r_cols_mask);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_iget(l_addr, l_rply, len, r_tid, r_addr, r_rply)\
({\
    __macro_crts_rma(r_tid, l_addr, r_addr, len, l_rply, r_rply,\
                     RMA_MODE_SINGLE_CORE, RMA_OPCODE_GET, 0);\
    0;\
})

#define CRTS_rma_iput(l_addr, l_rply, len, r_tid, r_addr, r_rply)\
({\
    __macro_crts_rma(r_tid, l_addr, r_addr, len, l_rply, r_rply,\
                     RMA_MODE_SINGLE_CORE, RMA_OPCODE_PUT, 0);\
    0;\
})

#define CRTS_rma_put(l_addr, len, r_tid, r_addr, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(r_tid, l_addr, r_addr, len, &__crts_rply,\
                     r_rply, RMA_MODE_SINGLE_CORE,\
                     RMA_OPCODE_PUT, 0);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_row_bcast(dst, src, len, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply, r_rply,\
                     RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 1<<CRTS_rid);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_row_bcast_coll(dst, src, len, row_root)\
({\
    __crts_rply = 0;\
    __crts_rma_rply = 0;\
    CRTS_ssync_row ();\
    if (CRTS_cid == (row_root))\
    {\
        __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply,\
                         &__crts_rma_rply,\
                         RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA,\
                         RMA_OPCODE_PUT, 1<<CRTS_rid);\
    }\
    while (__crts_rma_rply == 0);\
    CRTS_ssync_row ();\
    0;\
})

#define CRTS_rma_row_ibcast(dst, src, len, l_rply, r_rply)\
({\
    __macro_crts_rma(CRTS_tid, src, dst, len, l_rply, r_rply,\
                     RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 1<<CRTS_rid);\
    0;\
})

#define CRTS_rma_row_imcast(dst, src, len, l_rply,\
                            r_rows_mask, r_rply)\
({\
    __macro_crts_rma(CRTS_tid, src, dst, len, l_rply, r_rply,\
                     RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, r_rows_mask);\
    0;\
})

#define CRTS_rma_row_mcast(dst, src, len, r_rows_mask, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply, r_rply,\
                     RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, r_rows_mask);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_row_bcast_other(dst, src, len, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply, r_rply,\
                     RMA_MODE_ROW_BCAST,\
                     RMA_OPCODE_PUT, 1<<CRTS_rid);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_row_bcast_coll_other(dst, src, len, row_root)\
({\
    __crts_rply = 0;\
    __crts_rma_rply = 0;\
    CRTS_ssync_row ();\
    if (CRTS_cid == (row_root))\
    {\
        __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply,\
                         &__crts_rma_rply,\
                         RMA_MODE_ROW_BCAST,\
                         RMA_OPCODE_PUT, 1<<CRTS_rid);\
    }\
    while (__crts_rma_rply == 0);\
    CRTS_ssync_row ();\
    0;\
})

#define CRTS_rma_row_ibcast_other(dst, src, len, l_rply, r_rply)\
({\
    __macro_crts_rma(CRTS_tid, src, dst, len, l_rply, r_rply,\
                     RMA_MODE_ROW_BCAST,\
                     RMA_OPCODE_PUT, 1<<CRTS_rid);\
    0;\
})

#define CRTS_rma_row_imcast_other(dst, src, len, l_rply,\
                            r_rows_mask, r_rply)\
({\
    __macro_crts_rma(CRTS_tid, src, dst, len, l_rply, r_rply,\
                     RMA_MODE_ROW_BCAST,\
                     RMA_OPCODE_PUT, r_rows_mask);\
    0;\
})

#define CRTS_rma_row_mcast_other(dst, src, len, r_rows_mask, r_rply)\
({\
    __crts_rply = 0;\
    __macro_crts_rma(CRTS_tid, src, dst, len, &__crts_rply, r_rply,\
                     RMA_MODE_ROW_BCAST,\
                     RMA_OPCODE_PUT, r_rows_mask);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_rma_wait_value(rply, value)\
({\
    while (*(crts_rply_t*)(rply) < value);\
    0;\
})


//dma
//#include "crts_dma.h"
#include <slave.h>
#include <stdio.h>
#include <stdint.h>
#ifndef __cplusplus
#ifdef __sw_slave_ioproxy__
extern int my_printf(const char* ss,...);
#define printf my_printf
#endif
#endif
extern __thread_local_fix uint64_t __crts_reg_tmp[2] __attribute__ ((aligned(64)));
extern __thread_local_fix uint64_t __crts_RB;
extern __thread_local_fix uint64_t __crts_RC;
extern __thread_local_fix crts_rply_t __crts_rply;
extern __thread_local_fix crts_rply_t __crts_rma_rply;
extern __thread_local_fix int __ldm_size;
extern __thread_local_fix int __ldm_stack_size;

extern int CRTS_ssig_queue( int pid, int signum, const sigval_t value );
#define CRTS_ssig_host(content) \
({\
   int rv; \
   sigval_t sv;\
   sv.sival_ptr = content;\
   rv = CRTS_ssig_queue(crts_thrd_info_h.pinfo.pid, SIG_CUSTOM, (const union sigval)sv);\
})
//#define DMA_OP_PUT          0ULL
//#define DMA_OP_GET          1ULL
#define DMA_PUT_PHYSICAL 2ULL
#define DMA_GET_PHYSICAL 3ULL
#define DMA_OP_BARRIER      5ULL
#define DMA_OP_ALL_BARRIER  6ULL

#define DMA_MODE_SINGLE_CORE 0
#define DMA_MODE_ROW_BCAST   1
#define DMA_MODE_COL_BCAST   2

#define CRTS_MEMB()     asm volatile("memb\n")

#define __macro_crts_dma(Maddr,Laddr,size,rply,bsize,stride,direction)      \
({                                                                          \
    __crts_reg_tmp[1] = ((uint64_t)(stride)<<32)                            \
                        | ((uint64_t)(rply)&0x3ffff);                       \
    __crts_reg_tmp[0] = (((uint64_t)(direction)&0xf)<<56)                   \
                        | (((uint64_t)(bsize)&0x3ffff)<<32)                 \
                        | ((uint64_t)(size)&0x3ffff);                       \
    __crts_RB = (uint64_t)(Maddr)&0xffffffffffffULL;                        \
    __crts_RC = (uint64_t)(Laddr)&0x3ffff;                                  \
    __asm__("vldd $32,0(%0)\n"                                              \
            "dma  $32,%1,%2 \n"                                             \
            :                                                               \
            :"r"(__crts_reg_tmp),"r"(__crts_RB),"r"(__crts_RC)              \
            :"memory","$32");                                               \
    0;                                                                      \
})

#define __macro_crts_dma_mcast(Maddr,Laddr,size,rply,bsize,stride,mode,mask)\
({                                                                          \
    __crts_reg_tmp[1] = ((uint64_t)(stride)<<32)                            \
                        | ((uint64_t)((mask)&0xff)<<24)                     \
                        | ((uint64_t)(rply)&0x3ffff);                       \
    __crts_reg_tmp[0] = ((uint64_t)(mode)<<60) | (1ULL<<56)                 \
                        | ((uint64_t)(bsize)<<32) | (uint64_t)(size);       \
    __crts_RB = (uint64_t)(Maddr)&0xffffffffffffULL;                        \
    __crts_RC = (uint64_t)(Laddr)&0x3ffff;                                  \
    __asm__("vldd $32,0(%0)\n"                                              \
            "dma  $32,%1,%2 \n"                                             \
            :                                                               \
            :"r"(__crts_reg_tmp),"r"(__crts_RB),"r"(__crts_RC)              \
            :"memory","$32");                                               \
    0;                                                                      \
})

/* Added by QP, 2017-1018: for athread_put/get */
#define __macro_crts_dma_raw(Maddr,Laddr,size,rply,bsize,stride,            \
                             direction,mode,mask)                           \
({                                                                          \
    __crts_reg_tmp[1] = ((uint64_t)(stride)<<32)                            \
                        | ((uint64_t)((mask)&0xff)<<24)                     \
                        | ((uint64_t)(rply)&0x3ffff);                       \
    __crts_reg_tmp[0] = ((uint64_t)(mode)<<60)                              \
                        | ((uint64_t)((direction)&0xf)<<56)                 \
                        | ((uint64_t)(bsize)<<32) | (uint64_t)(size);       \
    __crts_RB = (uint64_t)(Maddr)&0xffffffffffffULL;                        \
    __crts_RC = (uint64_t)(Laddr)&0x3ffff;                                  \
    __asm__("vldd $32,0(%0)\n"                                              \
            "dma  $32,%1,%2 \n"                                             \
            :                                                               \
            :"r"(__crts_reg_tmp),"r"(__crts_RB),"r"(__crts_RC)              \
            :"memory","$32");                                               \
    0;                                                                      \
})

/*
 * void __builtin_sw_slave_athread_dma (mode, op, &mem, &ldm, len, 
 *                                      &rply, mask, stride, bsize)
 */
#define BUILTIN_DMA  __builtin_sw_slave_athread_dma

#define CRTS_dma_get(dest, src, len)\
({\
    __crts_rply = 0;\
    BUILTIN_DMA (PE_MODE, DMA_GET, (src), (dest), (len),\
                 (void*)&__crts_rply, 0, 0, 0);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_dma_get_stride(dest, src, len, bsize, stride)\
({\
    __crts_rply = 0;\
    BUILTIN_DMA (PE_MODE, DMA_GET, (src), (dest), (len),\
                 (void*)&__crts_rply, 0, stride, bsize);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_dma_ibcast(dest, src, len, reply)\
({\
    CRTS_dma_get(dest, src, len);\
    __macro_crts_rma(CRTS_tid, dest, dest, len,\
                     &__crts_rma_rply, reply,\
                     RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 0xff);\
    0;\
})

#define CRTS_dma_ibcast_stride(dest, src, len, bsize, stride, reply)\
({\
 	CRTS_dma_get_stride(dest, src, len, bsize, stride);\
    __macro_crts_rma(CRTS_tid, dest, dest, len,\
                     &__crts_rma_rply, reply,\
                     RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 0xff);\
    0;\
})

#define CRTS_dma_iget(dest, src, len, reply)\
({\
    BUILTIN_DMA (PE_MODE, DMA_GET, src, dest, len,\
                 (void*)(reply), 0, 0, 0);\
    0;\
})

#define CRTS_dma_iget_stride(dest, src, len, bsize, stride, reply)\
({\
    BUILTIN_DMA (PE_MODE, DMA_GET, src, dest, len,\
                 (void*)(reply), 0, stride, bsize);\
    0;\
})

#define CRTS_dma_iput(dest, src, len, reply)\
({\
    BUILTIN_DMA (PE_MODE, DMA_PUT, dest, src, len,\
                 (void*)(reply), 0, 0, 0);\
    0;\
})

#define CRTS_dma_iput_stride(dest, src, len, bsize, stride, reply)\
({\
    BUILTIN_DMA (PE_MODE, DMA_PUT, dest, src, len,\
                 (void*)(reply), 0, stride, bsize);\
    0;\
})

#define CRTS_dma_put(dest, src, len)\
({\
    __crts_rply = 0;\
    BUILTIN_DMA (PE_MODE, DMA_PUT, (dest), (src), (len),\
                 (void*)&__crts_rply, 0, 0, 0);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_dma_put_stride(dest, src, len, bsize, stride)\
({\
    __crts_rply = 0;\
    BUILTIN_DMA (PE_MODE, DMA_PUT, dest, src, len,\
                 (void*)&__crts_rply, 0, stride, bsize);\
    while (__crts_rply == 0);\
    0;\
})
#define MEM_TO_CACHE    0x90001000U
#define CACHE_TO_MEM    0x90002000U
#define MEM_TO_LDM      MEM_TO_CACHE
#define LDM_TO_MEM      CACHE_TO_MEM
extern int  CRTS_memcpy_sldm(void *dest_addr, void *src_addr, int size, int direction);
extern int  CRTS_sldm_get (void *dest_addr, void *src_addr, int size);
extern void  CRTS_sldm_get_arrayshare (void *dest_addr, void *src_addr, int size);
extern void CRTS_sio_fwrite(char* filename, void* src, unsigned long size);
/* ------------------------------------------------*/
/* 2018-1024, DMA functions using physical address */
/* ------------------------------------------------*/
#define CRTS_dma_phy_get(dest, src, len)\
({\
    __crts_rply = 0;\
    BUILTIN_DMA (PE_MODE, DMA_GET_PHYSICAL, src, dest, len,\
                 (void*)&__crts_rply, 0, 0, 0);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_dma_phy_get_stride(dest, src, len, bsize, stride)\
({\
    __crts_rply = 0;\
    BUILTIN_DMA (PE_MODE, DMA_GET_PHYSICAL, src, dest, len,\
                 (void*)&__crts_rply, 0, stride, bsize);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_dma_phy_iget(dest, src, len, reply)\
({\
    BUILTIN_DMA (PE_MODE, DMA_GET_PHYSICAL, src, dest, len,\
                 (void*)(reply), 0, 0, 0);\
    0;\
})

#define CRTS_dma_phy_iget_stride(dest, src, len, bsize, stride, reply)\
({\
    BUILTIN_DMA (PE_MODE, DMA_GET_PHYSICAL, src, dest, len,\
                 (void*)(reply), 0, stride, bsize);\
    0;\
})

#define CRTS_dma_phy_iput(dest, src, len, reply)\
({\
    BUILTIN_DMA (PE_MODE, DMA_PUT_PHYSICAL, dest, src, len,\
                 (void*)(reply), 0, 0, 0);\
    0;\
})

#define CRTS_dma_phy_iput_stride(dest, src, len, bsize, stride, reply)\
({\
    BUILTIN_DMA (PE_MODE, DMA_PUT_PHYSICAL, dest, src, len,\
                 (void*)(reply), 0, stride, bsize);\
    0;\
})

#define CRTS_dma_phy_put(dest, src, len)\
({\
    __crts_rply = 0;\
    BUILTIN_DMA (PE_MODE, DMA_PUT_PHYSICAL, dest, src, len,\
                 (void*)&__crts_rply, 0, 0, 0);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_dma_phy_put_stride(dest, src, len, bsize, stride)\
({\
    __crts_rply = 0;\
    BUILTIN_DMA (PE_MODE, DMA_PUT_PHYSICAL, dest, src, len,\
                 (void*)&__crts_rply, 0, stride, bsize);\
    while (__crts_rply == 0);\
    0;\
})

#define CRTS_dma_all_barrier()\
({\
    BUILTIN_DMA (PE_MODE, ALL_BARRIER, 0, 0, 0, 0, 0, 0, 0);\
    0;\
})

#define CRTS_dma_barrier()\
({\
    BUILTIN_DMA (PE_MODE, DMA_BARRIER, 0, 0, 0, 0, 0, 0, 0);\
    0;\
})

#define CRTS_dma_bcast_coll(dest, src, len)\
({\
    __crts_rma_rply = 0;\
    CRTS_ssync_array ();\
    if (CRTS_tid == 0)\
    {\
        CRTS_dma_get(dest, src, len);\
        __macro_crts_rma(CRTS_tid, dest, dest, len,\
                         &__crts_rply, &__crts_rma_rply,\
                         RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                         RMA_OPCODE_PUT, 0xff);\
    }\
    while (__crts_rma_rply == 0);\
    0;\
})

#define CRTS_dma_bcast_stride_coll(dest, src, len, bsize, stride)\
({\
    __crts_rma_rply = 0;\
    CRTS_ssync_array ();\
    if (CRTS_tid == 0)\
    {\
        CRTS_dma_get_stride(dest, src, len, bsize, stride);\
        __macro_crts_rma(CRTS_tid, dest, dest, len,\
                         &__crts_rply, &__crts_rma_rply,\
                         RMA_MODE_COL_BCAST_LOCAL_RECEIVE_DATA,\
                         RMA_OPCODE_PUT, 0xff);\
    }\
    while (__crts_rma_rply == 0);\
    0;\
})

#define CRTS_dma_row_bcast_coll(dest, src, len)\
({\
    __crts_rma_rply = 0;\
    CRTS_ssync_row ();\
    if (CRTS_cid == 0)\
    {\
    	CRTS_dma_get(dest, src, len);\
    	__macro_crts_rma(CRTS_tid, dest, dest, len, &__crts_rply, &__crts_rma_rply,\
        	             RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA,\
            	         RMA_OPCODE_PUT, 1<<CRTS_rid);\
    }\
    while (__crts_rma_rply == 0);\
    CRTS_ssync_row ();\
    0;\
})

#define CRTS_dma_row_bcast_stride_coll(dest, src, len, bsize, stride)\
({\
    __crts_rma_rply = 0;\
    CRTS_ssync_row ();\
    CRTS_dma_get_stride(dest, src, len, bsize, stride);\
    if (CRTS_cid == 0)\
    {\
    	__macro_crts_rma(CRTS_tid, dest, dest, len,\
        	             &__crts_rply, &__crts_rma_rply,\
            	         RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA,\
                	     RMA_OPCODE_PUT, 1<<CRTS_rid);\
    }\
    while (__crts_rma_rply == 0);\
    CRTS_ssync_row ();\
    0;\
})

#define CRTS_dma_col_bcast_coll(dest, src, len)\
({\
 	__crts_rply = 0;\
    CRTS_ssync_col ();\
    if (CRTS_rid == 0)\
    {\
        BUILTIN_DMA (COL_MODE, DMA_GET, src, dest, len, \
                     (void*)&__crts_rply, 1<<CRTS_cid, 0, 0);\
    }\
 	while (__crts_rply == 0);\
    CRTS_ssync_col();\
    0;\
})

#define CRTS_dma_col_bcast_stride_coll(dest, src, len, bsize, stride)\
({\
 	__crts_rply = 0;\
    CRTS_ssync_col ();\
    if (CRTS_rid == 0)\
    {\
        BUILTIN_DMA (COL_MODE, DMA_GET, src, dest, len,\
                     (void*)&__crts_rply, 1<<CRTS_cid,\
                     stride, bsize);\
    }\
 	while (__crts_rply == 0);\
    CRTS_ssync_col();\
    0;\
})

#define CRTS_dma_col_ibcast(dest, src, len, reply)\
({\
    BUILTIN_DMA (COL_MODE, DMA_GET, src, dest, len,\
                 (void*)(reply), 1<<CRTS_cid, 0, 0);\
    0;\
})

/////////////add by dem//////////////////////
#define CRTS_dma_row_ibcast(dest, src, len, reply)\
({\
    CRTS_dma_get(dest, src, len);\
    __macro_crts_rma(CRTS_tid, dest, dest, len, &__crts_rply, reply,\
                     RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 1<<CRTS_rid);\
    0;\
})
#define CRTS_dma_row_ibcast_stride(dest, src, len, bsize, stride, reply)\
({\
    CRTS_dma_get_stride(dest, src, len, bsize, stride);\
    __macro_crts_rma(CRTS_tid, dest, dest, len,\
                     &__crts_rply, reply,\
                     RMA_MODE_ROW_BCAST_LOCAL_RECEIVE_DATA,\
                     RMA_OPCODE_PUT, 1<<CRTS_rid);\
    0;\
})

#define CRTS_dma_col_ibcast_stride(dest, src, len,\
                                   bsize, stride, reply)\
({\
    BUILTIN_DMA (COL_MODE, DMA_GET, src, dest, len, \
                 (void*)(reply), 1<<CRTS_cid, stride, bsize);\
    0;\
})

/* ------------------------------------------------*/
/* ------------------------------------------------*/
/* row/col bcast DMA using physical address */

#define CRTS_dma_phy_col_bcast_coll(dest, src, len)\
({\
    __crts_rply = 0;\
    CRTS_ssync_col();\
    if (CRTS_rid == 0)\
    {\
        BUILTIN_DMA (COL_MODE, DMA_GET_PHYSICAL, src, dest, len, \
                     (void*)&__crts_rply, 1<<CRTS_cid, 0, 0);\
    }\
    while (__crts_rply == 0);\
    CRTS_ssync_col();\
    0;\
})

#define CRTS_dma_phy_col_bcast_stride_coll(dest, src, len, bsize, stride)\
({\
    __crts_rply = 0;\
    CRTS_ssync_col();\
    if (CRTS_rid == 0)\
    {\
        BUILTIN_DMA (COL_MODE, DMA_GET_PHYSICAL, src, dest, len,\
                     (void*)&__crts_rply, 1<<CRTS_cid,\
                     stride, bsize);\
    }\
    while (__crts_rply == 0);\
    CRTS_ssync_col();\
    0;\
})

#define CRTS_dma_phy_col_ibcast(dest, src, len, reply)\
({\
    BUILTIN_DMA (COL_MODE, DMA_GET_PHYSICAL, src, dest, len,\
                 (void*)(reply), 1<<CRTS_cid, 0, 0);\
    0;\
})

#define CRTS_dma_phy_col_ibcast_stride(dest, src, len,\
                                   bsize, stride, reply)\
({\
    BUILTIN_DMA (COL_MODE, DMA_GET_PHYSICAL, src, dest, len, \
                 (void*)(reply), 1<<CRTS_cid, stride, bsize);\
    0;\
})
/* ------------------------------------------------*/
/* ------------------------------------------------*/

#define CRTS_dma_wait_value(reply, value)\
({\
    while (*(volatile crts_rply_t*)(reply) < value);\
    0;\
})


//#define athread_dma_barrier() CRTS_dma_barrier()
#if 0
int athread_get (dma_mode mode, void *src, void *dest, int len,
                 crts_rply_t *reply, char mask, int stride, int bsize);
int athread_put (dma_mode mode, void *src, void *dest, int len,
                 crts_rply_t *reply, int stride, int bsize);
#else
#if 0
#define athread_get( mode, src, dest, len, reply, mask, stride, bsize)\
    BUILTIN_DMA( mode, DMA_GET, src, dest, len, (void*)(reply), mask, stride, bsize );\


#define athread_put( mode, src, dest, len, reply, stride, bsize)\
    BUILTIN_DMA( PE_MODE, DMA_PUT, dest, src, len, (void*)(reply), 0, stride, bsize );
#else
#define athread_get( mode, src, dest, len, reply, mask, stride, bsize)\
({\
    BUILTIN_DMA( mode, DMA_GET, src, dest, len, (void*)(reply), mask, stride, bsize );\
    0;\
})

#define athread_put( mode, src, dest, len, reply, stride, bsize)\
({\
    BUILTIN_DMA( PE_MODE, DMA_PUT, dest, src, len, (void*)(reply), 0, stride, bsize );\
    0;\
})
#endif

#endif
#define CRTS_smutex_lock_custom   lock_long
#define CRTS_smutex_unlock_custom unlock_long
#define lock_long(lockptr)\
({\
    int ret = 1;\
        do {\
                  ret = _crts_faa_long(lockptr);\
                      } while (ret != 0);\
                          0;\
})

#define unlock_long(lockptr)\
({\
    *(lockptr) = 0;\
        asm volatile ("memb\n");\
            0;\
})

#define CRTS_smutex_lock_8spe()     CRTS_smutex_lock_row()
#define CRTS_smutex_unlock_8spe()   CRTS_smutex_unlock_row()
#define CRTS_smutex_lock_4spc()     CRTS_smutex_lock_16spe()
#define CRTS_smutex_unlock_4spc()   CRTS_smutex_unlock_16spe()
#define CRTS_smutex_lock_8spc()     CRTS_smutex_lock_32spe()
#define CRTS_smutex_unlock_8spc()   CRTS_smutex_unlock_32spe()

//extern int CRTS_ssig_queue( int pid, int signum, const sigval_t value );
int CRTS_smutex_lock_node (void);
int CRTS_smutex_unlock_node (void);
int CRTS_smutex_lock_master_array(void);
int CRTS_smutex_unlock_master_array(void);
void CRTS_smutex_lock_node_all (void);
void CRTS_smutex_unlock_node_all (void);
//LDM malloc
//void print_ldm_malloc_state(void);
#define CRTS_get_free_addr get_allocatable_addr
uint32_t get_allocatable_size(void);
void init_ldm_stack_state(void);
void* get_allocatable_addr(void);
void* CRTS_pldm_malloc(size_t size);
void  CRTS_pldm_free(void *addr, size_t size);

//slave lock
void CRTS_initialize_lock_vars(void);
int CRTS_smutex_lock_array(void);
int CRTS_smutex_unlock_array(void);
int CRTS_smutex_init_array_custom(void);
int CRTS_smutex_lock_array_custom(int lockid);
int CRTS_smutex_unlock_array_custom(int lockid);
int CRTS_smutex_free_array_custom(int lockid);
int CRTS_smutex_lock_2spe(void);
int CRTS_smutex_unlock_2spe(void);
int CRTS_smutex_init_2spe_custom(void);
int CRTS_smutex_lock_2spe_custom(int lockid);
int CRTS_smutex_unlock_2spe_custom(int lockid);
int CRTS_smutex_free_2spe_custom(int lockid);
int CRTS_smutex_lock_1spc(void);
int CRTS_smutex_unlock_1spc(void);
int CRTS_smutex_init_1spc_custom(void);
int CRTS_smutex_lock_1spc_custom(int lockid);
int CRTS_smutex_unlock_1spc_custom(int lockid);
int CRTS_smutex_free_1spc_custom(int lockid);
int CRTS_smutex_lock_row(void);
int CRTS_smutex_unlock_row(void);
int CRTS_smutex_lock_col(void);
int CRTS_smutex_unlock_col(void);
int CRTS_smutex_lock_2spc(void);
int CRTS_smutex_unlock_2spc(void);
//int CRTS_smutex_init_8spe_custom(void);
//int CRTS_smutex_lock_8spe_custom(int lockid);
//int CRTS_smutex_unlock_8spe_custom(int lockid);
//int CRTS_smutex_free_8spe_custom(int lockid);
int CRTS_smutex_lock_16spe(void);
int CRTS_smutex_unlock_16spe(void);
int CRTS_smutex_init_16spe_custom(void);
int CRTS_smutex_lock_16spe_custom(int lockid);
int CRTS_smutex_unlock_16spe_custom(int lockid);
int CRTS_smutex_free_16spe_custom(int lockid);
int CRTS_smutex_lock_32spe(void);
int CRTS_smutex_unlock_32spe(void);
int CRTS_smutex_init_32spe_custom(void);
int CRTS_smutex_lock_32spe_custom(int lockid);
int CRTS_smutex_unlock_32spe_custom(int lockid);
int CRTS_smutex_free_32spe_custom(int lockid);

  
//signal
//int sigqueue( int pid, int signum, const sigval_t value );
//scache
#define CRTS_scache_get_size get_slave_cache_size
//#define CRTS_scache_flush_all flush_slave_cache
//#define CRTS_scache_evict evict_slave_cache_cont
//int  CRTS_scache_set_private (int no, void* start, void* end);
//void CRTS_scache_flush_all (void);
//void CRTS_scache_evict (void *start, void *end);

//ptab
//#ifdef _SIMD_SLAVE
typedef unsigned uintv16 __attribute__ ((__mode__(__V16SI__)));
#define CRTS_get_ptab_addr(ptable)\
({\
   uintv16  __attribute__ ((aligned (16))) re;\
   unsigned int ti_addr=((((unsigned int)&(ptable)[0])>>6)<<2); \
   re = ti_addr;\
   re;\
})


#define CRTS_ptab_lookup(voffset,vbase_addr) \
({ \
   uintv16 tmp; \
   asm volatile("PLUTW %1,%2,%0" \
	 :"=&f"(tmp) \
	 :"f" (voffset), "f"(vbase_addr));\
   tmp;\
})


#define CRTS_ptab_write(voffset,vbase_addr,value) \
({\
  asm volatile("PWRTW %2,%0,%3"\
	  :"=&f" (vbase_addr) \
	  :"0" (vbase_addr), "f"(voffset),"f"(value):"memory");\
})

extern uintv16 CRTS_ptab_cpy(unsigned int *orig_table_addr[16], int unit,int num,unsigned int *ptable);
extern uintv16 CRTS_ptab_cpy16(unsigned int *orig_table_addr, int unit,unsigned int *ptable);
//#endif
#endif    // End of __sw_host__

#endif
