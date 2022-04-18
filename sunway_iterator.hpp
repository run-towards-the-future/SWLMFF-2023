#include "dma_funcs.hpp"
template<typename DataType, int COUNT, MemPtrMode MODE>
struct LinearBuffer {
  DataType buffer[COUNT];
  DataType const* mem;
  DataType *ptr;
  int count;
  __always_inline LinearBuffer(DataType *mem) : mem(mem){
  };
  __always_inline void enter(off_t i, int count = COUNT){
    ptr = buffer - i;
    if (MODE == MP_IN || MODE == MP_INOUT)
      dma_getn(mem + i, buffer, count);
    if (MODE == MP_ZEROOUT) {
      memset(buffer, 0, sizeof(DataType) * count);
    }
  }
  __always_inline void leave(off_t i, int count = COUNT){
    if (MODE == MP_ZEROOUT || MODE == MP_INOUT)
      dma_putn(mem + i, buffer, count);
  }
  __always_inline DataType &operator[](int idx){
    return ptr[idx];
  }
};

template<int COUNT, typename DataType>
__always_inline LinearBuffer<DataType, COUNT, MP_IN> linear_in(DataType *ptr){
  return LinearBuffer<DataType, COUNT, MP_IN>(ptr);
}
template<int COUNT, typename DataType>
__always_inline LinearBuffer<DataType, COUNT, MP_OUT> linear_out(DataType *ptr){
  return LinearBuffer<DataType, COUNT, MP_OUT>(ptr);
}

template<int COUNT, typename DataType>
__always_inline LinearBuffer<DataType, COUNT, MP_INOUT> linear_inout(DataType *ptr){
  return LinearBuffer<DataType, COUNT, MP_INOUT>(ptr);
}

template<int COUNT, typename DataType>
__always_inline LinearBuffer<DataType, COUNT, MP_ZEROOUT> linear_zeroout(DataType *ptr){
  return LinearBuffer<DataType, COUNT, MP_ZEROOUT>(ptr);
}

template<int BLKSIZE, int STRIDE, typename ...Ts>
struct LinearIterator {
  std::tuple<Ts&...> buffers;
  int start, stop;
  __always_inline LinearIterator(int start, int stop, Ts&... buffers) : buffers(buffers...), start(start), stop(stop){
    
  }
  template<int ...Is>
  __always_inline void enter_blk(int i, std::integer_sequence<int, Is...> seq){
    int size = BLKSIZE;
    if (i + size > stop) size = stop - i;
    if (size <= 0) return;
    //printf("enter: %d %d\n", i, size);
    (int[]){(std::get<Is>(buffers).enter(i, size), 0)...};
  }
  template<int ...Is>
  __always_inline void leave_blk(int i, std::integer_sequence<int, Is...> seq){
    int size = BLKSIZE;
    if (i + size > stop) size = stop - i;
    if (size <= 0) return;
    //printf("leave: %d %d\n", i, size);
    (int[]){(std::get<Is>(buffers).leave(i, size), 0)...};
  }

  struct cursor {
    int i, inneri;
    LinearIterator &ite;
    static constexpr auto seq = std::make_integer_sequence<int, sizeof...(Ts)>{};
    __always_inline cursor(LinearIterator &ite) : ite(ite), i(ite.start), inneri(0){
      ite.enter_blk(i, seq);
    }
    __always_inline int operator*(){
      return i;
    }
    __always_inline cursor &operator++(){
      i ++;
      inneri ++;
      if (inneri == BLKSIZE) {
	ite.leave_blk(i - BLKSIZE, seq);
	i += STRIDE - BLKSIZE;
	ite.enter_blk(i, seq);
	inneri = 0;
      }
      return *this;
    }
    __always_inline bool operator!=(void *p){
      return i < ite.stop;
    }
    __always_inline ~cursor(){
      ite.leave_blk(i - inneri, seq);
    }
  };
  __always_inline cursor begin(){
    return cursor(*this);
  }
  __always_inline void *end(){
    return NULL;
  }
};
template<int BLKSIZE, typename ...Ts>
__always_inline LinearIterator<BLKSIZE, BLKSIZE * 64, Ts...> parallel_iterate(int start, int stop, Ts&... args){
  return LinearIterator<BLKSIZE, BLKSIZE * 64, Ts...>(start + BLKSIZE * _MYID, stop, args...);
}

template<int BLKSIZE, typename ...Ts>
__always_inline LinearIterator<BLKSIZE, BLKSIZE, Ts...> iterate(int start, int stop, Ts&... args){
  return LinearIterator<BLKSIZE, BLKSIZE, Ts...>(start, stop, args...);
}
