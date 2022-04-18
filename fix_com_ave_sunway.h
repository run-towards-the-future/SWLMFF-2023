#ifdef FIX_CLASS
// clang-format off
FixStyle(com/ave/sunway,FixComAveSunway);
// clang-format on
#else

#ifndef LMP_FIX_COM_AVE_SUNWAY_H
#define LMP_FIX_COM_AVE_SUNWAY_H

#include "fix.h"
struct reduce_req_t;
namespace LAMMPS_NS {

class FixComAveSunway : public Fix {
 public:
  FixComAveSunway(class LAMMPS *, int, char **);
  ~FixComAveSunway();
  int setmask() override;
  void end_of_step() override;
  int modify_param(int narg, char **arg) override;
  void init() override;
 private:
  static constexpr int LOG_FIELDS = 3;
  double xcm_one[3];
  double log_ave[LOG_FIELDS];
  int ave_freq, nacc;
  double masstotal;
  bool log_reset_flag;
  FILE *log_com;
  void reset_log();
  void flush_log();
};
}
#endif
#endif
