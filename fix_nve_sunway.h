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
FixStyle(nve/sunway,FixNVESunway);
// clang-format on
#else

#ifndef LMP_FIX_NVE_SUNWAY_H
#define LMP_FIX_NVE_SUNWAY_H

#include "fix.h"
#include "fix_nve.h"
namespace LAMMPS_NS {

class FixNVESunway : public FixNVE {
 public:
  FixNVESunway(class LAMMPS *, int, char **);

  void initial_integrate(int) override;
  void final_integrate() override;
  void parallel_initial_integrate();
  void parallel_final_integrate();
  // void initial_integrate_respa(int, int, int) override;
  // void final_integrate_respa(int, int) override;
  // void reset_dt() override;

 // protected:
 //  double dtv, dtf;
 //  double *step_respa;
 //  int mass_require;
};

}    // namespace LAMMPS_NS

#endif
#endif

/* ERROR/WARNING messages:

E: Illegal ... command

Self-explanatory.  Check the input script syntax and compare to the
documentation for the command.  You can use -echo screen as a
command-line option when running LAMMPS to see the offending line.

*/
