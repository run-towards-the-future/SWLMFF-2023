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
#include "slave_fixheader.h"
#include "pair_ilp_graphene_hbn_sunway.h"
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
#include "tokenizer.h"
//#include "ilp_time.h"
#include <cmath>
#include <cstring>
#include "cal.h"
using namespace LAMMPS_NS;
using namespace InterLayer;
#include "parallel_eval.h"
#include "util_sunway.h"
#include <sys/cdefs.h>
#include <cassert>
#ifdef __sw_slave__
template void fill_mem(std::tuple<cal_lock_t *, cal_lock_t, size_t> *);
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

__always_inline static bool check_vdw(tagint itag, tagint jtag, double *xi, double *xj) {
  if (itag > jtag) {
    if ((itag + jtag) % 2 == 0) return false;
  } else if (itag < jtag) {
    if ((itag + jtag) % 2 == 1) return false;
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
void PairILPGrapheneHBNSunway::calc_single_normal(int i, int *ILP_neigh, int nneigh, double *normal, double (*dnormdri)[3], double (*dnormdrk)[3][3])
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

#ifdef __sw_host__
#define LWPF_UNITS U(SLAVE_ILP)
#include "lwpf3/lwpf.h"
#endif
#ifdef __sw_slave__
extern "C"{
#include "slave.h"
}
#include "swdmapp.hpp"
#include "sunway_iterator.hpp"
#include "math_sunway.hpp"
#define exp fexp_valid
#define LWPF_UNIT U(SLAVE_ILP)
#define LWPF_KERNELS K(ALL) K(ILOOP) K(JLOOP_INTRA) K(NORMAL) K(JLOOP_INTER) K(DNORMAL) K(REP) K(VDW)
#define EVT_PC0 PC0_CYCLE
#define EVT_PC1 PC1_INST
#define EVT_PC2 PC2_L1IC_MISSTIME
#define EVT_PC4 PC4_DCACHE_ACCESS
#define EVT_PC5 PC5_DCACHE_MISS
#include "lwpf3/lwpf.h"
static constexpr int MAX_ELEM = 8;
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
template<int EFLAG, int VFLAG_EITHER, int TAP_FLAG>
void PairILPGrapheneHBNSunway::parallel_eval(){
  //if (_MYID == 0) puts(__PRETTY_FUNCTION__);
  lwpf_enter(SLAVE_ILP);
  lwpf_start(ALL);
  constexpr int EVFLAG = EFLAG || VFLAG_EITHER;
  int i, j, ii, jj, inum, jnum, itype, itype_map, jtype, k, kk;
  double prodnorm1, fkcx, fkcy, fkcz;
  double xtmp, ytmp, ztmp, delx, dely, delz, evdwl, fpair, fpair1;
  double rsq, r, Rcut, rhosq1, exp0, exp1, Tap, dTap, Vilp;
  double frho1, Erep, fsum, rdsq1;
  int *ilist, *jlist, *numneigh, **firstneigh;
  int *ILP_neighs_i;
  tagint itag, jtag;
  evdwl = 0.0;
  int ntypes = atom->ntypes;
  int typemax = ntypes + 1;
  //auto map = ptr_in<MAX_ELEM>(this->map, ntypes+1);
  auto cutILPsq = to2d(ptr_in<MAX_ELEM*MAX_ELEM>(this->cutILPsq[0], nelements*nelements), nelements);
  auto tcutILPsq = to2d(ptr_in<MAX_ELEM*MAX_ELEM>(this->type_cutILPsq[0], typemax*typemax), typemax);
  auto tparams = to2d(ptr_in<MAX_ELEM*MAX_ELEM>(this->type_params[0], typemax*typemax), typemax);
  double cutsq = this->cut_global * this->cut_global;
  double cutinv = 1 / sqrt(cutsq);
  double (*x)[3] = (double(*)[3])atom->x[0];
  double (*f)[3] = (double(*)[3])atom->f[0];
  //tagint *tag = atom->tag;
  //int *type = atom->type;
  int nlocal = atom->nlocal;
  int newton_pair = force->newton_pair;
  double dprodnorm1[3] = {0.0, 0.0, 0.0};
  double fp1[3] = {0.0, 0.0, 0.0};
  double fprod1[3] = {0.0, 0.0, 0.0};
  double delki[3] = {0.0, 0.0, 0.0};
  double fk[3] = {0.0, 0.0, 0.0};
  double dproddni[3] = {0.0, 0.0, 0.0};
  double cij;
  ForceCache<512, 8> fcache(f, force_locks, atom->nlocal + atom->nghost);  
  ReadOnlyCache<atompack_t, 512, 8> jcache(atompack);
  inum = list->inum;
  //ilist = list->ilist;
  //numneigh = list->numneigh;
  //firstneigh = list->firstneigh;
  tallyvar<2> tally;
  tally.init();
  constexpr int BLKSIZE = 64;
  constexpr int BLKSIZEJ = 512;
  int *layered_neigh = this->layered_neigh;
  //auto atompacki = linear_in<BLKSIZE>(this->atompack);
  auto neigh_index = linear_in<BLKSIZE>(this->neigh_index);

  lwpf_start(ILOOP);
  //if (_MYID == 0)
  for (int i : parallel_iterate<BLKSIZE>(0, inum, /*atompacki,*/ neigh_index)){
    double fxitmp = 0, fyitmp = 0, fzitmp = 0;
    auto xi = jcache[i];
    xtmp = xi.x;
    ytmp = xi.y;
    ztmp = xi.z;
    itype = xi.type;
    //itype_map = map[itype];
    auto jlist_intra = linear_in<BLKSIZEJ>(layered_neigh + neigh_index[i].first);
    auto jlist_inter = linear_in<BLKSIZEJ>(layered_neigh + neigh_index[i].first + neigh_index[i].nintra);
    int jnum_intra = neigh_index[i].nintra;
    int jnum_inter = neigh_index[i].ninter;
    int jnum_vdw =   neigh_index[i].nvdw;
    int ILP_neigh[3];
    int ILP_nneigh = 0;
    double dk[3][3];
    lwpf_start(JLOOP_INTRA);
    for (int jj : iterate<BLKSIZEJ>(0, jnum_intra, jlist_intra)){
      //for (jj = 0; jj < jnum_intra; jj++) {
      j = jlist_intra[jj];
      //j = layered_neigh[neigh_index[i].first + jj];
      atompack_t xj = jcache[j];
      int jtype = xj.type;
      delx = xj.x - xtmp;
      dely = xj.y - ytmp;
      delz = xj.z - ztmp;
      rsq = delx * delx + dely * dely + delz * delz;

      if (rsq < tcutILPsq[itype][jtype]) {
	ILP_neigh[ILP_nneigh] = j;
	dk[ILP_nneigh][0] = delx;
	dk[ILP_nneigh][1] = dely;
	dk[ILP_nneigh][2] = delz;
	ILP_nneigh ++;
      }
    }
    lwpf_stop(JLOOP_INTRA);
    dproddni[0] = 0.0;
    dproddni[1] = 0.0;
    dproddni[2] = 0.0;

    double norm[3], dnormdxi[3][3], dnormdxk[3][3][3];
    lwpf_start(NORMAL);
    calc_normal(dk, ILP_nneigh, norm, dnormdxi, dnormdxk);
    lwpf_stop(NORMAL);
    lwpf_start(JLOOP_INTER);

    for (int jj : iterate<BLKSIZEJ>(0, jnum_inter, jlist_inter)){
      j = jlist_inter[jj];
      atompack_t xj = jcache[j];
      jtype = xj.type;
      delx = xtmp - xj.x;
      dely = ytmp - xj.y;
      delz = ztmp - xj.z;
      rsq = delx * delx + dely * dely + delz * delz;

      // only include the interaction between different layers
      if (rsq < cutsq) {
	//lwpf_start(REP);
	Param &p = tparams[itype][jtype];
	double rinv = invsqrt(rsq);// = r * r2inv;
	double r2inv = rinv * rinv;
	double r = rsq * rinv;
	// if (_MYID == 0)
	//    printf("%f %f %f\n", rsq, rinv, rinv*rinv*rsq);
	// turn on/off taper function
	if (TAP_FLAG) {
	  Tap = calc_Tap_opt(r, cutinv);
	  dTap = calc_dTap_opt(r, cutinv);
	} else {
	  Tap = 1.0;
	  dTap = 0.0;
	}

	// Calculate the transverse distance
	prodnorm1 = norm[0] * delx + norm[1] * dely + norm[2] * delz;
	rhosq1 = rsq - prodnorm1 * prodnorm1;    // rho_ij
	rdsq1 = rhosq1 * p.delta2inv;            // (rho_ij/delta)^2

	// store exponents
	exp0 = exp(-p.lambda * (r - p.z0));
	exp1 = exp(-rdsq1);

	frho1 = exp1 * p.C;
	Erep = 0.5 * p.epsilon + frho1;

	Vilp = exp0 * Erep;

	// derivatives
	fpair = p.lambda * exp0 * rinv * Erep;
	fpair1 = 2.0 * exp0 * frho1 * p.delta2inv;
	fsum = fpair + fpair1;
	// derivatives of the product of rij and ni, the result is a vector

	fp1[0] = prodnorm1 * norm[0] * fpair1;
	fp1[1] = prodnorm1 * norm[1] * fpair1;
	fp1[2] = prodnorm1 * norm[2] * fpair1;

	fkcx = (delx * fsum - fp1[0]) * Tap - Vilp * dTap * delx * rinv;
	fkcy = (dely * fsum - fp1[1]) * Tap - Vilp * dTap * dely * rinv;
	fkcz = (delz * fsum - fp1[2]) * Tap - Vilp * dTap * delz * rinv;

	fxitmp += fkcx;
	fyitmp += fkcy;
	fzitmp += fkcz;
	  
	double fxjtmp = -fkcx;
	double fyjtmp = -fkcy;
	double fzjtmp = -fkcz;

	cij = -prodnorm1 * fpair1 * Tap;
	dproddni[0] += cij * delx;
	dproddni[1] += cij * dely;
	dproddni[2] += cij * delz;
	evdwl = Tap * Vilp;
	if (EFLAG) tally.ext(1) += Tap * Vilp;
	/* ----------------------------------------------------------------------
	   van der Waals forces and energy
	   ------------------------------------------------------------------------- */
	if (jj < jnum_vdw) {
	  double r6inv = r2inv * r2inv * r2inv;
	  double r8inv = r6inv * r2inv;

	  double TSvdw = 1.0 + exp(-p.d * (r * p.seffinv - 1.0));
	  double TSvdwsq = TSvdw * TSvdw;
	  double TSvdwinv = invsqrt(TSvdwsq);
	  double TSvdw2inv = TSvdwinv * TSvdwinv;//pow(TSvdw, -2.0);
	  Vilp = -p.C6 * r6inv * TSvdwinv;


	  fpair = -6.0 * p.C6 * r8inv * TSvdwinv +
	    p.C6 * p.d * p.seffinv * (TSvdw - 1.0) * TSvdw2inv * r8inv * r;
	  fsum = fpair * Tap - Vilp * dTap * rinv;

	  double fvdwx = fsum * delx;
	  double fvdwy = fsum * dely;
	  double fvdwz = fsum * delz;

	  fxitmp += fvdwx;
	  fyitmp += fvdwy;
	  fzitmp += fvdwz;
	  fxjtmp -= fvdwx;
	  fyjtmp -= fvdwy;
	  fzjtmp -= fvdwz;

	  if (EFLAG) tally.ext(0) += Tap * Vilp;
	  evdwl += Tap * Vilp;
	}
	if (EFLAG && VFLAG_EITHER){
	  tally.ev_xyz(evdwl, 0.0, fxjtmp, fyjtmp, fzjtmp, delx, dely, delz);
	} else if (EFLAG){
	  tally.e(evdwl, 0.0);
	}
	fcache.update(j, fxjtmp, fyjtmp, fzjtmp);
      }
    }    // loop over jj
    lwpf_stop(JLOOP_INTER);
    lwpf_start(DNORMAL);
    for (kk = 0; kk < ILP_nneigh; kk++) {
      k = ILP_neigh[kk];
      if (k == i) continue;
      fk[0] = dnormdxk[0][0][kk] * dproddni[0] + dnormdxk[1][0][kk] * dproddni[1] + dnormdxk[2][0][kk] * dproddni[2];
      fk[1] = dnormdxk[0][1][kk] * dproddni[0] + dnormdxk[1][1][kk] * dproddni[1] + dnormdxk[2][1][kk] * dproddni[2];
      fk[2] = dnormdxk[0][2][kk] * dproddni[0] + dnormdxk[1][2][kk] * dproddni[1] + dnormdxk[2][2][kk] * dproddni[2];

      fcache.update(k, fk[0], fk[1], fk[2]);
      if (VFLAG_EITHER){
	// delki[0] = x[k][0] - xtmp;
	// delki[1] = x[k][1] - ytmp;
	// delki[2] = x[k][2] - ztmp;
	tally.ev_xyz(0, 0, fk[0], fk[1], fk[2], dk[kk][0], dk[kk][1], dk[kk][2]);
      }
    }
    lwpf_stop(DNORMAL);
    fxitmp += dnormdxi[0][0] * dproddni[0] + dnormdxi[1][0] * dproddni[1] + dnormdxi[2][0] * dproddni[2];
    fyitmp += dnormdxi[0][1] * dproddni[0] + dnormdxi[1][1] * dproddni[1] + dnormdxi[2][1] * dproddni[2];
    fzitmp += dnormdxi[0][2] * dproddni[0] + dnormdxi[1][2] * dproddni[1] + dnormdxi[2][2] * dproddni[2];
    fcache.update(i, fxitmp, fyitmp, fzitmp);
  }
  lwpf_stop(ILOOP);
  // loop over ii  
  fcache.flush_all();
  tally.finish(eng_vdwl, eng_coul, virial, pvector);
  lwpf_stop(ALL);
  lwpf_exit(SLAVE_ILP);
  // exit(0);

}
#undef exp
template struct make_parallel_eval<PairILPGrapheneHBNSunway, 3>;
template void parallel_eval<1,1,1>(PairILPGrapheneHBNSunway*);
template void parallel_eval<0,1,1>(PairILPGrapheneHBNSunway*);
template void parallel_eval<1,0,1>(PairILPGrapheneHBNSunway*);
template void parallel_eval<0,0,1>(PairILPGrapheneHBNSunway*);
template void parallel_eval<1,1,0>(PairILPGrapheneHBNSunway*);
template void parallel_eval<0,1,0>(PairILPGrapheneHBNSunway*);
template void parallel_eval<1,0,0>(PairILPGrapheneHBNSunway*);
template void parallel_eval<0,0,0>(PairILPGrapheneHBNSunway*);
#endif

#ifdef __sw_host__

#define MAXLINE 1024
#define DELTA 4
#define PGDELTA 1

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

PairILPGrapheneHBNSunway::PairILPGrapheneHBNSunway(LAMMPS *lmp) : Pair(lmp)
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
  first_layered_neigh = nullptr;
  num_intra = nullptr;
  num_inter = nullptr;
  num_vdw = nullptr;
  neigh_index = nullptr;
  force_locks = nullptr;
  atompack = nullptr;
  inum_max = 0;
  jnum_max = 0;
  nmax = 0;
  maxlocal = 0;
  
  // always compute energy offset
  offset_flag = 1;

  // turn on the taper function by default
  tap_flag = 1;
}

/* ---------------------------------------------------------------------- */

PairILPGrapheneHBNSunway::~PairILPGrapheneHBNSunway()
{
  delete[] pvector;

  if (allocated) {
    memory->destroy(setflag);
    memory->destroy(cutsq);
    memory->destroy(offset);
  }

  memory->destroy(layered_neigh);
  memory->sfree(first_layered_neigh);
  memory->destroy(num_intra);
  memory->destroy(num_inter);
  memory->destroy(num_vdw);
  memory->destroy(atompack);
  memory->destroy(force_locks);
  memory->destroy(neigh_index);
  memory->destroy(elem2param);
  memory->destroy(cutILPsq);
  memory->destroy(cutinv);
  memory->destroy(type_params);
  memory->destroy(type_cutILPsq);
  memory->sfree(params);
}

/* ----------------------------------------------------------------------
   allocate all arrays
------------------------------------------------------------------------- */

void PairILPGrapheneHBNSunway::allocate()
{
  allocated = 1;
  int n = atom->ntypes + 1;

  memory->create(setflag, n, n, "pair:setflag");
  for (int i = 1; i < n; i++)
    for (int j = i; j < n; j++) setflag[i][j] = 0;

  memory->create(cutsq, n, n, "pair:cutsq");
  memory->create(offset, n, n, "pair:offset");
  memory->create(type_cutILPsq, n, n, "PairILPGrapheneHBNSunway:cutILPsq");
  memory->create(cutinv, n, n, "PairILPGrapheneHBNSunway:cutnv");
  memory->create(type_params, n, n, "PairILPGrapheneHBNSunway:type_params");
  map = new int[n];
}

/* ----------------------------------------------------------------------
   global settings
------------------------------------------------------------------------- */

void PairILPGrapheneHBNSunway::settings(int narg, char **arg)
{
  if (narg < 1 || narg > 2) error->all(FLERR, "Illegal pair_style command");
  if (strcmp(force->pair_style, "hybrid/overlay") != 0)
    error->all(FLERR, "ERROR: requires hybrid/overlay pair_style");

  cut_global = utils::numeric(FLERR, arg[0], false, lmp);
  if (narg == 2) tap_flag = utils::numeric(FLERR, arg[1], false, lmp);
}

/* ----------------------------------------------------------------------
   set coeffs for one or more type pairs
------------------------------------------------------------------------- */

void PairILPGrapheneHBNSunway::coeff(int narg, char **arg)
{
  if (!allocated) allocate();

  map_element2type(narg - 3, arg + 3);
  read_file(arg[2]);
}

/* ----------------------------------------------------------------------
   init for one type pair i,j and corresponding j,i
------------------------------------------------------------------------- */

double PairILPGrapheneHBNSunway::init_one(int i, int j)
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

void PairILPGrapheneHBNSunway::read_file(char *filename)
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

    MPI_Bcast(&nparams, 1, MPI_INT, 0, world);
    MPI_Bcast(&maxparam, 1, MPI_INT, 0, world);

    if (comm->me != 0) {
      params = (Param *) memory->srealloc(params, maxparam * sizeof(Param), "pair:params");
    }

    MPI_Bcast(params, maxparam * sizeof(Param), MPI_BYTE, 0, world);
  }
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

void PairILPGrapheneHBNSunway::init_style()
{
  if (force->newton_pair == 0)
    error->all(FLERR, "Pair style ilp/graphene/hbn requires newton pair on");
  if (!atom->molecule_flag)
    error->all(FLERR, "Pair style ilp/graphene/hbn requires atom attribute molecule");

  // need a full neighbor list, including neighbors of ghosts

  int irequest = neighbor->request(this, instance_me);
  neighbor->requests[irequest]->half = 0;
  neighbor->requests[irequest]->full = 1;
  neighbor->requests[irequest]->ghost = 1;
}

/* ---------------------------------------------------------------------- */
void PairILPGrapheneHBNSunway::compute(int eflag, int vflag)
{
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
  lwpf_init(NULL);
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
  lwpf_report_summary(stdout);
  if (vflag_fdotr) virial_fdotr_compute();
  
}
template <int EFLAG, int VFLAG_EITHER, int TAP_FLAG>
void PairILPGrapheneHBNSunway::eval(){
  if (!getenv("USEMPE")){
    auto pack_params = std::make_tuple(atompack, (double(*)[3])atom->x[0], atom->type, atom->nlocal + atom->nghost);
    athread_resolve_spawn(slave_pack_atoms, &pack_params);
    athread_join();
    parallel_eval_pair<EFLAG, VFLAG_EITHER, TAP_FLAG>(this);
    return;
  }
  constexpr int EVFLAG = EFLAG || VFLAG_EITHER;
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

  for (ii = 0; ii < inum; ii++) {
    i = ilist[ii];
    xtmp = x[i][0];
    ytmp = x[i][1];
    ztmp = x[i][2];
    itype = type[i];
    itype_map = map[type[i]];
    itag = tag[i];
    jlist = firstneigh[i];
    jnum = numneigh[i];
    int *jlist_intra = first_layered_neigh[i];
    int *jlist_inter = first_layered_neigh[i] + num_intra[i];
    int jnum_intra = num_intra[i];
    int jnum_inter = num_inter[i];
    int jnum_vdw = num_vdw[i];
    int ILP_neigh[3];
    int ILP_nneigh = 0;
    for (jj = 0; jj < jnum_intra; jj++) {
      j = jlist_intra[jj];

      jtype = map[type[j]];
      delx = xtmp - x[j][0];
      dely = ytmp - x[j][1];
      delz = ztmp - x[j][2];
      rsq = delx * delx + dely * dely + delz * delz;

      if (rsq < cutILPsq[itype_map][jtype]) {
	if (ILP_nneigh >= 3) error->one(FLERR, "There are too many neighbors for calculating normals");
	ILP_neigh[ILP_nneigh ++] = j;
      }
    }    // loop over jj

    dproddni[0] = 0.0;
    dproddni[1] = 0.0;
    dproddni[2] = 0.0;

    double norm[3], dnormdxi[3][3], dnormdxk[3][3][3];
    calc_single_normal(i, ILP_neigh, ILP_nneigh, norm, dnormdxi, dnormdxk);

    for (jj = 0; jj < jnum_inter; jj++) {
      j = jlist_inter[jj];
      jtype = type[j];
      jtag = tag[j];

      delx = xtmp - x[j][0];
      dely = ytmp - x[j][1];
      delz = ztmp - x[j][2];
      rsq = delx * delx + dely * dely + delz * delz;
      // if (i == 0) {
      // 	printf("%d %g %g %g\n", j, delx, dely, delz);
      // }

      // only include the interaction between different layers
      if (rsq < cutsq[itype][jtype]) {

        int iparam_ij = elem2param[map[itype]][map[jtype]];
        Param &p = params[iparam_ij];
	// if (i == 0) {
	//   printf("%d %d %f\n", j, iparam_ij, p.seff);
	// }
        r = sqrt(rsq);
	double r2inv = 1.0 / rsq;
	double rinv = r * r2inv;
        // turn on/off taper function
        if (TAP_FLAG) {
          Rcut = sqrt(cutsq[itype][jtype]);
          Tap = calc_Tap(r, Rcut);
          dTap = calc_dTap(r, Rcut);
        } else {
          Tap = 1.0;
          dTap = 0.0;
        }
	// if (i == 0) {
	//   printf("%d %g %g\n", j, Tap, rinv);
	// }

        // Calculate the transverse distance
        prodnorm1 = norm[0] * delx + norm[1] * dely + norm[2] * delz;
        rhosq1 = rsq - prodnorm1 * prodnorm1;    // rho_ij
        rdsq1 = rhosq1 * p.delta2inv;            // (rho_ij/delta)^2

        // store exponents
        exp0 = exp(-p.lambda * (r - p.z0));
        exp1 = exp(-rdsq1);

        frho1 = exp1 * p.C;
	Erep = 0.5 * p.epsilon + frho1;

        Vilp = exp0 * Erep;

        // derivatives
        fpair = p.lambda * exp0 * rinv * Erep;
        fpair1 = 2.0 * exp0 * frho1 * p.delta2inv;
        fsum = fpair + fpair1;
        // derivatives of the product of rij and ni, the result is a vector

        fp1[0] = prodnorm1 * norm[0] * fpair1;
        fp1[1] = prodnorm1 * norm[1] * fpair1;
        fp1[2] = prodnorm1 * norm[2] * fpair1;

        fkcx = (delx * fsum - fp1[0]) * Tap - Vilp * dTap * delx * rinv;
        fkcy = (dely * fsum - fp1[1]) * Tap - Vilp * dTap * dely * rinv;
        fkcz = (delz * fsum - fp1[2]) * Tap - Vilp * dTap * delz * rinv;

        f[i][0] += fkcx;
        f[i][1] += fkcy;
        f[i][2] += fkcz;
        f[j][0] -= fkcx;
        f[j][1] -= fkcy;
        f[j][2] -= fkcz;
	// if (i == 0){
	//   printf("%d %g %g %g\n", j, fkcx, fkcy, fkcz);
	// }
	cij = -prodnorm1 * fpair1 * Tap;
	dproddni[0] += cij * delx;
	dproddni[1] += cij * dely;
	dproddni[2] += cij * delz;

	if (EFLAG) pvector[1] += evdwl = Tap * Vilp;
        if (EVFLAG)
          ev_tally_xyz(i, j, nlocal, newton_pair, evdwl, 0.0, fkcx, fkcy, fkcz, delx, dely, delz);

	/* ----------------------------------------------------------------------
	      van der Waals forces and energy
	      ------------------------------------------------------------------------- */
	if (jj >= jnum_vdw) continue;
	double r6inv = r2inv * r2inv * r2inv;
	double r8inv = r6inv * r2inv;

	double TSvdw = 1.0 + exp(-p.d * (r * p.seffinv - 1.0));
	double TSvdwinv = 1.0 / TSvdw;
        double TSvdw2inv = TSvdwinv * TSvdwinv;//pow(TSvdw, -2.0);
        Vilp = -p.C6 * r6inv * TSvdwinv;

	fpair = -6.0 * p.C6 * r8inv * TSvdwinv +
	  p.C6 * p.d * p.seffinv * (TSvdw - 1.0) * TSvdw2inv * r8inv * r;
        fsum = fpair * Tap - Vilp * dTap * rinv;

	double fvdwx = fsum * delx;
	double fvdwy = fsum * dely;
	double fvdwz = fsum * delz;

	f[i][0] += fvdwx;
        f[i][1] += fvdwy;
        f[i][2] += fvdwz;
        f[j][0] -= fvdwx;
        f[j][1] -= fvdwy;
        f[j][2] -= fvdwz;
	// if (i == 0) {
	//   printf("%d %g %g %g\n", j, fvdwx, fvdwy, fvdwz);
	// }

        if (EFLAG) pvector[0] += evdwl = Tap * Vilp;
        if (EVFLAG)
          ev_tally_xyz(i, j, nlocal, newton_pair, evdwl, 0.0, fvdwx, fvdwy, fvdwz, delx, dely, delz);
      }
    }    // loop over jj

    //ILP_neighs_i = ILP_firstneigh[i];
    for (kk = 0; kk < ILP_nneigh; kk++) {
      k = ILP_neigh[kk];
      if (k == i) continue;
      // derivatives of the product of rij and ni respect to rk, k=0,1,2, where atom k is the neighbors of atom i
      fk[0] = dnormdxk[0][0][kk] * dproddni[0] + dnormdxk[1][0][kk] * dproddni[1] + dnormdxk[2][0][kk] * dproddni[2];
      fk[1] = dnormdxk[0][1][kk] * dproddni[0] + dnormdxk[1][1][kk] * dproddni[1] + dnormdxk[2][1][kk] * dproddni[2];
      fk[2] = dnormdxk[0][2][kk] * dproddni[0] + dnormdxk[1][2][kk] * dproddni[1] + dnormdxk[2][2][kk] * dproddni[2];

      f[k][0] += fk[0];
      f[k][1] += fk[1];
      f[k][2] += fk[2];
      
      delki[0] = x[k][0] - x[i][0];
      delki[1] = x[k][1] - x[i][1];
      delki[2] = x[k][2] - x[i][2];

      if (VFLAG_EITHER){
        ev_tally_xyz(k, i, nlocal, newton_pair, 0.0, 0.0, fk[0], fk[1], fk[2], delki[0],
                     delki[1], delki[2]);
	//printf("%f %f %f\n", fk[0]/delkj[0], fk[1] /delkj[1], fk[2] / delkj[2]);
      }
    }
    f[i][0] += dnormdxi[0][0] * dproddni[0] + dnormdxi[1][0] * dproddni[1] + dnormdxi[2][0] * dproddni[2];
    f[i][1] += dnormdxi[0][1] * dproddni[0] + dnormdxi[1][1] * dproddni[1] + dnormdxi[2][1] * dproddni[2];
    f[i][2] += dnormdxi[0][2] * dproddni[0] + dnormdxi[1][2] * dproddni[1] + dnormdxi[2][2] * dproddni[2];
  }      // loop over ii  
  // exit(0);
}
extern "C" {
extern void *libc_uncached_malloc (size_t __size);
extern void libc_uncached_free (void *__ptr);
}
void PairILPGrapheneHBNSunway::update_internal_list() {
  int jnum_sum = 0;
  int inum = atom->nlocal;
  int nall = atom->nlocal + atom->nghost;

  if (nall > nmax) {
    if (force_locks) libc_uncached_free(force_locks);
    memory->destroy(atompack);
    nmax = (int)ceil(nall / 0.618);
    memory->create(atompack, nmax, "PairILPGrapheneHBN:atompack");
    force_locks = (cal_lock_t*)libc_uncached_malloc(nmax * sizeof(cal_lock_t));
    auto fill_param = std::make_tuple(force_locks, cal_lock_t{0, 0}, (size_t)nmax);
    athread_resolve_spawn(slave_fill_mem, &fill_param);
    athread_join();
//memset(force_locks, 0, nmax * sizeof(cal_lock_t));
  }
  int *ilist = list->ilist;
  int *numneigh = list->numneigh;
  int **firstneigh = list->firstneigh;
  int *jlist, jnum;
  tagint *tag = atom->tag;
  double **x = atom->x;
  for (int ii = 0; ii < inum; ii ++){
    jnum_sum += numneigh[ilist[ii]];
  }
  if (inum > inum_max) {
    memory->destroy(num_intra);
    memory->destroy(num_inter);
    memory->destroy(num_vdw);
    memory->sfree(first_layered_neigh);
    memory->destroy(neigh_index);
    //golden ratio grow
    inum_max = (int)ceil(inum / 0.618);
    memory->create(num_intra, inum_max, "PairILPGrapheneHBNSunway:intra_layer_count");
    memory->create(num_inter, inum_max, "PairILPGrapheneHBNSunway:inter_layer_count");
    memory->create(num_vdw, inum_max, "PairILPGrapheneHBNSunway:vdw_count");
    memory->create(neigh_index, inum_max, "PairILPGrapheneHBNSunway:neigh_index");
    first_layered_neigh = (int**)memory->smalloc(inum_max * sizeof(int*), "PairILPGrapheneHBNSunway:first_layered_neigh");
  }
  if (jnum_sum > jnum_max) {
    memory->destroy(layered_neigh);
    jnum_max = (int)ceil(jnum_sum / 0.618);
    memory->create(layered_neigh, jnum_max, "PairILPGrapheneHBNSunway:layered_neigh");
  }

  double cut_intra = 0;
  for (int i = 0; i < nparams; i ++){
    if (params[i].rcut > cut_intra) {
      cut_intra = params[i].rcut;
    }
  }
  double cut_intra_listsq = (cut_intra + neighbor->skin) * (cut_intra + neighbor->skin);

  int total_neigh = 0;
  for (int ii = 0; ii < inum; ii ++){
    int i = ilist[ii];
    tagint itag = tag[i];
    int jnum = numneigh[i];
    int *jlist = firstneigh[i];
    neigh_index[i].first = total_neigh;
    int *jlist_layered = first_layered_neigh[i] = layered_neigh + total_neigh;
    int ninter = 0, nintra = 0;

    for (int jj = 0; jj < jnum; jj ++){
      int j = jlist[jj] & NEIGHMASK;
      if (atom->molecule[j] == atom->molecule[i]) {
	double delx = x[i][0] - x[j][0];
	double dely = x[i][1] - x[j][1];
	double delz = x[i][2] - x[j][2];
	double rsq = delx*delx + dely*dely + delz*delz;
	if (rsq < cut_intra_listsq)
	  jlist_layered[nintra++] = j;
      }
    }
    for (int jj = 0; jj < jnum; jj ++){
      int j = jlist[jj] & NEIGHMASK;
      tagint jtag = tag[j];
      if (atom->molecule[j] != atom->molecule[i]) {
	if (check_vdw(itag, jtag, x[i], x[j]))
	  jlist_layered[nintra + ninter++] = j;
      }
    }
    neigh_index[i].nintra = nintra;
    neigh_index[i].nvdw = ninter;
    num_vdw[i] = ninter;
    for (int jj = 0; jj < jnum; jj ++){
      int j = jlist[jj] & NEIGHMASK;
      tagint jtag = tag[j];
      if (atom->molecule[j] != atom->molecule[i]) {
	if (!check_vdw(itag, jtag, x[i], x[j]))
	  jlist_layered[nintra + ninter++] = j;
      }
    }
    neigh_index[i].ninter = ninter;
    num_intra[i] = nintra;
    num_inter[i] = ninter;
    
    total_neigh += nintra + ninter;
  }
}
/* ---------------------------------------------------------------------- */

double PairILPGrapheneHBNSunway::single(int /*i*/, int /*j*/, int itype, int jtype, double rsq,
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
