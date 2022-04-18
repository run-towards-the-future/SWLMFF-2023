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

#include "lammps.h"

#include "accelerator_kokkos.h"
#include "input.h"
#include "lmppython.h"
#include "gptl.h"

#if defined(LAMMPS_EXCEPTIONS)
#include "exceptions.h"
#endif

#include <cstdlib>
#include <mpi.h>

#if defined(LAMMPS_TRAP_FPE) && defined(_GNU_SOURCE)
#include <fenv.h>
#endif

// import MolSSI Driver Interface library
#if defined(LMP_MDI)
#include <mdi.h>
#endif
#ifdef __sw_64__
extern "C"{
#include <athread.h>
}
#include "qthread.h"
#include "bt.h"
#include "init_diag.h"
#endif
using namespace LAMMPS_NS;

/* ----------------------------------------------------------------------
   main program to drive LAMMPS
------------------------------------------------------------------------- */

int __rank;

int main(int argc, char **argv)
{
#ifdef __sw_64__
  init_pool(256, 262144);
  athread_init();
  CRTS_init();
#endif
  MPI_Init(&argc, &argv);
  GPTLinitialize();


#ifdef __sw_64__
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (getenv("WITH_PROF")){
    init_prof_(&rank);
    start_prof();
  }
  if (getenv("WITH_MLOG")){
    init_mlog_(&rank);
  }
#endif
  MPI_Comm lammps_comm = MPI_COMM_WORLD;

#if defined(LMP_MDI)
  // initialize MDI interface, if compiled in

  int mdi_flag;
  if (MDI_Init(&argc, &argv)) MPI_Abort(MPI_COMM_WORLD, 1);
  if (MDI_Initialized(&mdi_flag)) MPI_Abort(MPI_COMM_WORLD, 1);

  // get the MPI communicator that spans all ranks running LAMMPS
  // when using MDI, this may be a subset of MPI_COMM_WORLD

  if (mdi_flag)
    if (MDI_MPI_get_world_comm(&lammps_comm)) MPI_Abort(MPI_COMM_WORLD, 1);
#endif

#if defined(LAMMPS_TRAP_FPE) && defined(_GNU_SOURCE)
  // enable trapping selected floating point exceptions.
  // this uses GNU extensions and is only tested on Linux
  // therefore we make it depend on -D_GNU_SOURCE, too.
  fesetenv(FE_NOMASK_ENV);
  fedisableexcept(FE_ALL_EXCEPT);
  feenableexcept(FE_DIVBYZERO);
  feenableexcept(FE_INVALID);
  feenableexcept(FE_OVERFLOW);
#endif

  GPTLstart("Lammps");
#ifdef LAMMPS_EXCEPTIONS
  try {
    LAMMPS *lammps = new LAMMPS(argc, argv, lammps_comm);
    lammps->input->file();
    delete lammps;
  } catch (LAMMPSAbortException &ae) {
    KokkosLMP::finalize();
    Python::finalize();
    MPI_Abort(ae.universe, 1);
  } catch (LAMMPSException &) {
    KokkosLMP::finalize();
    Python::finalize();
    MPI_Barrier(lammps_comm);
    MPI_Finalize();
    exit(1);
  } catch (fmt::format_error &fe) {
    fprintf(stderr, "fmt::format_error: %s\n", fe.what());
    KokkosLMP::finalize();
    Python::finalize();
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
  }
#else
  try {
    LAMMPS *lammps = new LAMMPS(argc, argv, lammps_comm);
    lammps->input->file();
    delete lammps;
  } catch (fmt::format_error &fe) {
    fprintf(stderr, "fmt::format_error: %s\n", fe.what());
    KokkosLMP::finalize();
    Python::finalize();
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
  }
#endif

  GPTLstop("Lammps");

#ifdef __sw_64__
  if (getenv("WITH_PROF")){
    pause_prof();
    stop_prof();
  }
  if (getenv("WITH_MLOG")){
    stop_mlog();
  }
#endif
  KokkosLMP::finalize();
  Python::finalize();

  // if(__rank == 0) 
  //   GPTLpr_file("LammpsTime.0");
    
  GPTLpr_summary_file(MPI_COMM_WORLD, "LammpsTime.summary");

  MPI_Barrier(lammps_comm);
  MPI_Finalize();
}
