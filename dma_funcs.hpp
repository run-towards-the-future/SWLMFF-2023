#pragma once
#include <sys/cdefs.h>
//#define __sw_512bit_simd__
#ifdef __sw_512bit_simd__
typedef int desc_t __attribute__ ((__mode__(__V1XI__)));
#elif defined(__sw_256bit_simd__)
typedef int desc_t __attribute__ ((__mode__(__V1OI__)));
#endif
template <typename TM, typename TL>
__attribute__((artificial)) __always_inline void dma_getn(TM *mem, TL *ldm, int count) {
  //long dma_desc[4];
  desc_t dma_desc;
  asm volatile(
      "beq %[SIZE], 2f\n\t"
      "memb\n\t"
      "stl $31, 0(%[RPL])\n\t"
      "vinsw %[RPL], $31, 2, %[DESC]\n\t"
      "ldi %[DESC], 1($31)\n\t"
      "sll %[DESC], 56, %[DESC]\n\t"
      "vinsw %[SIZE], %[DESC], 0, %[DESC]\n\t"
      "dma %[DESC], %[MEM], %[LDM]\n\t"
      "1:\n\t"
      "ldw %[DESC], 0(%[RPL])\n\t"
      "beq %[DESC], 1b\n\t"
      "2:\n\t"
      "memb\n\t"
      : [ DESC ] "=&r"(dma_desc)
      : [ MEM ] "r"(mem), [ LDM ] "r"(ldm), [ RPL ] "r"(((long*)&dma_desc) + 2), [ SIZE ] "r"(count * sizeof(TL))
      : "memory");
}
template <typename TM, typename TL>
__attribute__((artificial)) __always_inline __attribute__((__artificial__)) void dma_putn(TM *mem, TL *ldm, int count) {
  desc_t dma_desc;
  asm volatile(
      "beq %[SIZE], 2f\n\t"
      "memb\n\t"
      "stl $31, 0(%[RPL])\n\t"
      "vinsw %[RPL], $31, 2, %[DESC]\n\t"
      "vinsw %[SIZE], %[DESC], 0, %[DESC]\n\t"
      "dma %[DESC], %[MEM], %[LDM]\n\t"
      "1:\n\t"
      "ldw %[DESC], 0(%[RPL])\n\t"
      "beq %[DESC], 1b\n\t"
      "2:\n\t"
      "memb\n\t"
      : [ DESC ] "=&r"(dma_desc)
      : [ MEM ] "r"(mem), [ LDM ] "r"(ldm), [ RPL ] "r"(((long*)&dma_desc) + 2), [ SIZE ] "r"(count * sizeof(TL))
      : "memory");
}


template <typename TM, typename TL>
__attribute__((artificial)) __always_inline void dma_getn_async(desc_t &dma_desc, TM *mem, TL *ldm, int count) {
  asm volatile(
      "beq %[SIZE], 2f\n\t"
      "memb\n\t"
      "vinsf $31, %[DESC], 2, %[DESC]\n\t"
      "vinsw %[RPL], $31, 2, %[DESC]\n\t"
      "ldi %[DESC], 1($31)\n\t"
      "sll %[DESC], 56, %[DESC]\n\t"
      "vinsw %[SIZE], %[DESC], 0, %[DESC]\n\t"
      "dma %[DESC], %[MEM], %[LDM]\n\t"
      : [ DESC ] "=&r"(dma_desc)
      : [ MEM ] "r"(mem), [ LDM ] "r"(ldm), [ RPL ] "r"(((long*)&dma_desc) + 2), [ SIZE ] "r"(count * sizeof(TL))
      : "memory");
}

__attribute__((artificial)) __always_inline void dma_wait_async(desc_t &dma_desc) {
  int tmp;
  asm volatile(
      "1:\n\t"
      "ldw %[TMP], 0(%[RPL])\n\t"
      "beq %[TMP], 1b\n\t"
      : [ TMP ] "=&r"(tmp)
      : [ RPL ] "r"(((long*)&dma_desc) + 2)
      : "memory");
}

__attribute__((artificial)) __always_inline void dma_set_done(desc_t &dma_desc) {
  int tmp = 1;
  asm volatile(
      "stl %[TMP], 0(%[RPL])\n\t"
      :: [ TMP ] "r"(tmp), [ RPL ] "r"(((long*)&dma_desc) + 2)
      : "memory");
}
