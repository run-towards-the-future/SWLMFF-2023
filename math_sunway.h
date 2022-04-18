#pragma once
#ifdef __sw_slave__
#include <simd.h>
#include <math.h>
#include <slave.h>
extern __ldm doublev8 vexp_tab[];
enum exp_entries{
  vexp_log2,
  vexp_log2inv,
  vexp_c0,
  vexp_c1,
  vexp_c2,
  vexp_c3,
  vexp_c4,
  vexp_c5,
  vexp_c6,
  vexp_c7,
  vexp_c8,
  vexp_c9,
  vexp_1_0,
  vexp_over,
  vexp_under,
  vexp_inf
};

__always_inline doublev8 vexp_finite(doublev8 x) {
  doublev8 x_log2 = x * vexp_tab[vexp_log2inv];
  int512 pint = (int512)(x_log2);
  doublev8 p = (doublev8)pint;
  doublev8 q = x - p * vexp_tab[vexp_log2];
  doublev8 q2 = q * q;
  doublev8 expq_even = vexp_tab[vexp_c0] * q2 + vexp_tab[vexp_c2];
  expq_even = expq_even * q2 + vexp_tab[vexp_c4];
  expq_even = expq_even * q2 + vexp_tab[vexp_c6];
  expq_even = expq_even * q2 + vexp_tab[vexp_c8];
  doublev8 expq_odd = vexp_tab[vexp_c1] * q2 + vexp_tab[vexp_c3];
  expq_odd = expq_odd * q2 + vexp_tab[vexp_c5];
  expq_odd = expq_odd * q2 + vexp_tab[vexp_c7];
  expq_odd = expq_odd * q2 + vexp_tab[vexp_c9];
  doublev8 expq = expq_even * q + expq_odd;
  int512 exppln2_raw = simd_sllx(pint, 52) + (*(int512*)&vexp_tab[vexp_1_0]);
  doublev8 exppln2;
  asm("vcpyse %1, $31, %0\n\t" : "=r"(exppln2) : "0"(exppln2_raw));
  return exppln2 * expq;
}

__always_inline double pexp_finite(double x) {
  double (*pexp_tab)[8] = (double(*)[8])vexp_tab;
  double x_log2 = x * pexp_tab[vexp_log2inv][0];
  long pint = (long)(x_log2);
  double p = (double)pint;
  double q = x - p * pexp_tab[vexp_log2][0];
  double q2 = q * q;
  double expq_even = pexp_tab[vexp_c0][0] * q2 + pexp_tab[vexp_c2][0];
  expq_even = expq_even * q2 + pexp_tab[vexp_c4][0];
  expq_even = expq_even * q2 + pexp_tab[vexp_c6][0];
  expq_even = expq_even * q2 + pexp_tab[vexp_c8][0];
  double expq_odd = pexp_tab[vexp_c1][0] * q2 + pexp_tab[vexp_c3][0];
  expq_odd = expq_odd * q2 + pexp_tab[vexp_c5][0];
  expq_odd = expq_odd * q2 + pexp_tab[vexp_c7][0];
  expq_odd = expq_odd * q2 + pexp_tab[vexp_c9][0];
  double expq = expq_even * q + expq_odd;
  long exppln2_raw = (pint << 52) + (*(long*)&pexp_tab[vexp_1_0][0]);
  double exppln2 = *(double*)&exppln2_raw;
  //asm("vcpyse %1, $31, %0\n\t" : "=r"(exppln2) : "0"(exppln2_raw));
  return exppln2 * expq;
}
template<int NITER=1>
__always_inline double invsqrt(double xsq){
  double xinv;
  asm("fsqrtrecd %1, %0": "=f"(xinv) : "f"(xsq));
  for (int i = 0; i < NITER; i ++){
    xinv = xinv * (1.5 - 0.5*xsq*xinv*xinv);
  }
  return xinv;
}
#endif
