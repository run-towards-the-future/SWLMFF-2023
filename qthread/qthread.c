#include "qthread_machine.h"
#ifdef __sw_host__
#include <stdio.h>
#include <qthread.h>

#include <signal.h>
#include <stdlib.h>
extern int sys_m_run(int cgid, unsigned long coremask, unsigned long pc, unsigned *status);
extern int sys_m_employ(int cgid, unsigned long coremask, int flags, int pid);
extern void sys_m_reset(int cgid, unsigned long coremask, int flags);
long cgid;
extern void slave_qthread_init();

#ifdef __sw_thl__
extern void cpe_err_handler(int, siginfo_t *, void*);
extern void pc_on_sig(int);
#endif
extern long *__qstat_addr;
extern void qthread_wait_cpe_init();
extern void qthread_spawn(void (*pc)(void*), void *arg);
extern void qthread_join();
extern long athread_idle();
typedef struct qasyn_task {
    void (*func)(void*);
    void *arg;
} qasyn_task_t;
struct {
    qasyn_task_t *tasks;
    int size, cnt;
} qasyn_pool = {NULL, 0, 0};

void qthread_pend_host(void (*func)(void*), void *arg) {
    if (qasyn_pool.tasks == NULL) {
        qasyn_pool.size = 16;
        qasyn_pool.tasks = malloc(sizeof(*qasyn_pool.tasks) * qasyn_pool.size);
        qasyn_pool.cnt = 0;
    }
    if (qasyn_pool.cnt >= qasyn_pool.size) {
        qasyn_pool.tasks = realloc(qasyn_pool.tasks, sizeof(qasyn_task_t) * qasyn_pool.size * 2);
        qasyn_pool.size = qasyn_pool.size * 2;
    }
    qasyn_pool.tasks[qasyn_pool.cnt].func = func;
    qasyn_pool.tasks[qasyn_pool.cnt].arg = arg;
    qasyn_pool.cnt ++;
}
void qthread_flush_host(){
    for (int i = 0; i < qasyn_pool.cnt; i ++) {
        qasyn_pool.tasks[i].func(qasyn_pool.tasks[i].arg);
    }
    qasyn_pool.cnt = 0;
}
extern void slave_qthread_boot_from_athread();
void qthread_init()
{
    athread_init();
    #ifdef __sw_ocn__
    CRTS_init();
    #endif
    if (athread_idle() != -1L){
      printf("qthread must be run with -cgsp 64! Exitting.");
      abort();
    }
    struct sigaction st;
    long cgid = sys_m_cgid();
    unsigned int status;
#ifdef __sw_thl__
    struct sigaction sa;
    sigaction(55, NULL, &sa);
    sa.sa_flags |= SA_SIGINFO;
    sa.sa_sigaction = cpe_err_handler;
    sigaction(55, &sa, NULL);
    signal(SIGUSR1, pc_on_sig);
    signal(SIGUSR2, pc_on_sig);
#endif
    __qstat_addr = 0;
    
    __real_athread_spawn(slave_qthread_boot_from_athread, 0);
    qthread_wait_cpe_init();
    __qstat_addr = (long*)CPE_LDM_ADDR(cgid, 0, __qstat_addr);
}

#endif
#ifdef __sw_slave__
#include <simd.h>
#define __thread_local __attribute__((section(".ldm")))
#define __thread_local_fix __attribute__((section(".ldm_fix")))
extern __thread_local int _MYID;
extern __thread_local char _ROW, _COL, _CGN;
extern long *__qstat_addr;
extern __thread_local intvec __qstat;
extern void qthread_waiting_for_task();
// extern void *_tdata_local_start, *_tdata_local_end, *_PC;
// extern __thread_local void *__ldm_stack_limit;
void qthread_init()
{
  if (_MYID == 0){
    long cid;
    asm("rcsr %0, 0\n\t"
        : "=r"(cid));
    _MYID = cid & 63;
    _CGN = cid >> 6;
    _ROW = _MYID >> 3;
    _COL = _MYID & 7;
    __qstat = 0;
  }
  //puts("before sync");
  asm("sync %0\n\t"
      "synr %0\n\t" ::"r"(0xff));
  // puts("in cpe init");
  if (_MYID == 0)
    {
      long sp;
      asm("ldi %0, 0($30)\n\t" : "=r"(sp));
      printf("%p %p\n", &__qstat, sp);
      __qstat_addr = &__qstat;
      flush_slave_cache();
      //printf("%p %d\n", &__qstat_addr, __qstat_addr);
    }
  
  asm("stl %0, 16+%1" :: "r"(qthread_init), "m"(__qstat));
  qthread_waiting_for_task();
  asm volatile("halt\n\t");
}
// void dummy() {}
#endif
