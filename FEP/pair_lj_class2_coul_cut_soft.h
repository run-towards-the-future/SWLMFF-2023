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

#ifdef PAIR_CLASS
// clang-format off
PairStyle(lj/class2/coul/cut/soft,PairLJClass2CoulCutSoft);
// clang-format on
#else

#ifndef LMP_PAIR_LJ_CLASS2_COUL_CUT_SOFT_H
#define LMP_PAIR_LJ_CLASS2_COUL_CUT_SOFT_H

#include "pair.h"

namespace LAMMPS_NS {

class PairLJClass2CoulCutSoft : public Pair {
 public:
  PairLJClass2CoulCutSoft(class LAMMPS *);
  ~PairLJClass2CoulCutSoft() override;
  void compute(int, int) override;
  void settings(int, char **) override;
  void coeff(int, char **) override;
  void init_style() override;
  double init_one(int, int) override;
  void write_restart(FILE *) override;
  void read_restart(FILE *) override;
  void write_restart_settings(FILE *) override;
  void read_restart_settings(FILE *) override;
  void write_data(FILE *) override;
  void write_data_all(FILE *) override;
  double single(int, int, int, int, double, double, double, double &) override;
  void *extract(const char *, int &) override;

 protected:
  double cut_lj_global, cut_coul_global;
  double **cut_lj, **cut_ljsq;
  double **cut_coul, **cut_coulsq;
  double **epsilon, **sigma, **lambda;
  double nlambda, alphalj, alphac;
  double **lj1, **lj2, **lj3, **lj4, **offset;

  virtual void allocate();
};

}    // namespace LAMMPS_NS

#endif
#endif

/* ERROR/WARNING messages:

E: Illegal ... command

Self-explanatory.  Check the input script syntax and compare to the
documentation for the command.  You can use -echo screen as a
command-line option when running LAMMPS to see the offending line.

E: Incorrect args for pair coefficients

Self-explanatory.  Check the input script or data file.

E: Pair style lj/class2/coul/cut/soft requires atom attribute q

The atom style defined does not have this attribute.

E: Pair lj/class2/coul/cut/soft different lambda values in mix

The value of lambda has to be the same for I J pairs.

*/