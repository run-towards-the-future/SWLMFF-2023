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
#include "fix_smd_sunway.h"
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

FixSMDSunway::FixSMDSunway(LAMMPS *lmp, int narg, char **arg) :
  FixSMD(lmp, narg, arg)
{
  myreq = new reduce_req_t;
  myreq->all = true;
  myreq->send_buf = xcm_one;
  myreq->recv_buf = xcm_reduce;
  myreq->count = 3;
  myreq->type = MPI_DOUBLE;
  myreq->op = MPI_SUM;
  myreq->comm = MPI_COMM_WORLD;
  myreq->next = nullptr;
  log_smd = nullptr;
  ave_freq = 0;
  nacc = 0;
  log_reset_flag = false;
}

FixSMDSunway::~FixSMDSunway(){
  if (log_smd != nullptr)
    fclose(log_smd);
}
/* ---------------------------------------------------------------------- */

int FixSMDSunway::setmask()
{
  int mask = 0;
  mask |= PRE_FORCE;
  mask |= END_OF_STEP;
  mask |= POST_FORCE;
  mask |= POST_FORCE_RESPA;
  return mask;
}

/* ---------------------------------------------------------------------- */

void FixSMDSunway::post_force(int vflag)
{
  // virial setup

  v_init(vflag);

  if (styleflag & SMD_TETHER) smd_tether();
  else smd_couple();

  if (styleflag & SMD_CVEL) {
    if (utils::strmatch(update->integrate_style,"^verlet"))
      r_old += v_smd * update->dt;
    else
      r_old += v_smd * ((Respa *) update->integrate)->step[ilevel_respa];
  }
}

void FixSMDSunway::pre_force(int vflag)
{
  // virial setup

  if (styleflag & SMD_TETHER) smd_tether_pre();
}
/* ---------------------------------------------------------------------- */

void FixSMDSunway::smd_tether_pre(){
  group->xcmone(igroup, masstotal, xcm_one);
  push_reduce_req(myreq);
}

int FixSMDSunway::modify_param(int narg, char **arg)
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
      if (log_smd != nullptr) fclose(log_smd);
      log_smd = fopen(log_path, log_mode);
      fprintf(log_smd, "#%9s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
              "step", "ftotal[0]", "ftotal[1]", "ftotal[2]", "fsmd", "r_old", "r_now", "pmf", "xcm[0]", "xcm[1]", "xcm[2]");
    }
    log_reset_flag = true;
    return ate;
  } else if (strcmp(arg[0], "nolog") == 0){
    if (log_smd != nullptr) {
      fclose(log_smd);
      log_smd = nullptr;
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

void FixSMDSunway::reset_log(){
  nacc = 0;
  for (int i = 0; i < LOG_FIELDS; i ++){
    log_ave[i] = 0;
  }
  log_reset_flag = false;
}
void FixSMDSunway::flush_log(){
  int nreduce = (styleflag & SMD_CVEL) ? 4 : 3;
  if (comm->me == 0){
    MPI_Reduce(MPI_IN_PLACE, log_ave, nreduce, MPI_DOUBLE, MPI_SUM, 0, world);
    for (int i = 0; i < LOG_FIELDS; i ++){
      log_ave[i] /= nacc;
    }
    fprintf(log_smd, "%10ld %10g %10g %10g %10g %10g %10g %10g %10g %10g %10g\n", update->ntimestep,
            log_ave[0], log_ave[1], log_ave[2], log_ave[3], log_ave[4], 
            log_ave[5], log_ave[6], log_ave[7], log_ave[8], log_ave[9]);
    fflush(log_smd);
  } else {
    MPI_Reduce(log_ave, nullptr, nreduce, MPI_DOUBLE, MPI_SUM, 0, world);
  }
  reset_log();
}
void FixSMDSunway::end_of_step(){
  if (ave_freq != 0){
    if (log_reset_flag) reset_log();
    log_ave[0] += ftotal[0];
    log_ave[1] += ftotal[1];
    log_ave[2] += ftotal[2];
    if (styleflag & SMD_CVEL) {
      log_ave[3] += ftotal[0]*xn+ftotal[1]*yn+ftotal[2]*zn;
    } else {
      log_ave[3] += f_smd;
    }
    log_ave[4] += r_old;
    log_ave[5] += r_now;
    log_ave[6] += pmf;
    log_ave[7] += xcm_reduce[0];
    log_ave[8] += xcm_reduce[1];
    log_ave[9] += xcm_reduce[2];
    nacc ++;
    if (nacc == ave_freq) flush_log();
  }
}

void FixSMDSunway::smd_tether()
{
  flush_reduce_req();
  double xcm[3];
  //group->xcm(igroup,masstotal,xcm);
  // if (comm->me == 0) {
  //   printf("xcm_diff: %f %f %f\n", xcm[0] - xcm_reduce[0], xcm[1] - xcm_reduce[1], xcm[2] - xcm_reduce[2]);
  //   //printf("xcm_pre: %f %f %f\n", xcm_reduce[0], xcm_reduce[1], xcm_reduce[2]);
  // }
  xcm[0] = xcm_reduce[0];
  xcm[1] = xcm_reduce[1];
  xcm[2] = xcm_reduce[2];
  
  double dt = update->dt;
  if (utils::strmatch(update->integrate_style,"^respa"))
    dt = ((Respa *) update->integrate)->step[ilevel_respa];

  // fx,fy,fz = components of k * (r-r0)
  if (comm->me == 0 && (update->ntimestep - update->beginstep) % 1000 == 0){
    printf("%ld: %g %g %g %g %g\n", update->ntimestep, xc, yc, zc, r_old, r_now);
  }
  double dx,dy,dz,fx,fy,fz,r,dr;

  dx = xcm[0] - xc;
  dy = xcm[1] - yc;
  dz = xcm[2] - zc;
  r_now = sqrt(dx*dx + dy*dy + dz*dz);

  if (!xflag) dx = 0.0;
  if (!yflag) dy = 0.0;
  if (!zflag) dz = 0.0;
  r = sqrt(dx*dx + dy*dy + dz*dz);
  if (styleflag & SMD_CVEL) {
    if (r > SMALL) {
      dr = r - r0 - r_old;
      fx = k_smd*dx*dr/r;
      fy = k_smd*dy*dr/r;
      fz = k_smd*dz*dr/r;
      pmf += (fx*xn + fy*yn + fz*zn) * v_smd * dt;
    } else {
      fx = 0.0;
      fy = 0.0;
      fz = 0.0;
    }
  } else {
    r_old = r;
    fx = f_smd*dx/r;
    fy = f_smd*dy/r;
    fz = f_smd*dz/r;
  }

  // apply restoring force to atoms in group
  // f = -k*(r-r0)*mass/masstotal

  double **x = atom->x;
  double **f = atom->f;
  imageint *image = atom->image;
  int *mask = atom->mask;
  int *type = atom->type;
  double *mass = atom->mass;
  double *rmass = atom->rmass;
  double massfrac;
  double unwrap[3],v[6];
  int nlocal = atom->nlocal;

  ftotal[0] = ftotal[1] = ftotal[2] = 0.0;
  force_flag = 0;

  if (rmass) {
    for (int i = 0; i < nlocal; i++)
      if (mask[i] & groupbit) {
        massfrac = rmass[i]/masstotal;
        f[i][0] -= fx*massfrac;
        f[i][1] -= fy*massfrac;
        f[i][2] -= fz*massfrac;
        ftotal[0] -= fx*massfrac;
        ftotal[1] -= fy*massfrac;
        ftotal[2] -= fz*massfrac;
        if (evflag) {
          domain->unmap(x[i],image[i],unwrap);
          v[0] = -fx*massfrac*unwrap[0];
          v[1] = -fy*massfrac*unwrap[1];
          v[2] = -fz*massfrac*unwrap[2];
          v[3] = -fx*massfrac*unwrap[1];
          v[4] = -fx*massfrac*unwrap[2];
          v[5] = -fy*massfrac*unwrap[2];
          v_tally(i,v);
        }
      }
  } else {
    for (int i = 0; i < nlocal; i++)
      if (mask[i] & groupbit) {
        massfrac = mass[type[i]]/masstotal;
        f[i][0] -= fx*massfrac;
        f[i][1] -= fy*massfrac;
        f[i][2] -= fz*massfrac;
        ftotal[0] -= fx*massfrac;
        ftotal[1] -= fy*massfrac;
        ftotal[2] -= fz*massfrac;
        if (evflag) {
          domain->unmap(x[i],image[i],unwrap);
          v[0] = -fx*massfrac*unwrap[0];
          v[1] = -fy*massfrac*unwrap[1];
          v[2] = -fz*massfrac*unwrap[2];
          v[3] = -fx*massfrac*unwrap[1];
          v[4] = -fx*massfrac*unwrap[2];
          v[5] = -fy*massfrac*unwrap[2];
          v_tally(i,v);
        }
      }
  }
  if (comm->me == 0 && (update->ntimestep - update->beginstep) % 1000 == 0){
    printf("ftot: %g %g %g\n", ftotal[0], ftotal[1], ftotal[2]);
  }
}

/* ---------------------------------------------------------------------- */

void FixSMDSunway::smd_couple()
{
  double xcm[3],xcm2[3];
  group->xcm(igroup,masstotal,xcm);
  group->xcm(igroup2,masstotal2,xcm2);

  double dt = update->dt;
  if (utils::strmatch(update->integrate_style,"^respa"))
    dt = ((Respa *) update->integrate)->step[ilevel_respa];

  // renormalize direction of spring
  double dx,dy,dz,r,dr;
  if (styleflag & SMD_AUTOX) dx = xcm2[0] - xcm[0];
  else dx = xn*r_old;
  if (styleflag & SMD_AUTOY) dy = xcm2[1] - xcm[1];
  else dy = yn*r_old;
  if (styleflag & SMD_AUTOZ) dz = xcm2[2] - xcm[2];
  else dz = zn*r_old;
  if (!xflag) dx = 0.0;
  if (!yflag) dy = 0.0;
  if (!zflag) dz = 0.0;
  r = sqrt(dx*dx + dy*dy + dz*dz);
  if (r > SMALL) {
    xn = dx/r; yn = dy/r; zn = dz/r;
  }

  double fx,fy,fz;
  if (styleflag & SMD_CVEL) {
    dx = xcm2[0] - xcm[0];
    dy = xcm2[1] - xcm[1];
    dz = xcm2[2] - xcm[2];
    r_now = sqrt(dx*dx + dy*dy + dz*dz);

    dx -= xn*r_old;
    dy -= yn*r_old;
    dz -= zn*r_old;

    if (!xflag) dx = 0.0;
    if (!yflag) dy = 0.0;
    if (!zflag) dz = 0.0;
    r = sqrt(dx*dx + dy*dy + dz*dz);
    dr = r - r0;

    if (r > SMALL) {
      double fsign;
      fsign  = (v_smd<0.0) ? -1.0 : 1.0;

      fx = k_smd*dx*dr/r;
      fy = k_smd*dy*dr/r;
      fz = k_smd*dz*dr/r;
      pmf += (fx*xn + fy*yn + fz*zn) * fsign * v_smd * dt;
    } else {
      fx = 0.0;
      fy = 0.0;
      fz = 0.0;
    }
  } else {
    dx = xcm2[0] - xcm[0];
    dy = xcm2[1] - xcm[1];
    dz = xcm2[2] - xcm[2];
    r_now = sqrt(dx*dx + dy*dy + dz*dz);
    r_old = r;

    fx = f_smd*xn;
    fy = f_smd*yn;
    fz = f_smd*zn;
  }

  // apply restoring force to atoms in group
  // f = -k*(r-r0)*mass/masstotal

  double **f = atom->f;
  int *mask = atom->mask;
  int *type = atom->type;
  double *mass = atom->mass;
  double *rmass = atom->rmass;
  int nlocal = atom->nlocal;

  ftotal[0] = ftotal[1] = ftotal[2] = 0.0;
  force_flag = 0;

  double massfrac;
  if (rmass) {
    for (int i = 0; i < nlocal; i++) {
      if (mask[i] & groupbit) {
        massfrac = rmass[i]/masstotal;
        f[i][0] += fx*massfrac;
        f[i][1] += fy*massfrac;
        f[i][2] += fz*massfrac;
        ftotal[0] += fx*massfrac;
        ftotal[1] += fy*massfrac;
        ftotal[2] += fz*massfrac;
      }
      if (mask[i] & group2bit) {
        massfrac = rmass[i]/masstotal2;
        f[i][0] -= fx*massfrac;
        f[i][1] -= fy*massfrac;
        f[i][2] -= fz*massfrac;
      }
    }
  } else {
    for (int i = 0; i < nlocal; i++) {
      if (mask[i] & groupbit) {
        massfrac = mass[type[i]]/masstotal;
        f[i][0] += fx*massfrac;
        f[i][1] += fy*massfrac;
        f[i][2] += fz*massfrac;
        ftotal[0] += fx*massfrac;
        ftotal[1] += fy*massfrac;
        ftotal[2] += fz*massfrac;
      }
      if (mask[i] & group2bit) {
        massfrac = mass[type[i]]/masstotal2;
        f[i][0] -= fx*massfrac;
        f[i][1] -= fy*massfrac;
        f[i][2] -= fz*massfrac;
      }
    }
  }
}

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */

void FixSMDSunway::post_force_respa(int vflag, int ilevel, int /*iloop*/)
{
  if (ilevel == ilevel_respa) post_force(vflag);
}

/* ----------------------------------------------------------------------
   return components of total smd force on fix group
------------------------------------------------------------------------- */

#endif
