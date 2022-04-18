/* ----------------------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   https://www.lammps.org/, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   Contributing author: Wengen Ouyang (Tel Aviv University)
   e-mail: w.g.ouyang at gmail dot com

   This is a full version of the potential described in
   [Maaravi et al, J. Phys. Chem. C 121, 22826-22835 (2017)]
   The definition of normals are the same as that in
   [Kolmogorov & Crespi, Phys. Rev. B 71, 235415 (2005)]
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   Optimization author: Xiaohui Duan (National Supercomputing Center in Wuxi)
   e-mail: sunrise_duan at 126 dot com

   Provides some bugfixes and performance optimizations in this potential.
*/
#include "mpi.h"
#include "slave_fixheader.h"
#include "pair_lmff_sunway.h"
#include "pair_tersoff.h"
#include "atom.h"
#include "citeme.h"
#include "comm.h"
#include "error.h"
#include "force.h"
#include "interlayer_taper.h"
#include "memory.h"
#include "my_page.h"
#include "neigh_list.h"
#include "neigh_request.h"
#include "neighbor.h"
#include "potential_file_reader.h"
#include "math_const.h"
#include "math_extra.h"
#include "math_special.h"


#include "gptl.h"

#include "tokenizer.h"
//#include "ilp_time.h"
#include <cmath>
#include <cstddef>
#include <cstring>
#include "cal.h"
#include <simd.h>
#include <cassert>
using namespace LAMMPS_NS;
using namespace InterLayer;
using namespace MathConst;
using namespace MathSpecial;
using namespace MathExtra;
#include "parallel_eval.h"
#include "util_sunway.h"
#include <sys/cdefs.h>
#ifdef __sw_host__
int internal_list_count = 0;
#else
extern int internal_list_count;
#endif
namespace LAMMPS_NS{
class PairTersoffInsp : public PairTersoff {
public:
PairTersoffInsp(class LAMMPS *lmp) : PairTersoff(lmp){};
friend class PairLMFFSunway;
};
};
constexpr int BLKSIZEI = 8;
#ifdef __sw_slave__
template void fill_mem(std::tuple<cal_lock_t *, cal_lock_t, size_t> *);
#include "math_sunway.hpp"
#define sin fsin_reduced
#define cos fcos_reduced
#define exp fexp_valid
#define log flog_valid
#define pow(x, y) exp(log(x)*(y))

//#define sin fsin
//#define cos fcos
//#define exp fexp
//#define log flog
//#define pow(x, y) exp(log(x)*(y))

#endif

static inline double calc_Tap_opt(double r_ij, double Rcutinv)
{
  double Tap, r;

  r = r_ij * Rcutinv;
  if (r >= 1.0) {
    Tap = 0.0;
  } else {
    Tap = Tap_coeff[7] * r + Tap_coeff[6];
    Tap = Tap * r + Tap_coeff[5];
    Tap = Tap * r + Tap_coeff[4];
    Tap = Tap * r + Tap_coeff[3];
    Tap = Tap * r + Tap_coeff[2];
    Tap = Tap * r + Tap_coeff[1];
    Tap = Tap * r + Tap_coeff[0];
  }

  return (Tap);
}

  /* ----Calculate the derivatives of long-range cutoff term */
static inline double calc_dTap_opt(double r_ij, double Rcutinv)
{
  double dTap, r;

  r = r_ij * Rcutinv;
  if (r >= 1.0) {
    dTap = 0.0;
  } else {
    dTap = 7.0 * Tap_coeff[7] * r + 6.0 * Tap_coeff[6];
    dTap = dTap * r + 5.0 * Tap_coeff[5];
    dTap = dTap * r + 4.0 * Tap_coeff[4];
    dTap = dTap * r + 3.0 * Tap_coeff[3];
    dTap = dTap * r + 2.0 * Tap_coeff[2];
    dTap = dTap * r + Tap_coeff[1];
    dTap = dTap * Rcutinv;
  }

  return (dTap);
}

__always_inline void Tap_opt(double &Tap, double &dTap, double r_ij, double Rcutinv){
  double r;

  r = r_ij * Rcutinv;
  // if (r >= 1.0) {
  //   dTap = 0.0;
  //   Tap = 0.0;
  // } else {
  dTap = 7.0 * Tap_coeff[7] * r + 6.0 * Tap_coeff[6];
  dTap = dTap * r + 5.0 * Tap_coeff[5];
  dTap = dTap * r + 4.0 * Tap_coeff[4];
  dTap = dTap * r + 3.0 * Tap_coeff[3];
  dTap = dTap * r + 2.0 * Tap_coeff[2];
  dTap = dTap * r + Tap_coeff[1];
  dTap = dTap * Rcutinv;
    
  Tap = Tap_coeff[7] * r + Tap_coeff[6];
  Tap = Tap * r + Tap_coeff[5];
  Tap = Tap * r + Tap_coeff[4];
  Tap = Tap * r + Tap_coeff[3];
  Tap = Tap * r + Tap_coeff[2];
  Tap = Tap * r + Tap_coeff[1];
  Tap = Tap * r + Tap_coeff[0];

  // }

}

#ifdef __sw_slave__
__always_inline void Tap_optv8(doublev8 &Tap, doublev8 &dTap, doublev8 r_ij, doublev8 Rcutinv, doublev8 *Tap_coeffv8, doublev8 *dTap_coeffv8){
  doublev8 r;

  r = r_ij * Rcutinv;
  // if (r >= 1.0) {
  //   dTap = 0.0;
  //   Tap = 0.0;
  // } else {
  // dTap = 7.0 * Tap_coeff[7] * r + 6.0 * Tap_coeff[6];
  // dTap = dTap * r + 5.0 * Tap_coeff[5];
  // dTap = dTap * r + 4.0 * Tap_coeff[4];
  // dTap = dTap * r + 3.0 * Tap_coeff[3];
  // dTap = dTap * r + 2.0 * Tap_coeff[2];
  // dTap = dTap * r + Tap_coeff[1];
  // dTap = dTap * Rcutinv;
    

  doublev8 r2 = r * r;
  doublev8 dTap_odd = dTap_coeffv8[7] * r2 + dTap_coeffv8[5];
  dTap_odd = dTap_odd * r2 + dTap_coeffv8[3];
  dTap_odd = dTap_odd * r2 + Tap_coeffv8[1];
  doublev8 dTap_even = dTap_coeffv8[6] * r2 + dTap_coeffv8[4];
  dTap_even = dTap_even * r2 + dTap_coeffv8[2];
  //dTap_even = dTap_even * r2;
  dTap = (dTap_even * r + dTap_odd) * Rcutinv;
  doublev8 Tap_odd = Tap_coeffv8[7] * r2 + Tap_coeffv8[5];
  Tap_odd = Tap_odd * r2 + Tap_coeffv8[3];
  Tap_odd = Tap_odd * r2 + Tap_coeffv8[1];
  doublev8 Tap_even = Tap_coeffv8[6] * r2 + Tap_coeffv8[4];
  Tap_even = Tap_even * r2 + Tap_coeffv8[2];
  Tap_even = Tap_even * r2 + Tap_coeffv8[0];
  Tap = Tap_odd * r + Tap_even;
  // }
  // Tap = Tap_coeff[7] * r + Tap_coeff[6];
  // Tap = Tap * r + Tap_coeff[5];
  // Tap = Tap * r + Tap_coeff[4];
  // Tap = Tap * r + Tap_coeff[3];
  // Tap = Tap * r + Tap_coeff[2];
  // Tap = Tap * r + Tap_coeff[1];
  // Tap = Tap * r + Tap_coeff[0];

}
#endif
__always_inline static bool check_vdw(tagint itag, tagint jtag, double *xi, double *xj) {
  if (itag > jtag) {
    if (((itag + jtag) & 1) == 0) return false;
  } else if (itag < jtag) {
    if (((itag + jtag) & 1) == 1) return false;
  } else {
    if (xj[2] < xi[2]) return false;
    if (xj[2] == xi[2] && xj[1] < xi[1]) return false;
    if (xj[2] == xi[2] && xj[1] == xi[1] && xj[0] < xi[0]) return false;
  }
  return true;
}

/* ----------------------------------------------------------------------
   Calculate the normals for one atom
   ------------------------------------------------------------------------- */
void PairLMFFSunway::calc_single_normal(int i, int *ILP_neigh, int nneigh, double *normal, double (*dnormdri)[3], double (*dnormdrk)[3][3])
{
  int j, ii, jj, inum, jnum;
  int cont, id, ip, m;
  double nn, xtp, ytp, ztp, delx, dely, delz, nn2;
  int *ilist, *jlist;
  double pv12[3], pv31[3], pv23[3], n1[3], dni[3], dnn[3][3], vet[3][3], dpvdri[3][3];
  double dn1[3][3][3], dpv12[3][3][3], dpv23[3][3][3], dpv31[3][3][3];

  double **x = atom->x;

  for (id = 0; id < 3; id++) {
    pv12[id] = 0.0;
    pv31[id] = 0.0;
    pv23[id] = 0.0;
    n1[id] = 0.0;
    dni[id] = 0.0;
    normal[id] = 0.0;
    for (ip = 0; ip < 3; ip++) {
      vet[ip][id] = 0.0;
      dnn[ip][id] = 0.0;
      dpvdri[ip][id] = 0.0;
      dnormdri[ip][id] = 0.0;
      for (m = 0; m < 3; m++) {
        dpv12[ip][id][m] = 0.0;
        dpv31[ip][id][m] = 0.0;
        dpv23[ip][id][m] = 0.0;
        dn1[ip][id][m] = 0.0;
        dnormdrk[ip][id][m] = 0.0;
      }
    }
  }

  xtp = x[i][0];
  ytp = x[i][1];
  ztp = x[i][2];

  cont = 0;
  jlist = ILP_neigh;
  jnum = nneigh;
  for (jj = 0; jj < jnum; jj++) {
    j = jlist[jj];
    j &= NEIGHMASK;

    delx = x[j][0] - xtp;
    dely = x[j][1] - ytp;
    delz = x[j][2] - ztp;
    vet[cont][0] = delx;
    vet[cont][1] = dely;
    vet[cont][2] = delz;
    cont++;
  }

  if (cont <= 1) {
    normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 1.0;
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) {
        dnormdri[id][ip] = 0.0;
        for (m = 0; m < 3; m++) { dnormdrk[id][ip][m] = 0.0; }
      }
    }
  } else if (cont == 2) {
    pv12[0] = vet[0][1] * vet[1][2] - vet[1][1] * vet[0][2];
    pv12[1] = vet[0][2] * vet[1][0] - vet[1][2] * vet[0][0];
    pv12[2] = vet[0][0] * vet[1][1] - vet[1][0] * vet[0][1];
    // derivatives of pv12[0] to ri
    dpvdri[0][0] = 0.0;
    dpvdri[0][1] = vet[0][2] - vet[1][2];
    dpvdri[0][2] = vet[1][1] - vet[0][1];
    // derivatives of pv12[1] to ri
    dpvdri[1][0] = vet[1][2] - vet[0][2];
    dpvdri[1][1] = 0.0;
    dpvdri[1][2] = vet[0][0] - vet[1][0];
    // derivatives of pv12[2] to ri
    dpvdri[2][0] = vet[0][1] - vet[1][1];
    dpvdri[2][1] = vet[1][0] - vet[0][0];
    dpvdri[2][2] = 0.0;

    dpv12[0][0][0] = 0.0;
    dpv12[0][1][0] = vet[1][2];
    dpv12[0][2][0] = -vet[1][1];
    dpv12[1][0][0] = -vet[1][2];
    dpv12[1][1][0] = 0.0;
    dpv12[1][2][0] = vet[1][0];
    dpv12[2][0][0] = vet[1][1];
    dpv12[2][1][0] = -vet[1][0];
    dpv12[2][2][0] = 0.0;

    // derivatives respect to the second neighbor, atom l
    dpv12[0][0][1] = 0.0;
    dpv12[0][1][1] = -vet[0][2];
    dpv12[0][2][1] = vet[0][1];
    dpv12[1][0][1] = vet[0][2];
    dpv12[1][1][1] = 0.0;
    dpv12[1][2][1] = -vet[0][0];
    dpv12[2][0][1] = -vet[0][1];
    dpv12[2][1][1] = vet[0][0];
    dpv12[2][2][1] = 0.0;

    // derivatives respect to the third neighbor, atom n
    // derivatives of pv12 to rn is zero
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dpv12[id][ip][2] = 0.0; }
    }

    n1[0] = pv12[0];
    n1[1] = pv12[1];
    n1[2] = pv12[2];
    // the magnitude of the normal vector
    nn2 = n1[0] * n1[0] + n1[1] * n1[1] + n1[2] * n1[2];
    nn = sqrt(nn2);
    //if (nn == 0) error->one(FLERR, "The magnitude of the normal vector is zero");
    // the unit normal vector
    normal[0] = n1[0] / nn;
    normal[1] = n1[1] / nn;
    normal[2] = n1[2] / nn;
    // derivatives of nn, dnn:3x1 vector
    dni[0] = (n1[0] * dpvdri[0][0] + n1[1] * dpvdri[1][0] + n1[2] * dpvdri[2][0]) / nn;
    dni[1] = (n1[0] * dpvdri[0][1] + n1[1] * dpvdri[1][1] + n1[2] * dpvdri[2][1]) / nn;
    dni[2] = (n1[0] * dpvdri[0][2] + n1[1] * dpvdri[1][2] + n1[2] * dpvdri[2][2]) / nn;
    // derivatives of unit vector ni respect to ri, the result is 3x3 matrix
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) {
        dnormdri[id][ip] = dpvdri[id][ip] / nn - n1[id] * dni[ip] / nn2;
      }
    }
    // derivatives of non-normalized normal vector, dn1:3x3x3 array
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) {
        for (m = 0; m < 3; m++) { dn1[id][ip][m] = dpv12[id][ip][m]; }
      }
    }
    // derivatives of nn, dnn:3x3 vector
    // dnn[id][m]: the derivative of nn respect to r[id][m], id,m=0,1,2
    // r[id][m]: the id's component of atom m
    for (m = 0; m < 3; m++) {
      for (id = 0; id < 3; id++) {
        dnn[id][m] = (n1[0] * dn1[0][id][m] + n1[1] * dn1[1][id][m] + n1[2] * dn1[2][id][m]) / nn;
      }
    }
    // dnormdrk[id][ip][m][i]: the derivative of normal[id] respect to r[ip][m], id,ip=0,1,2
    // for atom m, which is a neighbor atom of atom i, m=0,jnum-1
    for (m = 0; m < 3; m++) {
      for (id = 0; id < 3; id++) {
        for (ip = 0; ip < 3; ip++) {
          dnormdrk[id][ip][m] = dn1[id][ip][m] / nn - n1[id] * dnn[ip][m] / nn2;
        }
      }
    }
  }
  //##############################################################################################

  else if (cont == 3) {
    pv12[0] = vet[0][1] * vet[1][2] - vet[1][1] * vet[0][2];
    pv12[1] = vet[0][2] * vet[1][0] - vet[1][2] * vet[0][0];
    pv12[2] = vet[0][0] * vet[1][1] - vet[1][0] * vet[0][1];
    // derivatives respect to the first neighbor, atom k
    dpv12[0][0][0] = 0.0;
    dpv12[0][1][0] = vet[1][2];
    dpv12[0][2][0] = -vet[1][1];
    dpv12[1][0][0] = -vet[1][2];
    dpv12[1][1][0] = 0.0;
    dpv12[1][2][0] = vet[1][0];
    dpv12[2][0][0] = vet[1][1];
    dpv12[2][1][0] = -vet[1][0];
    dpv12[2][2][0] = 0.0;
    // derivatives respect to the second neighbor, atom l
    dpv12[0][0][1] = 0.0;
    dpv12[0][1][1] = -vet[0][2];
    dpv12[0][2][1] = vet[0][1];
    dpv12[1][0][1] = vet[0][2];
    dpv12[1][1][1] = 0.0;
    dpv12[1][2][1] = -vet[0][0];
    dpv12[2][0][1] = -vet[0][1];
    dpv12[2][1][1] = vet[0][0];
    dpv12[2][2][1] = 0.0;

    // derivatives respect to the third neighbor, atom n
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dpv12[id][ip][2] = 0.0; }
    }

    pv31[0] = vet[2][1] * vet[0][2] - vet[0][1] * vet[2][2];
    pv31[1] = vet[2][2] * vet[0][0] - vet[0][2] * vet[2][0];
    pv31[2] = vet[2][0] * vet[0][1] - vet[0][0] * vet[2][1];
    // derivatives respect to the first neighbor, atom k
    dpv31[0][0][0] = 0.0;
    dpv31[0][1][0] = -vet[2][2];
    dpv31[0][2][0] = vet[2][1];
    dpv31[1][0][0] = vet[2][2];
    dpv31[1][1][0] = 0.0;
    dpv31[1][2][0] = -vet[2][0];
    dpv31[2][0][0] = -vet[2][1];
    dpv31[2][1][0] = vet[2][0];
    dpv31[2][2][0] = 0.0;
    // derivatives respect to the third neighbor, atom n
    dpv31[0][0][2] = 0.0;
    dpv31[0][1][2] = vet[0][2];
    dpv31[0][2][2] = -vet[0][1];
    dpv31[1][0][2] = -vet[0][2];
    dpv31[1][1][2] = 0.0;
    dpv31[1][2][2] = vet[0][0];
    dpv31[2][0][2] = vet[0][1];
    dpv31[2][1][2] = -vet[0][0];
    dpv31[2][2][2] = 0.0;
    // derivatives respect to the second neighbor, atom l
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dpv31[id][ip][1] = 0.0; }
    }

    pv23[0] = vet[1][1] * vet[2][2] - vet[2][1] * vet[1][2];
    pv23[1] = vet[1][2] * vet[2][0] - vet[2][2] * vet[1][0];
    pv23[2] = vet[1][0] * vet[2][1] - vet[2][0] * vet[1][1];
    // derivatives respect to the second neighbor, atom k
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dpv23[id][ip][0] = 0.0; }
    }
    // derivatives respect to the second neighbor, atom l
    dpv23[0][0][1] = 0.0;
    dpv23[0][1][1] = vet[2][2];
    dpv23[0][2][1] = -vet[2][1];
    dpv23[1][0][1] = -vet[2][2];
    dpv23[1][1][1] = 0.0;
    dpv23[1][2][1] = vet[2][0];
    dpv23[2][0][1] = vet[2][1];
    dpv23[2][1][1] = -vet[2][0];
    dpv23[2][2][1] = 0.0;
    // derivatives respect to the third neighbor, atom n
    dpv23[0][0][2] = 0.0;
    dpv23[0][1][2] = -vet[1][2];
    dpv23[0][2][2] = vet[1][1];
    dpv23[1][0][2] = vet[1][2];
    dpv23[1][1][2] = 0.0;
    dpv23[1][2][2] = -vet[1][0];
    dpv23[2][0][2] = -vet[1][1];
    dpv23[2][1][2] = vet[1][0];
    dpv23[2][2][2] = 0.0;

    //############################################################################################
    // average the normal vectors by using the 3 neighboring planes
    n1[0] = (pv12[0] + pv31[0] + pv23[0]) / cont;
    n1[1] = (pv12[1] + pv31[1] + pv23[1]) / cont;
    n1[2] = (pv12[2] + pv31[2] + pv23[2]) / cont;
    // the magnitude of the normal vector
    nn2 = n1[0] * n1[0] + n1[1] * n1[1] + n1[2] * n1[2];
    nn = sqrt(nn2);
    //if (nn == 0) error->one(FLERR, "The magnitude of the normal vector is zero");
    // the unit normal vector
    normal[0] = n1[0] / nn;
    normal[1] = n1[1] / nn;
    normal[2] = n1[2] / nn;

    // for the central atoms, dnormdri is always zero
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dnormdri[id][ip] = 0.0; }
    }

    // derivatives of non-normalized normal vector, dn1:3x3x3 array
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) {
        for (m = 0; m < 3; m++) {
          dn1[id][ip][m] = (dpv12[id][ip][m] + dpv23[id][ip][m] + dpv31[id][ip][m]) / cont;
        }
      }
    }
    // derivatives of nn, dnn:3x3 vector
    // dnn[id][m]: the derivative of nn respect to r[id][m], id,m=0,1,2
    // r[id][m]: the id's component of atom m
    for (m = 0; m < 3; m++) {
      for (id = 0; id < 3; id++) {
        dnn[id][m] = (n1[0] * dn1[0][id][m] + n1[1] * dn1[1][id][m] + n1[2] * dn1[2][id][m]) / nn;
      }
    }
    // dnormdrk[id][ip][m][i]: the derivative of normal[id] respect to r[ip][m], id,ip=0,1,2
    // for atom m, which is a neighbor atom of atom i, m=0,jnum-1
    for (m = 0; m < 3; m++) {
      for (id = 0; id < 3; id++) {
        for (ip = 0; ip < 3; ip++) {
          dnormdrk[id][ip][m] = dn1[id][ip][m] / nn - n1[id] * dnn[ip][m] / nn2;
        }
      }
    }
  } else {
    //error->one(FLERR, "There are too many neighbors for calculating normals");
  }

    //##############################################################################################
}

__always_inline double PairLMFFSunway::ProxyTersoff::fc(double r, Param *param)
{
  double ters_R = param->bigr;
  double ters_D = param->bigd;

  if (r < ters_R-ters_D) return 1.0;
  if (r > ters_R+ters_D) return 0.0;
  return 0.5*(1.0 - sin(MY_PI2*(r - ters_R)/ters_D));
}

/* ---------------------------------------------------------------------- */

__always_inline double PairLMFFSunway::ProxyTersoff::fc_d(double r, Param *param)
{
  double ters_R = param->bigr;
  double ters_D = param->bigd;

  if (r < ters_R-ters_D) return 0.0;
  if (r > ters_R+ters_D) return 0.0;
  return -(MY_PI4/ters_D) * cos(MY_PI2*(r - ters_R)/ters_D);
}

/* ---------------------------------------------------------------------- */

__always_inline double PairLMFFSunway::ProxyTersoff::fa(double r, Param *param)
{
  if (r > param->bigr + param->bigd) return 0.0;
  return -param->bigb * exp(-param->lam2 * r) * fc(r,param);
}

/* ---------------------------------------------------------------------- */

__always_inline double PairLMFFSunway::ProxyTersoff::fa_d(double r, Param *param)
{
  if (r > param->bigr + param->bigd) return 0.0;
  return param->bigb * exp(-param->lam2 * r) *
    (param->lam2 * fc(r,param) - fc_d(r,param));
}

/* ---------------------------------------------------------------------- */

__always_inline double PairLMFFSunway::ProxyTersoff::bij(double zeta, Param *param)
{
  double tmp = param->beta * zeta;
  if (tmp > param->c1) return 1.0/sqrt(tmp);
  if (tmp > param->c2)
    return (1.0 - pow(tmp,-param->powern) / (2.0*param->powern))/sqrt(tmp);
  if (tmp < param->c4) return 1.0;
  if (tmp < param->c3)
    return 1.0 - pow(tmp,param->powern)/(2.0*param->powern);
  return pow(1.0 + pow(tmp,param->powern), -1.0/(2.0*param->powern));
}

/* ---------------------------------------------------------------------- */

__always_inline double PairLMFFSunway::ProxyTersoff::bij_d(double zeta, Param *param)
{
  double tmp = param->beta * zeta;
  if (tmp > param->c1) return param->beta * -0.5*pow(tmp,-1.5);
  if (tmp > param->c2)
    return param->beta * (-0.5*pow(tmp,-1.5) *
                          // error in negligible 2nd term fixed 9/30/2015
                          // (1.0 - 0.5*(1.0 +  1.0/(2.0*param->powern)) *
                          (1.0 - (1.0 +  1.0/(2.0*param->powern)) *
                           pow(tmp,-param->powern)));
  if (tmp < param->c4) return 0.0;
  if (tmp < param->c3)
    return -0.5*param->beta * pow(tmp,param->powern-1.0);

  double tmp_n = pow(tmp,param->powern);
  return -0.5 * pow(1.0+tmp_n, -1.0-(1.0/(2.0*param->powern)))*tmp_n / zeta;
}

/* ---------------------------------------------------------------------- */

__always_inline void PairLMFFSunway::ProxyTersoff::zetaterm_d(double prefactor,
                                  double *rij_hat, double rij, double rijinv,
                                  double *rik_hat, double rik, double rikinv,
                                  double *dri, double *drj, double *drk,
                                  Param *param)
{
  double gijk,gijk_d,ex_delr,ex_delr_d,ffc,dfc,cos_theta,tmp;
  double dcosdri[3],dcosdrj[3],dcosdrk[3];

  ffc = fc(rik,param);
  dfc = fc_d(rik,param);
  if (param->powermint == 3) tmp = cube(param->lam3 * (rij-rik));
  else tmp = param->lam3 * (rij-rik);

  if (tmp > 69.0776) ex_delr = 1.e30;
  else if (tmp < -69.0776) ex_delr = 0.0;
  else ex_delr = exp(tmp);

  if (param->powermint == 3)
    ex_delr_d = 3.0*cube(param->lam3) * square(rij-rik)*ex_delr;
  else ex_delr_d = param->lam3 * ex_delr;

  cos_theta = dot3(rij_hat,rik_hat);
  gijk = calc_gijk(cos_theta,param);
  gijk_d = calc_gijk_d(cos_theta,param);
  costheta_d(rij_hat,rijinv,rik_hat,rikinv,dcosdri,dcosdrj,dcosdrk);

  // compute the derivative wrt Ri
  // dri = -dfc*gijk*ex_delr*rik_hat;
  // dri += fc*gijk_d*ex_delr*dcosdri;
  // dri += fc*gijk*ex_delr_d*(rik_hat - rij_hat);

  scale3(-dfc*gijk*ex_delr,rik_hat,dri);
  scaleadd3(ffc*gijk_d*ex_delr,dcosdri,dri,dri);
  scaleadd3(ffc*gijk*ex_delr_d,rik_hat,dri,dri);
  scaleadd3(-ffc*gijk*ex_delr_d,rij_hat,dri,dri);
  scale3(prefactor,dri);

  // compute the derivative wrt Rj
  // drj = fc*gijk_d*ex_delr*dcosdrj;
  // drj += fc*gijk*ex_delr_d*rij_hat;

  scale3(ffc*gijk_d*ex_delr,dcosdrj,drj);
  scaleadd3(ffc*gijk*ex_delr_d,rij_hat,drj,drj);
  scale3(prefactor,drj);

  // compute the derivative wrt Rk
  // drk = dfc*gijk*ex_delr*rik_hat;
  // drk += fc*gijk_d*ex_delr*dcosdrk;
  // drk += -fc*gijk*ex_delr_d*rik_hat;

  scale3(dfc*gijk*ex_delr,rik_hat,drk);
  scaleadd3(ffc*gijk_d*ex_delr,dcosdrk,drk,drk);
  scaleadd3(-ffc*gijk*ex_delr_d,rik_hat,drk,drk);
  scale3(prefactor,drk);
}

/* ---------------------------------------------------------------------- */

__always_inline void PairLMFFSunway::ProxyTersoff::costheta_d(double *rij_hat, double rijinv,
                             double *rik_hat, double rikinv,
                             double *dri, double *drj, double *drk)
{
  // first element is devative wrt Ri, second wrt Rj, third wrt Rk

  double cos_theta = dot3(rij_hat,rik_hat);

  scaleadd3(-cos_theta,rij_hat,rik_hat,drj);
  scale3(rijinv,drj);
  scaleadd3(-cos_theta,rik_hat,rij_hat,drk);
  scale3(rikinv,drk,drk);
  add3(drj,drk,dri);
  scale3(-1.0,dri);
}


static constexpr int MAX_ELEM = 8;
static constexpr int MAX_NEIGH = 3;
constexpr int BUNDLE_RATIO = 3;
#ifdef __sw_host__
#include <algorithm>
//#define LWPF_SPAWN(func, arg) qthread_spawn(func, arg)
//#define LWPF_JOIN qthread_join
extern void slave_build_layered_list(PairLMFFSunway *);
#define LWPF_UNITS U(SLAVE_LMFF)
#include "lwpf3/lwpf.h"
#endif
#ifdef __sw_slave__
extern "C"{
#include "slave.h"
}
#undef printf
#define printf(...)						\
  do{								\
    char tmp[200];						\
    sprintf(tmp, __VA_ARGS__);					\
    fputs(tmp, stdout);						\
  } while (0)							\

#include "swdmapp.hpp"
#include "sunway_iterator.hpp"

#define LWPF_UNIT U(SLAVE_LMFF)
#define LWPF_KERNELS K(ALL) K(ILOOP) K(JLOOP_INTRA) K(NORMAL) K(JLOOP_INTER) K(INTER_COMP) K(DNORMAL) K(TERSOFF)
#define EVT_PC0 PC0_CYCLE
#define EVT_PC1 PC1_INST
//#define EVT_PC2 PC2_L1IC_MISSTIME
//#define EVT_PC4 PC4_DCACHE_ACCESS
//#define EVT_PC5 PC5_DCACHE_MISS
#define EVT_PC2 PC2_INST_SCALAR_FLOAT_ADDETC
#define EVT_PC3 PC3_INST_SCALAR_FLOAT_MULETC
#define EVT_PC4 PC4_INST_SCALAR_FLOAT_DIVETC
#define EVT_PC5 PC5_INST_VECTOR_FLOAT_OTHER
#define EVT_PC6 PC6_INST_VECTOR_FLOAT_ADDS
#define EVT_PC7 PC7_INST_VECTOR_FLOAT_MULS
#include "lwpf3/lwpf.h"
#include <algorithm>
void build_layered_list(PairLMFFSunway *pair){
  pair->build_layered_list();
}

void PairLMFFSunway::build_layered_list(){
  double cut_intra = 0;
  for (int i = 0; i < nparams; i ++){
    if (params[i].rcut > cut_intra) {
      cut_intra = params[i].rcut;
    }
  }
  double cut_intra_listsq = (cut_intra + neighbor->skin) * (cut_intra + neighbor->skin);
  int nall = atom->nlocal + atom->nghost;
  int inum = atom->nlocal;
  int *numneigh = list->numneigh;
  int **firstneigh = list->firstneigh;
  int *jlist, jnum;
  tagint *tag = atom->tag;
  double (*x)[3] = (double(*)[3])atom->x[0];
  for (int iist = _MYID*8; iist < inum; iist += 64*8){
    for (int ii = iist; ii < std::min(iist + 8, inum); ii ++){
      int i = ii;
      tagint itag = tag[i];
      int jnum = numneigh[i];
      int *jlist = firstneigh[i];
      neigh_index[i].first = ii * neighbor->oneatom;
      int *jlist_layered = layered_neigh + ii * neighbor->oneatom;
      int ninter = 0, nintra = 0;
      
      for (int jj = 0; jj < jnum; jj ++){
	int j = jlist[jj] & NEIGHMASK;
	if (atom->molecule[j] == atom->molecule[i]) {
	  double delx = x[i][0] - x[j][0];
	  double dely = x[i][1] - x[j][1];
	  double delz = x[i][2] - x[j][2];
	  double rsq = delx*delx + dely*dely + delz*delz;
	  if (rsq < cut_intra_listsq) 
	    jlist_layered[nintra++] = j | (int)(check_vdw(itag, tag[j], x[i], x[j])) << NEWTON_SHIFT;
	}
      }
      for (int jj = 0; jj < jnum; jj ++){
	int j = jlist[jj] & NEIGHMASK;
	tagint jtag = tag[j];
	//Newer VDW masking strategy: because ILP is interlayer, so no atoms need vdw has same molecule id.
	if (atom->molecule[j] != atom->molecule[i]) {
	  jlist_layered[nintra + ninter++] = j;
	}
      }
    
      std::sort(jlist_layered + nintra, jlist_layered + nintra + ninter);
      jlist_layered[nintra + ninter] = nall; //push a guarding node
      neigh_index[i].nintra = nintra;
      neigh_index[i].ninter = ninter;
      // num_intra[i] = nintra;
      // num_inter[i] = ninter;
    }


    int *jptr[8];

    for (int i = iist; i < std::min(iist + 8, inum); i ++){
      jptr[i - iist] = layered_neigh + neigh_index[i].first + neigh_index[i].nintra;
      //jend[i - iist] = jptr[i - iist] + neigh_index[i].ninter;
    }

    neigh_index[iist].firstbundled = (iist >> 3) * neighbor->oneatom*BUNDLE_RATIO;
    int *bundled_neigh_i = bundled_neigh + neigh_index[iist].firstbundled;
    int nbundled_i = 0;

    int icnt = std::min(8, inum - iist);
    int isel, iadd;
    while(1) {
      isel = 0;
      for (int i = 1; i < icnt; i ++){
        if (*jptr[i] < *jptr[isel]) isel = i;
      }
      iadd = *jptr[isel];
      if (iadd == nall) break;
      jptr[isel] ++;
      if (nbundled_i == 0 || iadd != bundled_neigh_i[nbundled_i - 1]) {
        bundled_neigh_i[nbundled_i++] = iadd;
        assert(nbundled_i <= neighbor->oneatom*BUNDLE_RATIO);
      }
    }
    neigh_index[iist].nbundled = nbundled_i;

    // int jptr[8], jtop[8];
    // int *jlists[8];
    // int icnt = std::min(iist + 8, inum) - iist;
    
    // neigh_index[iist].firstbundled = (iist >> 3) * neighbor->oneatom;
    // int *bundled_neigh_i = bundled_neigh + neigh_index[iist].firstbundled;
    // int nbundled_i = 0;
    // for (int ii = iist; ii < iist + icnt; ii ++){
    //   jlists[ii - iist] = layered_neigh + ii * neighbor->oneatom + neigh_index[ii].nintra;
    //   jtop[ii - iist] = neigh_index[ii].ninter;
    // }
    // auto done = [](){
    //   for (int ii = 0; ii < icnt; ii ++){
    // 	jptr[ii] == 
    //   }
    // }
    // for (int j : jlist_merged){
    //   assert(j < nall && j >= 0);
    //   bundled_neigh_i[nbundled_i ++] = j;
    // }
    // //printf("%d\n", jlist_merged.size());
    // neigh_index[iist].nbundled = nbundled_i;

  }


}
__always_inline void calc_normal(double (*dk)[3], int nneigh, double *normal, double (*dnormdri)[3], double (*dnormdrk)[3][3]){
  int j, ii, jj, inum, jnum;
  int cont, id, ip, m;
  double nn, nn2;
  int *ilist, *jlist;
  double pv12[3], pv31[3], pv23[3], n1[3], dni[3], dnn[3][3], vet[3][3], dpvdri[3][3];
  double dn1[3][3][3], dpv12[3][3][3], dpv23[3][3][3], dpv31[3][3][3];


  for (id = 0; id < 3; id++) {
    pv12[id] = 0.0;
    pv31[id] = 0.0;
    pv23[id] = 0.0;
    n1[id] = 0.0;
    dni[id] = 0.0;
    normal[id] = 0.0;
    for (ip = 0; ip < 3; ip++) {
      vet[ip][id] = 0.0;
      dnn[ip][id] = 0.0;
      dpvdri[ip][id] = 0.0;
      dnormdri[ip][id] = 0.0;
      for (m = 0; m < 3; m++) {
        dpv12[ip][id][m] = 0.0;
        dpv31[ip][id][m] = 0.0;
        dpv23[ip][id][m] = 0.0;
        dn1[ip][id][m] = 0.0;
        dnormdrk[ip][id][m] = 0.0;
      }
    }
  }

  // xtp = x[i][0];
  // ytp = x[i][1];
  // ztp = x[i][2];

  for (jj = 0; jj < nneigh; jj++) {
    vet[jj][0] = dk[jj][0];
    vet[jj][1] = dk[jj][1];
    vet[jj][2] = dk[jj][2];
  }
  cont = nneigh;
  double continv;
  switch (cont){
  case 1:
    continv = 1.0;
  case 2:
    continv = 0.5;
  case 3:
    continv = 1.0 / 3.0;
  }
  if (cont <= 1) {
    normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 1.0;
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) {
        dnormdri[id][ip] = 0.0;
        for (m = 0; m < 3; m++) { dnormdrk[id][ip][m] = 0.0; }
      }
    }
  } else if (cont == 2) {
    pv12[0] = vet[0][1] * vet[1][2] - vet[1][1] * vet[0][2];
    pv12[1] = vet[0][2] * vet[1][0] - vet[1][2] * vet[0][0];
    pv12[2] = vet[0][0] * vet[1][1] - vet[1][0] * vet[0][1];
    // derivatives of pv12[0] to ri
    dpvdri[0][0] = 0.0;
    dpvdri[0][1] = vet[0][2] - vet[1][2];
    dpvdri[0][2] = vet[1][1] - vet[0][1];
    // derivatives of pv12[1] to ri
    dpvdri[1][0] = vet[1][2] - vet[0][2];
    dpvdri[1][1] = 0.0;
    dpvdri[1][2] = vet[0][0] - vet[1][0];
    // derivatives of pv12[2] to ri
    dpvdri[2][0] = vet[0][1] - vet[1][1];
    dpvdri[2][1] = vet[1][0] - vet[0][0];
    dpvdri[2][2] = 0.0;

    dpv12[0][0][0] = 0.0;
    dpv12[0][1][0] = vet[1][2];
    dpv12[0][2][0] = -vet[1][1];
    dpv12[1][0][0] = -vet[1][2];
    dpv12[1][1][0] = 0.0;
    dpv12[1][2][0] = vet[1][0];
    dpv12[2][0][0] = vet[1][1];
    dpv12[2][1][0] = -vet[1][0];
    dpv12[2][2][0] = 0.0;

    // derivatives respect to the second neighbor, atom l
    dpv12[0][0][1] = 0.0;
    dpv12[0][1][1] = -vet[0][2];
    dpv12[0][2][1] = vet[0][1];
    dpv12[1][0][1] = vet[0][2];
    dpv12[1][1][1] = 0.0;
    dpv12[1][2][1] = -vet[0][0];
    dpv12[2][0][1] = -vet[0][1];
    dpv12[2][1][1] = vet[0][0];
    dpv12[2][2][1] = 0.0;

    // derivatives respect to the third neighbor, atom n
    // derivatives of pv12 to rn is zero
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dpv12[id][ip][2] = 0.0; }
    }

    n1[0] = pv12[0];
    n1[1] = pv12[1];
    n1[2] = pv12[2];
    // the magnitude of the normal vector
    nn2 = n1[0] * n1[0] + n1[1] * n1[1] + n1[2] * n1[2];
    //nn = sqrt(nn2);
    double nninv = invsqrt(nn2);
    double nninv2 = nninv * nninv;

    //if (nn == 0) error->one(FLERR, "The magnitude of the normal vector is zero");
    // the unit normal vector
    normal[0] = n1[0] * nninv;
    normal[1] = n1[1] * nninv;
    normal[2] = n1[2] * nninv;
    // derivatives of nn, dnn:3x1 vector
    dni[0] = (n1[0] * dpvdri[0][0] + n1[1] * dpvdri[1][0] + n1[2] * dpvdri[2][0]) * nninv;
    dni[1] = (n1[0] * dpvdri[0][1] + n1[1] * dpvdri[1][1] + n1[2] * dpvdri[2][1]) * nninv;
    dni[2] = (n1[0] * dpvdri[0][2] + n1[1] * dpvdri[1][2] + n1[2] * dpvdri[2][2]) * nninv;
    // derivatives of unit vector ni respect to ri, the result is 3x3 matrix
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) {
        dnormdri[id][ip] = dpvdri[id][ip] * nninv - n1[id] * dni[ip] * nninv2;
      }
    }
    // derivatives of non-normalized normal vector, dn1:3x3x3 array
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) {
        for (m = 0; m < 3; m++) { dn1[id][ip][m] = dpv12[id][ip][m]; }
      }
    }
    // derivatives of nn, dnn:3x3 vector
    // dnn[id][m]: the derivative of nn respect to r[id][m], id,m=0,1,2
    // r[id][m]: the id's component of atom m
    for (m = 0; m < 3; m++) {
      for (id = 0; id < 3; id++) {
        dnn[id][m] = (n1[0] * dn1[0][id][m] + n1[1] * dn1[1][id][m] + n1[2] * dn1[2][id][m]) * nninv;
      }
    }
    // dnormdrk[id][ip][m][i]: the derivative of normal[id] respect to r[ip][m], id,ip=0,1,2
    // for atom m, which is a neighbor atom of atom i, m=0,jnum-1
    for (m = 0; m < 3; m++) {
      for (id = 0; id < 3; id++) {
        for (ip = 0; ip < 3; ip++) {
          dnormdrk[id][ip][m] = dn1[id][ip][m] * nninv - n1[id] * dnn[ip][m] * nninv2;
        }
      }
    }
  }
  //##############################################################################################

  else if (cont == 3) {
    pv12[0] = vet[0][1] * vet[1][2] - vet[1][1] * vet[0][2];
    pv12[1] = vet[0][2] * vet[1][0] - vet[1][2] * vet[0][0];
    pv12[2] = vet[0][0] * vet[1][1] - vet[1][0] * vet[0][1];
    // derivatives respect to the first neighbor, atom k
    dpv12[0][0][0] = 0.0;
    dpv12[0][1][0] = vet[1][2];
    dpv12[0][2][0] = -vet[1][1];
    dpv12[1][0][0] = -vet[1][2];
    dpv12[1][1][0] = 0.0;
    dpv12[1][2][0] = vet[1][0];
    dpv12[2][0][0] = vet[1][1];
    dpv12[2][1][0] = -vet[1][0];
    dpv12[2][2][0] = 0.0;
    // derivatives respect to the second neighbor, atom l
    dpv12[0][0][1] = 0.0;
    dpv12[0][1][1] = -vet[0][2];
    dpv12[0][2][1] = vet[0][1];
    dpv12[1][0][1] = vet[0][2];
    dpv12[1][1][1] = 0.0;
    dpv12[1][2][1] = -vet[0][0];
    dpv12[2][0][1] = -vet[0][1];
    dpv12[2][1][1] = vet[0][0];
    dpv12[2][2][1] = 0.0;

    // derivatives respect to the third neighbor, atom n
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dpv12[id][ip][2] = 0.0; }
    }

    pv31[0] = vet[2][1] * vet[0][2] - vet[0][1] * vet[2][2];
    pv31[1] = vet[2][2] * vet[0][0] - vet[0][2] * vet[2][0];
    pv31[2] = vet[2][0] * vet[0][1] - vet[0][0] * vet[2][1];
    // derivatives respect to the first neighbor, atom k
    dpv31[0][0][0] = 0.0;
    dpv31[0][1][0] = -vet[2][2];
    dpv31[0][2][0] = vet[2][1];
    dpv31[1][0][0] = vet[2][2];
    dpv31[1][1][0] = 0.0;
    dpv31[1][2][0] = -vet[2][0];
    dpv31[2][0][0] = -vet[2][1];
    dpv31[2][1][0] = vet[2][0];
    dpv31[2][2][0] = 0.0;
    // derivatives respect to the third neighbor, atom n
    dpv31[0][0][2] = 0.0;
    dpv31[0][1][2] = vet[0][2];
    dpv31[0][2][2] = -vet[0][1];
    dpv31[1][0][2] = -vet[0][2];
    dpv31[1][1][2] = 0.0;
    dpv31[1][2][2] = vet[0][0];
    dpv31[2][0][2] = vet[0][1];
    dpv31[2][1][2] = -vet[0][0];
    dpv31[2][2][2] = 0.0;
    // derivatives respect to the second neighbor, atom l
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dpv31[id][ip][1] = 0.0; }
    }

    pv23[0] = vet[1][1] * vet[2][2] - vet[2][1] * vet[1][2];
    pv23[1] = vet[1][2] * vet[2][0] - vet[2][2] * vet[1][0];
    pv23[2] = vet[1][0] * vet[2][1] - vet[2][0] * vet[1][1];
    // derivatives respect to the second neighbor, atom k
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dpv23[id][ip][0] = 0.0; }
    }
    // derivatives respect to the second neighbor, atom l
    dpv23[0][0][1] = 0.0;
    dpv23[0][1][1] = vet[2][2];
    dpv23[0][2][1] = -vet[2][1];
    dpv23[1][0][1] = -vet[2][2];
    dpv23[1][1][1] = 0.0;
    dpv23[1][2][1] = vet[2][0];
    dpv23[2][0][1] = vet[2][1];
    dpv23[2][1][1] = -vet[2][0];
    dpv23[2][2][1] = 0.0;
    // derivatives respect to the third neighbor, atom n
    dpv23[0][0][2] = 0.0;
    dpv23[0][1][2] = -vet[1][2];
    dpv23[0][2][2] = vet[1][1];
    dpv23[1][0][2] = vet[1][2];
    dpv23[1][1][2] = 0.0;
    dpv23[1][2][2] = -vet[1][0];
    dpv23[2][0][2] = -vet[1][1];
    dpv23[2][1][2] = vet[1][0];
    dpv23[2][2][2] = 0.0;

    //############################################################################################
    // average the normal vectors by using the 3 neighboring planes
    n1[0] = (pv12[0] + pv31[0] + pv23[0]) * continv;
    n1[1] = (pv12[1] + pv31[1] + pv23[1]) * continv;
    n1[2] = (pv12[2] + pv31[2] + pv23[2]) * continv;
    // the magnitude of the normal vector
    nn2 = n1[0] * n1[0] + n1[1] * n1[1] + n1[2] * n1[2];
    //nn = sqrt(nn2);
    double nninv = invsqrt(nn2);
    double nninv2 = nninv * nninv;
    //if (nn == 0) error->one(FLERR, "The magnitude of the normal vector is zero");
    // the unit normal vector
    normal[0] = n1[0] * nninv;
    normal[1] = n1[1] * nninv;
    normal[2] = n1[2] * nninv;

    // for the central atoms, dnormdri is always zero
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) { dnormdri[id][ip] = 0.0; }
    }

    // derivatives of non-normalized normal vector, dn1:3x3x3 array
    for (id = 0; id < 3; id++) {
      for (ip = 0; ip < 3; ip++) {
        for (m = 0; m < 3; m++) {
          dn1[id][ip][m] = (dpv12[id][ip][m] + dpv23[id][ip][m] + dpv31[id][ip][m]) * continv;
        }
      }
    }
    // derivatives of nn, dnn:3x3 vector
    // dnn[id][m]: the derivative of nn respect to r[id][m], id,m=0,1,2
    // r[id][m]: the id's component of atom m
    for (m = 0; m < 3; m++) {
      for (id = 0; id < 3; id++) {
        dnn[id][m] = (n1[0] * dn1[0][id][m] + n1[1] * dn1[1][id][m] + n1[2] * dn1[2][id][m]) * nninv;
      }
    }
    // dnormdrk[id][ip][m][i]: the derivative of normal[id] respect to r[ip][m], id,ip=0,1,2
    // for atom m, which is a neighbor atom of atom i, m=0,jnum-1
    for (m = 0; m < 3; m++) {
      for (id = 0; id < 3; id++) {
        for (ip = 0; ip < 3; ip++) {
          dnormdrk[id][ip][m] = dn1[id][ip][m] * nninv - n1[id] * dnn[ip][m] * nninv2;
        }
      }
    }
  } else {
    //error->one(FLERR, "There are too many neighbors for calculating normals");
  }
}
static constexpr int VECSIZEI = 8;
__always_inline void make_transposed(double (*out)[VECSIZEI], double (*in)[MAX_ELEM], int *itypes, int icnt, int ntypes){
  for (int jtype = 1; jtype <= ntypes; jtype ++) {
    for (int i = 0; i < icnt; i ++){
      out[jtype][i] = in[itypes[i]][jtype];
    }
  }
}
typedef int512 vlong;
typedef doublev8 vdouble;
doublev8 simd_vldd(double *addr){
  return __builtin_sw_slave_vldd_f(addr);
}
template<int N>
__always_inline vdouble vlookup(double (*from)[N], int *is, int j) {
  return simd_set_doublev8(from[is[0]][j], from[is[1]][j], from[is[2]][j], from[is[3]][j], from[is[4]][j], from[is[5]][j], from[is[6]][j], from[is[7]][j]);
}
bool notfinite(double x){
  long xi = *(long*)&x;
  return (xi >> 52 & 0x7ff) == 0x7ff;
}
template<int EFLAG, int VFLAG_EITHER, int TAP_FLAG>
void PairLMFFSunway::parallel_eval(){
  //if (_MYID == 0) puts(__PRETTY_FUNCTION__);
  lwpf_enter(SLAVE_LMFF);
  lwpf_start(ALL);
  constexpr int EVFLAG = EFLAG || VFLAG_EITHER;
  int ntypes = atom->ntypes;
  int typemax = ntypes + 1;
  //auto map = ptr_in<MAX_ELEM>(this->map, ntypes+1);
  //auto cutILPsq = to2d(ptr_in<MAX_ELEM*MAX_ELEM>(this->cutILPsq[0], nelements*nelements), nelements);
  auto tcutILPsq = to2d(ptr_in<MAX_ELEM*MAX_ELEM>(this->type_cutILPsq[0], typemax*typemax), typemax);
  //auto tparams = to2d(ptr_in<MAX_ELEM*MAX_ELEM>(this->type_params[0], typemax*typemax), typemax);
  double ILP_delta2inv[MAX_ELEM][MAX_ELEM];
  double ILP_lambda[MAX_ELEM][MAX_ELEM];
  double ILP_C[MAX_ELEM][MAX_ELEM];
  double ILP_epsilon[MAX_ELEM][MAX_ELEM];
  double ILP_d[MAX_ELEM][MAX_ELEM];
  double ILP_C6[MAX_ELEM][MAX_ELEM];
  double ILP_z0[MAX_ELEM][MAX_ELEM];
  double ILP_seffinv[MAX_ELEM][MAX_ELEM];
  for (int i = 1; i <= ntypes; i ++) {
    for (int j = 1; j <= ntypes; j ++){
      ILP_delta2inv[i][j] = type_params[i][j].delta2inv;
      ILP_lambda[i][j] = type_params[i][j].lambda;
      ILP_C[i][j] = type_params[i][j].C;
      ILP_z0[i][j] = type_params[i][j].z0;
      ILP_epsilon[i][j] = type_params[i][j].epsilon;
      ILP_d[i][j] = type_params[i][j].d;
      ILP_C6[i][j] = type_params[i][j].C6;
      ILP_seffinv[i][j] = type_params[i][j].seffinv;
    }
  }
  double cutsq = this->cut_global * this->cut_global;
  double cutinv = invsqrt(cutsq);
  double (*x)[3] = (double(*)[3])atom->x[0];
  double (*f)[3] = (double(*)[3])atom->f[0];
  //tagint *tag = atom->tag;
  //int *type = atom->type;
  int nlocal = atom->nlocal;
  int newton_pair = force->newton_pair;
  ForceCache<256, 16> fcache_inter(f, force_locks, atom->nlocal + atom->nghost);
  ForceCache<64, 16> fcache_intra(f, force_locks, atom->nlocal + atom->nghost);
  ReadOnlyCache<atompack_mol_t, 128, 16> jcache_inter(atompack);
  ReadOnlyCache<atompack_mol_t, 16, 16> jcache_intra(atompack);
  //auto &jcache_intra = jcache_inter;
  //ReadOnlyCache<atompack_mol_t, 64, 8> jcache_intra(atompack);
  int inum = list->inum;
  //ilist = list->ilist;
  //numneigh = list->numneigh;
  //firstneigh = list->firstneigh;
  tallyvar<2> tally;
  tally.init();
  constexpr int BLKSIZEJ = 64;
  int *layered_neigh = this->layered_neigh;
  //auto atompacki = linear_in<BLKSIZE>(this->atompack);
  //auto neigh_index = linear_in<BLKSIZEI>(this->neigh_index);
  vdouble vcutsq = simd_vcpyfd(cutsq);
  vdouble vcutinv = simd_vcpyfd(cutinv);
  vdouble Tap_coeffv8[8], dTap_coeffv8[8];
  for (int i = 0; i < 8; i ++){
    asm("vcpyf %1, %0\n\t" : "=f"(Tap_coeffv8[i]) : "f"(Tap_coeff[i]));
    asm("vcpyf %1, %0\n\t" : "=f"(dTap_coeffv8[i]) : "f"(i * Tap_coeff[i]));
  }

  lwpf_start(ILOOP);
  //if (_MYID == 0)
  //for (int iist = VECSIZEI * _MYID; iist < inum; iist += VECSIZEI){
  for (int iist = VECSIZEI * _MYID; iist < inum; iist += VECSIZEI * 64){
    atompack_mol_t packi[VECSIZEI];
    int itypes[VECSIZEI];
    long imols[VECSIZEI];
    int icnt = std::min(VECSIZEI, inum - iist);
    double dk[VECSIZEI][MAX_NEIGH][3];
    int ILP_neigh[VECSIZEI][MAX_NEIGH];
    int ILP_nneigh[VECSIZEI];
    bool newtonk[VECSIZEI][MAX_NEIGH];
    int ktype[VECSIZEI][MAX_NEIGH];
    double fitmp[VECSIZEI][3];
    double norm[VECSIZEI][3];
    double dnormdxi[VECSIZEI][3][3];
    double dnormdxk[VECSIZEI][3][3][MAX_NEIGH];
    double dproddni[VECSIZEI][3];
    for (int ii = 0; ii < icnt; ii ++){
      fitmp[ii][0] = fitmp[ii][1] = fitmp[ii][2] = 0;
      int i = iist + ii;
      auto &xi = packi[ii] = jcache_intra[i];

      int itype = xi.type;
      itypes[ii] = xi.type;
      imols[ii] = xi.mol;
      auto jlist_intra = linear_in<BLKSIZEJ>(layered_neigh + neigh_index[i].first);
      int jnum_intra = neigh_index[i].nintra;
      ILP_nneigh[ii] = 0;
      lwpf_start(JLOOP_INTRA);
      for (int jj : iterate<BLKSIZEJ>(0, jnum_intra, jlist_intra)){
	int jraw = jlist_intra[jj];
	int j = jraw & ~NEWTON_MASK;
	int newton = jraw >> NEWTON_SHIFT;
	atompack_mol_t xj = jcache_intra[j];
	int jtype = xj.type;
	double delx = xj.x - xi.x;
	double dely = xj.y - xi.y;
	double delz = xj.z - xi.z;
	double rsq = delx * delx + dely * dely + delz * delz;

	if (rsq < tcutILPsq[itype][jtype]) {
	  //if (ILP_nneigh[ii] < 3){
          if (ILP_nneigh[ii] >= 3){
            printf("%d saw extra neighbors, existing rsq: %g %g %g, incoming %g\n", comm->me,
                   dk[ii][0][0]*dk[ii][0][0]+dk[ii][0][1]*dk[ii][0][1]+dk[ii][0][2]*dk[ii][0][2],
                   dk[ii][1][0]*dk[ii][1][0]+dk[ii][1][1]*dk[ii][1][1]+dk[ii][1][2]*dk[ii][1][2],
                   dk[ii][2][0]*dk[ii][2][0]+dk[ii][2][1]*dk[ii][2][1]+dk[ii][2][2]*dk[ii][2][2], rsq);
            double rsqfar = dk[ii][0][0]*dk[ii][0][0] + dk[ii][0][1]*dk[ii][0][1] + dk[ii][0][2]*dk[ii][0][2];
	    int far = 0;
	    for (int kk = 1; kk < 3; kk ++){
	      double rsqk = dk[ii][kk][0]*dk[ii][kk][0] + dk[ii][kk][1]*dk[ii][kk][1] + dk[ii][kk][2]*dk[ii][kk][2];
	      if (rsqk > rsqfar){
		rsqfar = rsqk;
		far = kk;
	      }
	    }
            if (rsq < rsqfar){
              ILP_neigh[ii][far] = j;
              dk[ii][far][0] = delx;
              dk[ii][far][1] = dely;
              dk[ii][far][2] = delz;
              ktype[ii][far] = jtype;
              newtonk[ii][far] = newton;
            }
          } else {
            ILP_neigh[ii][ILP_nneigh[ii]] = j;
            dk[ii][ILP_nneigh[ii]][0] = delx;
            dk[ii][ILP_nneigh[ii]][1] = dely;
            dk[ii][ILP_nneigh[ii]][2] = delz;
            ktype[ii][ILP_nneigh[ii]] = jtype;
            newtonk[ii][ILP_nneigh[ii]] = newton;
            ILP_nneigh[ii] ++;
          }
	}
	    //}
	  /*else {
	    double rsqfar = dk[ii][0][0]*dk[ii][0][0] + dk[ii][0][1]*dk[ii][0][1] + dk[ii][0][2]*dk[ii][0][2];
	    int far = 0;
	    for (int kk = 1; kk < 3; kk ++){
	      double rsq = dk[ii][kk][0]*dk[ii][kk][0] + dk[ii][kk][1]*dk[ii][kk][1] + dk[ii][kk][2]*dk[ii][kk][2];
	      if (rsq > rsqfar){
		rsqfar = rsq;
		far = kk;
	      }
	    }
	    ILP_neigh[ii][far] = j;
	    dk[ii][far][0] = delx;
	    dk[ii][far][1] = dely;
	    dk[ii][far][2] = delz;
	    ktype[ii][far] = jtype;
	    newtonk[ii][far] = newton;
	  }
	  }*/
      }
      lwpf_stop(JLOOP_INTRA);
      lwpf_start(NORMAL);
      calc_normal(dk[ii], ILP_nneigh[ii], norm[ii], dnormdxi[ii], dnormdxk[ii]);
      
      lwpf_stop(NORMAL);
      dproddni[ii][0] = 0.0;
      dproddni[ii][1] = 0.0;
      dproddni[ii][2] = 0.0;
    }
    struct vparam_t {
      vdouble delta2inv;
      vdouble lambda;
      vdouble C;
      vdouble epsilon;
      vdouble d;
      vdouble C6;
      vdouble z0;
      vdouble seffinv;
    };
    for (int ii = icnt; ii < VECSIZEI; ii ++){
      norm[ii][0] = norm[icnt-1][0];
      norm[ii][1] = norm[icnt-1][1];
      norm[ii][2] = norm[icnt-1][2];
      packi[ii].x = packi[icnt-1].x;
      packi[ii].y = packi[icnt-1].y;
      packi[ii].z = packi[icnt-1].z;
      itypes[ii] = itypes[icnt-1];
      imols[ii] = imols[icnt-1];
    }

    vparam_t vparams[MAX_ELEM];
    for (int jtype = 1; jtype <= ntypes; jtype ++){
      vparams[jtype].delta2inv = vlookup(ILP_delta2inv, itypes, jtype);
      vparams[jtype].lambda    = vlookup(ILP_lambda   , itypes, jtype);
      vparams[jtype].C         = vlookup(ILP_C        , itypes, jtype);
      vparams[jtype].epsilon   = vlookup(ILP_epsilon  , itypes, jtype);
      vparams[jtype].d         = vlookup(ILP_d        , itypes, jtype);
      vparams[jtype].C6        = vlookup(ILP_C6       , itypes, jtype);
      vparams[jtype].z0        = vlookup(ILP_z0       , itypes, jtype);
      vparams[jtype].seffinv   = vlookup(ILP_seffinv  , itypes, jtype);
    }

    auto jlist_bundle = linear_in<BLKSIZEJ>(bundled_neigh + neigh_index[iist].firstbundled);
    int jnum_bundle = neigh_index[iist].nbundled;
    //if (internal_list_count == 3) cal_locked_printf("%d %d %d\n", iist, neigh_index[iist].firstbundled, neigh_index[iist].nbundled);
    vdouble normx = simd_set_doublev8(norm[0][0], norm[1][0], norm[2][0], norm[3][0], norm[4][0], norm[5][0], norm[6][0], norm[7][0]);
    vdouble normy = simd_set_doublev8(norm[0][1], norm[1][1], norm[2][1], norm[3][1], norm[4][1], norm[5][1], norm[6][1], norm[7][1]);
    vdouble normz = simd_set_doublev8(norm[0][2], norm[1][2], norm[2][2], norm[3][2], norm[4][2], norm[5][2], norm[6][2], norm[7][2]);
    vdouble vcutsqi = simd_vcond(vcutsq, (vdouble)0.0, (void*)(long)((VECSIZEI - icnt) * 8));
    vlong imoll = simd_set_doublev8(imols[0], imols[1], imols[2], imols[3], imols[4], imols[5], imols[6], imols[7]);

    vdouble xitmp = simd_set_doublev8(packi[0].x, packi[1].x, packi[2].x, packi[3].x, packi[4].x, packi[5].x, packi[6].x, packi[7].x);
    vdouble yitmp = simd_set_doublev8(packi[0].y, packi[1].y, packi[2].y, packi[3].y, packi[4].y, packi[5].y, packi[6].y, packi[7].y);
    vdouble zitmp = simd_set_doublev8(packi[0].z, packi[1].z, packi[2].z, packi[3].z, packi[4].z, packi[5].z, packi[6].z, packi[7].z);

    lwpf_start(JLOOP_INTER);
    vdouble fitmpx = 0, fitmpy = 0, fitmpz = 0;
    vdouble dprod_nix = 0, dprod_niy = 0, dprod_niz = 0;
    vdouble e_rep = 0, e_vdw = 0;
    
    for (int jj : iterate<BLKSIZEJ>(0, jnum_bundle, jlist_bundle)){
      int j = jlist_bundle[jj];
      //if (internal_list_count == 3) printf("%d %d\n", atom->nlocal + atom->nghost, j);
      atompack_mol_t xj = jcache_inter[j];

      long jtype = xj.type;
      vdouble xjtmp = simd_vcpyfd(xj.x);
      vdouble yjtmp = simd_vcpyfd(xj.y);
      vdouble zjtmp = simd_vcpyfd(xj.z);


      vlong jmoll = simd_vcpyl(xj.mol);
      vdouble delx = xitmp - xjtmp;
      vdouble dely = yitmp - yjtmp;
      vdouble delz = zitmp - zjtmp;

      vdouble rsq = delx*delx + dely*dely + delz*delz;

      vdouble ltcut = simd_vfcmpltd(rsq, vcutsqi);

      vlong molsub = imoll - jmoll;
      vdouble moldiff = *(vdouble*)&molsub;
      ltcut = simd_vfseleqd(moldiff, (vdouble)0.0, ltcut);
      auto &vpij = vparams[jtype];
      bool allzero;
      asm("ucmpeqx %1,$31,%0\n\t":"=r"(allzero) : "r"(ltcut));
      if (!allzero){
	rsq = simd_vfseleqd(ltcut, vcutsq, rsq);
	//assert(!notfinite(simd_reduc_plusd(rsq)));
	vdouble rinv = invsqrt(rsq);
      	//assert(!notfinite(simd_reduc_plusd(rinv)));
	vdouble r2inv = rinv * rinv;
	vdouble r = rinv * rsq;
	vdouble Tap, dTap;
	if (TAP_FLAG){
	  Tap_optv8(Tap, dTap, r, vcutinv, Tap_coeffv8, dTap_coeffv8);
	} else {
	  dTap = 0.0;
	  Tap = simd_vfseleqd(ltcut, (vdouble)0.0, vmworld.f1_0.v);
	}
	//assert(!notfinite(simd_reduc_plusd(Tap)));
	//assert(!notfinite(simd_reduc_plusd(dTap)));
	vdouble prodnorm1 = normx*delx + normy*dely + normz*delz;
	prodnorm1 = simd_vfseleqd(ltcut, (vdouble)rsq, prodnorm1);
	vdouble rhosq1 = rsq - prodnorm1*prodnorm1;
	vdouble rdsq1 = rhosq1*vpij.delta2inv;
	
	vdouble exp0 = vexp_valid(-vpij.lambda*(r - vpij.z0));	
	vdouble exp1 = vexp_valid(-rdsq1);
	//assert(!notfinite(simd_reduc_plusd(exp0)));
	// if (notfinite(simd_reduc_plusd(exp1))){
	//   printf("%d", icnt);
	//   simd_print_doublev8(r);
	//   simd_print_doublev8(rsq);
	//   simd_print_doublev8(prodnorm1);
	//   simd_print_doublev8(rhosq1);
	//   simd_print_doublev8(vpij.delta2inv);
	//   simd_print_doublev8(-rdsq1);
	//   simd_print_doublev8(exp1);
	//   //assert(notfinite(simd_reduc_plusd(exp1)));
	// }
	vdouble frho1 = exp1*vpij.C;
	vdouble Erep = vmworld.f0_5.v*vpij.epsilon + frho1;
	//assert(!notfinite(simd_reduc_plusd(vpij.C)));

	vdouble Vrep = exp0*Erep;
	//assert(!notfinite(simd_reduc_plusd(Vrep)));
	vdouble fpair0 = vpij.lambda*exp0*rinv*Erep;
	vdouble fpair1 = vmworld.f2_0.v*exp0*frho1*vpij.delta2inv;
	vdouble fsum0 = fpair0 + fpair1;

	vdouble fp1[3];
	fp1[0] = prodnorm1 * normx * fpair1;
	fp1[1] = prodnorm1 * normy * fpair1;
	fp1[2] = prodnorm1 * normz * fpair1;
      
	//
	vdouble fkcx = (delx*fsum0 - fp1[0])*Tap - Vrep*dTap*delx*rinv;
	vdouble fkcy = (dely*fsum0 - fp1[1])*Tap - Vrep*dTap*dely*rinv;
	vdouble fkcz = (delz*fsum0 - fp1[2])*Tap - Vrep*dTap*delz*rinv;

	//assert(!notfinite(simd_reduc_plusd(rinv)));
	fkcx = simd_vfseleqd(ltcut, (doublev8)0.0, fkcx);
	fkcy = simd_vfseleqd(ltcut, (doublev8)0.0, fkcy);
	fkcz = simd_vfseleqd(ltcut, (doublev8)0.0, fkcz);
	
	vdouble cij = -prodnorm1*fpair1*Tap;
	cij = simd_vfseleqd(ltcut, (doublev8)0.0, cij);
	dprod_nix += cij * delx;
	dprod_niy += cij * dely;
	dprod_niz += cij * delz;

	vdouble TapVrep = Tap * Vrep;
      
	if (EFLAG)
	  e_rep += TapVrep;


	vdouble r6inv = r2inv*r2inv*r2inv;
	vdouble r8inv = r2inv*r6inv;

	vdouble TSvdw = vmworld.f1_0.v + vexp_valid(-vpij.d * (r*vpij.seffinv - vmworld.f1_0.v));
	vdouble TSvdwsq = TSvdw * TSvdw;
	vdouble TSvdwinv = invsqrt(TSvdwsq);
	vdouble TSvdw2inv = TSvdwinv * TSvdwinv;

	vdouble Vvdw = -vpij.C6*r6inv*TSvdwinv;

	vdouble fpair2 = -vmworld.f6_0.v*vpij.C6*r8inv*TSvdwinv + vpij.C6*vpij.d*vpij.seffinv*(TSvdw-vmworld.f1_0.v)*TSvdw2inv*r8inv*r;
	vdouble fsum1 = fpair2*Tap - Vvdw*dTap*rinv;
	fsum1 = simd_vfseleqd(ltcut, (doublev8)0.0, fsum1);
	fsum1 = simd_vfselltd(moldiff, fsum1, (doublev8)0.0);
	vdouble fvdwx = fsum1 * delx;
	vdouble fvdwy = fsum1 * dely;
	vdouble fvdwz = fsum1 * delz;

	// fvdwx = simd_vfseleqd(ltcut, (doublev8)0.0, fvdwx);
	// fvdwy = simd_vfseleqd(ltcut, (doublev8)0.0, fvdwy);
	// fvdwz = simd_vfseleqd(ltcut, (doublev8)0.0, fvdwz);

	fitmpx += fvdwx + fkcx;
	fitmpy += fvdwy + fkcy;
	fitmpz += fvdwz + fkcz;

	vdouble fxj = -(fvdwx + fkcx);
	vdouble fyj = -(fvdwy + fkcy);
	vdouble fzj = -(fvdwz + fkcz);

	vdouble TapVvdw = Tap * Vvdw;
	TapVvdw = simd_vfselltd(moldiff, TapVvdw, (doublev8)0.0);
	//assert(!notfinite(simd_reduc_plusd(TapVvdw)));
	if (EFLAG)
	  e_vdw += TapVvdw;

	double fxjs = simd_reduc_plusd(fxj);
	double fyjs = simd_reduc_plusd(fyj);
	double fzjs = simd_reduc_plusd(fzj);
#warning "Virial not processed"
	// if (VFLAG_EITHER){
	// 	tally.v_xyz(fxji, fyji, fzji, delx, dely, delz);
	// }
	//asm("#comp stop");
	fcache_inter.update(j, fxjs, fyjs, fzjs);
      }
    }
    double e_vdws = simd_reduc_plusd(e_vdw);
    double e_reps = simd_reduc_plusd(e_rep);

    tally.e(e_vdws + e_reps, 0);
    double *fitmpxs = (double*)&fitmpx;
    double *fitmpys = (double*)&fitmpy;
    double *fitmpzs = (double*)&fitmpz;
    double *dprodnix = (double*)&dprod_nix;
    double *dprodniy = (double*)&dprod_niy;
    double *dprodniz = (double*)&dprod_niz;
    for (int ii = 0; ii < icnt; ii ++){
      fitmp[ii][0] += fitmpxs[ii];
      fitmp[ii][1] += fitmpys[ii];
      fitmp[ii][2] += fitmpzs[ii];
      dproddni[ii][0] += dprodnix[ii];
      dproddni[ii][1] += dprodniy[ii];
      dproddni[ii][2] += dprodniz[ii];
    }
    lwpf_stop(JLOOP_INTER);
    //continue;
    for (int ii = 0; ii < icnt; ii ++){
      int i = iist + ii;
      auto &xi = packi[ii];
      int itype = xi.type;
      double fters[MAX_NEIGH+1][3];
      for (int k = 0; k <= ILP_nneigh[ii]; k ++){
	fters[k][0] = 0;
	fters[k][1] = 0;
	fters[k][2] = 0;
      }
      //memset(fters, 0, sizeof(fters));
      lwpf_start(TERSOFF);
      eval_tersoff<EFLAG, VFLAG_EITHER>(tally, fters, dk[ii], itype, ktype[ii], newtonk[ii], ILP_nneigh[ii]);
      lwpf_stop(TERSOFF);
      // if (i == 0){
      //   printf("f0: %g %g %g\n", fters[ILP_nneigh][0], fters[ILP_nneigh][1], fters[ILP_nneigh][2]);
      // }
      fitmp[ii][0] += fters[ILP_nneigh[ii]][0];
      fitmp[ii][1] += fters[ILP_nneigh[ii]][1];
      fitmp[ii][2] += fters[ILP_nneigh[ii]][2];
    
      lwpf_start(DNORMAL);
      for (int kk = 0; kk < ILP_nneigh[ii]; kk++) {
	int k = ILP_neigh[ii][kk];
	//if (k == i) continue;
	double fk[3];
	fk[0] = dnormdxk[ii][0][0][kk] * dproddni[ii][0] + dnormdxk[ii][1][0][kk] * dproddni[ii][1] + dnormdxk[ii][2][0][kk] * dproddni[ii][2];
	fk[1] = dnormdxk[ii][0][1][kk] * dproddni[ii][0] + dnormdxk[ii][1][1][kk] * dproddni[ii][1] + dnormdxk[ii][2][1][kk] * dproddni[ii][2];
	fk[2] = dnormdxk[ii][0][2][kk] * dproddni[ii][0] + dnormdxk[ii][1][2][kk] * dproddni[ii][1] + dnormdxk[ii][2][2][kk] * dproddni[ii][2];
	fk[0] += fters[kk][0];
	fk[1] += fters[kk][1];
	fk[2] += fters[kk][2];
	fcache_intra.update(k, fk[0], fk[1], fk[2]);

	if (VFLAG_EITHER){
	  // delki[0] = x[k][0] - xtmp;
	  // delki[1] = x[k][1] - ytmp;
	  // delki[2] = x[k][2] - ztmp;
	  tally.ev_xyz(0, 0, fk[0], fk[1], fk[2], dk[ii][kk][0], dk[ii][kk][1], dk[ii][kk][2]);
	}
      }
      lwpf_stop(DNORMAL);
      fitmp[ii][0] += dnormdxi[ii][0][0] * dproddni[ii][0] + dnormdxi[ii][1][0] * dproddni[ii][1] + dnormdxi[ii][2][0] * dproddni[ii][2];
      fitmp[ii][1] += dnormdxi[ii][0][1] * dproddni[ii][0] + dnormdxi[ii][1][1] * dproddni[ii][1] + dnormdxi[ii][2][1] * dproddni[ii][2];
      fitmp[ii][2] += dnormdxi[ii][0][2] * dproddni[ii][0] + dnormdxi[ii][1][2] * dproddni[ii][1] + dnormdxi[ii][2][2] * dproddni[ii][2];
      fcache_intra.update(i, fitmp[ii][0], fitmp[ii][1], fitmp[ii][2]);
    }

  }	   
  lwpf_stop(ILOOP);
  // loop over ii  
  fcache_inter.flush_all();
  fcache_intra.flush_all();
  
  lwpf_stop(ALL);
  lwpf_exit(SLAVE_LMFF);
  //exit(0);
  //for (int i = 0; i < 64; i ++){
  //if (_MYID == i){
  //  printf("%d %d %ld\n", fcache_inter.nswap, fcache_inter.naccess, fcache_inter.acquire_cycles);
  //printf("%d %d\n", fcache_intra.nswap, fcache_intra .naccess);
  //}
  athread_syn(ARRAY_SCOPE, 0xffff);
  //}
  if (vflag_fdotr){
    struct vec3 {
      double x, y, z;
    };
    auto x = linear_in<32>((vec3*)atom->x[0]);
    auto f = linear_in<32>((vec3*)atom->f[0]);
    for (int i : parallel_iterate<32>(0, atom->nlocal + atom->nghost, x, f)) {
      tally.v_xyz(f[i].x, f[i].y, f[i].z, x[i].x, x[i].y, x[i].z);
    }
  }
  if (_MYID == 0) vflag_fdotr = 0;
  tally.finish(eng_vdwl, eng_coul, virial, pvector);
  flush_slave_cache();
}


template struct make_parallel_eval<PairLMFFSunway, 3>;
template void parallel_eval<1,1,1>(PairLMFFSunway*);
template void parallel_eval<0,1,1>(PairLMFFSunway*);
template void parallel_eval<1,0,1>(PairLMFFSunway*);
template void parallel_eval<0,0,1>(PairLMFFSunway*);
template void parallel_eval<1,1,0>(PairLMFFSunway*);
template void parallel_eval<0,1,0>(PairLMFFSunway*);
template void parallel_eval<1,0,0>(PairLMFFSunway*);
template void parallel_eval<0,0,0>(PairLMFFSunway*);

template<int EFLAG, int VFLAG_EITHER>
__always_inline void PairLMFFSunway::eval_tersoff(tallyvar<2> &tally, double (*fk)[3], double (*rk)[3], int itype, int *ktypes, bool *newton, int nneigh){
  double fc[MAX_NEIGH], dfc[MAX_NEIGH];
  double rsq[MAX_NEIGH], rinv[MAX_NEIGH];
  double fxitmp = 0, fyitmp = 0, fzitmp = 0;
  for (int k = 0; k < nneigh; k ++){
    rsq[k] = rk[k][0]*rk[k][0] + rk[k][1]*rk[k][1] + rk[k][2]*rk[k][2];
    rinv[k] = invsqrt(rsq[k]);
    double r = rinv[k] * rsq[k];
    ProxyTersoff::Param &p = tersoff_params[tersoff_elem3param[itype][ktypes[k]][ktypes[k]]];
    fc[k] = ProxyTersoff::fc(r, &p);
    dfc[k] = ProxyTersoff::fc_d(r, &p);
    
    if (rsq[k] < p.cutsq && newton[k]){
      double fr = exp(-p.lam1 * r);
      double fpair = p.biga * fr * (dfc[k] - fc[k]*p.lam1) * rinv[k];
      double eng = fc[k] * p.biga * fr;
      fk[k][0] -= rk[k][0] * fpair;
      fk[k][1] -= rk[k][1] * fpair;
      fk[k][2] -= rk[k][2] * fpair;
      fxitmp += rk[k][0] * fpair;
      fyitmp += rk[k][1] * fpair;
      fzitmp += rk[k][2] * fpair;
      if (EFLAG && VFLAG_EITHER){
	tally.ev(eng, 0, fpair, rk[k][0], rk[k][1], rk[k][2]);
      } else if (EFLAG) {
	tally.e(eng, 0);
      } else if (VFLAG_EITHER){
	tally.v(fpair, rk[k][0], rk[k][1], rk[k][2]);
      }

      //printf("repulsive: %g %g %g\n", rk[k][0] * fpair, rk[k][1] * fpair, rk[k][2] * fpair);
      //tally.ev(e, 0, fpair, rk[k][0],rk[k][1], rk[k][2]);
    }
  }
  
  for (int j = 0; j < nneigh; j ++){
    double r = rinv[j] * rsq[j];
    ProxyTersoff::Param &pij = tersoff_params[tersoff_elem3param[itype][ktypes[j]][ktypes[j]]];
    double rijhat[3];
    rijhat[0] = rk[j][0] * rinv[j];
    rijhat[1] = rk[j][1] * rinv[j];
    rijhat[2] = rk[j][2] * rinv[j];
    double rij = rinv[j] * rsq[j];
    double zetaij = 0;

    double fkatt[MAX_NEIGH][3], fiatt[MAX_NEIGH][3], fjatt[MAX_NEIGH][3];
    for (int k = 0; k < nneigh; k ++){
      if (k == j) continue;
      ProxyTersoff::Param &pijk = tersoff_params[tersoff_elem3param[itype][ktypes[j]][ktypes[k]]];
      if (rsq[k] >= pijk.cutsq) continue;
      double rikhat[3];
      rikhat[0] = rk[k][0] * rinv[k];
      rikhat[1] = rk[k][1] * rinv[k];
      rikhat[2] = rk[k][2] * rinv[k];
      double rik = rinv[k] * rsq[k];
      double costheta = rijhat[0]*rikhat[0] + rijhat[1]*rikhat[1] + rijhat[2]*rikhat[2];
      double lam3_delr = pijk.lam3 * (rij - rik);
      if (pijk.powermint == 3) lam3_delr = lam3_delr * lam3_delr * lam3_delr;
      double ex_delr;
      if (lam3_delr > 69.0776) ex_delr = 1e30;
      else if (lam3_delr < -69.0776) ex_delr = 0;
      else ex_delr = exp(lam3_delr);
      double ex_delr_d;
      if (pijk.powermint == 3)
	ex_delr_d = 3.0*cube(pijk.lam3) * square(rij-rik)*ex_delr;
      else ex_delr_d = pijk.lam3 * ex_delr;

      double gijk = ProxyTersoff::calc_gijk(costheta, &pijk);
      double gijk_d = ProxyTersoff::calc_gijk_d(costheta, &pijk);

      zetaij += fc[k] * gijk * ex_delr;
      double dcosdri[3], dcosdrj[3], dcosdrk[3];
      ProxyTersoff::costheta_d(rijhat, rinv[j], rikhat, rinv[k], dcosdri, dcosdrj, dcosdrk);
      fiatt[k][0] = -dfc[k]*gijk*ex_delr*rikhat[0] + fc[k]*gijk_d*ex_delr*dcosdri[0] + fc[k]*gijk*ex_delr_d*(rikhat[0]-rijhat[0]);
      fiatt[k][1] = -dfc[k]*gijk*ex_delr*rikhat[1] + fc[k]*gijk_d*ex_delr*dcosdri[1] + fc[k]*gijk*ex_delr_d*(rikhat[1]-rijhat[1]);
      fiatt[k][2] = -dfc[k]*gijk*ex_delr*rikhat[2] + fc[k]*gijk_d*ex_delr*dcosdri[2] + fc[k]*gijk*ex_delr_d*(rikhat[2]-rijhat[2]);

      fjatt[k][0] = fc[k]*gijk_d*ex_delr*dcosdrj[0] + fc[k]*gijk*ex_delr_d*rijhat[0];
      fjatt[k][1] = fc[k]*gijk_d*ex_delr*dcosdrj[1] + fc[k]*gijk*ex_delr_d*rijhat[1];
      fjatt[k][2] = fc[k]*gijk_d*ex_delr*dcosdrj[2] + fc[k]*gijk*ex_delr_d*rijhat[2];

      fkatt[k][0] = (dfc[k]*gijk*ex_delr - fc[k]*gijk*ex_delr_d)*rikhat[0] + fc[k]*gijk_d*ex_delr*dcosdrk[0];
      fkatt[k][1] = (dfc[k]*gijk*ex_delr - fc[k]*gijk*ex_delr_d)*rikhat[1] + fc[k]*gijk_d*ex_delr*dcosdrk[1];
      fkatt[k][2] = (dfc[k]*gijk*ex_delr - fc[k]*gijk*ex_delr_d)*rikhat[2] + fc[k]*gijk_d*ex_delr*dcosdrk[2];		  
    }
    double fa = ProxyTersoff::fa(rij, &pij);
    double fa_d = ProxyTersoff::fa_d(rij, &pij);
    double bij = ProxyTersoff::bij(zetaij, &pij);
    double fpair = 0.5*bij*fa_d * rinv[j];
    double prefactor = -0.5*fa*ProxyTersoff::bij_d(zetaij, &pij);
    double eng = 0.5 * bij * fa;
    
    fxitmp += rk[j][0] * fpair;
    fyitmp += rk[j][1] * fpair;
    fzitmp += rk[j][2] * fpair;

    double fxjtmp = -rk[j][0] * fpair;
    double fyjtmp = -rk[j][1] * fpair;
    double fzjtmp = -rk[j][2] * fpair;
    //printf("zeta: %g %g %g %g %g %g\n", zetaij, bij, fa_d, rk[j][0]*fpair, rk[j][1]*fpair, rk[j][2]*fpair);
    if (EFLAG && VFLAG_EITHER){
      tally.ev(eng, 0, fpair, rk[j][0], rk[j][1], rk[j][2]);
    } else if (EFLAG) {
      tally.e(eng, 0);
    } else if (VFLAG_EITHER){
      tally.v(fpair, rk[j][0], rk[j][1], rk[j][2]);
    }
    
    for (int k = 0; k < nneigh; k ++) {
      if (k == j) continue;
      fxitmp += fiatt[k][0] * prefactor;
      fyitmp += fiatt[k][1] * prefactor;
      fzitmp += fiatt[k][2] * prefactor;

      fxjtmp += fjatt[k][0] * prefactor;
      fyjtmp += fjatt[k][1] * prefactor;
      fzjtmp += fjatt[k][2] * prefactor;

      fk[k][0] += fkatt[k][0] * prefactor;
      fk[k][1] += fkatt[k][1] * prefactor;
      fk[k][2] += fkatt[k][2] * prefactor;

      if (VFLAG_EITHER){
	tally.v_xyz(fkatt[k][0] * prefactor, fkatt[k][1] * prefactor, fkatt[k][2] * prefactor, rk[k][0], rk[k][1], rk[k][2]);
	tally.v_xyz(fjatt[j][0] * prefactor, fjatt[j][1] * prefactor, fjatt[j][2] * prefactor, rk[j][0], rk[j][1], rk[j][2]);
      }
    }
    fk[j][0] += fxjtmp;
    fk[j][1] += fyjtmp;
    fk[j][2] += fzjtmp;
  }
  fk[nneigh][0] += fxitmp;
  fk[nneigh][1] += fyitmp;
  fk[nneigh][2] += fzitmp;
}

#undef exp
#undef log
#undef pow
#undef sin
#undef cos
#endif
#ifdef __sw_host__

#define MAXLINE 1024
#define DELTA 4
#define PGDELTA 1
void PairLMFFSunway::ProxyTersoff::coeff(int narg, char **arg){
  for (int i = 0; i < narg; i ++){
    //puts(arg[i] ? arg[i] : "(nil)");
  }
  PairTersoff::coeff(narg, arg);
}
void PairLMFFSunway::ProxyTersoff::export_params(PairLMFFSunway *parent)
{
  int n = atom->ntypes + 1;
  memory->create(parent->tersoff_params, nparams, "PairLMFFSunway:tersoff_params");
  for (int i = 0; i < nparams ; i ++)
    parent->tersoff_params[i] = params[i];
  memory->create(parent->tersoff_elem3param, n, n, n, "PairLMFFSunway:tersoff_elem3param");
  for (int i = 1; i < n; i ++){
    if (map[i] == -1) continue;
    for (int j = 1; j < n; j ++){
      if (map[j] == -1) continue;
      for (int k = 1; k < n; k ++){
	if (map[k] == -1) continue;
	 
	parent->tersoff_elem3param[i][j][k] = elem3param[map[i]][map[j]][map[k]];
      }
    }
  }
  parent->tersoff_nparams = nparams;
};
static const char cite_ilp[] =
    "@Article{Ouyang2018\n"
    " author = {W. Ouyang, D. Mandelli, M. Urbakh, and O. Hod},\n"
    " title = {Nanoserpents: Graphene Nanoribbon Motion on Two-Dimensional Hexagonal Materials},\n"
    " journal = {Nano Letters},\n"
    " volume =  18,\n"
    " pages =   {6009}\n"
    " year =    2018,\n"
    "}\n\n";

/* ---------------------------------------------------------------------- */
extern "C" {
extern void *libc_uncached_malloc (size_t __size);
extern void libc_uncached_free (void *__ptr);
}
PairLMFFSunway::PairLMFFSunway(LAMMPS *lmp) : Pair(lmp)
{
  restartinfo = 0;
  one_coeff = 1;
  manybody_flag = 1;
  centroidstressflag = CENTROID_NOTAVAIL;
  unit_convert_flag = utils::get_supported_conversions(utils::ENERGY);

  if (lmp->citeme) lmp->citeme->add(cite_ilp);

  nextra = 2;
  pvector = new double[nextra];

  // initialize element to parameter maps
  params = nullptr;
  cutILPsq = nullptr;
  cutinv = nullptr;
  type_params = nullptr;
  type_cutILPsq = nullptr;
  layered_neigh = nullptr;
  bundled_neigh = nullptr;
  //first_layered_neigh = nullptr;
  // num_intra = nullptr;
  // num_inter = nullptr;
  // num_vdw = nullptr;
  neigh_index = nullptr;
  force_locks = nullptr;
  atompack = nullptr;
  inum_max = 0;
  jnum_max = 0;
  nmax = 0;
  fmax = 0;
  maxlocal = 0;
  
  // always compute energy offset
  offset_flag = 1;

  // turn on the taper function by default
  tap_flag = 1;
}

/* ---------------------------------------------------------------------- */

PairLMFFSunway::~PairLMFFSunway()
{
  delete[] pvector;

  if (allocated) {
    memory->destroy(setflag);
    memory->destroy(cutsq);
    memory->destroy(offset);
  }

  memory->destroy(layered_neigh);
  memory->destroy(bundled_neigh);
  //memory->sfree(first_layered_neigh);
  // memory->destroy(num_intra);
  // memory->destroy(num_inter);
  // memory->destroy(num_vdw);
  memory->destroy(atompack);
  memory->destroy(force_locks);
  memory->destroy(neigh_index);
  memory->destroy(elem2param);
  memory->destroy(cutILPsq);
  memory->destroy(cutinv);
  memory->destroy(type_params);
  memory->destroy(type_cutILPsq);
  memory->sfree(params);
  if (force_locks) libc_uncached_free(force_locks);
  memory->destroy(tersoff_params);
  memory->destroy(tersoff_elem3param);
}

/* ----------------------------------------------------------------------
   allocate all arrays
------------------------------------------------------------------------- */

void PairLMFFSunway::allocate()
{
  //tersoff->allocate();
  allocated = 1;
  int n = atom->ntypes + 1;

  memory->create(setflag, n, n, "pair:setflag");
  for (int i = 1; i < n; i++)
    for (int j = i; j < n; j++) setflag[i][j] = 0;

  memory->create(cutsq, n, n, "pair:cutsq");
  memory->create(offset, n, n, "pair:offset");
  memory->create(type_cutILPsq, n, n, "PairLMFFSunway:cutILPsq");
  memory->create(cutinv, n, n, "PairLMFFSunway:cutnv");
  memory->create(type_params, n, n, "PairLMFFSunway:type_params");
  map = new int[n];
}

/* ----------------------------------------------------------------------
   global settings
------------------------------------------------------------------------- */

void PairLMFFSunway::settings(int narg, char **arg)
{
  if (narg < 1 || narg > 2) error->all(FLERR, "Illegal pair_style command");
  // if (strcmp(force->pair_style, "hybrid/overlay") != 0)
  //   error->all(FLERR, "ERROR: requires hybrid/overlay pair_style");

  cut_global = utils::numeric(FLERR, arg[0], false, lmp);
  if (narg == 2) tap_flag = utils::numeric(FLERR, arg[1], false, lmp);
}

/* ----------------------------------------------------------------------
   set coeffs for one or more type pairs
------------------------------------------------------------------------- */

void PairLMFFSunway::coeff(int narg, char **arg)
{
  if (!allocated) allocate();

  map_element2type(atom->ntypes, arg + 3);
  
  read_file(arg[2]);
  // for (int i = 0; i < narg; i ++){
  //   puts(arg[i]);
  // }

  //int argptr = arg + 3 + atom->ntypes;
  ProxyTersoff *tmp = new ProxyTersoff(lmp);
  
  //char *arg_tersoff[] = {"*", "*", "C.opt.tersoff", "C(O)", "C(O)", "C(O)", "C(O)", "C(O)", "C(O)"};
  //Caution: this requres the intralayer potential accept coeffs other than "*" "*"
  tmp->coeff(atom->ntypes + 3, arg + 1 + atom->ntypes);
  tmp->export_params(this);
  // for (int i = 0; i < tersoff_nparams; i ++){
  //   printf("%f %f %f\n", tersoff_params[i].lam1, tersoff_params[i].lam2, tersoff_params[i].lam3);
  // }
  // for (int i = 0; i < atom->ntypes + 1; i ++){
  //   printf("%d %d\n", tmp->map[i], map[i]);
  // }
  delete tmp;
}

/* ----------------------------------------------------------------------
   init for one type pair i,j and corresponding j,i
------------------------------------------------------------------------- */

double PairLMFFSunway::init_one(int i, int j)
{
  if (setflag[i][j] == 0) error->all(FLERR, "All pair coeffs are not set");
  if (!offset_flag) error->all(FLERR, "Must use 'pair_modify shift yes' with this pair style");

  if (offset_flag && (cut_global > 0.0)) {
    int iparam_ij = elem2param[map[i]][map[j]];
    Param &p = params[iparam_ij];
    offset[i][j] =
        -p.C6 * pow(1.0 / cut_global, 6) / (1.0 + exp(-p.d * (cut_global / p.seff - 1.0)));
  } else
    offset[i][j] = 0.0;
  offset[j][i] = offset[i][j];

  return cut_global;
}

/* ----------------------------------------------------------------------
   read Interlayer potential file
------------------------------------------------------------------------- */

void PairLMFFSunway::read_file(char *filename)
{
  memory->sfree(params);
  params = nullptr;
  nparams = maxparam = 0;

  // open file on proc 0

  if (comm->me == 0) {
    PotentialFileReader reader(lmp, filename, "ilp/graphene/hbn", unit_convert_flag);
    char *line;

    // transparently convert units for supported conversions

    int unit_convert = reader.get_unit_convert();
    double conversion_factor = utils::get_conversion_factor(utils::ENERGY, unit_convert);

    while ((line = reader.next_line(NPARAMS_PER_LINE))) {
      //puts(line);
      try {
        ValueTokenizer values(line);

        std::string iname = values.next_string();
        std::string jname = values.next_string();

        // ielement,jelement = 1st args
        // if both args are in element list, then parse this line
        // else skip to next entry in file
        int ielement, jelement;

        for (ielement = 0; ielement < nelements; ielement++)
          if (iname == elements[ielement]) break;
        if (ielement == nelements) continue;
        for (jelement = 0; jelement < nelements; jelement++)
          if (jname == elements[jelement]) break;
        if (jelement == nelements) continue;

        // expand storage, if needed

        if (nparams == maxparam) {
          maxparam += DELTA;
          params = (Param *) memory->srealloc(params, maxparam * sizeof(Param), "pair:params");

          // make certain all addional allocated storage is initialized
          // to avoid false positives when checking with valgrind

          memset(params + nparams, 0, DELTA * sizeof(Param));
        }

        // load up parameter settings and error check their values

        params[nparams].ielement = ielement;
        params[nparams].jelement = jelement;
        params[nparams].z0 = values.next_double();
        params[nparams].alpha = values.next_double();
        params[nparams].delta = values.next_double();
        params[nparams].epsilon = values.next_double();
        params[nparams].C = values.next_double();
        params[nparams].d = values.next_double();
        params[nparams].sR = values.next_double();
        params[nparams].reff = values.next_double();
        params[nparams].C6 = values.next_double();
        // S provides a convenient scaling of all energies
        params[nparams].S = values.next_double();
        params[nparams].rcut = values.next_double();
      } catch (TokenizerException &e) {
        error->one(FLERR, e.what());
      }

      // energies in meV further scaled by S
      // S = 43.3634 meV = 1 kcal/mol

      double meV = 1e-3 * params[nparams].S;
      if (unit_convert) meV *= conversion_factor;

      params[nparams].C *= meV;
      params[nparams].C6 *= meV;
      params[nparams].epsilon *= meV;

      // precompute some quantities
      params[nparams].delta2inv = pow(params[nparams].delta, -2.0);
      params[nparams].lambda = params[nparams].alpha / params[nparams].z0;
      params[nparams].seff = params[nparams].sR * params[nparams].reff;
      params[nparams].seffinv = 1.0 / params[nparams].seff;
      nparams++;
    }
  }
  //printf("id: %d, n: %d, max: %d\n", comm->me, nparams, maxparam);
  MPI_Bcast(&nparams, 1, MPI_INT, 0, world);
  MPI_Bcast(&maxparam, 1, MPI_INT, 0, world);

  if (comm->me != 0) {
    params = (Param *) memory->srealloc(params, maxparam * sizeof(Param), "pair:params");
  }

  MPI_Bcast(params, maxparam * sizeof(Param), MPI_BYTE, 0, world);
  //printf("id: %d, n: %d, max: %d\n", comm->me, nparams, maxparam);
  memory->destroy(elem2param);
  memory->destroy(cutILPsq);
  memory->create(elem2param, nelements, nelements, "pair:elem2param");
  memory->create(cutILPsq, nelements, nelements, "pair:cutILPsq");
  for (int i = 0; i < nelements; i++) {
    for (int j = 0; j < nelements; j++) {
      int n = -1;
      for (int m = 0; m < nparams; m++) {
        if (i == params[m].ielement && j == params[m].jelement) {
          if (n >= 0) error->all(FLERR, "ILP Potential file has duplicate entry");
          n = m;
        }
      }
      if (n < 0) error->all(FLERR, "Potential file is missing an entry");
      elem2param[i][j] = n;
      cutILPsq[i][j] = params[n].rcut * params[n].rcut;
    }
  }
  for (int i = 1; i <= atom->ntypes; i ++){
    for (int j = 1; j <= atom->ntypes; j ++){
      type_params[i][j] = params[elem2param[map[i]][map[j]]];
      type_cutILPsq[i][j] = cutILPsq[map[i]][map[j]];
    }
  }
}

/* ----------------------------------------------------------------------
   init specific to this pair style
------------------------------------------------------------------------- */

void PairLMFFSunway::init_style()
{
  if (force->newton_pair == 0)
    error->all(FLERR, "Pair style ilp/graphene/hbn requires newton pair on");
  if (!atom->molecule_flag)
    error->all(FLERR, "Pair style ilp/graphene/hbn requires atom attribute molecule");

  // need a full neighbor list, including neighbors of ghosts

  int irequest = neighbor->request(this, instance_me);
  neighbor->requests[irequest]->half = 0;
  neighbor->requests[irequest]->full = 1;
  neighbor->requests[irequest]->ghost = 0;
  neighbor->requests[irequest]->sunway = 1;
}

/* ---------------------------------------------------------------------- */
void PairLMFFSunway::compute(int eflag, int vflag)
{
  GPTLstart("LMFF");
  int i, j, ii, jj, inum, jnum, itype, itype_map, jtype, k, kk;
  double prodnorm1, fkcx, fkcy, fkcz;
  double xtmp, ytmp, ztmp, delx, dely, delz, evdwl, fpair, fpair1;
  double rsq, r, Rcut, rhosq1, exp0, exp1, Tap, dTap, Vilp;
  double frho1, Erep, fsum, rdsq1;
  int *ilist, *jlist, *numneigh, **firstneigh;
  int *ILP_neighs_i;
  tagint itag, jtag;
  evdwl = 0.0;

  double **x = atom->x;
  double **f = atom->f;
  tagint *tag = atom->tag;
  int *type = atom->type;
  int nlocal = atom->nlocal;
  int newton_pair = force->newton_pair;
  double dprodnorm1[3] = {0.0, 0.0, 0.0};
  double fp1[3] = {0.0, 0.0, 0.0};
  double fprod1[3] = {0.0, 0.0, 0.0};
  double delki[3] = {0.0, 0.0, 0.0};
  double fk[3] = {0.0, 0.0, 0.0};
  double dproddni[3] = {0.0, 0.0, 0.0};
  double cij;

  inum = list->inum;
  ilist = list->ilist;
  numneigh = list->numneigh;
  firstneigh = list->firstneigh;

  ev_init(eflag, vflag);
  pvector[0] = pvector[1] = 0.0;
  
  if (neighbor->ago == 0) update_internal_list();
  
  //lwpf_init(NULL);
  //ilp_start();  
  if (eflag_global || eflag_atom) {
    if (vflag_either){
      if (tap_flag) {
	eval<1,1,1>();
      } else{
	eval<1,1,0>();
      }
    } else {
      if (tap_flag) {
	eval<1,0,1>();
      } else{
	eval<1,0,0>();
      }
    }
  } else {
    if (vflag_either){
      if (tap_flag) {
	eval<0,1,1>();
      } else{
	eval<0,1,0>();
      }
    } else {
      if (tap_flag) {
	eval<0,0,1>();
      } else{
	eval<0,0,0>();
      }
    }
  }
  //ilp_stop();
  //lwpf_report_summary(stdout);
  //puts("?????");
  //
  // printf("includegrp: %d\n", neighbor->includegroup);
  // if (vflag_fdotr) virial_fdotr_compute();  

  GPTLstop("LMFF");
}
extern "C"{
  void penv_cg_tbox6_total_trans_init();
  void penv_cg_tbox6_total_trans_count(unsigned long*);
}
template <int EFLAG, int VFLAG_EITHER, int TAP_FLAG>
void PairLMFFSunway::eval(){
  //if (!getenv("USEMPE")){
  auto pack_params = std::make_tuple(atompack, (double(*)[3])atom->x[0], atom->type, atom->molecule, atom->nlocal + atom->nghost);
    athread_resolve_spawn(slave_pack_atoms_mol, &pack_params);
    athread_resolve_join();
    //penv_cg_tbox6_total_trans_init();
    unsigned long trans_st[16], trans_ed[16];
    //penv_cg_tbox6_total_trans_count(trans_st);
    parallel_eval_pair<EFLAG, VFLAG_EITHER, TAP_FLAG>(this);
    //penv_cg_tbox6_total_trans_count(trans_ed);
    long trans_tot = 0;
    for (int i = 0; i < 16; i ++){
      //trans_tot += trans_ed[i] - trans_st[i];
      //printf("%d\n", trans_ed[i] - trans_st[i]);
    }
    //if (vflag_fdotr) virial_fdotr_compute();
    //printf("total memory access: %ld\n", trans_tot);
    return;
    //}
  // constexpr int EVFLAG = EFLAG || VFLAG_EITHER;
  // int i, j, ii, jj, inum, jnum, itype, itype_map, jtype, k, kk;
  // double prodnorm1, fkcx, fkcy, fkcz;
  // double xtmp, ytmp, ztmp, delx, dely, delz, evdwl, fpair, fpair1;
  // double rsq, r, Rcut, rhosq1, exp0, exp1, Tap, dTap, Vilp;
  // double frho1, Erep, fsum, rdsq1;
  // int *ilist, *jlist, *numneigh, **firstneigh;
  // int *ILP_neighs_i;
  // tagint itag, jtag;
  // evdwl = 0.0;

  // double **x = atom->x;
  // double **f = atom->f;
  // tagint *tag = atom->tag;
  // int *type = atom->type;
  // int nlocal = atom->nlocal;
  // int newton_pair = force->newton_pair;
  // double dprodnorm1[3] = {0.0, 0.0, 0.0};
  // double fp1[3] = {0.0, 0.0, 0.0};
  // double fprod1[3] = {0.0, 0.0, 0.0};
  // double delki[3] = {0.0, 0.0, 0.0};
  // double fk[3] = {0.0, 0.0, 0.0};
  // double dproddni[3] = {0.0, 0.0, 0.0};
  // double cij;

  // inum = list->inum;
  // ilist = list->ilist;
  // numneigh = list->numneigh;
  // firstneigh = list->firstneigh;

  // for (ii = 0; ii < inum; ii++) {
  //   i = ilist[ii];
  //   xtmp = x[i][0];
  //   ytmp = x[i][1];
  //   ztmp = x[i][2];
  //   itype = type[i];
  //   itype_map = map[type[i]];
  //   itag = tag[i];
  //   jlist = firstneigh[i];
  //   jnum = numneigh[i];
  //   int *jlist_intra = first_layered_neigh[i];
  //   int *jlist_inter = first_layered_neigh[i] + num_intra[i];
  //   int jnum_intra = num_intra[i];
  //   int jnum_inter = num_inter[i];
  //   int jnum_vdw = num_vdw[i];
  //   int ILP_neigh[3];
  //   int ILP_nneigh = 0;
  //   for (jj = 0; jj < jnum_intra; jj++) {
  //     j = jlist_intra[jj] & ~NEWTON_MASK;

  //     jtype = map[type[j]];
  //     delx = xtmp - x[j][0];
  //     dely = ytmp - x[j][1];
  //     delz = ztmp - x[j][2];
  //     rsq = delx * delx + dely * dely + delz * delz;

  //     if (rsq < cutILPsq[itype_map][jtype]) {
  //       if (ILP_nneigh >= 3) error->one(FLERR, "There are too many neighbors for calculating normals");
  //       ILP_neigh[ILP_nneigh ++] = j;
  //     }
  //   }    // loop over jj

  //   dproddni[0] = 0.0;
  //   dproddni[1] = 0.0;
  //   dproddni[2] = 0.0;

  //   double norm[3], dnormdxi[3][3], dnormdxk[3][3][3];
  //   calc_single_normal(i, ILP_neigh, ILP_nneigh, norm, dnormdxi, dnormdxk);

  //   for (jj = 0; jj < jnum_inter; jj++) {
  //     j = jlist_inter[jj];
  //     jtype = type[j];
  //     jtag = tag[j];

  //     delx = xtmp - x[j][0];
  //     dely = ytmp - x[j][1];
  //     delz = ztmp - x[j][2];
  //     rsq = delx * delx + dely * dely + delz * delz;
  //     // if (i == 0) {
  //     // 	printf("%d %g %g %g\n", j, delx, dely, delz);
  //     // }

  //     // only include the interaction between different layers
  //     if (rsq < cutsq[itype][jtype]) {

  //       int iparam_ij = elem2param[map[itype]][map[jtype]];
  //       Param &p = params[iparam_ij];
  //       // if (i == 0) {
  //       //   printf("%d %d %f\n", j, iparam_ij, p.seff);
  //       // }
  //       r = sqrt(rsq);
  //       double r2inv = 1.0 / rsq;
  //       double rinv = r * r2inv;
  //       // turn on/off taper function
  //       if (TAP_FLAG) {
  //         Rcut = sqrt(cutsq[itype][jtype]);
  //         Tap = calc_Tap(r, Rcut);
  //         dTap = calc_dTap(r, Rcut);
  //       } else {
  //         Tap = 1.0;
  //         dTap = 0.0;
  //       }
  //       // if (i == 0) {
  //       //   printf("%d %g %g\n", j, Tap, rinv);
  //       // }

  //       // Calculate the transverse distance
  //       prodnorm1 = norm[0] * delx + norm[1] * dely + norm[2] * delz;
  //       rhosq1 = rsq - prodnorm1 * prodnorm1;    // rho_ij
  //       rdsq1 = rhosq1 * p.delta2inv;            // (rho_ij/delta)^2

  //       // store exponents
  //       exp0 = exp(-p.lambda * (r - p.z0));
  //       exp1 = exp(-rdsq1);

  //       frho1 = exp1 * p.C;
  //       Erep = 0.5 * p.epsilon + frho1;

  //       Vilp = exp0 * Erep;

  //       // derivatives
  //       fpair = p.lambda * exp0 * rinv * Erep;
  //       fpair1 = 2.0 * exp0 * frho1 * p.delta2inv;
  //       fsum = fpair + fpair1;
  //       // derivatives of the product of rij and ni, the result is a vector

  //       fp1[0] = prodnorm1 * norm[0] * fpair1;
  //       fp1[1] = prodnorm1 * norm[1] * fpair1;
  //       fp1[2] = prodnorm1 * norm[2] * fpair1;

  //       fkcx = (delx * fsum - fp1[0]) * Tap - Vilp * dTap * delx * rinv;
  //       fkcy = (dely * fsum - fp1[1]) * Tap - Vilp * dTap * dely * rinv;
  //       fkcz = (delz * fsum - fp1[2]) * Tap - Vilp * dTap * delz * rinv;

  //       f[i][0] += fkcx;
  //       f[i][1] += fkcy;
  //       f[i][2] += fkcz;
  //       f[j][0] -= fkcx;
  //       f[j][1] -= fkcy;
  //       f[j][2] -= fkcz;
  //       // if (i == 0){
  //       //   printf("%d %g %g %g\n", j, fkcx, fkcy, fkcz);
  //       // }
  //       cij = -prodnorm1 * fpair1 * Tap;
  //       dproddni[0] += cij * delx;
  //       dproddni[1] += cij * dely;
  //       dproddni[2] += cij * delz;

  //       if (EFLAG) pvector[1] += evdwl = Tap * Vilp;
  //       if (EVFLAG)
  //         ev_tally_xyz(i, j, nlocal, newton_pair, evdwl, 0.0, fkcx, fkcy, fkcz, delx, dely, delz);

  //       /* ----------------------------------------------------------------------
  //             van der Waals forces and energy
  //             ------------------------------------------------------------------------- */
  //       if (jj >= jnum_vdw) continue;
  //       double r6inv = r2inv * r2inv * r2inv;
  //       double r8inv = r6inv * r2inv;

  //       double TSvdw = 1.0 + exp(-p.d * (r * p.seffinv - 1.0));
  //       double TSvdwinv = 1.0 / TSvdw;
  //       double TSvdw2inv = TSvdwinv * TSvdwinv;//pow(TSvdw, -2.0);
  //       Vilp = -p.C6 * r6inv * TSvdwinv;

  //       fpair = -6.0 * p.C6 * r8inv * TSvdwinv +
  //         p.C6 * p.d * p.seffinv * (TSvdw - 1.0) * TSvdw2inv * r8inv * r;
  //       fsum = fpair * Tap - Vilp * dTap * rinv;

  //       double fvdwx = fsum * delx;
  //       double fvdwy = fsum * dely;
  //       double fvdwz = fsum * delz;

  //       f[i][0] += fvdwx;
  //       f[i][1] += fvdwy;
  //       f[i][2] += fvdwz;
  //       f[j][0] -= fvdwx;
  //       f[j][1] -= fvdwy;
  //       f[j][2] -= fvdwz;
  //       // if (i == 0) {
  //       //   printf("%d %g %g %g\n", j, fvdwx, fvdwy, fvdwz);
  //       // }

  //       if (EFLAG) pvector[0] += evdwl = Tap * Vilp;
  //       if (EVFLAG)
  //         ev_tally_xyz(i, j, nlocal, newton_pair, evdwl, 0.0, fvdwx, fvdwy, fvdwz, delx, dely, delz);
  //     }
  //   }    // loop over jj

  //   //ILP_neighs_i = ILP_firstneigh[i];
  //   for (kk = 0; kk < ILP_nneigh; kk++) {
  //     k = ILP_neigh[kk];
  //     if (k == i) continue;
  //     // derivatives of the product of rij and ni respect to rk, k=0,1,2, where atom k is the neighbors of atom i
  //     fk[0] = dnormdxk[0][0][kk] * dproddni[0] + dnormdxk[1][0][kk] * dproddni[1] + dnormdxk[2][0][kk] * dproddni[2];
  //     fk[1] = dnormdxk[0][1][kk] * dproddni[0] + dnormdxk[1][1][kk] * dproddni[1] + dnormdxk[2][1][kk] * dproddni[2];
  //     fk[2] = dnormdxk[0][2][kk] * dproddni[0] + dnormdxk[1][2][kk] * dproddni[1] + dnormdxk[2][2][kk] * dproddni[2];

  //     f[k][0] += fk[0];
  //     f[k][1] += fk[1];
  //     f[k][2] += fk[2];
      
  //     delki[0] = x[k][0] - x[i][0];
  //     delki[1] = x[k][1] - x[i][1];
  //     delki[2] = x[k][2] - x[i][2];

  //     if (VFLAG_EITHER){
  //       ev_tally_xyz(k, i, nlocal, newton_pair, 0.0, 0.0, fk[0], fk[1], fk[2], delki[0],
  //                    delki[1], delki[2]);
  //       //printf("%f %f %f\n", fk[0]/delkj[0], fk[1] /delkj[1], fk[2] / delkj[2]);
  //     }
  //   }
  //   f[i][0] += dnormdxi[0][0] * dproddni[0] + dnormdxi[1][0] * dproddni[1] + dnormdxi[2][0] * dproddni[2];
  //   f[i][1] += dnormdxi[0][1] * dproddni[0] + dnormdxi[1][1] * dproddni[1] + dnormdxi[2][1] * dproddni[2];
  //   f[i][2] += dnormdxi[0][2] * dproddni[0] + dnormdxi[1][2] * dproddni[1] + dnormdxi[2][2] * dproddni[2];
  // }      // loop over ii  
  // exit(0);
}

bool greater(int a, int b){
  return a > b;
}
bool less(int a, int b){
  return a < b;
}
void PairLMFFSunway::update_internal_list() {
  internal_list_count ++;
  //puts("updating internal list");
  int jnum_sum = 0;
  int inum = atom->nlocal;
  int nall = atom->nlocal + atom->nghost;
  
  //printf("nall=%d\n", nall);
  if (nall > fmax * 8){
    if (force_locks == nullptr){
      int nallmax;
      MPI_Allreduce(&nall, &nallmax, 1, MPI_INT, MPI_MAX, world);
      fmax = nallmax;
    } else {
      fmax = nall;
      libc_uncached_free(force_locks);
    }
    force_locks = (cal_lock_t*)libc_uncached_malloc(fmax * sizeof(cal_lock_t));
    auto fill_param = std::make_tuple(force_locks, cal_lock_t{0, 0}, (size_t)fmax);
    athread_resolve_spawn(slave_fill_mem, &fill_param);
    athread_resolve_join();
  }
  if (nall > nmax) {
    if (atompack == nullptr){
      int nallmax;
      MPI_Allreduce(&nall, &nallmax, 1, MPI_INT, MPI_MAX, world);
      nmax = nallmax * 1.2;
    } else {
      nmax = nall * 1.2;
    }
    memory->destroy(atompack);
    //nmax = (int)ceil(nall * 1.1);
    memory->create(atompack, nmax, "PairLMFF:atompack");
    //if (nall > nmax * 8){
    //}
    //memset(force_locks, 0, nmax * sizeof(cal_lock_t));
  }
  int *ilist = list->ilist;
  int *numneigh = list->numneigh;
  int **firstneigh = list->firstneigh;
  int *jlist, jnum;
  tagint *tag = atom->tag;
  double **x = atom->x;
  // for (int ii = 0; ii < inum; ii ++){
  //   jnum_sum += numneigh[ilist[ii]];
  // }
  
  jnum_sum = inum * neighbor->oneatom;
  if (inum > inum_max) {
    if (inum_max == 0){
      MPI_Allreduce(&inum, &inum_max, 1, MPI_INT, MPI_MAX, world);
      inum_max *= 1.2;
      if (comm->me == 0) printf("all: inum_max grow to %d\n", inum_max);
    } else {
      inum_max = inum * 1.2;
      printf("%d: inum_max grow to %d\n", comm->me, inum_max);
    }
    //printf("%d: grow_internal_list %d %d\n", comm->me, inum, inum_max);
    //puts("grow i");
    // memory->destroy(num_intra);
    // memory->destroy(num_inter);
    // memory->destroy(num_vdw);
    // memory->sfree(first_layered_neigh);
    memory->destroy(neigh_index);
    memory->destroy(layered_neigh);
    memory->destroy(bundled_neigh);
    
    //golden ratio grow
    //inum_max = inum * 2;
    // memory->create(num_intra, inum_max, "PairLMFFSunway:intra_layer_count");
    // memory->create(num_inter, inum_max, "PairLMFFSunway:inter_layer_count");
    // memory->create(num_vdw, inum_max, "PairLMFFSunway:vdw_count");
    memory->create(neigh_index, inum_max, "PairLMFFSunway:neigh_index");
    // first_layered_neigh = (int**)memory->smalloc(inum_max * sizeof(int*), "PairLMFFSunway:first_layered_neigh");
    memory->create(layered_neigh, inum_max * neighbor->oneatom, "PairLMFFSunway:layered_neigh");
    memory->create(bundled_neigh, (inum_max + 7 >> 3) * neighbor->oneatom*BUNDLE_RATIO, "PairLMFFSunway:bundled_neigh");
  }

  double cut_intra = 0;
  for (int i = 0; i < nparams; i ++){
    if (params[i].rcut > cut_intra) {
      cut_intra = params[i].rcut;
    }
  }
  double cut_intra_listsq = (cut_intra + neighbor->skin) * (cut_intra + neighbor->skin);

  athread_resolve_spawn(slave_build_layered_list, this);
  athread_resolve_join();
}
/* ---------------------------------------------------------------------- */

double PairLMFFSunway::single(int /*i*/, int /*j*/, int itype, int jtype, double rsq,
                                  double /*factor_coul*/, double factor_lj, double &fforce)
{
  double r, r2inv, r6inv, r8inv, forcelj, philj, fpair;
  double Tap, dTap, Vilp, TSvdw, TSvdw2inv;

  int iparam_ij = elem2param[map[itype]][map[jtype]];
  Param &p = params[iparam_ij];

  r = sqrt(rsq);
  // turn on/off taper function
  if (tap_flag) {
    Tap = calc_Tap(r, sqrt(cutsq[itype][jtype]));
    dTap = calc_dTap(r, sqrt(cutsq[itype][jtype]));
  } else {
    Tap = 1.0;
    dTap = 0.0;
  }

  r2inv = 1.0 / rsq;
  r6inv = r2inv * r2inv * r2inv;
  r8inv = r2inv * r6inv;

  TSvdw = 1.0 + exp(-p.d * (r / p.seff - 1.0));
  TSvdw2inv = pow(TSvdw, -2.0);
  Vilp = -p.C6 * r6inv / TSvdw;
  // derivatives
  fpair = -6.0 * p.C6 * r8inv / TSvdw + p.d / p.seff * p.C6 * (TSvdw - 1.0) * r6inv * TSvdw2inv / r;
  forcelj = fpair;
  fforce = factor_lj * (forcelj * Tap - Vilp * dTap / r);

  philj = Vilp * Tap;
  return factor_lj * philj;
}
#endif
