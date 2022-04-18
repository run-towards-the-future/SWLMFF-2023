#pragma once
#include <stdarg.h>
extern "C"{
#include <slave.h>
}
#ifndef __cplusplus
typedef struct cal_lock{
  long req, cur;
} cal_lock_t;
#else
struct cal_lock_t{
  long req, cur;
};
#endif
//__attribute__((section(".uncached_rw")))
static __uncached cal_lock_t __cal_global_lock = {1, 1} ;
#ifdef __sw_slave__
static void cal_lock(cal_lock_t *lock){
  int req, cur;
  long ptr;
  asm volatile("ldi  %3, %2\n\t"
	       "faal %0, 0(%3)\n\t"
	       "1:\n\t"
	       "ldl  %1, 8(%3)\n\t"
	       "subl %1, %0, %1\n\t"
	       "bne  %1, 1b\n\t"
	       : "=r"(req), "=r"(cur), "+m"(*lock), "=r"(ptr) :: "memory");
}

static void cal_unlock(cal_lock_t *lock){
  long ptr;
  asm volatile ("ldi  %1, %0\n\t"
		"faal $31, 8(%1)\n\t"
		:"+m"(*lock), "=r"(ptr) :: "memory");
}
static void cal_global_lock(){
  cal_lock(&__cal_global_lock);
}

static void cal_global_unlock(){
  cal_unlock(&__cal_global_lock);
}

static void cal_locked_printf(const char *fmt, ...){
  volatile long vsprintf_addr = (long)vsprintf;
  volatile long printf_addr = (long)printf;
  int (*vsprintf_ptr)(char *, const char *, va_list) = (int(*)(char *, const char *, va_list))vsprintf_addr;
  char buf[256];
  va_list va;
  va_start(va, fmt);
  vsprintf_ptr(buf, fmt, va);
  va_end(va);
  cal_global_lock();
  fputs(buf, stdout);
  fflush(stdout);
  cal_global_unlock();
}
#endif
