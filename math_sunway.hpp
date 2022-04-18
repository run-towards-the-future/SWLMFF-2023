#ifdef __sw_slave__
#include <simd.h>
#include <slave.h>

typedef union dunion {
  //long larr[8];
  doublev8 v;
  double f;
  int512 iv;
  long i;
} dunion;
struct vmath_world_t{
  dunion log2, log2inv;
  dunion f1_0, f0_5;
  dunion f2_0, f6_0;
  dunion l1023;
  dunion inf, nan;
  dunion sqrt2, invsqrt2;
  dunion f2pi, f2piinv;
  dunion exp_over, exp_under;
  dunion c_exp[10];
  dunion c_log[12];
  dunion c_sin[9], c_cos[9];
};

extern __ldm vmath_world_t vmworld;

__always_inline doublev8 vexp_valid(doublev8 x) {
  doublev8 x_log2 = x * vmworld.log2inv.v;
  //The conversion is to nearest int, so approximation should be in -0.5ln2 to ln2
  int512 pint = (int512)(x_log2);
  doublev8 p = (doublev8)pint;
  doublev8 q = x - p * vmworld.log2.v;
  doublev8 q2 = q * q;
  doublev8 expq_even = vmworld.c_exp[0].v * q2 + vmworld.c_exp[2].v;
  expq_even = expq_even * q2 + vmworld.c_exp[4].v;
  expq_even = expq_even * q2 + vmworld.c_exp[6].v;
  expq_even = expq_even * q2 + vmworld.c_exp[8].v;
  doublev8 expq_odd = vmworld.c_exp[1].v * q2 + vmworld.c_exp[3].v;
  expq_odd = expq_odd * q2 + vmworld.c_exp[5].v;
  expq_odd = expq_odd * q2 + vmworld.c_exp[7].v;
  expq_odd = expq_odd * q2 + vmworld.c_exp[9].v;
  doublev8 expq = expq_even * q + expq_odd;
  int512 exppln2_raw = simd_sllx(pint, 52) + vmworld.f1_0.iv;
  //asm volatile("vcpyse %1, $31, %0\n\t" : "=r"(exppln2_raw) : "0"(exppln2_raw));
  //exppln2_raw += ;
  doublev8 exppln2;
  asm("vcpyse %1, $31, %0\n\t" : "=r"(exppln2) : "0"(exppln2_raw));
  return exppln2 * expq;
}

__always_inline double fexp_valid(double x) {
  double x_log2 = x * vmworld.log2inv.f;
  //The conversion is to nearest int, so approximation should be in -0.5ln2 to ln2
  long pint = (long)(x_log2);
  double p = (double)pint;
  double q = x - p * vmworld.log2.f;
  double q2 = q * q;
  double expq_even = vmworld.c_exp[0].f * q2 + vmworld.c_exp[2].f;
  expq_even = expq_even * q2 + vmworld.c_exp[4].f;
  expq_even = expq_even * q2 + vmworld.c_exp[6].f;
  expq_even = expq_even * q2 + vmworld.c_exp[8].f;
  double expq_odd = vmworld.c_exp[1].f * q2 + vmworld.c_exp[3].f;
  expq_odd = expq_odd * q2 + vmworld.c_exp[5].f;
  expq_odd = expq_odd * q2 + vmworld.c_exp[7].f;
  expq_odd = expq_odd * q2 + vmworld.c_exp[9].f;
  double expq = expq_even * q + expq_odd;
  long exppln2_raw = (pint << 52) + vmworld.f1_0.i;  //simd_sllx(pint, 52) + (*(long*)&vmworld.f1_0.v);
  double exppln2 = *(double*)&exppln2_raw;
  return exppln2 * expq;
}
__always_inline doublev8 vexp(doublev8 x) {
  doublev8 underflow = simd_vfcmpltd(x, vmworld.exp_under.v);
  doublev8 overflow = simd_vfcmpltd(vmworld.exp_over.v, x);
  doublev8 expraw = vexp_valid(x);
  doublev8 expunder = simd_vfseleqd(underflow, expraw, (doublev8)0);
  doublev8 expover = simd_vfseleqd(overflow, expunder, vmworld.inf.v);
  return expover;
}
__always_inline double fexp(double x) {
  if (x < vmworld.exp_under.f) return 0;
  if (x > vmworld.exp_over.f) return INFINITY;
  return fexp_valid(x);
}
__always_inline doublev8 vlog_valid(doublev8 x) {
  doublev8 q = simd_vcpysed(vmworld.f1_0.v, x);
  doublev8 p52 = simd_vcpysed(x, (doublev8)0);
  int512 p52int = *(int512*)&p52;
  int512 pint = simd_vsubl(simd_srlx(p52int, 52), vmworld.l1023.iv);
  doublev8 p = (doublev8)pint;

  doublev8 qlt = simd_vfcmpltd(q, vmworld.sqrt2.v);
  doublev8 scale = simd_vfseleqd(qlt, vmworld.invsqrt2.v, vmworld.f1_0.v);
  doublev8 add = simd_vfseleqd(qlt, vmworld.f0_5.v, (doublev8)0);
  p = p + add;
  doublev8 log2p = p * vmworld.log2.v;
  doublev8 s = q*scale - vmworld.f1_0.v;
  doublev8 s2 = s * s;
  doublev8 logq_even = vmworld.c_log[0].v * s2 + vmworld.c_log[2].v;
  logq_even = logq_even * s2 + vmworld.c_log[4].v;
  logq_even = logq_even * s2 + vmworld.c_log[6].v;
  logq_even = logq_even * s2 + vmworld.c_log[8].v;
  logq_even = logq_even * s2 + vmworld.c_log[10].v;

  doublev8 logq_odd = vmworld.c_log[1].v * s2 + vmworld.c_log[3].v;
  logq_odd = logq_odd * s2 + vmworld.c_log[5].v;
  logq_odd = logq_odd * s2 + vmworld.c_log[7].v;
  logq_odd = logq_odd * s2 + vmworld.c_log[9].v;
  logq_odd = logq_odd * s2 + vmworld.c_log[11].v;
  doublev8 logq = logq_odd * s + logq_even * s2;
  //simd_print_doublev8(logq + log2p);
  return log2p + logq;
}

__always_inline double flog_valid(double x) {
  double q;// = __builtin_fcpysed(vmworld.f1_0.f, x);
  double p52;// = __builtin_fcpysed(x, (double)0);
  asm volatile("fcpyse %1, %2, %0\n\t":"=r"(q): "r"(vmworld.f1_0.f), "r"(x));
  asm volatile("fcpyse %1, $31, %0\n\t":"=r"(p52): "r"(x));
  long p52int = *(long*)&p52;
  long pint = (p52int >> 52) - 1023; //simd_vsubl(simd_srlx(p52int, 52), vmworld.l1023.iv);
  double p = (double)pint;

  
  bool qlt = q < vmworld.sqrt2.f;//simd_vfcmpltd(q, vmworld.sqrt2.v);
  double scale = qlt ? vmworld.f1_0.f : vmworld.invsqrt2.f; //simd_vfseleqd(qlt, vmworld.invsqrt2.v, vmworld.f1_0.v);
  double add = qlt ? 0 : vmworld.f0_5.f; //simd_vfseleqd(qlt, vmworld.f0_5.v, (double)0);
  p = p + add;
  double log2p = p * vmworld.log2.f;
  double s = q*scale - vmworld.f1_0.f;
  double s2 = s * s;
  double logq_even = vmworld.c_log[0].f * s2 + vmworld.c_log[2].f;
  logq_even = logq_even * s2 + vmworld.c_log[4].f;
  logq_even = logq_even * s2 + vmworld.c_log[6].f;
  logq_even = logq_even * s2 + vmworld.c_log[8].f;
  logq_even = logq_even * s2 + vmworld.c_log[10].f;

  double logq_odd = vmworld.c_log[1].f * s2 + vmworld.c_log[3].f;
  logq_odd = logq_odd * s2 + vmworld.c_log[5].f;
  logq_odd = logq_odd * s2 + vmworld.c_log[7].f;
  logq_odd = logq_odd * s2 + vmworld.c_log[9].f;
  logq_odd = logq_odd * s2 + vmworld.c_log[11].f;
  double logq = logq_odd * s + logq_even * s2;
  //simd_print_double(logq + log2p);
  return log2p + logq;
}

__always_inline doublev8 vlog(doublev8 x){
  //小于0，从nan计算
  doublev8 xx = simd_vfselltd(x, vmworld.nan.v, x);
  //处理inf
  doublev8 ret = vlog_valid(xx);
  doublev8 isinf = simd_vfcmpeqd(x, vmworld.inf.v);
  ret = simd_vfseleqd(isinf, ret, vmworld.inf.v);
  return simd_vfseleqd(x, -vmworld.inf.v, x);
}
__always_inline double flog(double x){
  if (x < 0) return vmworld.nan.f;
  if (x == 0) return -vmworld.inf.f;
  if (x == INFINITY) return vmworld.inf.f;
  return flog_valid(x);
}

__always_inline doublev8 vsin_reduced(doublev8 x){
  doublev8 x2 = x * x;
  doublev8 x4 = x2 * x2;
  doublev8 sin_even = x4 * vmworld.c_sin[0].v + vmworld.c_sin[2].v;
  sin_even = x4 * sin_even + vmworld.c_sin[4].v;
  sin_even = x4 * sin_even + vmworld.c_sin[6].v;
  sin_even = x4 * sin_even + vmworld.c_sin[8].v;
  doublev8 sin_odd = x4 * vmworld.c_sin[1].v + vmworld.c_sin[3].v;
  sin_odd = x4 * sin_odd + vmworld.c_sin[5].v;
  sin_odd = x4 * sin_odd + vmworld.c_sin[7].v;
  return (sin_odd*x2 + sin_even)*x;
}
__always_inline doublev8 vcos_reduced(doublev8 x){
  doublev8 x2 = x * x;
  doublev8 x4 = x2 * x2;
  doublev8 cos_even = x4 * vmworld.c_cos[0].v + vmworld.c_cos[2].v;
  cos_even = x4 * cos_even + vmworld.c_cos[4].v;
  cos_even = x4 * cos_even + vmworld.c_cos[6].v;
  cos_even = x4 * cos_even + vmworld.c_cos[8].v;
  doublev8 cos_odd = x4 * vmworld.c_cos[1].v + vmworld.c_cos[3].v;
  cos_odd = x4 * cos_odd + vmworld.c_cos[5].v;
  cos_odd = x4 * cos_odd + vmworld.c_cos[7].v;
  return (cos_odd*x2 + cos_even)*x2 + vmworld.f1_0.v;
}
__always_inline doublev8 vsin(doublev8 x){
  doublev8 x_pi2 = x * vmworld.f2piinv.v;
  int512 x_pi2int = (int512)x_pi2;
  doublev8 x_pi2round = (doublev8)x_pi2int;
  doublev8 x_modpi2 = x - x_pi2round * vmworld.f2pi.v;
  return vsin_reduced(x_modpi2);
}
__always_inline doublev8 vcos(doublev8 x){
  doublev8 x_pi2 = x * vmworld.f2piinv.v;
  int512 x_pi2int = (int512)x_pi2;
  //simd_print_int512(x_pi2int);
  doublev8 x_pi2round = (doublev8)x_pi2int;
  doublev8 x_modpi2 = x - x_pi2round * vmworld.f2pi.v;
  return vcos_reduced(x_modpi2);
}

__always_inline double fsin_reduced(double x){
  double x2 = x * x;
  double x4 = x2 * x2;
  double sin_even = x4 * vmworld.c_sin[0].f + vmworld.c_sin[2].f;
  sin_even = x4 * sin_even + vmworld.c_sin[4].f;
  sin_even = x4 * sin_even + vmworld.c_sin[6].f;
  sin_even = x4 * sin_even + vmworld.c_sin[8].f;
  double sin_odd = x4 * vmworld.c_sin[1].f + vmworld.c_sin[3].f;
  sin_odd = x4 * sin_odd + vmworld.c_sin[5].f;
  sin_odd = x4 * sin_odd + vmworld.c_sin[7].f;
  return (sin_odd*x2 + sin_even)*x;
}
__always_inline double fcos_reduced(double x){
  double x2 = x * x;
  double x4 = x2 * x2;
  double cos_even = x4 * vmworld.c_cos[0].f + vmworld.c_cos[2].f;
  cos_even = x4 * cos_even + vmworld.c_cos[4].f;
  cos_even = x4 * cos_even + vmworld.c_cos[6].f;
  cos_even = x4 * cos_even + vmworld.c_cos[8].f;
  double cos_odd = x4 * vmworld.c_cos[1].f + vmworld.c_cos[3].f;
  cos_odd = x4 * cos_odd + vmworld.c_cos[5].f;
  cos_odd = x4 * cos_odd + vmworld.c_cos[7].f;
  return (cos_odd*x2 + cos_even)*x2 + vmworld.f1_0.f;
}
__always_inline double fsin(double x){
  double x_pi2 = x * vmworld.f2piinv.f;
  long x_pi2int = (long)x_pi2;
  double x_pi2round = (double)x_pi2int;
  double x_modpi2 = x - x_pi2round * vmworld.f2pi.f;
  return fsin_reduced(x_modpi2);
}
__always_inline double fcos(double x){
  double x_pi2 = x * vmworld.f2piinv.f;
  long x_pi2int = (long)x_pi2;
  //simd_print_long(x_pi2int);
  double x_pi2round = (double)x_pi2int;
  double x_modpi2 = x - x_pi2round * vmworld.f2pi.f;
  return fcos_reduced(x_modpi2);
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

template<int NITER=1>
__always_inline doublev8 invsqrt(doublev8 xsq){
  doublev8 xinv;
  asm("vfsqrtrecd %1, %0": "=f"(xinv) : "f"(xsq));
  for (int i = 0; i < NITER; i ++){
    xinv = xinv * (1.5 - 0.5*xsq*xinv*xinv);
  }
  return xinv;
}
#endif
