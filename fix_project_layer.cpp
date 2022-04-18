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

#include "fix_project_layer.h"
#include <cstring>
#include "error.h"
#include "update.h"
#include "comm.h"
#include "domain.h"
#include "utils.h"
#include "memory.h"
#include "atom.h"
#include "molecule.h"
using namespace LAMMPS_NS;
using namespace FixConst;

/* ---------------------------------------------------------------------- */

FixProjectLayer::FixProjectLayer(LAMMPS *lmp, int narg, char **arg) :
  Fix(lmp, narg, arg)
{
  if (comm->me == 0){
    if (narg < 7) {
      error->one(FLERR, "not enough arguments for fix project/layer");
    }
    if (domain->triclinic){
      error->one(FLERR, "project/layer cannot be used with triclinic box");
    }
  }
  // for (int i = 0; i < narg; i ++){
  //   printf("%d %s\n", i, arg[i]);
  // }
  freq = 5;
  nx = 1000;
  ny = 1000;
  freq = utils::inumeric(FLERR, arg[3], true, lmp);
  nx = utils::inumeric(FLERR, arg[4], true, lmp);
  ny = utils::inumeric(FLERR, arg[5], true, lmp);
  path = utils::strdup(arg[6]);
  const char *flag = "w";
  if (narg == 8){
    if (!strcmp(arg[7], "bin")) {
      output_bin = 1;
      flag = "wb";
    } else if (!strcmp(arg[7], "text")) {
      output_bin = 0;
      flag = "w";
    } else {
      error->all(FLERR, "Invalid format specification for project/layer");
    }
  }

  pzsum = nullptr;
  pzcnt = nullptr;
  f = nullptr;
  // puts(path);
  // puts(flag);
  nmol = 0;
  laststep = -1;
  if (comm->me == 0) f = fopen(path, flag);
}

/* ---------------------------------------------------------------------- */

int FixProjectLayer::setmask()
{
  int mask = 0;
  mask |= END_OF_STEP;
  return mask;

}

void FixProjectLayer::setup(int){
  //printf("setup: %d\n", update->ntimestep);
  int nlocal = atom->nlocal;
  int nmol_new = 0;
  for (int i = 0; i < nlocal; i ++){
    if (atom->molecule[i] > nmol_new) nmol_new = atom->molecule[i];
  }
  MPI_Allreduce(&nmol_new, &nmol_new, 1, MPI_LMP_TAGINT, MPI_MAX, MPI_COMM_WORLD);
  nmol_new += 1;
  if (nmol_new > nmol){
    nmol = nmol_new;
    if (comm->me == 0){
      printf("Found %d molecules\n", nmol);
    }
    memory->destroy(pzcnt);
    memory->destroy(pzsum);
    
    memory->create(pzsum, nx, ny, nmol, "project/layer:zsum");
    memory->create(pzcnt, nx, ny, nmol, "project/layer:zcnt");
  }
  //if (update->first_update == 0)
  project();
}
void FixProjectLayer::end_of_step(){
  project();
}
#include <cassert>
template<int PX, int PY, int PZ>
void FixProjectLayer::do_projection() {
  int nlocal = atom->nlocal;
  // atom->nmolecule;
  for (int i = 0; i < nx; i ++){
    for (int j = 0; j < ny; j ++){
      for (int m = 0; m < nmol; m ++){
	pzsum[i][j][m] = 0;
	pzcnt[i][j][m] = 0;
      }
    }
  }
  double xprd = domain->xprd;
  double yprd = domain->yprd;
  double zprd = domain->zprd;
  double rxcell = nx / domain->xprd;
  double rycell = ny / domain->yprd;
  double xcell = domain->xprd / nx;
  double ycell = domain->yprd / ny;
  double xmin = domain->boxlo[0];
  double ymin = domain->boxlo[1];
  double zmin = domain->boxlo[2];
  double xmax = domain->boxhi[0];
  double ymax = domain->boxhi[1];
  double zmax = domain->boxhi[2];
  tagint *molecule = atom->molecule;
  double **x = atom->x;
  for (int i = 0; i < nlocal; i ++){
    int m = molecule[i];
    double xi = x[i][0];
    double yi = x[i][1];
    double zi = x[i][2];
    if (PX) {
      while (xi < xmin) xi += xprd;
      while (xi >= xmax) xi -= xprd;
    } else if (xi < xmin || xi >= xmax) continue;
    
    if (PY) {
      while (yi <  ymin) yi += yprd;
      while (yi >= ymax) yi -= yprd;
    } else if (yi < ymin || yi >= ymax) continue;
    
    if (PZ) {
      while (zi <  zmin) zi += zprd;
      while (zi >= zmax) zi -= zprd;
    } else if (zi < zmin || zi >= zmax) continue;

    int cx = floor((xi-xmin) * rxcell);
    int cy = floor((yi-ymin) * rycell);
    if (cx == nx) cx -= 1;
    if (cy == ny) cy -= 1;
    if (cx == -1) cx += 1;
    if (cy == -1) cy += 1;
    // pzmax[cx][cy][m] = std::max(pzmax[cx][cy][m], zi);
    // pzmin[cx][cy][m] = std::min(pzmin[cx][cy][m], zi);
    //printf("%d %d %d %f %f %f\n", cx, cy, m, zi, pzmax[cx][cy][m], pzmin[cx][cy][m]);
    assert(cx < nx && cx >= 0);
    assert(cy < ny && cy >= 0);
    assert(m >=0 && m <nmol);
    pzsum[cx][cy][m] = pzsum[cx][cy][m] + zi;
    pzcnt[cx][cy][m] = pzcnt[cx][cy][m] + 1;
  }
  // puts("mapping done");
  if (comm->me == 0){
    MPI_Reduce(MPI_IN_PLACE, &pzsum[0][0][0], nx*ny*nmol, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(MPI_IN_PLACE, &pzcnt[0][0][0], nx*ny*nmol, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  } else {
    MPI_Reduce(&pzsum[0][0][0], nullptr, nx*ny*nmol, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&pzcnt[0][0][0], nullptr, nx*ny*nmol, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  }
  //MPI_Allreduce(&pzmin_out[0][0][0], &pzmin[0][0][0], nx*ny*nmol, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
  //MPI_Reduce(pzmax_out[0][0], pzmax[0][0], nx*ny*nmol, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  //tagint ntotal, ntotal_in = nlocal;

  if (comm->me == 0){
    for (int i = 0; i < nx; i ++){
      for (int j = 0; j < ny; j ++){
	for (int m = 0; m < nmol; m ++){
	  pzsum[i][j][m] /= pzcnt[i][j][m];
	}
      }
    }
    if (!output_bin){
      fprintf(f, "%d %d %d %ld\n", nx, ny, nmol, update->ntimestep);
      for (int i = 0; i < nx; i ++){
	for (int j = 0; j < ny; j ++){
	  fprintf(f, "%f %f", (i + 0.5) * xcell + xmin, (j + 0.5) * ycell + ymin);
	  for (int m = 0; m < nmol; m ++){
	    fprintf(f, " %f", pzsum[i][j][m]);
	  }
	  fprintf(f, "\n");
	}
      }
    } else {
      struct header_t{
	long nx, ny, nmol, timestep;
	double xstart, xstep, ystart, ystep;
      } header = {nx, ny, nmol, update->ntimestep, 0.5*xcell + xmin, xcell, 0.5*ycell + ymin, ycell};

      fwrite(&header, sizeof(header), 1, f);
      fwrite(pzsum[0][0], sizeof(double), nx*ny*nmol, f);
    }
    fflush(f);
  }
}

void FixProjectLayer::project() {
  //printf("end of step %d\n", update->ntimestep);
  if (update->ntimestep % freq == 0 && update->ntimestep != laststep){
    laststep = update->ntimestep;
    if (comm->me == 0){
      printf("matched timestep %d\n", update->ntimestep);
      printf("bounding box high %f %f %f\n", domain->boxhi[0], domain->boxhi[1], domain->boxhi[2]);
      printf("bounding box low  %f %f %f\n", domain->boxlo[0], domain->boxlo[1], domain->boxlo[2]);
      printf("bounding box prd %f %f %f\n", domain->xprd, domain->yprd, domain->zprd);
    }
    if (domain->periodicity[0]){
      if (domain->periodicity[1]){
	if (domain->periodicity[2]){
	  do_projection<1, 1, 1>();
	} else {
	  do_projection<1, 1, 0>();
	}
      } else {
	if (domain->periodicity[2]){
	  do_projection<1, 0, 1>();
	} else {
	  do_projection<1, 0, 0>();
	}
      }
    } else {
      if (domain->periodicity[1]){
	if (domain->periodicity[2]){
	  do_projection<0, 1, 1>();
	} else {
	  do_projection<0, 1, 0>();
	}
      } else {
	if (domain->periodicity[2]){
	  do_projection<0, 0, 1>();
	} else {
	  do_projection<0, 0, 0>();
	}
      }
    }
  }
}
