#ifndef PARALLEL_EVAL_H_
#define PARALLEL_EVAL_H_
#ifdef __sw_host__
extern "C"{
#include <athread.h>
}
#include "qthread.h"
#include <mpi.h>
struct reduce_req_t {
  bool all;
  void *send_buf;
  void *recv_buf;
  int count;
  MPI_Datatype type;
  MPI_Op op;
  MPI_Comm comm;
  int root;
  reduce_req_t *next;
};
template<typename T>
void athread_resolve_spawn(void (*func)(T), T arg){
  __real_athread_spawn((void*)func, arg, 1);
  //qthread_spawn(func, arg);
}
static void athread_resolve_join(){
  //qthread_join();
  athread_join();
}
template<int ...FLAGS, typename T>
extern void slave_parallel_eval(T *ptr_in);
void flush_reduce_req();
void push_reduce_req(reduce_req_t *req);
template<int ...FLAGS, typename T>
void parallel_eval_pair(T *ptr_in){
  athread_resolve_spawn(slave_parallel_eval<FLAGS...>, ptr_in);
  flush_reduce_req();
  athread_resolve_join();
}
#endif

#ifdef __sw_slave__
template<int ...FLAGS, typename T>
__attribute__((noinline)) void parallel_eval(T *ptr_in){
  ptr_in->template parallel_eval<FLAGS...>();
}
template<typename T, int NFLAGS, int ...FLAGS>
struct make_parallel_eval{
  __attribute__((noinline)) static void gen(T *arg);
};
template<typename T, int ...FLAGS>
struct make_parallel_eval<T, 0, FLAGS...>{
  __attribute__((noinline)) static void gen(T *arg);
};
template<typename T, int NFLAGS, int ...FLAGS>
__attribute__((noinline)) void make_parallel_eval<T, NFLAGS, FLAGS...>::gen(T* arg){
  make_parallel_eval<T, NFLAGS-1, FLAGS..., 0>::gen(arg);
  make_parallel_eval<T, NFLAGS-1, FLAGS..., 1>::gen(arg);
}
template<typename T, int ...FLAGS>
__attribute__((noinline)) void make_parallel_eval<T, 0, FLAGS...>::gen(T* arg){
  parallel_eval<FLAGS...>(arg);
}

#endif
#endif
