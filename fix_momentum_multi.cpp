#include "slave_fixheader.h"
#ifdef __sw_host__
#include "fix_momentum_multi_sunway.h"

#include "atom.h"
#include "domain.h"
#include "error.h"
#include "group.h"
#include "memory.h"

#include "parallel_eval.h"
#include <cmath>
#include <cstring>

using namespace LAMMPS_NS;
using namespace FixConst;

FixMomentumMultiSunway::FixMomentumMultiSunway(LAMMPS *lmp, int narg, char **arg) : Fix(lmp, narg, arg) {
  if (narg < 5) error->all(FLERR,"Illegal fix momentum/multi/sunway command");
  nevery = utils::inumeric(FLERR,arg[3],false,lmp);
  if (nevery <= 0) error->all(FLERR,"Illegal fix momentum/multi/sunway command");
  ngroups = utils::inumeric(FLERR,arg[4],false,lmp);
  memory->create(groups, ngroups, "FixMomentumMultiSunway:groups");
  int iarg = 5;
  int ig = 0;
  xcm_cnt = 0;
  end_step_cnt = 0;
  while (iarg < narg){
    const char *groupname = arg[iarg];
    int groupid = group->find(groupname);
    if (groupid == -1)
      error->all(FLERR,"Could not find fix group ID");
    groups[ig].igroup = groupid;
    groups[ig].xcm_off = xcm_cnt;
    groups[ig].end_step_off = end_step_cnt;
    if (narg <= iarg + 1) error->all(FLERR,"Illegal fix momentum/multi/sunway command");
    if (strcmp(arg[iarg+1], "linear") == 0){
      if (narg <= iarg + 4) error->all(FLERR,"Illegal fix momentum/multi/sunway command");
      groups[ig].xflag = utils::inumeric(FLERR,arg[iarg+2],false,lmp);
      groups[ig].yflag = utils::inumeric(FLERR,arg[iarg+3],false,lmp);
      groups[ig].zflag = utils::inumeric(FLERR,arg[iarg+4],false,lmp);
      groups[ig].angular = 0;
      end_step_cnt += 3;
      iarg += 5;
    } else if (strcmp(arg[iarg+1], "angular") == 0) {
      groups[ig].angular = 1;
      xcm_cnt += 3;
      end_step_cnt += 12;
      iarg += 2;
    } else error->all(FLERR,"Illegal fix momentum/multi/sunway command");
    ig += 1;
  }
  memory->create(xcm_buf, xcm_cnt, "FixMomentumMultiSunway:xcm");
  memory->create(end_step_buf, end_step_cnt, "FixMomentumMultiSunway:xcm");
  memory->create(my_req, 1, "FixMomentumMultiSunway:req");

  my_req->all = true;
  my_req->send_buf = MPI_IN_PLACE;
  my_req->recv_buf = xcm_buf;
  my_req->count = 3;
  my_req->type = MPI_DOUBLE;
  my_req->op = MPI_SUM;
  my_req->comm = MPI_COMM_WORLD;
  my_req->next = nullptr;
}

int FixMomentumMultiSunway::setmask(){
  return PRE_FORCE | END_OF_STEP;
}
void FixMomentumMultiSunway::init(){
  for (int i = 0; i < ngroups; i ++){
    groups[i].masstotal = group->mass(groups[i].igroup);
  }
}
void FixMomentumMultiSunway::pre_force(int){
  for (int i = 0; i < ngroups; i ++){
    MomGroup &grp = groups[i];
    if (grp.angular){
      group->xcmone(grp.igroup, grp.masstotal, xcm_buf + grp.xcm_off);
    }
  }
  push_reduce_req(my_req);
}

void FixMomentumMultiSunway::end_of_step(){
  flush_reduce_req();
  for (int i = 0; i < ngroups; i ++){
    MomGroup &grp = groups[i];
    if (grp.angular){
      //group->xcmone(grp.igroup, grp.masstotal, xcm_buf + grp.xcm_off);
      group->angmom_one(grp.igroup, xcm_buf + grp.xcm_off, end_step_buf + grp.end_step_off);
      group->inertia_one(grp.igroup, xcm_buf + grp.xcm_off, (double(*)[3])(end_step_buf + grp.end_step_off + 3));
    } else {
      group->vcmone(grp.igroup, grp.masstotal, end_step_buf + grp.end_step_off);
    }
  }
  MPI_Allreduce(MPI_IN_PLACE, end_step_buf, end_step_cnt, MPI_DOUBLE, MPI_SUM, world);
  int nlocal = atom->nlocal;
  int *mask = atom->mask;
  imageint *image = atom->image;
  double (*x)[3] = (double(*)[3])atom->x[0];
  double (*v)[3] = (double(*)[3])atom->v[0];
  for (int i = 0; i < ngroups; i ++){
    MomGroup &grp = groups[i];
    int groupbit = group->bitmask[igroup];
    if (grp.angular){
      //group->xcmone(grp.igroup, grp.masstotal, xcm_buf + grp.xcm_off);
      double *xcm = xcm_buf + grp.xcm_off;
      double *angmom = end_step_buf + grp.end_step_off;
      double (*inertia)[3] = (double(*)[3])(end_step_buf + grp.end_step_off + 3);
      double omega[3];
      group->omega(angmom, inertia, omega);
      double dx,dy,dz;
      double unwrap[3];
      for (int i = 0; i < nlocal; i++)
	if (mask[i] & groupbit) {
	  domain->unmap(x[i],image[i],unwrap);
	  dx = unwrap[0] - xcm[0];
	  dy = unwrap[1] - xcm[1];
	  dz = unwrap[2] - xcm[2];
	  v[i][0] -= omega[1]*dz - omega[2]*dy;
	  v[i][1] -= omega[2]*dx - omega[0]*dz;
	  v[i][2] -= omega[0]*dy - omega[1]*dx;
	}

    } else {
      double *vcm = end_step_buf + grp.end_step_off;
      
      for (int i = 0; i < nlocal; i++)
	if (mask[i] & groupbit) {
	  if (grp.xflag) v[i][0] -= vcm[0];
	  if (grp.yflag) v[i][1] -= vcm[1];
	  if (grp.zflag) v[i][2] -= vcm[2];
	}
    }
  }
}

FixMomentumMultiSunway::~FixMomentumMultiSunway(){
  memory->destroy(my_req);
  memory->destroy(groups);
  memory->destroy(xcm_buf);
  memory->destroy(end_step_buf);
}
#endif
