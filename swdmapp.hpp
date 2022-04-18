#pragma once
#include "dma_funcs.hpp"
#include <cstddef>
#include <sys/types.h>
#include <utility>
#include "cal.h"
enum MemPtrMode {
  MP_IN,
  MP_OUT,
  MP_INOUT,
  MP_ZEROOUT
};

template<typename DataType, int COUNT, MemPtrMode MODE>
struct MemPtr {
  DataType buffer[COUNT];
  const DataType *mem;
  const int count;
  __always_inline MemPtr(DataType *mem, int count = COUNT) : mem(mem), count(count){
    //printf("%d\n", sizeof(DataType));
    if (MODE == MP_IN || MODE == MP_INOUT)
      dma_getn(mem, buffer, count);
    if (MODE == MP_ZEROOUT) {
      memset(buffer, 0, sizeof(DataType) * count);
    }
  }
  __always_inline ~MemPtr(){
    if (MODE == MP_ZEROOUT || MODE == MP_INOUT)
      dma_putn(mem, buffer, count);
  }
  __always_inline DataType &operator[](off_t index) {
    return buffer[index];
  }
  __always_inline const DataType &operator[](off_t index) const {
    return buffer[index];
  }
};

template<int COUNT, typename DataType>
__always_inline MemPtr<DataType, COUNT, MP_IN> ptr_in(DataType *ptr, int count){
  return MemPtr<DataType, COUNT, MP_IN>(ptr);
}
template<int COUNT, typename DataType>
__always_inline MemPtr<DataType, COUNT, MP_OUT> ptr_out(DataType *ptr, int count){
  return MemPtr<DataType, COUNT, MP_OUT>(ptr);
}

template<int COUNT, typename DataType>
__always_inline MemPtr<DataType, COUNT, MP_INOUT> ptr_inout(DataType *ptr, int count){
  return MemPtr<DataType, COUNT, MP_INOUT>(ptr);
}

template<int COUNT, typename DataType>
__always_inline MemPtr<DataType, COUNT, MP_ZEROOUT> ptr_zeroout(DataType *ptr, int count){
  return MemPtr<DataType, COUNT, MP_ZEROOUT>(ptr);
}

template<typename T>
struct Arr2D{
  T &base;
  int len;
  __always_inline Arr2D(T &base, int len) : base(base), len(len){
  }
  __always_inline decltype(base[0]) &operator()(int i, int j) {
    return base[i * len + j];
  }
  __always_inline decltype(&base[0]) operator[](int i){
    return &base[i * len];
  }
};
template<typename T>
__always_inline Arr2D<T> to2d(T &&base, int len){
  return Arr2D<T>(base, len);
}
#include <tuple>
template<int COUNT, typename ...Ts>
std::tuple<MemPtr<Ts, COUNT, MP_IN>...> batch_ptr_in(off_t offset, int count, Ts*... mems){
  return std::make_tuple(ptr_in<COUNT>(mems + offset, count)...);
}
template<int I, off_t VAL>
struct __log2i_helper{
  static constexpr int value = (1L << I) == VAL ? I : __log2i_helper<I-1, VAL>::value;
};
template <off_t VAL>
struct __log2i_helper<-1, VAL>{
  static constexpr int value = -1;
};
template<off_t VAL>
static constexpr int log2i(){
  return __log2i_helper<63, VAL>::value;
}

template<typename DataType, int NPAGES, int PAGESIZE>
struct ReadOnlyCache {
  DataType cache[NPAGES][PAGESIZE];
  int tags[NPAGES];
  DataType *mem;
  desc_t prefetch_desc;
  static constexpr off_t NPAGE_MASK = NPAGES - 1;
  static constexpr off_t NPAGE_SHIFT = log2i<NPAGES>();
  static constexpr off_t PAGESIZE_MASK = PAGESIZE - 1;
  static constexpr off_t PAGESIZE_SHIFT = log2i<PAGESIZE>();
  __always_inline ReadOnlyCache(DataType *mem) : mem(mem) {
    static_assert(NPAGE_SHIFT != -1, "number of pages should be a power of 2");
    static_assert(PAGESIZE_SHIFT != -1, "pages size should be a power of 2");
    for (int i = 0; i < NPAGES; i ++){
      tags[i] = -1;
    }
    dma_set_done(prefetch_desc);
  }
  __always_inline DataType &operator[](off_t index){
    int igblk = index >> PAGESIZE_SHIFT;
    int off = index & PAGESIZE_MASK;
    int ilblk = igblk & NPAGE_MASK;
    //dma_wait_async(prefetch_desc);
    if (tags[ilblk] != igblk){
      off_t mem_start = igblk * PAGESIZE;
      dma_getn(mem + mem_start, cache[ilblk], PAGESIZE);
      tags[ilblk] = igblk;
    }
    off_t pageoff = index & PAGESIZE_MASK;
    return cache[ilblk][pageoff];
  }
  __always_inline void prefetch(off_t index){
    //  int igblk = index >> PAGESIZE_SHIFT;
    // // int off = index & PAGESIZE_MASK;
    //  int ilblk = igblk & NPAGE_MASK;

    //  if (tags[ilblk] != igblk){
    //    off_t mem_start = igblk * PAGESIZE;
    //    dma_getn_async(prefetch_desc, mem + mem_start, cache[ilblk], PAGESIZE);
    //    tags[ilblk] = igblk;
    //  }
  }
};
#include <cassert>
template <int NBLKS, int BLKSIZE>
class ForceCache{
  typedef int tag_t;
  double ldm[NBLKS][BLKSIZE][3];
  double tmp[BLKSIZE][3];
  tag_t tags[NBLKS];
  double (*mem)[3];
  cal_lock_t *locks;
  int n;
public:
  int nswap, naccess;
  long acquire_cycles;
  __always_inline ForceCache(double (*mem)[3], cal_lock_t *locks, int n): mem(mem), locks(locks), n(n), nswap(0), naccess(0), acquire_cycles(0) {
    for (int i = 0; i < NBLKS; i ++) tags[i] = -1;
  }

  __always_inline void update(tag_t index, double fx, double fy, double fz){
    //naccess ++;
    int igblk = index / BLKSIZE;
    int off = index % BLKSIZE;
    int ilblk = igblk % NBLKS;
    if (tags[ilblk] != igblk) {
      swapblk(ilblk, igblk);
    }
    ldm[ilblk][off][0] += fx;
    ldm[ilblk][off][1] += fy;
    ldm[ilblk][off][2] += fz;
  }

  __always_inline void lock(int igblk) {
    cal_lock(locks + igblk);
  }
  __always_inline void unlock(int igblk) {
    cal_unlock(locks + igblk);
  }
  __always_inline void swapblk(int ilblk, int igblk){
    //long st, ed;
    //asm ("rcsr %0, 4\n\t" : "=r"(st) ::"memory");
    // assert(igblk >= -1);
    // assert(igblk * BLKSIZE < n);

    if (tags[ilblk] != -1) {
      int flushblk = tags[ilblk];
      lock(flushblk);
      dma_getn(mem + flushblk * BLKSIZE, tmp, BLKSIZE);
      //nswap += 1;
      for (int i = 0; i < BLKSIZE; i ++){
	tmp[i][0] += ldm[ilblk][i][0];
	tmp[i][1] += ldm[ilblk][i][1];
	tmp[i][2] += ldm[ilblk][i][2];
      }
      int flushsize = n - flushblk * BLKSIZE;
      if (flushsize > BLKSIZE) flushsize = BLKSIZE;
      dma_putn(mem + flushblk * BLKSIZE, tmp, flushsize);
      unlock(flushblk);
    }
    tags[ilblk] = igblk;
    if (igblk != -1)
    for (int i = 0; i < BLKSIZE; i ++){
      ldm[ilblk][i][0] = 0;
      ldm[ilblk][i][1] = 0;
      ldm[ilblk][i][2] = 0;
    }
    //asm ("memb\n\trcsr %0, 4\n\t" : "=r"(ed) ::"memory");
    //acquire_cycles += ed - st;

  }
  __always_inline void flush_all(){
    for (int i = 0; i < NBLKS; i ++){
      swapblk(i, -1);
    }
  }
};

