#ifndef ATHREAD_EXT_H_
#define ATHREAD_EXT_H_
#ifdef __sw_host__
extern "C"{
#include <athread.h>
}

template<typename T>
void athread_resolve_spawn(void (*func)(T), T arg){
  __real_athread_spawn((void*)func, arg, 1);
}

template<int ...FLAGS, typename T>
extern void slave_parallel_eval(T *ptr_in);

template<int ...FLAGS, typename T>
void parallel_eval_pair(T *ptr_in){
  athread_resolve_spawn(slave_parallel_eval<FLAGS...>, ptr_in);
  athread_join();
}
#endif
#endif
