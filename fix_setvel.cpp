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

#include "fix_setvel.h"

#include "atom.h"
#include "domain.h"
#include "error.h"
#include "input.h"
#include "memory.h"
#include "modify.h"
#include "region.h"
#include "respa.h"
#include "update.h"
#include "variable.h"

#include <cstring>

using namespace LAMMPS_NS;
using namespace FixConst;

enum{NONE,CONSTANT,EQUAL,ATOM};

/* ---------------------------------------------------------------------- */

FixSetVel::FixSetVel(LAMMPS *lmp, int narg, char **arg) :
  Fix(lmp, narg, arg)
{
  if (narg < 6) error->all(FLERR,"Illegal fix setvel command");

  vector_flag = 1;
  size_vector = 3;
  global_freq = 1;
  extvector = 1;
  xstr = ystr = zstr = nullptr;

  if (utils::strmatch(arg[3],"^v_")) {
    xstr = utils::strdup(arg[3]+2);
  } else if (strcmp(arg[3],"NULL") == 0) {
    xstyle = NONE;
  } else {
    xvalue = utils::numeric(FLERR,arg[3],false,lmp);
    xstyle = CONSTANT;
  }
  if (utils::strmatch(arg[4],"^v_")) {
    ystr = utils::strdup(arg[4]+2);
  } else if (strcmp(arg[4],"NULL") == 0) {
    ystyle = NONE;
  } else {
    yvalue = utils::numeric(FLERR,arg[4],false,lmp);
    ystyle = CONSTANT;
  }
  if (utils::strmatch(arg[5],"^v_")) {
    zstr = utils::strdup(arg[5]+2);
  } else if (strcmp(arg[5],"NULL") == 0) {
    zstyle = NONE;
  } else {
    zvalue = utils::numeric(FLERR,arg[5],false,lmp);
    zstyle = CONSTANT;
  }

  force_flag = 0;
  foriginal[0] = foriginal[1] = foriginal[2] = 0.0;

  maxatom = 1;
}

/* ---------------------------------------------------------------------- */

FixSetVel::~FixSetVel()
{
  if (copymode) return;

  delete [] xstr;
  delete [] ystr;
  delete [] zstr;
}

/* ---------------------------------------------------------------------- */

int FixSetVel::setmask()
{
  int mask = 0;
  mask |= POST_FORCE;
  mask |= POST_FORCE_RESPA;
  mask |= MIN_POST_FORCE;
  return mask;
}

/* ---------------------------------------------------------------------- */

void FixSetVel::init()
{
  // check variables

  if (xstr) {
    xvar = input->variable->find(xstr);
    if (xvar < 0)
      error->all(FLERR,"Variable name for fix setforce does not exist");
    if (input->variable->equalstyle(xvar)) xstyle = EQUAL;
    else if (input->variable->atomstyle(xvar)) xstyle = ATOM;
    else error->all(FLERR,"Variable for fix setforce is invalid style");
  }
  if (ystr) {
    yvar = input->variable->find(ystr);
    if (yvar < 0)
      error->all(FLERR,"Variable name for fix setforce does not exist");
    if (input->variable->equalstyle(yvar)) ystyle = EQUAL;
    else if (input->variable->atomstyle(yvar)) ystyle = ATOM;
    else error->all(FLERR,"Variable for fix setforce is invalid style");
  }
  if (zstr) {
    zvar = input->variable->find(zstr);
    if (zvar < 0)
      error->all(FLERR,"Variable name for fix setforce does not exist");
    if (input->variable->equalstyle(zvar)) zstyle = EQUAL;
    else if (input->variable->atomstyle(zvar)) zstyle = ATOM;
    else error->all(FLERR,"Variable for fix setforce is invalid style");
  }

  if (xstyle == ATOM || ystyle == ATOM || zstyle == ATOM)
    varflag = ATOM;
  else if (xstyle == EQUAL || ystyle == EQUAL || zstyle == EQUAL)
    varflag = EQUAL;
  else varflag = CONSTANT;

  // cannot use non-zero forces for a minimization since no energy is integrated
  // use fix addforce instead
}

/* ---------------------------------------------------------------------- */

void FixSetVel::setup(int vflag)
{
  if (utils::strmatch(update->integrate_style,"^verlet"))
    post_force(vflag);
  else
    error->all(FLERR, "Only verlet is supported");
}

/* ---------------------------------------------------------------------- */

void FixSetVel::post_force(int /*vflag*/)
{
  double **x = atom->x;
  double **v = atom->v;
  double **f = atom->f;
  int *mask = atom->mask;
  int nlocal = atom->nlocal;

  foriginal[0] = foriginal[1] = foriginal[2] = 0.0;
  force_flag = 0;

  if (varflag == CONSTANT) {
    for (int i = 0; i < nlocal; i++)
      if (mask[i] & groupbit) {
        foriginal[0] += f[i][0];
        foriginal[1] += f[i][1];
        foriginal[2] += f[i][2];
        if (xstyle) v[i][0] = xvalue;
        if (ystyle) v[i][1] = yvalue;
        if (zstyle) v[i][2] = zvalue;
        if (xstyle) f[i][0] = 0;
        if (ystyle) f[i][1] = 0;
        if (zstyle) f[i][2] = 0;
      }

  // variable force, wrap with clear/add

  } else {
    error->all(FLERR, "currently not supported for variables");
  }
}

/* ---------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
   return components of total force on fix group before force was changed
------------------------------------------------------------------------- */

double FixSetVel::compute_vector(int n)
{
  // only sum across procs one time

  if (force_flag == 0) {
    MPI_Allreduce(foriginal,foriginal_all,3,MPI_DOUBLE,MPI_SUM,world);
    force_flag = 1;
  }
  return foriginal_all[n];
}

/* ----------------------------------------------------------------------
   memory usage of local atom-based array
------------------------------------------------------------------------- */

double FixSetVel::memory_usage()
{
  double bytes = 0.0;
  return bytes;
}
