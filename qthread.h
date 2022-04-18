#ifndef QTHREAD_H_
#define QTHREAD_H_
// #define DEBUG_WITH_ATHREAD 
#ifdef DEBUG_WITH_ATHREAD
#ifdef __sw_host__
#include <athread.h>
#endif
#define qthread_init athread_init
#define qthread_spawn(x, y) __real_athread_spawn((void*)(x), y, 0)
#define qthread_join athread_join
#else
#ifdef __sw_host__
#ifdef __cplusplus
extern "C"{
#endif
extern long *__qstat_addr;
// extern void qthread_wait_cpe_init();
/* initialize slave threads */
extern void qthread_init();
/* spawn 64 slave threads with entrance pc and argument arg */
extern void qthread_spawn(void (*func)(void*), void *arg);
/* wait threads created by qthread_spawn exit */
extern void qthread_join();
/* pend an async task on MPE */
extern void qthread_pend_host(void (*func)(void*), void *arg);
/* run all async tasks */
extern void qthread_flush_host();
#ifdef __cplusplus
}
template<typename T>
__attribute__((always_inline)) inline void qthread_spawn(void (*f)(T), T arg){
  qthread_spawn((void (*)(void*))f, (void*)arg);
}
#endif//cplusplus
#endif//sw_host
#ifdef __cplusplus
#include "qthread_spawn_misc.hpp"
#endif
#endif//debug with...
#endif//qthread_h_
