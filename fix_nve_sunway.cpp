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

#include "slave_fixheader.h"
#include "fix_nve_sunway.h"

#include "atom.h"
#include "error.h"
#include "force.h"
#include "respa.h"
#include "update.h"

using namespace LAMMPS_NS;
using namespace FixConst;
#ifdef __sw_slave__
#include "slave.h"
#include "swdmapp.hpp"
#include "sunway_iterator.hpp"
static constexpr int BLKSIZEI = 32;
void FixNVESunway::parallel_initial_integrate(){
  // double (*x)[3] = (double(*)[3])atom->x[0];
  // double (*v)[3] = (double(*)[3])atom->v[0];
  // double (*f)[3] = (double(*)[3])atom->f[0];
  // double *rmass = atom->rmass;

  // int *type = atom->type;
  // int *mask = atom->mask;
  double *mass = atom->mass;
  int nlocal = atom->nlocal;
  if (igroup == atom->firstgroup) nlocal = atom->nfirst;
  
  auto x = linear_inout<BLKSIZEI>((double(*)[3])atom->x[0]);
  auto v = linear_inout<BLKSIZEI>((double(*)[3])atom->v[0]);
  auto f = linear_in<BLKSIZEI>((double(*)[3])atom->f[0]);
  auto type = linear_in<BLKSIZEI>(atom->type);
  auto mask = linear_in<BLKSIZEI>(atom->mask);
  auto groupbit = this->groupbit;
  if (atom->rmass) {
    auto rmass = linear_in<BLKSIZEI>(atom->rmass);
    for (int i : parallel_iterate<BLKSIZEI>(0, nlocal, x, v, f, type, mask))
    //for (int i = 0; i < nlocal; i++)
      if (mask[i] & groupbit) {
        double dtfm = dtf / rmass[i];
        v[i][0] += dtfm * f[i][0];
        v[i][1] += dtfm * f[i][1];
        v[i][2] += dtfm * f[i][2];
        x[i][0] += dtv * v[i][0];
        x[i][1] += dtv * v[i][1];
        x[i][2] += dtv * v[i][2];
      }

  } else {
    for (int i : parallel_iterate<BLKSIZEI>(0, nlocal, x, v, f, type, mask))
      //for (int i = 0; i < nlocal; i++)
      if (mask[i] & groupbit) {
        double dtfm = dtf / mass[type[i]];
        v[i][0] += dtfm * f[i][0];
        v[i][1] += dtfm * f[i][1];
        v[i][2] += dtfm * f[i][2];
        x[i][0] += dtv * v[i][0];
        x[i][1] += dtv * v[i][1];
        x[i][2] += dtv * v[i][2];
      }
  }
  //flush_slave_cache();
}

void fixnve_parallel_initial_integrate(FixNVESunway *fix){
  long st, ed;
  asm("rcsr %0, 4\n\t" : "=r"(st) :: "memory");
  fix->parallel_initial_integrate();
  asm("rcsr %0, 4\n\t" : "=r"(ed) :: "memory");
}
void FixNVESunway::parallel_final_integrate()
{
  // update v of atoms in group

  // double **v = atom->v;
  // double **f = atom->f;
  //double *rmass = atom->rmass;
  // double *mass = atom->mass;
  // int *type = atom->type;
  // int *mask = atom->mask;
  // int nlocal = atom->nlocal;
  // if (igroup == atom->firstgroup) nlocal = atom->nfirst;
  double *mass = atom->mass;
  int nlocal = atom->nlocal;
  if (igroup == atom->firstgroup) nlocal = atom->nfirst;
  
  auto v = linear_inout<BLKSIZEI>((double(*)[3])atom->v[0]);
  auto f = linear_in<BLKSIZEI>((double(*)[3])atom->f[0]);
  auto type = linear_in<BLKSIZEI>(atom->type);
  auto mask = linear_in<BLKSIZEI>(atom->mask);
  auto groupbit = this->groupbit;

  if (atom->rmass) {
    auto rmass = linear_in<BLKSIZEI>(atom->rmass);
    for (int i : parallel_iterate<BLKSIZEI>(0, nlocal, v, f, type, mask, rmass))
      //for (int i = 0; i < nlocal; i++)
      if (mask[i] & groupbit) {
        double dtfm = dtf / rmass[i];
        v[i][0] += dtfm * f[i][0];
        v[i][1] += dtfm * f[i][1];
        v[i][2] += dtfm * f[i][2];
      }

  } else {
    for (int i : parallel_iterate<BLKSIZEI>(0, nlocal, v, f, type, mask))
      //for (int i = 0; i < nlocal; i++)
      if (mask[i] & groupbit) {
        double dtfm = dtf / mass[type[i]];
        v[i][0] += dtfm * f[i][0];
        v[i][1] += dtfm * f[i][1];
        v[i][2] += dtfm * f[i][2];
      }
  }
  //flush_slave_cache();
}
void fixnve_parallel_final_integrate(FixNVESunway *fix){
  fix->parallel_final_integrate();
}
#endif

#ifdef __sw_host__
extern "C"{
  #include <athread.h>
}
#include "parallel_eval.h"
/* ---------------------------------------------------------------------- */

FixNVESunway::FixNVESunway(LAMMPS *lmp, int narg, char **arg) :
  FixNVE(lmp, narg, arg)
{

}

/* ---------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   allow for both per-type and per-atom mass
------------------------------------------------------------------------- */
extern void slave_fixnve_parallel_initial_integrate(FixNVESunway *);
extern void slave_fixnve_parallel_final_integrate(FixNVESunway *);
void FixNVESunway::initial_integrate(int /*vflag*/)
{
  long st, ed;
  asm("rtc %0\n\t" : "=r"(st) ::"memory");
  athread_resolve_spawn(slave_fixnve_parallel_initial_integrate, this);
  athread_resolve_join();
  asm("rtc %0\n\t" : "=r"(ed) ::"memory");
  return;
}

/* ---------------------------------------------------------------------- */

void FixNVESunway::final_integrate()
{
  athread_resolve_spawn(slave_fixnve_parallel_final_integrate, this);
  athread_resolve_join();
}

#endif
