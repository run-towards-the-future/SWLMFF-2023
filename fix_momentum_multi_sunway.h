#ifdef FIX_CLASS
// clang-format off
FixStyle(momentum/multi/sunway,FixMomentumMultiSunway);
// clang-format on
#else

#ifndef LMP_FIX_MOMENTUM_MULTI_SUNWAY_H
#define LMP_FIX_MOMENTUM_MULTI_SUNWAY_H

#include "fix.h"
struct reduce_req_t;
namespace LAMMPS_NS {

class FixMomentumMultiSunway : public Fix {
 public:
  FixMomentumMultiSunway(class LAMMPS *, int, char **);
  ~FixMomentumMultiSunway();
  int setmask() override;
  void init() override;
  void pre_force(int) override;
  void end_of_step() override;

 protected:
  int ngroups;
  int xcm_cnt, end_step_cnt;
  double *xcm_buf, *end_step_buf;
  reduce_req_t *my_req;
  struct MomGroup {
    double masstotal;
    int xcm_off, end_step_off;
    int igroup;
    int xflag, yflag, zflag, angular;
  };
  MomGroup *groups;
};

}    // namespace LAMMPS_NS

#endif
#endif
