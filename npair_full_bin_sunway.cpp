// clang-format off
/* ----------------------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   https://www.lammps.org/, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */
#include "mpi.h"
#include "slave_fixheader.h"
#include "npair_full_bin_sunway.h"
#include "neigh_list.h"
#include "atom.h"
#include "atom_vec.h"
#include "molecule.h"
#include "domain.h"
#include "my_page.h"
#include "error.h"
#include "neighbor.h"
#include "memory.h"
#include <tuple>
using namespace LAMMPS_NS;

/* ---------------------------------------------------------------------- */

NPairFullBinSunway::NPairFullBinSunway(LAMMPS *lmp) : NPair(lmp) {
  nmax = 0;
  preallocated = nullptr;
}

/* ----------------------------------------------------------------------
   binned neighbor list construction for all neighbors
   every neighbor pair appears in list of both atoms i and j
------------------------------------------------------------------------- */
#ifdef __sw_slave__
extern "C"{
#include "slave.h"
}
#include <cassert>
void NPairFullBinSunway::parallel_build(NeighList *list)
{

  int nlocal = atom->nlocal;

  int *ilist = list->ilist;
  int *numneigh = list->numneigh;
  int **firstneigh = list->firstneigh;
  // MyPage<int> *ipage = list->ipage;

  // int inum = 0;
  // ipage->reset();
  double (*x)[3] = (double(*)[3])atom->x[0];
  int *type = atom->type;
  //if (_MYID == 0)
  for (int i = _MYID; i < nlocal; i += 64) {
    int n = 0;
    //neighptr = ipage->vget();
    int *neighptr = preallocated + i * neighbor->oneatom;
    int itype = type[i];
    double xtmp = x[i][0];
    double ytmp = x[i][1];
    double ztmp = x[i][2];
    // loop over all atoms in surrounding bins in stencil including self
    // skip i = j

    int ibin = atom2bin[i];

    for (int k = 0; k < nstencil; k++) {
      for (int j = binhead[ibin+stencil[k]]; j >= 0; j = bins[j]) {
        if (i == j) continue;

        int jtype = type[j];
        //if (exclude && exclusion(i,j,itype,jtype,mask,molecule)) continue;

        double delx = xtmp - x[j][0];
        double dely = ytmp - x[j][1];
        double delz = ztmp - x[j][2];
        double rsq = delx*delx + dely*dely + delz*delz;

        if (rsq <= cutneighsq[itype][jtype]) {
          neighptr[n++] = j;
          assert(n <= neighbor->oneatom);
        }
      }
    }

    ilist[i] = i;
    firstneigh[i] = neighptr;
    numneigh[i] = n;
  }

  list->inum = nlocal;
  list->gnum = 0;
}

void parallel_build(std::tuple<NPairFullBinSunway *, NeighList *> *param){
  std::get<0>(*param)->parallel_build(std::get<1>(*param));
}
#endif
#ifdef __sw_host__
#include "parallel_eval.h"
extern void slave_parallel_build(std::tuple<NPairFullBinSunway *, NeighList *> *);
void NPairFullBinSunway::build(NeighList *list)
{
  if (atom->nlocal > nmax) {
    if (preallocated == nullptr){
      int nlocalmax;
      MPI_Allreduce(&atom->nlocal, &nlocalmax, 1, MPI_INT, MPI_MAX, world);
      nmax = nlocalmax * 1.2;
    } else {
      nmax = atom->nlocal * 1.2;
    }
    memory->destroy(preallocated);
    memory->create(preallocated, nmax * neighbor->oneatom, "NPairFullBinSunway:preallocated list");
  }
  auto param = std::make_tuple(this, list);
  athread_resolve_spawn(slave_parallel_build, &param);
  athread_resolve_join();
}
#endif
