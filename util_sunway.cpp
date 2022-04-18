#include "util_sunway.h"
#ifdef __sw_slave__
extern "C"{
#include <slave.h>
}
#include "dma_funcs.hpp"

void pack_atoms(std::tuple<atompack_t*, double(*)[3], int*, int> *params) {
  //if (_MYID > 0) return;
  auto [out, gx, gt, n] = *params;
  static constexpr int BLKSIZE = 64;
  atompack_t pack[BLKSIZE];
  double x[BLKSIZE][3];
  int type[BLKSIZE];
  for (int i = _MYID*BLKSIZE; i < n; i += BLKSIZE * 64){
  //for (int i = 0; i < n; i += BLKSIZE){
    int cnt = BLKSIZE;
    if (i + cnt > n) cnt = n - i;
    dma_getn(gx + i, x, cnt);
    dma_getn(gt + i, type, cnt);
    for (int j = 0; j < cnt; j ++){
      pack[j].x = x[j][0];
      pack[j].y = x[j][1];
      pack[j].z = x[j][2];
      pack[j].type = type[j];
    }
    //printf("%p %d\n", pack, i);
    dma_putn(out + i, pack, cnt);
  }
}

void pack_atoms_mol(std::tuple<atompack_mol_t*, double(*)[3], int*, tagint *, int> *params) {
  //if (_MYID > 0) return;
  auto [out, gx, gt, gmol, n] = *params;
  static constexpr int BLKSIZE = 64;
  atompack_mol_t pack[BLKSIZE];
  double x[BLKSIZE][3];
  int type[BLKSIZE];
  tagint mol[BLKSIZE];
  for (int i = _MYID*BLKSIZE; i < n; i += BLKSIZE * 64){
  //for (int i = 0; i < n; i += BLKSIZE){
    int cnt = BLKSIZE;
    if (i + cnt > n) cnt = n - i;
    dma_getn(gx + i, x, cnt);
    dma_getn(gt + i, type, cnt);
    dma_getn(gmol + i, mol, cnt);
    for (int j = 0; j < cnt; j ++){
      pack[j].x = x[j][0];
      pack[j].y = x[j][1];
      pack[j].z = x[j][2];
      pack[j].type = type[j];
      pack[j].mol = mol[j];
    }
    //printf("%p %d\n", pack, i);
    dma_putn(out + i, pack, cnt);
  }
}
#endif
