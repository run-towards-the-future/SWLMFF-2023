#ifndef QTHREAD_SLAVE_H_
#define QTHREAD_SLAVE_H_
#define __ldm_fix __attribute__ ((section (".ldm_fix")))
#define __ldm __attribute__ ((section (".ldm")))
__attribute__((always_inline)) inline void qthread_syn() {
  asm volatile("sync %0\n\tsynr%0\n\t" :: "r"(0xff));
}
__attribute__((always_inline)) inline void qthread_synr(){
  asm volatile("synr %0\n\t" :: "r"(0xff));
} 
__attribute__((always_inline)) inline void qthread_sync(){
  asm volatile("sync %0\n\t" :: "r"(0xff));
}
__attribute__((always_inline)) inline long qthread_faal(long *addr){
  long ret;
  asm volatile("ldi %0, %1\n\t"
               "faal %0, 0(%0)\n\t": "=&r"(ret), "+m"(*addr));
  return ret;
}
__attribute__((always_inline)) inline int qthread_faaw(int *addr){
  int ret;
  asm volatile("ldi %0, %1\n\t"
               "faaw %0, 0(%0)\n\t": "=&r"(ret), "+m"(*addr));
  return ret;
}
extern __ldm char  _CGN,_ROW,_COL,_PEN;
extern __ldm int   _MYID;
#define __thread_local __ldm
typedef enum {
        PE_MODE,
        BCAST_MODE,
        ROW_MODE,
        BROW_MODE,
        RANK_MODE
} dma_mode;

typedef enum {
        DMA_PUT,
        DMA_GET,
        DMA_PUT_P,
        DMA_GET_P,
        DMA_BARRIER = 5
} DMA_OP;

typedef enum {
	ROW_SCOPE,
	COL_SCOPE,
	ARRAY_SCOPE,
} scope;
#endif
