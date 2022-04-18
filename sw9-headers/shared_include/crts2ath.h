//#include "crts_akernel.h"
#include <crts.h>
#ifdef __sw_host__
int  CRTS_init(void);
void CRTS_sync_node(void);
void CRTS_sync_master_spe(int tid);
void CRTS_sync_master_array(void);
int CRTS_mutex_lock_node (void);
int CRTS_mutex_trylock_node (void);
int CRTS_mutex_unlock_node (void);
int CRTS_mutex_lock(volatile int64_t *lock);
int CRTS_mutex_trylock(volatile int64_t *lock);
int CRTS_mutex_unlock(volatile int64_t *lock);
#define athread_time_cycle()                                   \
({                                                          \
    unsigned long tmp;                                      \
    asm volatile ("rtc %0\n":"=&r"(tmp) ::"memory"); tmp;   \
})
#define		athread_sync_node		CRTS_sync_node
#define		athread_sync_master_spe		CRTS_sync_master_spe
#define		athread_sync_master_array	CRTS_sync_master_array
#define		athread_mutex_lock_node		CRTS_mutex_lock_node
#define		athread_mutex_lock_node_all		CRTS_mutex_lock_node_all
#define		athread_mutex_trylock_node	CRTS_mutex_trylock_node
#define		athread_mutex_unlock_node	CRTS_mutex_unlock_node
#define		athread_mutex_unlock_node_all	CRTS_mutex_unlock_node_all
#define		athread_mutex_lock		CRTS_mutex_lock
#define		athread_mutex_trylock		CRTS_mutex_trylock
#define		athread_mutex_unlock		CRTS_mutex_unlock

#elif defined __sw_slave__
#define		athread_get_tid()		(_PEN)
#define		athread_get_rid()		(_ROW)
#define		athread_get_cid()		(_COL)
#define		athread_get_cgn()		(_CGN)

#define		athread_tid			_PEN
#define		athread_rid			_ROW
#define		athread_cid			_COL
#define		athread_cgn	  		_CGN

#define athread_ssync(scp,mask) \
({\
	switch(scp)\
	{\
	case ROW_SCOPE:\
		asm volatile("synr %0          \n"\
                	     :\
                    	     :"r"(mask));\
		break;\
	case COL_SCOPE:\
		asm volatile("sync %0          \n"\
                	     :\
                     	     :"r"(mask));\
		break;\
	case PP_SCOPE:\
		asm volatile ("synp  %0\n"::"r" (mask):"memory");\
		break;\
	default:\
		return -1;\
	}\
	0;\
})

#define		athread_ssync_array		CRTS_ssync_array
#define		athread_ssync_sldm		CRTS_ssync_sldm
#define		athread_ssync_master_spe	CRTS_ssync_master_spe
#define		athread_ssync_master_array	CRTS_ssync_master_array
#define		athread_ssync_node		CRTS_ssync_node
//#define         athread_get(mode,src,dest,len,reply,mask,stride,bsize)\
    __builtin_sw_slave_athread_dma(mode,DMA_GET,src,dest,len,reply,mask,stride,bsize)
//#define         athread_put(mode,src,dest,len,reply,mask,stride,bsize)\
    __builtin_sw_slave_athread_dma(mode,DMA_PUT,src,dest,len,reply,mask,stride,bsize)
/*
#define athread_get( mode, src, dest, len, reply, mask, stride, bsize)\
    __builtin_sw_slave_athread_dma( mode, DMA_GET, src, dest, len, (void*)(reply), mask, stride, bsize );

#define athread_put( mode, src, dest, len, reply, stride, bsize)\
    __builtin_sw_slave_athread_dma( PE_MODE, DMA_PUT, dest, src, len, (void*)(reply), 0, stride, bsize );
*/
//#endif
#define		athread_dma_get		        CRTS_dma_get
#define		athread_dma_get_stride		CRTS_dma_get_stride
#define		athread_dma_iget		CRTS_dma_iget
#define		athread_dma_iget_stride		CRTS_dma_iget_stride
#define		athread_dma_wait_value		CRTS_dma_wait_value
#define		athread_dma_put		        CRTS_dma_put
#define		athread_dma_put_stride		CRTS_dma_put_stride
#define		athread_dma_iput		CRTS_dma_iput
#define		athread_dma_iput_stride		CRTS_dma_iput_stride
#define		athread_dma_ibcast		CRTS_dma_ibcast
#define		athread_dma_ibcast_stride	CRTS_dma_ibcast_stride
#define		athread_dma_bcast_coll		CRTS_dma_bcast_coll
#define		athread_dma_bcast_stride_coll	CRTS_dma_bcast_stride_coll
#define		athread_dma_row_ibcast		CRTS_dma_row_ibcast
#define		athread_dma_row_ibcast_stride	CRTS_dma_row_ibcast_stride
#define		athread_dma_row_bcast_coll	CRTS_dma_row_bcast_coll
#define		athread_dma_row_bcast_stride_coll		CRTS_dma_row_bcast_stride_coll
#define		athread_dma_col_ibcast		CRTS_dma_col_ibcast
#define		athread_dma_col_ibcast_stride	CRTS_dma_col_ibcast_stride
#define		athread_dma_col_bcast_coll	CRTS_dma_col_bcast_coll
#define		athread_dma_col_bcast_stride_coll		CRTS_dma_col_bcast_stride_coll
#define		athread_memcpy_sldm		CRTS_memcpy_sldm
#define		athread_dma_barrier		CRTS_dma_barrier
#define		athread_dma_all_barrier		CRTS_dma_all_barrier
#define		athread_rma_get			CRTS_rma_get
#define		athread_rma_iget		CRTS_rma_iget
#define		athread_rma_wait_value		CRTS_rma_wait_value
#define		athread_rma_put			CRTS_rma_put
#define		athread_rma_iput		CRTS_rma_iput
#define		athread_rma_bcast		CRTS_rma_bcast
#define		athread_rma_ibcast		CRTS_rma_ibcast
#define		athread_rma_bcast_coll		CRTS_rma_bcast_coll
#define		athread_rma_row_bcast		CRTS_rma_row_bcast
#define		athread_rma_row_ibcast		CRTS_rma_row_ibcast
#define		athread_rma_row_bcast_coll	CRTS_rma_row_bcast_coll
#define		athread_rma_row_mcast		CRTS_rma_row_mcast
#define		athread_rma_row_imcast		CRTS_rma_row_imcast
#define		athread_rma_col_bcast		CRTS_rma_col_bcast
#define		athread_rma_col_ibcast		CRTS_rma_col_ibcast
#define		athread_rma_col_bcast_coll	CRTS_rma_col_bcast_coll
#define		athread_rma_col_mcast		CRTS_rma_col_mcast
#define		athread_rma_col_imcast		CRTS_rma_col_imcast
 #define	athread_sldm_get		CRTS_sldm_get
#define		athread_rma_barrier		CRTS_rma_barrier
#define		athread_rma_all_barrier		CRTS_rma_all_barrier
#define		athread_stime_cycle		CRTS_stime_cycle
 #define	athread_alltoall		CRTS_scoll_alltoall
 #define	athread_alltoall_inplace	CRTS_scoll_alltoall_inplace
 #define	athread_redurt			CRTS_scoll_redurt
 #define        athread_sync_master_spe_all     CRTS_sync_master_spe_all
 #define        athread_sync_master_array_all   CRTS_sync_master_array_all
#define		athread_smutex_lock_node_all		CRTS_smutex_lock_node_all
#define		athread_smutex_unlock_node_all	CRTS_smutex_unlock_node_all

//lockptr shouldbe long*
#define athread_lock(lockptr)\
({\
    int ret = 1;\
        do {\
                  ret = _crts_faa_long(lockptr);\
                      } while (ret != 0);\
                          0;\
})

#define athread_unlock(lockptr)\
({\
    *(lockptr) = 0;\
        asm volatile ("memb\n");\
            0;\
})
#define athread_int       0x0
#define athread_uint      0x1
#define athread_long      0x2
#define athread_ulong     0x3
#define athread_float     0x4
#define athread_double    0x5
#define athread_intv8     0x6
#define athread_uintv8    0x7
#define athread_int256    0x8
#define athread_uint256   0x9
#define athread_floatv4   0xa
#define athread_doublev4  0xb
#define athread_intv16    0xc
#define athread_uintv16   0xd
#define athread_int512    0xe
#define athread_uint512   0xf
#define athread_floatv8   0x10
#define athread_doublev8  0x11

#define OP_add        0x1
#define OP_and        0x2
#define OP_or         0x3
#define OP_xor        0x4
#define OP_eqv        0x5
#define OP_min        0x6
#define OP_max        0x7
typedef volatile long athread_rply_t;
#endif
