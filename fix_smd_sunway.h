/* -*- c++ -*- ----------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   https://www.lammps.org/, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

#ifdef FIX_CLASS
// clang-format off
FixStyle(smd/sunway,FixSMDSunway);
// clang-format on
#else

#ifndef LMP_FIX_SMD_SUNWAY_H
#define LMP_FIX_SMD_SUNWAY_H

#include "fix_smd.h"
struct reduce_req_t;
namespace LAMMPS_NS {

class FixSMDSunway : public FixSMD {
 public:
  FixSMDSunway(class LAMMPS *, int, char **);
  ~FixSMDSunway();
  int setmask() override;
  void pre_force(int) override;
  void post_force(int) override;
  void post_force_respa(int, int, int) override;
  void end_of_step() override;
  int modify_param(int narg, char **arg) override;
 private:
  static constexpr int LOG_FIELDS = 10;
  double xcm_one[3], xcm_reduce[3];
  void smd_tether();
  void smd_tether_pre();
  void smd_couple();
  reduce_req_t *myreq;
  double log_ave[LOG_FIELDS];
  int ave_freq, nacc;
  bool log_reset_flag;
  FILE *log_smd;
  void reset_log();
  void flush_log();
};

}    // namespace LAMMPS_NS

#endif
#endif
