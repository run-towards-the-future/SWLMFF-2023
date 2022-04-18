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
PairStyle(lmff/sunway,PairLMFFSunway);
// clang-format on
#else

#ifndef LMP_PAIR_LMFF_SUNWAY_H
#define LMP_PAIR_LMFF_SUNWAY_H
constexpr int NEWTON_MASK = 1 << 30;
constexpr int NEWTON_SHIFT = 30;
#include "pair.h"
#include "pair_tersoff.h"
struct cal_lock_t;
struct atompack_t;
struct atompack_mol_t;
template<int>
struct tallyvar;
namespace LAMMPS_NS {

class PairLMFFSunway : public Pair {
 public:
  PairLMFFSunway(class LAMMPS *);
  virtual ~PairLMFFSunway();

  virtual void compute(int, int);
  void settings(int, char **);
  void coeff(int, char **);
  double init_one(int, int);
  void init_style();
  //void calc_normal();
  void calc_single_normal(int i, int *ILP_neigh, int nneigh, double *normal, double (*dnormdri)[3], double (*dnormdrk)[3][3]);
  //void calc_FRep(int, int);
  //void calc_FvdW(int, int);
  void update_internal_list();
  void build_layered_list();
  double single(int, int, int, int, double, double, double, double &);
  //void eval_tersoff(tallyvar<2> &tally, double (*fk)[3], double (*dk)[3], int itype, int *ktypes, bool *newton, int nneigh);
    //void eval_tersoff(double (*dk)[3], int *ktypes, int nneigh);
  template<int,int,int> void eval();
  template<int,int> void eval_tersoff(tallyvar<2> &tally, double (*fk)[3], double (*rk)[3], int itype, int *ktypes, bool *newton, int nneigh);
  static constexpr int NPARAMS_PER_LINE = 13;
  
  class ProxyTersoff: public PairTersoff{
  public :
    ProxyTersoff(class LAMMPS *lmp) : PairTersoff(lmp){}
    void coeff(int narg, char **arg);
    void export_params(PairLMFFSunway *parent);
  static double fc(double, Param *);
  static double fc_d(double, Param *);
  static double fa(double, Param *);
  static double fa_d(double, Param *);
  static double bij(double, Param *);
  static double bij_d(double, Param *);

  static void zetaterm_d(double, double *, double, double, double *, double, double, double *, double *, double *, Param *);
    static void costheta_d(double *rij_hat, double rijinv,
                             double *rik_hat, double rikinv,
			   double *dri, double *drj, double *drk);
    static inline double calc_gijk(const double costheta, const Param *const param)
  {
    const double ters_c = param->c * param->c;
    const double ters_d = param->d * param->d;
    const double hcth = param->h - costheta;

    return param->gamma * (1.0 + ters_c / ters_d - ters_c / (ters_d + hcth * hcth));
  }
    static 
    inline double calc_gijk_d(const double costheta, const Param *const param)
    {
      const double ters_c = param->c * param->c;
      const double ters_d = param->d * param->d;
      const double hcth = param->h - costheta;
      const double numerator = -2.0 * ters_c * hcth;
      const double denominator = 1.0 / (ters_d + hcth * hcth);
      return param->gamma * numerator * denominator * denominator;
    }

    friend class PairLMFFSunway;
  };
 protected:
  int me;
  int maxlocal;            // size of numneigh, firstneigh arrays
  int tap_flag;            // flag to turn on/off taper function

  struct Param {
    double z0, alpha, epsilon, C, delta, d, sR, reff, C6, S;
    double delta2inv, seff, lambda, rcut, seffinv;
    int ielement, jelement;
  };
  Param *params;    // parameter set for I-J interactions
  //int nmax;         // max # of atoms
  ProxyTersoff::Param *tersoff_params;
  int tersoff_nparams, ***tersoff_elem3param;
  double cut_global;
  double cut_normal;
  double **cutILPsq;    // mapping the cutoff from element pairs to parameters
  double **offset;
  double **cutinv;
  Param **type_params;
  double **type_cutILPsq;
  int *layered_neigh;
  int *bundled_neigh;
  //int **first_layered_neigh;
  //int *num_intra, *num_inter, *num_vdw;
  struct layer_neigh_info {
    int first, nintra, ninter, nintervdw;
    int firstbundled, nbundled;
  };
  layer_neigh_info *neigh_index;
  int inum_max, jnum_max, nmax;
  atompack_mol_t *atompack;
  cal_lock_t *force_locks;
  int fmax;
  void read_file(char *);
  void allocate();
public:
  template<int EFLAG, int VFLAG_EITHER, int TAP_FLAG>
  void parallel_eval();
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

E: All pair coeffs are not set

All pair coefficients must be set in the data file or by the
pair_coeff command before running a simulation.

*/
