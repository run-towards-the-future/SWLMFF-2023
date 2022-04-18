#pragma once
#include <simd.h>
#include <tuple>
#include "lmptype.h"
using namespace LAMMPS_NS;
struct atompack_t {
  double x, y, z;
  int type;
};

struct atompack_mol_t {
  double x, y, z;
  int type, mol;
};

#ifdef __sw_slave__
extern "C"{
#include <slave.h>
}
template<int EXT_FIELDS>
struct tallyvar {
#ifdef __sw_512bit_simd__
  static constexpr int NV = (8 + EXT_FIELDS + 7) / 8;
#endif
#ifdef __sw_256bit_simd__
  static constexpr int NV = (8 + EXT_FIELDS + 3) / 4;
#endif
  union {
    struct {
      double evdwl, ecoul;
      double virial[6];
      double ext_fields[EXT_FIELDS];
    } field;
#ifdef __sw_512bit_simd__
    doublev8 vval[NV];
#endif
#ifdef __sw_256bit_simd__
    doublev4 vval[NV];
#endif
  } u;
  __always_inline void init(){
    for (int i = 0; i < NV; i ++){
      u.vval[i] = 0;
    }
  }
  __always_inline void ev(double ev, double ec, double fpair, double dx, double dy, double dz){
    u.field.evdwl += ev;
    u.field.ecoul += ec;
    u.field.virial[0] += fpair*dx*dx;
    u.field.virial[1] += fpair*dy*dy;
    u.field.virial[2] += fpair*dz*dz;
    u.field.virial[3] += fpair*dx*dy;
    u.field.virial[4] += fpair*dx*dz;
    u.field.virial[5] += fpair*dy*dz;
  }
  __always_inline void v(double fpair, double dx, double dy, double dz){
    u.field.virial[0] += fpair*dx*dx;
    u.field.virial[1] += fpair*dy*dy;
    u.field.virial[2] += fpair*dz*dz;
    u.field.virial[3] += fpair*dx*dy;
    u.field.virial[4] += fpair*dx*dz;
    u.field.virial[5] += fpair*dy*dz;
  }
  __always_inline void ev_xyz(double ev, double ec, double fx, double fy, double fz, double dx, double dy, double dz){
    u.field.evdwl += ev;
    u.field.ecoul += ec;
    u.field.virial[0] += dx*fx;
    u.field.virial[1] += dy*fy;
    u.field.virial[2] += dz*fz;
    u.field.virial[3] += dx*fy;
    u.field.virial[4] += dx*fz;
    u.field.virial[5] += dy*fz;
  }
  __always_inline void e(double ev, double ec){
    u.field.evdwl += ev;
    u.field.ecoul += ec;
  }
  __always_inline void v_xyz(double fx, double fy, double fz, double dx, double dy, double dz) {
    u.field.virial[0] += dx*fx;
    u.field.virial[1] += dy*fy;
    u.field.virial[2] += dz*fz;
    u.field.virial[3] += dx*fy;
    u.field.virial[4] += dx*fz;
    u.field.virial[5] += dy*fz;    
  }
  __always_inline double &ext(int i){
    return u.field.ext_fields[i];
  }
  __always_inline void finish(double &evdwl, double &ecoul, double *virial, double *ext_fields){
#ifdef __sw_512bit_simd__
    doublev8 reduce_buf[NV];
    for (int i = 0; i < 6; i ++){
      int peer = _MYID ^ 1 << i;
      if (i < 3) athread_syn(ROW_SCOPE, 0xff);
      else athread_syn(COL_SCOPE, 0xff);
      //athread_syn(PP_SCOPE, peer);
      if (_MYID & 1 << i){
	long vaddr = (long)&reduce_buf;
	vaddr |= 1L << 45 | peer << 20;
	doublev8 *remote_reduce_buf = (doublev8*)vaddr;
	for (int j = 0; j < NV; j ++){
	  remote_reduce_buf[j] = u.vval[j];
	}
      }
      if (i < 3) athread_syn(ROW_SCOPE, 0xff);
      else athread_syn(COL_SCOPE, 0xff);
      //athread_syn(PP_SCOPE, peer);

      for (int j = 0; j < NV; j ++){
	u.vval[j] += reduce_buf[j];
      }

    }
    if (_MYID == 0){
      evdwl += u.field.evdwl;
      ecoul += u.field.ecoul;
      virial[0] += u.field.virial[0];
      virial[1] += u.field.virial[1];
      virial[2] += u.field.virial[2];
      virial[3] += u.field.virial[3];
      virial[4] += u.field.virial[4];
      virial[5] += u.field.virial[5];
      for (int i = 0; i < EXT_FIELDS; i ++){
	ext_fields[i] += u.field.ext_fields[i];
      }
      //return;
    }
#else
    static_assert(0, "reduction using register communication should be filled here");
#endif
  }
};
#endif
#ifdef __sw_slave__
#include <slave.h>
template<typename T>
void fill_mem(std::tuple<T *, T, size_t> *params){
  auto [ptr, val, cnt] = *params;
  static constexpr int BLKSIZE = 64;
  T buffer[BLKSIZE];
  for (int i = 0; i < BLKSIZE; i ++){
    buffer[i] = val;
  }
  for (int i = _MYID*BLKSIZE; i < cnt; i += BLKSIZE * 64){
    int putcnt = BLKSIZE;
    if (i + putcnt > cnt) putcnt = cnt - i;
    dma_putn(ptr + i, buffer, putcnt);
  }
}


#endif
#ifdef __sw_host__
template<typename T>
void slave_fill_mem(std::tuple<T *, T, size_t> *params);
void slave_pack_atoms(std::tuple<atompack_t *, double(*)[3], int*, int> *params);
void slave_pack_atoms_mol(std::tuple<atompack_mol_t*, double(*)[3], int*, tagint *, int> *params);
#endif

