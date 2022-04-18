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

/* ----------------------------------------------------------------------
   Contributing author: Xiaohui Duan
   based on fix SMD by: Axel Kohlmeyer (UPenn)
------------------------------------------------------------------------- */
#include "slave_fixheader.h"
#ifdef __sw_host__
#include "fix_com_ave_sunway.h"
#include "parallel_eval.h"
#include "atom.h"
#include "comm.h"
#include "domain.h"
#include "error.h"
#include "group.h"
#include "respa.h"
#include "update.h"

#include <cmath>
#include <cstring>

using namespace LAMMPS_NS;
using namespace FixConst;

enum { SMD_NONE=0,
       SMD_TETHER=1<<0, SMD_COUPLE=1<<1,
       SMD_CVEL=1<<2, SMD_CFOR=1<<3,
       SMD_AUTOX=1<<4, SMD_AUTOY=1<<5, SMD_AUTOZ=1<<6};

#define SMALL 0.001

/* ---------------------------------------------------------------------- */

FixComAveSunway::FixComAveSunway(LAMMPS *lmp, int narg, char **arg) :
  Fix(lmp, narg, arg)
{
  log_com = nullptr;
  ave_freq = 0;
  nacc = 0;
  log_reset_flag = false;
  masstotal = 0.0;
}

FixComAveSunway::~FixComAveSunway(){
  if (log_com != nullptr)
    fclose(log_com);
}
/* ---------------------------------------------------------------------- */

void FixComAveSunway::init(){
  masstotal = group->mass(igroup);
}
int FixComAveSunway::setmask()
{
  int mask = 0;
  mask |= PRE_FORCE;
  mask |= END_OF_STEP;
  return mask;
}

int FixComAveSunway::modify_param(int narg, char **arg)
{
  if (strcmp(arg[0], "log") == 0){
    if (narg < 2) return 0;
    const char *log_path = arg[1];
    const char *log_mode = "w";
    log_reset_flag = true;
    int ate = 2;
    if (narg > 2 && strcmp(arg[2], "append") == 0){
      log_mode = "a";
      ate ++;
    }
    if (comm->me == 0){
      if (log_com != nullptr) fclose(log_com);
      log_com = fopen(log_path, log_mode);
      fprintf(log_com, "#%9s %10s %10s %10s\n",
              "step", "xcm[0]", "xcm[1]", "xcm[2]");
    }
    log_reset_flag = true;
    return ate;
  } else if (strcmp(arg[0], "nolog") == 0){
    if (log_com != nullptr) {
      fclose(log_com);
      log_com = nullptr;
    }
    ave_freq = 0;
    return 1;
  } else if (strcmp(arg[0], "ave") == 0){
    if (narg < 2) return 0;
    ave_freq = utils::inumeric(FLERR, arg[1], true, lmp);
    log_reset_flag = true;
    return 2;
  }
  return 0;
}

void FixComAveSunway::reset_log(){
  nacc = 0;
  for (int i = 0; i < LOG_FIELDS; i ++){
    log_ave[i] = 0;
  }
  log_reset_flag = false;
}
void FixComAveSunway::flush_log(){
  if (comm->me == 0){
    MPI_Reduce(MPI_IN_PLACE, log_ave, 3, MPI_DOUBLE, MPI_SUM, 0, world);
    for (int i = 0; i < LOG_FIELDS; i ++){
      log_ave[i] /= nacc;
    }
    fprintf(log_com, "%10ld %10g %10g %10g\n", update->ntimestep,
            log_ave[0], log_ave[1], log_ave[2]);
    fflush(log_com);
  } else {
    MPI_Reduce(log_ave, nullptr, 3, MPI_DOUBLE, MPI_SUM, 0, world);
  }
  reset_log();
}
void FixComAveSunway::end_of_step(){
  group->xcmone(igroup, masstotal, xcm_one);
  if (ave_freq != 0){
    if (log_reset_flag) reset_log();
    log_ave[0] += xcm_one[0];
    log_ave[1] += xcm_one[1];
    log_ave[2] += xcm_one[2];
    nacc += 1;
    if (nacc == ave_freq) flush_log();
  }
}

#endif
