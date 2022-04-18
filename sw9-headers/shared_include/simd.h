#ifndef _SIMD_H_INCLUDED
#define _SIMD_H_INCLUDED

/********************************************************************************/
/*         Declare the HOST AND SLAVE TARGET THE SAME SIMD operations           */
/********************************************************************************/
#include<stdio.h>
#define simd_load(dest,src) 				\
  ({						\
	(dest) = __builtin_sw_load ((src));	\
	(dest);				\
  })

#define simd_load_u(dest,src)				\
  ({						\
	(dest) = __builtin_sw_load_u ((src));	\
	(dest);                                    \
  })

#define simd_loadu(dest,src)				\
  ({						\
	(dest) = __builtin_sw_loadu ((src));	\
	(dest);                                    \
  })

#define simd_loade(dest,src)				\
  ({						\
	(dest) = __builtin_sw_host_loade ((src));	\
	(dest);					\
  })

#define simd_store(src,dest) __builtin_sw_store (dest, src)
#define simd_storeu(src,dest) __builtin_sw_storeu (dest, src)
#define simd_store_u(src,dest) __builtin_sw_store_u (dest, src)


//Added by FANGYF20170327,for SIMD in CCC program
#if	_SIMD_HOST || _SIMD_CCC
/********************************************************************************/
/*         Declare the HOST TARGET SIMD operations                              */
/********************************************************************************/
typedef int intv8 __attribute__ ((__mode__(__V8SI__)));
typedef unsigned uintv8 __attribute__ ((__mode__(__V8SI__)));
typedef int int256 __attribute__ ((__mode__(__V1OI__)));
typedef unsigned uint256 __attribute__ ((__mode__(__V1OI__)));
typedef float floatv4 __attribute__ ((__mode__(__V4SF__)));
typedef double doublev4 __attribute__ ((__mode__(__V4DF__)));
#endif

#ifdef _SIMD_HOST
static __inline void
simd_fprint_intv8 (FILE *fp, intv8 a)
{
  union {
    int __a[8];
    intv8 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %d, %d, %d, %d, %d, %d, %d, %d ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void
simd_fprint_intv8_X (FILE *fp, intv8 a)
{
  union {
    int __a[8];
    intv8 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void
simd_fprint_uintv8 (FILE *fp, uintv8 a)
{
  union {
    unsigned int __a[8];
    uintv8 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %u, %u, %u, %u, %u, %u, %u, %u ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void
simd_fprint_uintv8_X (FILE *fp, uintv8 a)
{
  union {
    unsigned int __a[8];
    uintv8 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

/*test.c:intv8 va; int256 vb; vb=va; simd_print_int256(vb);
 *-O3 will generate: vstd $f10,16($15)
 *so add volatile
 */
static __inline void
simd_fprint_int256 (FILE *fp, int256 a)
{
  volatile union {
    long __a[4];
    int256 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %ld, %ld, %ld, %ld ]\n", __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void
simd_fprint_int256_X (FILE *fp, int256 a)
{
  volatile union {
    long __a[4];
    int256 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%lx, 0x%lx, 0x%lx, 0x%lx ]\n", __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void
simd_fprint_uint256 (FILE *fp, uint256 a)
{
  volatile union {
    unsigned long __a[4];
    uint256 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %lu, %lu, %lu, %lu ]\n", __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void
simd_fprint_uint256_X (FILE *fp, uint256 a)
{
  volatile union {
    unsigned long __a[4];
    uint256 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%lx, 0x%lx, 0x%lx, 0x%lx ]\n", __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void
simd_fprint_doublev4 (FILE *fp, doublev4 a)
{
  union {
    double __a[4];
    doublev4 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %e, %e, %e, %e ]\n", __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void
simd_fprint_doublev4_X (FILE *fp, doublev4 a)
{
  union {
    double __a[4];
    doublev4 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%lx, 0x%lx, 0x%lx, 0x%lx ]\n", *(long *)(&__u.__a[3]), *(long *)(&__u.__a[2]), *(long *)(&__u.__a[1]), *(long *)(&__u.__a[0]));
}

/* test.c:floatv4 va; doublev4 vb; va=vb; simd_print_floatv4(va);
 * -O3 will error in reload:
 *  (insn 58 8 53 2 (set (reg:V4DF 4 $4 [85])
 *          (reg:V4DF 42 $f10)) 4.c:9 391 {*movv4df}
 *               (nil))
 */
static __inline void
simd_fprint_floatv4 (FILE *fp, floatv4 a)
{
  union {
    float __a[4];
    floatv4 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %e, %e, %e, %e ]\n", __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void
simd_fprint_floatv4_X (FILE *fp, floatv4 a)
{
  union {
    float __a[4];
    floatv4 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%x, 0x%x, 0x%x, 0x%x ]\n", *(int *)(&__u.__a[3]), *(int *)(&__u.__a[2]), *(int *)(&__u.__a[1]), *(int *)(&__u.__a[0]));
}

static __inline intv8
simd_set_intv8 (int __S, int __T, int __U, int __V, int __W, int __X, int __Y, int __Z)
{
    union {
    int __a[8] __attribute__ ((aligned (32)));
    intv8 __v;
  } __u;

  __u.__a[0] = __S;
  __u.__a[1] = __T;
  __u.__a[2] = __U;
  __u.__a[3] = __V;
  __u.__a[4] = __W;
  __u.__a[5] = __X;
  __u.__a[6] = __Y;
  __u.__a[7] = __Z;

  return __u.__v;
}

static __inline uintv8
simd_set_uintv8 (unsigned int __S, unsigned int __T, unsigned int __U, unsigned int __V, unsigned int __W, unsigned int __X, unsigned int __Y, unsigned int __Z)
{
    union {
    unsigned int __a[8] __attribute__ ((aligned (32)));
    uintv8 __v;
  } __u;

  __u.__a[0] = __S;
  __u.__a[1] = __T;
  __u.__a[2] = __U;
  __u.__a[3] = __V;
  __u.__a[4] = __W;
  __u.__a[5] = __X;
  __u.__a[6] = __Y;
  __u.__a[7] = __Z;

  return __u.__v;
}

static __inline int256
simd_set_int256 (long __W, long __X, long __Y, long __Z)
{
    union {
    long __a[4];
    int256 __v;
  } __u;

  __u.__a[0] = __W;
  __u.__a[1] = __X;
  __u.__a[2] = __Y;
  __u.__a[3] = __Z;

  return __u.__v;
}

static __inline uint256
simd_set_uint256 (unsigned long __W, unsigned long __X, unsigned long __Y, unsigned long __Z)
{
    union {
    unsigned long __a[4];
    uint256 __v;
  } __u;

  __u.__a[0] = __W;
  __u.__a[1] = __X;
  __u.__a[2] = __Y;
  __u.__a[3] = __Z;

  return __u.__v;
}

static __inline floatv4
simd_set_floatv4 (float __W, float __X, float __Y, float __Z)
{
    union {
    float __a[4] __attribute__ ((aligned (32)));
    floatv4 __v;
  } __u;

  __u.__a[0] = __W;
  __u.__a[1] = __X;
  __u.__a[2] = __Y;
  __u.__a[3] = __Z;

  return __u.__v;
}

static __inline doublev4
simd_set_doublev4 (double __W, double __X, double __Y, double __Z)
{
    union {
    double __a[4] __attribute__ ((aligned (32)));
    doublev4 __v;
  } __u;

  __u.__a[0] = __W;
  __u.__a[1] = __X;
  __u.__a[2] = __Y;
  __u.__a[3] = __Z;

  return __u.__v;
}

static __inline void simd_print_intv8 (intv8 a) { simd_fprint_intv8 (stdout, a); }
static __inline void simd_print_intv8_X (intv8 a) { simd_fprint_intv8_X (stdout, a); }
static __inline void simd_print_uintv8 (uintv8 a) { simd_fprint_uintv8 (stdout, a); }
static __inline void simd_print_uintv8_X (uintv8 a) { simd_fprint_uintv8_X (stdout, a); }
static __inline void simd_print_int256 (int256 a) { simd_fprint_int256 (stdout, a); }
static __inline void simd_print_int256_X (int256 a) { simd_fprint_int256_X (stdout, a); }
static __inline void simd_print_uint256 (uint256 a) { simd_fprint_uint256 (stdout, a); }
static __inline void simd_print_uint256_X (uint256 a) { simd_fprint_uint256_X (stdout, a); }
static __inline void simd_print_floatv4 (floatv4 a) { simd_fprint_floatv4 (stdout, a); }
static __inline void simd_print_floatv4_X (floatv4 a) { simd_fprint_floatv4_X (stdout, a); }
static __inline void simd_print_doublev4 (doublev4 a) { simd_fprint_doublev4 (stdout, a); }
static __inline void simd_print_doublev4_X (doublev4 a) { simd_fprint_doublev4_X (stdout, a); }


#define simd_loade(dest,src)				\
  ({						\
	(dest) = __builtin_sw_host_loade ((src));	\
	(dest);					\
  })

#define simd_storeul(src,dest) __builtin_sw_host_storeul (dest, src)
#define simd_storeuh(src,dest) __builtin_sw_host_storeuh (dest, src)

static __inline intv8 __attribute__((__always_inline__))
simd_veqvw (intv8 __A, intv8 __B)
{
        return (intv8)__builtin_sw_host_veqvw(__A, __B);
}

static __inline int __attribute__((__always_inline__))
simd_vcmpgew (intv8 __A, intv8 __B)
{
        return (int)__builtin_sw_host_vcmpgew(__A, __B);
}
static __inline int __attribute__((__always_inline__))
simd_vcmpgewi (intv8 __A, const int __B)
{
        return (int)__builtin_sw_host_vcmpgewi(__A, __B);
}

#ifndef SHIPS20180426_SW6ASIMD
static __inline intv8 __attribute__((__always_inline__))
simd_vcmpeqw (intv8 __A, intv8 __B)
{
        return (intv8)(__A == __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vcmpeqwi (intv8 __A, const int __B)
{
        return (intv8)__builtin_sw_host_vcmpeqwi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vcmplew (intv8 __A, intv8 __B)
{
        return (intv8)(__A <= __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vcmplewi (intv8 __A, const int __B)
{
        return (intv8)__builtin_sw_host_vcmplewi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vcmpltw (intv8 __A, intv8 __B)
{
        return (intv8)(__A < __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vcmpltwi (intv8 __A, const int __B)
{
        return (intv8)__builtin_sw_host_vcmpltwi(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpulew (uintv8 __A, uintv8 __B)
{
        return (uintv8)(__A <= __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpulewi (uintv8 __A, const unsigned int __B)
{
        return (uintv8)__builtin_sw_host_vcmpulewi(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpultw (uintv8 __A, uintv8 __B)
{
        return (uintv8)(__A < __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpultwi (uintv8 __A, const unsigned int __B)
{
        return (uintv8)__builtin_sw_host_vcmpultwi(__A, __B);
}
#else
static __inline intv8 __attribute__((__always_inline__))
simd_vcmpeqw (intv8 __A, intv8 __B)
{
        return (intv8)__builtin_sw_host_vcmpeqw(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vcmpeqwi (intv8 __A, const int __B)
{
        return (intv8)__builtin_sw_host_vcmpeqwi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vcmplew (intv8 __A, intv8 __B)
{
        return (intv8)__builtin_sw_host_vcmplew(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vcmplewi (intv8 __A, const int __B)
{
        return (intv8)__builtin_sw_host_vcmplewi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vcmpltw (intv8 __A, intv8 __B)
{
        return (intv8)__builtin_sw_host_vcmpltw(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vcmpltwi (intv8 __A, const int __B)
{
        return (intv8)__builtin_sw_host_vcmpltwi(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpulew (uintv8 __A, uintv8 __B)
{
        return (uintv8)__builtin_sw_host_vcmpulew(__A, __B);
}
static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpulewi (uintv8 __A, const unsigned int __B)
{
        return (uintv8)__builtin_sw_host_vcmpulewi(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpultw (uintv8 __A, uintv8 __B)
{
        return (uintv8)__builtin_sw_host_vcmpultw(__A, __B);
}
static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpultwi (uintv8 __A, const unsigned int __B)
{
        return (uintv8)__builtin_sw_host_vcmpultwi(__A, __B);
}
#endif

static __inline int __attribute__((__always_inline__))
simd_ctpopoww (intv8 __A)
{
        return (int) __builtin_sw_host_ctpopoww(__A);
}

static __inline int __attribute__((__always_inline__))
simd_ctpopow (int256 __A)
{
        return (int) __builtin_sw_host_ctpopow(__A);
}

static __inline int __attribute__((__always_inline__))
simd_ctlzow (int256 __A)
{
        return (int) __builtin_sw_host_ctlzow(__A);
}

static __inline int __attribute__((__always_inline__))
simd_reduc_plusw (intv8 __A)
{
        return (int) __builtin_sw_host_reduc_plusw(__A);
}

static __inline float __attribute__((__always_inline__))
simd_reduc_pluss (floatv4 __A)
{
        return (float) __builtin_sw_host_reduc_pluss(__A);
}

static __inline double __attribute__((__always_inline__))
simd_reduc_plusd (doublev4 __A)
{
        return (double) __builtin_sw_host_reduc_plusd(__A);
}

static __inline intv8 __attribute__((__always_inline__))
simd_smaxw (intv8 __A, intv8 __B)
{
        return (intv8) __builtin_sw_host_smaxw(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_umaxw (uintv8 __A, uintv8 __B)
{
        return (uintv8) __builtin_sw_host_umaxw(__A, __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_smaxs (floatv4 __A, floatv4 __B)
{
        return (floatv4) __builtin_sw_host_smaxs(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_smaxd (doublev4 __A, doublev4 __B)
{
        return (doublev4) __builtin_sw_host_smaxd(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_sminw (intv8 __A, intv8 __B)
{
        return (intv8) __builtin_sw_host_sminw(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_uminw (uintv8 __A, uintv8 __B)
{
        return (uintv8) __builtin_sw_host_uminw(__A, __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_smins (floatv4 __A, floatv4 __B)
{
        return (floatv4) __builtin_sw_host_smins(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_smind (doublev4 __A, doublev4 __B)
{
        return (doublev4) __builtin_sw_host_smind(__A, __B);
}

static __inline int __attribute__((__always_inline__))
simd_reduc_smaxw (intv8 __A)
{
        return (int) __builtin_sw_host_reduc_smaxw(__A);
}

static __inline unsigned int __attribute__((__always_inline__))
simd_reduc_umaxw (uintv8 __A)
{
        return (unsigned int) __builtin_sw_host_reduc_umaxw(__A);
}

static __inline float __attribute__((__always_inline__))
simd_reduc_smaxs (floatv4 __A)
{
        return (float) __builtin_sw_host_reduc_smaxs(__A);
}

static __inline double __attribute__((__always_inline__))
simd_reduc_smaxd (doublev4 __A)
{
        return (double) __builtin_sw_host_reduc_smaxd(__A);
}

static __inline int __attribute__((__always_inline__))
simd_reduc_sminw (intv8 __A)
{
        return (int) __builtin_sw_host_reduc_sminw(__A);
}

static __inline unsigned int __attribute__((__always_inline__))
simd_reduc_uminw (uintv8 __A)
{
        return (unsigned int) __builtin_sw_host_reduc_uminw(__A);
}

static __inline float __attribute__((__always_inline__))
simd_reduc_smins (floatv4 __A)
{
        return (float) __builtin_sw_host_reduc_smins(__A);
}

static __inline double __attribute__((__always_inline__))
simd_reduc_smind (doublev4 __A)
{
        return (double) __builtin_sw_host_reduc_smind(__A);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vaddw (intv8 __A, intv8 __B)
{
	return (intv8)  (__A + __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vaddwi (intv8 __A, const int __B)
{
	return (intv8)  (__A + __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsubw (intv8 __A, intv8 __B)
{
	return (intv8)  (__A - __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsubwi (intv8 __A, const int __B)
{
	return (intv8)  (-__B + __A);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vucaddw (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vucaddw(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vucaddwi (intv8 __A, const int __B)
{
	return (intv8)  __builtin_sw_host_vucaddwi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vucsubw (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vucsubw(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vucsubwi (intv8 __A, const int __B)
{
	return (intv8)  __builtin_sw_host_vucsubwi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vucaddh (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vucaddh(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vucaddhi (intv8 __A, const int __B)
{
	return (intv8)  __builtin_sw_host_vucaddhi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vucsubh (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vucsubh(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vucsubhi (intv8 __A, const int __B)
{
	return (intv8)  __builtin_sw_host_vucsubhi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vucaddb (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vucaddb(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vucaddbi (intv8 __A, const int __B)
{
	return (intv8)  __builtin_sw_host_vucaddbi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vucsubb (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vucsubb(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vucsubbi (intv8 __A, const int __B)
{
	return (intv8)  __builtin_sw_host_vucsubbi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vandw (intv8 __A, intv8 __B)
{
	return (intv8)  (__A & __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vbicw (intv8 __A, intv8 __B)
{
	return (intv8)  (__A &~ __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vbisw (intv8 __A, intv8 __B)
{
	return (intv8)  (__A | __B);
}

#ifdef ZHENGH20180308_SW6ASIMD
static __inline floatv4 __attribute__((__always_inline__))
simd_vbiss (floatv4 __A, floatv4 __B)
{
	return (floatv4)  (__A | __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vbisd (doublev4 __A, doublev4 __B)
{
	return (doublev4)  (__A | __B);
}
#endif

static __inline intv8 __attribute__((__always_inline__))
simd_vornotw (intv8 __A, intv8 __B)
{
	return (intv8)  (__A |~ __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vnotw (intv8 __A)
{
	return (intv8)  (~__A);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vxorw (intv8 __A, intv8 __B)
{
	return (intv8)  (__A ^ __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_vandl (int256 __A, int256 __B)
{
	return (int256)  (__A & __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_vbicl (int256 __A, int256 __B)
{
	return (int256)  (__A &~ __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_vbisl (int256 __A, int256 __B)
{
	return (int256)  (__A | __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_vornotl (int256 __A, int256 __B)
{
	return (int256)  (__A |~ __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_vnotl (int256 __A)
{
	return (int256)  (~__A);
}

static __inline int256 __attribute__((__always_inline__))
simd_vxorl (int256 __A, int256 __B)
{
	return (int256)  (__A ^ __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_vaddl (int256 __A, int256 __B)
{
	return (int256)  (__A + __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_vaddli (int256 __A, const int __B)
{
	return (int256)  (__A + __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_vsubl (int256 __A, int256 __B)
{
	return (int256)  (__A - __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_vsubli (int256 __A, const int __B)
{
	return (int256)  (-__B + __A);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vadds (floatv4 __A, floatv4 __B)
{
	return (floatv4)(__A + __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vsubs (floatv4 __A, floatv4 __B)
{
	return (floatv4)(__A - __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vmuls (floatv4 __A, floatv4 __B)
{
	return (floatv4)(__A * __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vdivs (floatv4 __A, floatv4 __B)
{
	return (floatv4)(__A / __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vaddd (doublev4 __A, doublev4 __B)
{
	return (doublev4)(__A + __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vsubd (doublev4 __A, doublev4 __B)
{
	return (doublev4)(__A - __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vmuld (doublev4 __A, doublev4 __B)
{
	return (doublev4)(__A * __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vdivd (doublev4 __A, doublev4 __B)
{
	return (doublev4)(__A / __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vsllw (uintv8 __A, int __B) 
{
        return (uintv8)  (__A << __B);
}
//#define simd_vsllwi(x,y) x<<y
static __inline uintv8 __attribute__((__always_inline__))
simd_vsllwi (uintv8 __A, const int __B) 
{
        return (uintv8)  (__A << __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vslls1 (floatv4 __A) 
{
        return (floatv4)  __builtin_sw_host_vslls(__A, 1);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vslls2 (floatv4 __A) 
{
        return (floatv4)  __builtin_sw_host_vslls(__A, 2);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vslls3 (floatv4 __A) 
{
        return (floatv4)  __builtin_sw_host_vslls(__A, 3);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vslld1 (doublev4 __A) 
{
        return (doublev4)  __builtin_sw_host_vslld(__A, 1);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vslld2 (doublev4 __A) 
{
        return (doublev4)  __builtin_sw_host_vslld(__A, 2);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vslld3 (doublev4 __A) 
{
        return (doublev4)  __builtin_sw_host_vslld(__A, 3);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vsrls1 (floatv4 __A) 
{
        return (floatv4)  __builtin_sw_host_vsrls(__A, 1);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vsrls2 (floatv4 __A) 
{
        return (floatv4)  __builtin_sw_host_vsrls(__A, 2);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vsrls3 (floatv4 __A) 
{
        return (floatv4)  __builtin_sw_host_vsrls(__A, 3);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vsrld1 (doublev4 __A) 
{
        return (doublev4)  __builtin_sw_host_vsrld(__A, 1);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vsrld2 (doublev4 __A) 
{
        return (doublev4)  __builtin_sw_host_vsrld(__A, 2);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vsrld3 (doublev4 __A) 
{
        return (doublev4)  __builtin_sw_host_vsrld(__A, 3);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vsrlw (uintv8 __A, int __B) 
{
        return (uintv8) (__A >> __B);
}
//#define simd_vsrlwi(x,y) x>>y
static __inline uintv8 __attribute__((__always_inline__))
simd_vsrlwi (uintv8 __A, const int __B) 
{
        return (uintv8)  (__A >> __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsraw (intv8 __A, int __B) 
{
        return (intv8)  (__A >> __B);
}
//#define simd_vsrawi(x,y) x>>y
static __inline intv8 __attribute__((__always_inline__))
simd_vsrawi (intv8 __A, const int __B) 
{
        return (intv8)  (__A >> __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vrolw (intv8 __A, int __B) 
{
        return (intv8)  __builtin_sw_host_vrolw(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vrolwi (intv8 __A, const int __B) 
{
        return (intv8)  __builtin_sw_host_vrolwi(__A, __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_sllow (int256 __A, int __B) 
{
        //return (int256)  (__A<<__B);
        return (int256)  __builtin_sw_host_sllow(__A, __B);
}
static __inline int256 __attribute__((__always_inline__))
simd_sllowi (int256 __A, const int __B) 
{
        //return (int256)  (__A<<__B);
        return (int256)  __builtin_sw_host_sllowi(__A, __B);
}

static __inline int256 __attribute__((__always_inline__))
simd_srlow (int256 __A, int __B) 
{
        return (int256)  (__A>>__B);
}
static __inline int256 __attribute__((__always_inline__))
simd_srlowi (int256 __A, const int __B) 
{
        return (int256)  (__A>>__B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vsqrts (floatv4 __x)
{
	return (floatv4) __builtin_sw_host_vsqrts (__x);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vsqrtd (doublev4 __x)
{
	return (doublev4) __builtin_sw_host_vsqrtd (__x);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vlog (intv8 __x, intv8 __y, intv8 __z, const int i)
{
	return (intv8) __builtin_inst_opt_vlogzz_v8si(__x, __y, __z, i);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vmas (floatv4 __A, floatv4 __B, floatv4 __C) 
{
        return (__A*__B+__C);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vmss (floatv4 __A, floatv4 __B, floatv4 __C) 
{
        return (__A*__B-__C);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vnmas (floatv4 __A, floatv4 __B, floatv4 __C) 
{
        return (-__A*__B+__C);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vnmss (floatv4 __A, floatv4 __B, floatv4 __C) 
{
        return (-__A*__B-__C);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vmad (doublev4 __A, doublev4 __B, doublev4 __C)
{
        return (__A*__B+__C);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vmsd (doublev4 __A, doublev4 __B, doublev4 __C)
{
        return (__A*__B-__C);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vnmad (doublev4 __A, doublev4 __B, doublev4 __C)
{
        return (-__A*__B+__C);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vnmsd (doublev4 __A, doublev4 __B, doublev4 __C)
{
        return (-__A*__B-__C);
}

#ifndef SHIPS20180426
static __inline floatv4 __attribute__((__always_inline__))
simd_vfcmpeqs (floatv4 __A, floatv4 __B)
{
        return (floatv4)(__A == __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vfcmples (floatv4 __A, floatv4 __B)
{
        return (floatv4)(__A <= __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vfcmplts (floatv4 __A, floatv4 __B)
{
        return (floatv4)(__A < __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vfcmpuns (floatv4 __A, floatv4 __B)
{
        return (floatv4)__builtin_sw_host_vfcmpuns(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfcmpeqd (doublev4 __A, doublev4 __B)
{
        return (doublev4)(__A == __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfcmpled (doublev4 __A, doublev4 __B)
{
        return (doublev4)(__A <= __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfcmpltd (doublev4 __A, doublev4 __B)
{
        return (doublev4)(__A < __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfcmpund (doublev4 __A, doublev4 __B)
{
        return (doublev4)__builtin_sw_host_vfcmpund(__A, __B);
}
#else
static __inline floatv4 __attribute__((__always_inline__))
simd_vfcmpeqs (floatv4 __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vfcmpeqs(__A, __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vfcmples (floatv4 __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vfcmples(__A, __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vfcmplts (floatv4 __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vfcmplts(__A, __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vfcmpuns (floatv4 __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vfcmpuns(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfcmpeqd (doublev4 __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vfcmpeqd(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfcmpled (doublev4 __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vfcmpled(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfcmpltd (doublev4 __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vfcmpltd(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfcmpund (doublev4 __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vfcmpund(__A, __B);
}
#endif

static __inline floatv4 __attribute__((__always_inline__))
simd_vcpyss (floatv4 __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vcpyss(__A, __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vcpysns (floatv4 __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vcpysns(__A, __B);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vcpyses (floatv4 __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vcpyses(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vcpysd (doublev4 __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vcpysd(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vcpysnd (doublev4 __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vcpysnd(__A, __B);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vcpysed (doublev4 __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vcpysed(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vseleqw (intv8 __A, intv8 __B, intv8 __C)
{
        return (intv8)__builtin_sw_host_vseleqw(__A, __B, __C);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vseleqwi (intv8 __A, intv8 __B, const int __C)
{
        return (intv8)__builtin_sw_host_vseleqwi(__A, __B, __C);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vselltw (intv8 __A, intv8 __B, intv8 __C)
{
        return (intv8)__builtin_sw_host_vselltw(__A, __B, __C);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vselltwi (intv8 __A, intv8 __B, const int __C)
{
        return (intv8)__builtin_sw_host_vselltwi(__A, __B, __C);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsellew (intv8 __A, intv8 __B, intv8 __C)
{
        return (intv8)__builtin_sw_host_vsellew(__A, __B, __C);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vsellewi (intv8 __A, intv8 __B, const int __C)
{
        return (intv8)__builtin_sw_host_vsellewi(__A, __B, __C);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsellbcw (intv8 __A, intv8 __B, intv8 __C)
{
        return (intv8)__builtin_sw_host_vsellbcw(__A, __B, __C);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vsellbcwi (intv8 __A, intv8 __B, const int __C)
{
        return (intv8)__builtin_sw_host_vsellbcwi(__A, __B, __C);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vfseleqs (floatv4 __A, floatv4 __B, floatv4 __C) 
{
        return (floatv4)__builtin_sw_host_vfseleqs(__A, __B, __C);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vfselles (floatv4 __A, floatv4 __B, floatv4 __C) 
{
        return (floatv4)__builtin_sw_host_vfselles(__A, __B, __C);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vfsellts (floatv4 __A, floatv4 __B, floatv4 __C) 
{
        return (floatv4)__builtin_sw_host_vfsellts(__A, __B, __C);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfseleqd (doublev4 __A, doublev4 __B, doublev4 __C) 
{
        return (doublev4)__builtin_sw_host_vfseleqd(__A, __B, __C);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfselled (doublev4 __A, doublev4 __B, doublev4 __C) 
{
        return (doublev4)__builtin_sw_host_vfselled(__A, __B, __C);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vfselltd (doublev4 __A, doublev4 __B, doublev4 __C) 
{
        return (doublev4)__builtin_sw_host_vfselltd(__A, __B, __C);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vinsfs (float __A, floatv4 __B, const int __C) 
{
        return (floatv4)__builtin_sw_host_vinsfs(__A, __B, __C);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vinsfs0 (float __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vinsfs(__A, __B, 0);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vinsfs1 (float __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vinsfs(__A, __B, 1);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vinsfs2 (float __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vinsfs(__A, __B, 2);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vinsfs3 (float __A, floatv4 __B) 
{
        return (floatv4)__builtin_sw_host_vinsfs(__A, __B, 3);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vinsfd (double __A, doublev4 __B, const int __C) 
{
        return (doublev4)__builtin_sw_host_vinsfd(__A, __B, __C);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vinsfd0 (double __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vinsfd(__A, __B, 0);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vinsfd1 (double __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vinsfd(__A, __B, 1);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vinsfd2 (double __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vinsfd(__A, __B, 2);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vinsfd3 (double __A, doublev4 __B) 
{
        return (doublev4)__builtin_sw_host_vinsfd(__A, __B, 3);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vinsw (int __A, intv8 __B, const int __C) 
{
        return (intv8)__builtin_sw_host_vinsw(__A, __B, __C);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vinsw0 (int __A, intv8 __B) 
{
        return (intv8)__builtin_sw_host_vinsw(__A, __B, 0);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vinsw1 (int __A, intv8 __B) 
{
        return (intv8)__builtin_sw_host_vinsw(__A, __B, 1);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vinsw2 (int __A, intv8 __B) 
{
        return (intv8)__builtin_sw_host_vinsw(__A, __B, 2);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vinsw3 (int __A, intv8 __B) 
{
        return (intv8)__builtin_sw_host_vinsw(__A, __B, 3);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vinsw4 (int __A, intv8 __B) 
{
        return (intv8)__builtin_sw_host_vinsw(__A, __B, 4);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vinsw5 (int __A, intv8 __B) 
{
        return (intv8)__builtin_sw_host_vinsw(__A, __B, 5);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vinsw6 (int __A, intv8 __B) 
{
        return (intv8)__builtin_sw_host_vinsw(__A, __B, 6);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vinsw7 (int __A, intv8 __B) 
{
        return (intv8)__builtin_sw_host_vinsw(__A, __B, 7);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vcpyfs (float __A) 
{
        return (floatv4)__builtin_sw_host_vcpyfs(__A);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vcpyfd (double __A) 
{
        return (doublev4)__builtin_sw_host_vcpyfd(__A);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vcpyw (int __A) 
{
        return (intv8)__builtin_sw_host_vcpyw(__A);
}

static __inline int256 __attribute__((__always_inline__))
simd_vcpyl (long __A) 
{
        return (int256)__builtin_sw_host_vcpyfl(__A);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs (floatv4 __A, const int __B) 
{
        return (float)__builtin_sw_host_vextfs(__A, __B);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs0 (floatv4 __A) 
{
        return (float)__builtin_sw_host_vextfs(__A, 0);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs1 (floatv4 __A) 
{
        return (float)__builtin_sw_host_vextfs(__A, 1);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs2 (floatv4 __A) 
{
        return (float)__builtin_sw_host_vextfs(__A, 2);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs3 (floatv4 __A) 
{
        return (float)__builtin_sw_host_vextfs(__A, 3);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd (doublev4 __A, const int __B) 
{
        return (double)__builtin_sw_host_vextfd(__A, __B);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd0 (doublev4 __A) 
{
        return (double)__builtin_sw_host_vextfd(__A, 0);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd1 (doublev4 __A) 
{
        return (double)__builtin_sw_host_vextfd(__A, 1);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd2 (doublev4 __A) 
{
        return (double)__builtin_sw_host_vextfd(__A, 2);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd3 (doublev4 __A) 
{
        return (double)__builtin_sw_host_vextfd(__A, 3);
}

static __inline int __attribute__((__always_inline__))
simd_vextw (intv8 __A, const int __B) 
{
        return (int)__builtin_sw_host_vextw(__A, __B);
}

static __inline int __attribute__((__always_inline__))
simd_vextw0 (intv8 __A) 
{
        return (int)__builtin_sw_host_vextw(__A, 0);
}

static __inline int __attribute__((__always_inline__))
simd_vextw1 (intv8 __A) 
{
        return (int)__builtin_sw_host_vextw(__A, 1);
}

static __inline int __attribute__((__always_inline__))
simd_vextw2 (intv8 __A) 
{
        return (int)__builtin_sw_host_vextw(__A, 2);
}

static __inline int __attribute__((__always_inline__))
simd_vextw3 (intv8 __A) 
{
        return (int)__builtin_sw_host_vextw(__A, 3);
}

static __inline int __attribute__((__always_inline__))
simd_vextw4 (intv8 __A) 
{
        return (int)__builtin_sw_host_vextw(__A, 4);
}

static __inline int __attribute__((__always_inline__))
simd_vextw5 (intv8 __A) 
{
        return (int)__builtin_sw_host_vextw(__A, 5);
}

static __inline int __attribute__((__always_inline__))
simd_vextw6 (intv8 __A) 
{
        return (int)__builtin_sw_host_vextw(__A, 6);
}

static __inline int __attribute__((__always_inline__))
simd_vextw7 (intv8 __A) 
{
        return (int)__builtin_sw_host_vextw(__A, 7);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vconw (intv8 __A, intv8 __B,  void * __C)
{
        return (intv8)__builtin_sw_host_vconw(__A, __B, __C);
}

static __inline floatv4 __attribute__((__always_inline__))
simd_vcons (floatv4 __A, floatv4 __B, void * __C)
{
        return (floatv4)__builtin_sw_host_vcons(__A, __B, __C);
}

static __inline doublev4 __attribute__((__always_inline__))
simd_vcond (doublev4 __A, doublev4 __B, void * __C)
{
        return (doublev4)__builtin_sw_host_vcond(__A, __B, __C);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vshfw (intv8 __A, intv8 __B, double __C)
{
        return (intv8)__builtin_sw_host_vshfw(__A, __B, __C);
}

#ifdef ZHENGH20180425_SW6BG
#ifdef __sw_64_sw6b__
static __inline intv8 __attribute__((__always_inline__))
simd_vsllb (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vsllb(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsllbi (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vsllbi(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vsrlb (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vsrlb(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsrlbi (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vsrlbi(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vsrab (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vsrab(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsrabi (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vsrabi(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vrolb (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vrolb(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vrolbi (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vrolbi(__A, __B);
}


static __inline intv8 __attribute__((__always_inline__))
simd_vsllh (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vsllh(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsllhi (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vsllhi(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vsrlh (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vsrlh(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsrlhi (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vsrlhi(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vsrah (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vsrah(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsrahi (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vsrahi(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vrolh (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vrolh(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vrolhi (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vrolhi(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vslll (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vslll(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsllli (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vsllli(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vsrll (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vsrll(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsrlli (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vsrlli(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vsral (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vsral(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vsrali (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vsrali(__A, __B);
}
static __inline intv8 __attribute__((__always_inline__))
simd_vrol (intv8 __A, int __B)
{
	return (intv8)__builtin_sw_host_vrol(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vroli (intv8 __A, const int __B)
{
	return (intv8)__builtin_sw_host_vroli(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vmaxb (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vmaxb(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vminb (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vminb(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vmaxh (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vmaxh(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vminh (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vminh(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vmaxw (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vmaxw(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vminw (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vminw(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vmaxl (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vmaxl(__A, __B);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vminl (intv8 __A, intv8 __B)
{
	return (intv8)  __builtin_sw_host_vminl(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vumaxb (uintv8 __A, uintv8 __B)
{
	return (uintv8)  __builtin_sw_host_vumaxb(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vuminb (uintv8 __A, uintv8 __B)
{
	return (uintv8)  __builtin_sw_host_vuminb(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vumaxh (uintv8 __A, uintv8 __B)
{
	return (uintv8)  __builtin_sw_host_vumaxh(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vuminh (uintv8 __A, uintv8 __B)
{
	return (uintv8)  __builtin_sw_host_vuminh(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vumaxw (uintv8 __A, uintv8 __B)
{
	return (uintv8)  __builtin_sw_host_vumaxw(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vuminw (uintv8 __A, uintv8 __B)
{
	return (uintv8)  __builtin_sw_host_vuminw(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vumaxl (uintv8 __A, uintv8 __B)
{
	return (uintv8)  __builtin_sw_host_vumaxl(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vuminl (uintv8 __A, uintv8 __B)
{
	return (uintv8)  __builtin_sw_host_vuminl(__A, __B);
}


static __inline int256 __attribute__((__always_inline__))
simd_sraow (int256 __A, int __B) 
{
        return (int256) __builtin_sw_host_sraow(__A, __B);
}
static __inline int256 __attribute__((__always_inline__))
simd_sraowi (int256 __A, const int __B) 
{
        return (int256)  __builtin_sw_host_sraowi(__A, __B);
}


static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpueqb (uintv8 __A, uintv8 __B)
{
        return (uintv8)__builtin_sw_host_vcmpueqb(__A, __B);
}
static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpueqbi (uintv8 __A, const unsigned int __B)
{
        return (uintv8)__builtin_sw_host_vcmpueqbi(__A, __B);
}

static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpugtb (uintv8 __A, uintv8 __B)
{
        return (uintv8)__builtin_sw_host_vcmpugtb(__A, __B);
}
static __inline uintv8 __attribute__((__always_inline__))
simd_vcmpugtbi (uintv8 __A, const unsigned int __B)
{
        return (uintv8)__builtin_sw_host_vcmpugtbi(__A, __B);
}

static __inline int __attribute__((__always_inline__))
simd_vsumw (intv8 __A)
{
        return (int) __builtin_sw_host_vsumw(__A);
}

static __inline long __attribute__((__always_inline__))
simd_vsuml (int256 __A)
{
        return (long) __builtin_sw_host_vsuml(__A);
}

static __inline intv8 __attribute__((__always_inline__))
simd_vbinvw (intv8 __x)
{
	return (intv8) __builtin_sw_host_vbinvw (__x);
}

static __inline int256 __attribute__((__always_inline__))
simd_vwinv (int256 __x)
{
	return (int256) __builtin_sw_host_vwinv (__x);
}

#endif


#endif



#endif /*_SIMD_HOST*/

#if _SIMD_SLAVE || _SIMD_CCC

/********************************************************************************/
/*         Declare the SLAVE TARGET SIMD operations                              */
/********************************************************************************/
typedef int intv16 __attribute__ ((__mode__(__V16SI__)));
typedef unsigned uintv16 __attribute__ ((__mode__(__V16SI__)));
typedef int int512 __attribute__ ((__mode__(__V1XI__)));
typedef unsigned uint512 __attribute__ ((__mode__(__V1XI__)));
typedef float floatv8 __attribute__ ((__mode__(__V8SF__)));
typedef double doublev8 __attribute__ ((__mode__(__V8DF__)));

#endif
#ifdef _SIMD_SLAVE
#pragma ccc thread_code_begin 
static __inline intv16 __attribute__((__always_inline__))
simd_set_intv16 (int __K, int __L, int __M, int __N, int __O, int __P, int __Q, int __R, int __S, int __T, int __U, int __V, int __W, int __X, int __Y, int __Z)
{
    union {
    int __a[16] __attribute__ ((aligned (64)));
    intv16 __v;
  } __u;

  __u.__a[0] = __K;
  __u.__a[1] = __L;
  __u.__a[2] = __M;
  __u.__a[3] = __N;
  __u.__a[4] = __O;
  __u.__a[5] = __P;
  __u.__a[6] = __Q;
  __u.__a[7] = __R;
  __u.__a[8] = __S;
  __u.__a[9] = __T;
  __u.__a[10] = __U;
  __u.__a[11] = __V;
  __u.__a[12] = __W;
  __u.__a[13] = __X;
  __u.__a[14] = __Y;
  __u.__a[15] = __Z;

  return __u.__v;
}

static __inline uintv16 __attribute__((__always_inline__))
simd_set_uintv16 (unsigned int __K, unsigned int __L, unsigned int __M, unsigned int __N, unsigned int __O, unsigned int __P, unsigned int __Q, unsigned int __R, unsigned int __S, unsigned int __T, unsigned int __U, unsigned int __V, unsigned int __W, unsigned int __X, unsigned int __Y, unsigned int __Z)
{
    union {
    unsigned int __a[16] __attribute__ ((aligned (64)));
    uintv16 __v;
  } __u;

  __u.__a[0] = __K;
  __u.__a[1] = __L;
  __u.__a[2] = __M;
  __u.__a[3] = __N;
  __u.__a[4] = __O;
  __u.__a[5] = __P;
  __u.__a[6] = __Q;
  __u.__a[7] = __R;
  __u.__a[8] = __S;
  __u.__a[9] = __T;
  __u.__a[10] = __U;
  __u.__a[11] = __V;
  __u.__a[12] = __W;
  __u.__a[13] = __X;
  __u.__a[14] = __Y;
  __u.__a[15] = __Z;

  return __u.__v;
}

static __inline int512 
simd_set_int512 (long __K, long __L, long __M, long __N, long __O, long __P, long __Q, long __R)
{
  union {
    long __a[8];
    int512 __v;
  } __u;
  __u.__a[0] = __K;
  __u.__a[1] = __L;
  __u.__a[2] = __M;
  __u.__a[3] = __N;
  __u.__a[4] = __O;
  __u.__a[5] = __P;
  __u.__a[6] = __Q;
  __u.__a[7] = __R;
  return __u.__v;  
}

static __inline uint512
simd_set_uint512 (unsigned long __K, unsigned long __L, unsigned long __M, unsigned long __N, unsigned long __O, unsigned long __P, unsigned long __Q, unsigned long __R)
{
  union {
    unsigned long __a[8];
    uint512 __v;
  } __u;
  __u.__a[0] = __K;
  __u.__a[1] = __L;
  __u.__a[2] = __M;
  __u.__a[3] = __N;
  __u.__a[4] = __O;
  __u.__a[5] = __P;
  __u.__a[6] = __Q;
  __u.__a[7] = __R;
  return __u.__v;  
}

static __inline doublev8
simd_set_doublev8 (double __K, double __L, double __M, double __N, double __O, double __P, double __Q, double __R)
{
  union {
    double __a[8];
    doublev8 __v;
  } __u;
  __u.__a[0] = __K;
  __u.__a[1] = __L;
  __u.__a[2] = __M;
  __u.__a[3] = __N;
  __u.__a[4] = __O;
  __u.__a[5] = __P;
  __u.__a[6] = __Q;
  __u.__a[7] = __R;
  return __u.__v;  
}

static __inline floatv8
simd_set_floatv8 (float __K, float __L, float __M, float __N, float __O, float __P, float __Q, float __R)
{
  union {
    float __a[8];
    floatv8 __v;
  } __u;
  __u.__a[0] = __K;
  __u.__a[1] = __L;
  __u.__a[2] = __M;
  __u.__a[3] = __N;
  __u.__a[4] = __O;
  __u.__a[5] = __P;
  __u.__a[6] = __Q;
  __u.__a[7] = __R;
  return __u.__v;  
}

static __inline void __attribute__((__always_inline__))
simd_fprint_intv16 (FILE *fp, intv16 a)
{
  union {
    int __a[16];
    intv16 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d ]\n", __u.__a[15], __u.__a[14], __u.__a[13], __u.__a[12], __u.__a[11], __u.__a[10], __u.__a[9], __u.__a[8], __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_intv16_X (FILE *fp, intv16 a)
{
  union {
    int __a[16];
    intv16 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ]\n", __u.__a[15], __u.__a[14], __u.__a[13], __u.__a[12], __u.__a[11], __u.__a[10], __u.__a[9], __u.__a[8], __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_uintv16 (FILE *fp, uintv16 a)
{
  union {
    unsigned int __a[16];
    uintv16 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u ]\n", __u.__a[15], __u.__a[14], __u.__a[13], __u.__a[12], __u.__a[11], __u.__a[10], __u.__a[9], __u.__a[8], __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_uintv16_X (FILE *fp, uintv16 a)
{
  union {
    unsigned int __a[16];
    uintv16 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ]\n", __u.__a[15], __u.__a[14], __u.__a[13], __u.__a[12], __u.__a[11], __u.__a[10], __u.__a[9], __u.__a[8], __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_int512 (FILE *fp, int512 a)
{
  volatile union {
    long __a[8];
    int512 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_int512_X (FILE *fp, int512 a)
{
  volatile union {
    long __a[8];
    int512 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_uint512 (FILE *fp, uint512 a)
{
  volatile union {
    unsigned long __a[8];
    uint512 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_uint512_X (FILE *fp, uint512 a)
{
  volatile union {
    unsigned long __a[8];
    uint512 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_doublev8 (FILE *fp, doublev8 a)
{
  union {
    double __a[8];
    doublev8 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %e, %e, %e, %e, %e, %e, %e, %e ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_doublev8_X (FILE *fp, doublev8 a)
{
  union {
    double __a[8];
    doublev8 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx ]\n", *(long *)(&__u.__a[7]),*(long *)(&__u.__a[6]), *(long *)(&__u.__a[5]), *(long *)(&__u.__a[4]), *(long *)(&__u.__a[3]), *(long *)(&__u.__a[2]), *(long *)(&__u.__a[1]), *(long *)(&__u.__a[0]));
}

static __inline void __attribute__((__always_inline__))
simd_fprint_floatv8 (FILE *fp, floatv8 a)
{
  union {
    float __a[8];
    floatv8 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ %e, %e, %e, %e, %e, %e, %e, %e ]\n", __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_floatv8_X (FILE *fp, floatv8 a)
{
  union {
    float __a[8];
    floatv8 __v;
  } __u;
  __u.__v = a;
  fprintf (fp, "[ 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ]\n", *(int *)(&__u.__a[7]), *(int *)(&__u.__a[6]), *(int *)(&__u.__a[5]), *(int *)(&__u.__a[4]), *(int *)(&__u.__a[3]), *(int *)(&__u.__a[2]), *(int *)(&__u.__a[1]), *(int *)(&__u.__a[0]));
}

static __inline void __attribute__((__always_inline__)) simd_print_doublev8 (doublev8 a) { simd_fprint_doublev8 (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_doublev8_X (doublev8 a) { simd_fprint_doublev8_X (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_floatv8 (floatv8 a) { simd_fprint_floatv8 (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_floatv8_X (floatv8 a) { simd_fprint_floatv8_X (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_intv16 (intv16 a) { simd_fprint_intv16 (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_intv16_X (intv16 a) { simd_fprint_intv16_X (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_uintv16 (uintv16 a) { simd_fprint_uintv16 (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_uintv16_X (uintv16 a) { simd_fprint_uintv16_X (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_int512 (int512 a) { simd_fprint_int512 (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_int512_X (int512 a) { simd_fprint_int512_X (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_uint512 (uint512 a) { simd_fprint_uint512 (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_uint512_X (uint512 a) { simd_fprint_uint512_X (stdout, a); }

static __inline intv16 __attribute__((__always_inline__))
simd_vandw (intv16 __A, intv16 __B)
{
	return (intv16)  (__A & __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vbicw (intv16 __A, intv16 __B)
{
	return (intv16)  (__A &~ __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vbisw (intv16 __A, intv16 __B)
{
	return (intv16)  (__A | __B);
}

#ifdef SHENL20180319
static __inline floatv8 __attribute__((__always_inline__))
simd_vbiss (floatv8 __A, floatv8 __B)
{
	return (floatv8)  (__A | __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vbisd (doublev8 __A, doublev8 __B)
{
	return (doublev8)  (__A | __B);
}
#endif

static __inline intv16 __attribute__((__always_inline__))
simd_vornotw (intv16 __A, intv16 __B)
{
	return (intv16)  (__A |~ __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vnotw (intv16 __A)
{
	return (intv16)  (~__A);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vxorw (intv16 __A, intv16 __B)
{
	return (intv16)  (__A ^ __B);
}

static __inline int512 __attribute__((__always_inline__))
simd_vandl (int512 __A, int512 __B)
{
	return (int512)  (__A & __B);
}

static __inline int512 __attribute__((__always_inline__))
simd_vbicl (int512 __A, int512 __B)
{
	return (int512)  (__A &~ __B);
}

static __inline int512 __attribute__((__always_inline__))
simd_vbisl (int512 __A, int512 __B)
{
	return (int512)  (__A | __B);
}

static __inline int512 __attribute__((__always_inline__))
simd_vornotl (int512 __A, int512 __B)
{
	return (int512)  (__A |~ __B);
}

static __inline int512 __attribute__((__always_inline__))
simd_vnotl (int512 __A)
{
	return (int512)  (~__A);
}

static __inline int512 __attribute__((__always_inline__))
simd_vxorl (int512 __A, int512 __B)
{
	return (int512)  (__A ^ __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vaddw (intv16 __A, intv16 __B)
{
	return (intv16)  (__A + __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vaddwi (intv16 __A, const int __B)
{
	return (intv16)  (__A + __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vsubw (intv16 __A, intv16 __B)
{
	return (intv16)  (__A - __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vsubwi (intv16 __A, const int __B)
{
	return (intv16)  (-__B + __A);
}

static __inline int512 __attribute__((__always_inline__))
simd_vaddl (int512 __A, int512 __B)
{
	return (int512)  (__A + __B);
}

static __inline int512 __attribute__((__always_inline__))
simd_vaddli (int512 __A, const int __B)
{
	return (int512)  (__A + __B);
}

static __inline int512 __attribute__((__always_inline__))
simd_vsubl (int512 __A, int512 __B)
{
	return (int512)  (__A - __B);
}

static __inline int512 __attribute__((__always_inline__))
simd_vsubli (int512 __A, const int __B)
{
	return (int512)  (-__B + __A);
}

static __inline uintv16 __attribute__((__always_inline__))
simd_vsllw (uintv16 __A, uintv16 __B) 
{
        return (uintv16)  __builtin_sw_slave_vsllw(__A, __B);
}

static __inline uintv16 __attribute__((__always_inline__))
simd_vsllwi (uintv16 __A, const int __B) 
{
	return (uintv16) __builtin_sw_slave_vsllwi(__A, __B);
}

static __inline uintv16 __attribute__((__always_inline__))
simd_vsrlw (uintv16 __A, uintv16 __B)
{
	return __builtin_sw_slave_vsrlw(__A, __B);
}

static __inline uintv16 __attribute__((__always_inline__))
simd_vsrlwi (uintv16 __A, const int __B) 
{
	return __builtin_sw_slave_vsrlwi(__A, __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vsraw (intv16 __A, intv16 __B) 
{
        return __builtin_sw_slave_vsraw(__A, __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vsrawi (intv16 __A, const int __B) 
{
	return __builtin_sw_slave_vsrawi(__A, __B);
}

static __inline int512 __attribute__((__always_inline__))
simd_sllx (int512 __A, int __B)
{
        return (int512)  (__A<<__B);
}

static __inline int512 __attribute__((__always_inline__))
simd_srlx (int512 __A, int __B)
{
        return (int512)  (__A>>__B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vrolw (intv16 __A, intv16 __B) 
{
        return (intv16)  __builtin_sw_slave_vrolw(__A, __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vrolwi (intv16 __A, const int __B) 
{
        return (intv16)  __builtin_sw_slave_vrolwi(__A, __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vadds (floatv8 __A, floatv8 __B)
{
	return (floatv8)(__A + __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vsubs (floatv8 __A, floatv8 __B)
{
	return (floatv8)(__A - __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vmuls (floatv8 __A, floatv8 __B)
{
	return (floatv8)(__A * __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vaddd (doublev8 __A, doublev8 __B)
{
	return (doublev8)(__A + __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vsubd (doublev8 __A, doublev8 __B)
{
	return (doublev8)(__A - __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vmuld (doublev8 __A, doublev8 __B)
{
	return (doublev8)(__A * __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfcmpeqs (floatv8 __A, floatv8 __B) 
{
        //return (floatv8)__builtin_sw_slave_vfcmpeqs(__A, __B);
        return (floatv8)(__A == __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfcmples (floatv8 __A, floatv8 __B) 
{
        //return (floatv8)__builtin_sw_slave_vfcmples(__A, __B);
        return (floatv8)(__A <= __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfcmplts (floatv8 __A, floatv8 __B) 
{
        //return (floatv8)__builtin_sw_slave_vfcmplts(__A, __B);
        return (floatv8)(__A < __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfcmpuns (floatv8 __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vfcmpuns(__A, __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vfcmpeqd (doublev8 __A, doublev8 __B) 
{
        //return (doublev8)__builtin_sw_slave_vfcmpeqd(__A, __B);
        return (doublev8)(__A == __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vfcmpled (doublev8 __A, doublev8 __B) 
{
        //return (doublev8)__builtin_sw_slave_vfcmpled(__A, __B);
        return (doublev8)(__A <= __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vfcmpltd (doublev8 __A, doublev8 __B) 
{
        //return (doublev8)__builtin_sw_slave_vfcmpltd(__A, __B);
        return (doublev8)(__A < __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vfcmpund (doublev8 __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vfcmpund(__A, __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vcpyss (floatv8 __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vcpyss(__A, __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vcpysns (floatv8 __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vcpysns(__A, __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vcpyses (floatv8 __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vcpyses(__A, __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vcpysd (doublev8 __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vcpysd(__A, __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vcpysnd (doublev8 __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vcpysnd(__A, __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vcpysed (doublev8 __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vcpysed(__A, __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vmas (floatv8 __A, floatv8 __B, floatv8 __C) 
{
        //return (floatv8)__builtin_sw_slave_vmas(__A, __B, __C);
        return (__A*__B+__C);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vmss (floatv8 __A, floatv8 __B, floatv8 __C) 
{
        //return (floatv8)__builtin_sw_slave_vmss(__A, __B, __C);
        return (__A*__B-__C);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vnmas (floatv8 __A, floatv8 __B, floatv8 __C) 
{
        //return (floatv8)__builtin_sw_slave_vnmas(__A, __B, __C);
        return (-__A*__B+__C);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vnmss (floatv8 __A, floatv8 __B, floatv8 __C) 
{
        //return (floatv8)__builtin_sw_slave_vnmss(__A, __B, __C);
        return (-__A*__B-__C);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vmad (doublev8 __A, doublev8 __B, doublev8 __C)
{
        //return (doublev8)__builtin_sw_slave_vmad(__A, __B, __C);
        return (__A*__B+__C);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vmsd (doublev8 __A, doublev8 __B, doublev8 __C)
{
        //return (doublev8)__builtin_sw_slave_vmsd(__A, __B, __C);
        return (__A*__B-__C);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vnmad (doublev8 __A, doublev8 __B, doublev8 __C)
{
        //return (doublev8)__builtin_sw_slave_vnmad(__A, __B, __C);
        return (-__A*__B+__C);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vnmsd (doublev8 __A, doublev8 __B, doublev8 __C)
{
        //return (doublev8)__builtin_sw_slave_vnmsd(__A, __B, __C);
        return (-__A*__B-__C);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfseleqs (floatv8 __A, floatv8 __B, floatv8 __C) 
{
        return (floatv8)__builtin_sw_slave_vseleqs(__A, __B, __C);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfselles (floatv8 __A, floatv8 __B, floatv8 __C) 
{
        return (floatv8)__builtin_sw_slave_vselles(__A, __B, __C);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfsellts (floatv8 __A, floatv8 __B, floatv8 __C) 
{
        return (floatv8)__builtin_sw_slave_vsellts(__A, __B, __C);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vfseleqd (doublev8 __A, doublev8 __B, doublev8 __C) 
{
        return (doublev8)__builtin_sw_slave_vseleqd(__A, __B, __C);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vfselled (doublev8 __A, doublev8 __B, doublev8 __C) 
{
        return (doublev8)__builtin_sw_slave_vselled(__A, __B, __C);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vfselltd (doublev8 __A, doublev8 __B, doublev8 __C) 
{
        return (doublev8)__builtin_sw_slave_vselltd(__A, __B, __C);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vconw (intv16 __A, intv16 __B, void * __C)
{
        return (intv16)__builtin_sw_slave_vconw(__A, __B, __C);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vcons (floatv8 __A, floatv8 __B, void * __C)
{
        return (floatv8)__builtin_sw_slave_vcons(__A, __B, __C);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vcond (doublev8 __A, doublev8 __B, void * __C)
{
        return (doublev8)__builtin_sw_slave_vcond(__A, __B, __C);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vinsfs (float __A, floatv8 __B, const int __C) 
{
        return (floatv8)__builtin_sw_slave_vinsfs(__A, __B, __C);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vinsfs0 (float __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vinsfs(__A, __B, 0);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vinsfs1 (float __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vinsfs(__A, __B, 1);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vinsfs2 (float __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vinsfs(__A, __B, 2);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vinsfs3 (float __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vinsfs(__A, __B, 3);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vinsfs4 (float __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vinsfs(__A, __B, 4);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vinsfs5 (float __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vinsfs(__A, __B, 5);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vinsfs6 (float __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vinsfs(__A, __B, 6);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vinsfs7 (float __A, floatv8 __B) 
{
        return (floatv8)__builtin_sw_slave_vinsfs(__A, __B, 7);
}
static __inline doublev8 __attribute__((__always_inline__))
simd_vinsfd (double __A, doublev8 __B, const int __C) 
{
        return (doublev8)__builtin_sw_slave_vinsfd(__A, __B, __C);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vinsfd0 (double __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vinsfd(__A, __B, 0);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vinsfd1 (double __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vinsfd(__A, __B, 1);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vinsfd2 (double __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vinsfd(__A, __B, 2);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vinsfd3 (double __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vinsfd(__A, __B, 3);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vinsfd4 (double __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vinsfd(__A, __B, 4);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vinsfd5 (double __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vinsfd(__A, __B, 5);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vinsfd6 (double __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vinsfd(__A, __B, 6);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vinsfd7 (double __A, doublev8 __B) 
{
        return (doublev8)__builtin_sw_slave_vinsfd(__A, __B, 7);
}
//fanxj20210112
static __inline int512 __attribute__((__always_inline__))
simd_vinsl (long __A, int512 __B, const int __C) 
{
        return (int512)__builtin_sw_slave_vinsl(__A, __B, __C);
}

static __inline int512 __attribute__((__always_inline__))
simd_vinsl0 (long __A, int512 __B) 
{
        return (int512)__builtin_sw_slave_vinsl(__A, __B, 0);
}

static __inline int512 __attribute__((__always_inline__))
simd_vinsl1 (long __A, int512 __B) 
{
        return (int512)__builtin_sw_slave_vinsl(__A, __B, 1);
}

static __inline int512 __attribute__((__always_inline__))
simd_vinsl2 (long __A, int512 __B) 
{
        return (int512)__builtin_sw_slave_vinsl(__A, __B, 2);
}

static __inline int512 __attribute__((__always_inline__))
simd_vinsl3 (long __A, int512 __B) 
{
        return (int512)__builtin_sw_slave_vinsl(__A, __B, 3);
}

static __inline int512 __attribute__((__always_inline__))
simd_vinsl4 (long __A, int512 __B) 
{
        return (int512)__builtin_sw_slave_vinsl(__A, __B, 4);
}

static __inline int512 __attribute__((__always_inline__))
simd_vinsl5 (long __A, int512 __B) 
{
        return (int512)__builtin_sw_slave_vinsl(__A, __B, 5);
}

static __inline int512 __attribute__((__always_inline__))
simd_vinsl6 (long __A, int512 __B) 
{
        return (int512)__builtin_sw_slave_vinsl(__A, __B, 6);
}

static __inline int512 __attribute__((__always_inline__))
simd_vinsl7 (long __A, int512 __B) 
{
        return (int512)__builtin_sw_slave_vinsl(__A, __B, 7);
}
//fanxj20210112 end
static __inline intv16 __attribute__((__always_inline__))
simd_vinsw (int __A, intv16 __B, const int __C) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, __C);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw0 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 0);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw1 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 1);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw2 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 2);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw3 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 3);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw4 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 4);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw5 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 5);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw6 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 6);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw7 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 7);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw8 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 8);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw9 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 9);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw10 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 10);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw11 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 11);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw12 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 12);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw13 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 13);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw14 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 14);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vinsw15 (int __A, intv16 __B) 
{
        return (intv16)__builtin_sw_slave_vinsw(__A, __B, 15);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs (floatv8 __A, const int __B) 
{
        return (float)__builtin_sw_slave_vextfs(__A, __B);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs0 (floatv8 __A) 
{
        return (float)__builtin_sw_slave_vextfs(__A, 0);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs1 (floatv8 __A) 
{
        return (float)__builtin_sw_slave_vextfs(__A, 1);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs2 (floatv8 __A) 
{
        return (float)__builtin_sw_slave_vextfs(__A, 2);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs3 (floatv8 __A) 
{
        return (float)__builtin_sw_slave_vextfs(__A, 3);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs4 (floatv8 __A) 
{
        return (float)__builtin_sw_slave_vextfs(__A, 4);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs5 (floatv8 __A) 
{
        return (float)__builtin_sw_slave_vextfs(__A, 5);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs6 (floatv8 __A) 
{
        return (float)__builtin_sw_slave_vextfs(__A, 6);
}

static __inline float __attribute__((__always_inline__))
simd_vextfs7 (floatv8 __A) 
{
        return (float)__builtin_sw_slave_vextfs(__A, 7);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd (doublev8 __A, const int __B) 
{
        return (double)__builtin_sw_slave_vextfd(__A, __B);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd0 (doublev8 __A) 
{
        return (double)__builtin_sw_slave_vextfd(__A, 0);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd1 (doublev8 __A) 
{
        return (double)__builtin_sw_slave_vextfd(__A, 1);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd2 (doublev8 __A) 
{
        return (double)__builtin_sw_slave_vextfd(__A, 2);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd3 (doublev8 __A) 
{
        return (double)__builtin_sw_slave_vextfd(__A, 3);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd4 (doublev8 __A) 
{
        return (double)__builtin_sw_slave_vextfd(__A, 4);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd5 (doublev8 __A) 
{
        return (double)__builtin_sw_slave_vextfd(__A, 5);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd6 (doublev8 __A) 
{
        return (double)__builtin_sw_slave_vextfd(__A, 6);
}

static __inline double __attribute__((__always_inline__))
simd_vextfd7 (doublev8 __A) 
{
        return (double)__builtin_sw_slave_vextfd(__A, 7);
}
static __inline int __attribute__((__always_inline__))
simd_vextw (intv16 __A, const int __B) 
{
        return (int)__builtin_sw_slave_vextw(__A, __B);
}

static __inline int __attribute__((__always_inline__))
simd_vextw0 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 0);
}

static __inline int __attribute__((__always_inline__))
simd_vextw1 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 1);
}

static __inline int __attribute__((__always_inline__))
simd_vextw2 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 2);
}

static __inline int __attribute__((__always_inline__))
simd_vextw3 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 3);
}

static __inline int __attribute__((__always_inline__))
simd_vextw4 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 4);
}

static __inline int __attribute__((__always_inline__))
simd_vextw5 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 5);
}

static __inline int __attribute__((__always_inline__))
simd_vextw6 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 6);
}

static __inline int __attribute__((__always_inline__))
simd_vextw7 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 7);
}

static __inline int __attribute__((__always_inline__))
simd_vextw8 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 8);
}

static __inline int __attribute__((__always_inline__))
simd_vextw9 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 9);
}

static __inline int __attribute__((__always_inline__))
simd_vextw10 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 10);
}

static __inline int __attribute__((__always_inline__))
simd_vextw11 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 11);
}

static __inline int __attribute__((__always_inline__))
simd_vextw12 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 12);
}

static __inline int __attribute__((__always_inline__))
simd_vextw13 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 13);
}

static __inline int __attribute__((__always_inline__))
simd_vextw14 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 14);
}

static __inline int __attribute__((__always_inline__))
simd_vextw15 (intv16 __A) 
{
        return (int)__builtin_sw_slave_vextw(__A, 15);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vcpyfs (float __A) 
{
        return (floatv8)__builtin_sw_slave_vcpyfs(__A);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vcpyfd (double __A) 
{
        return (doublev8)__builtin_sw_slave_vcpyfd(__A);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vcpyw (int __A) 
{
        return (intv16)__builtin_sw_slave_vcpyw(__A);
}

static __inline int512 __attribute__((__always_inline__))
simd_vcpyl (long __A) 
{
        return (int512)__builtin_sw_slave_vcpyfl(__A);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vlog2xi(intv16 __A,intv16 __B,const int __C)
{
	return (intv16)__builtin_sw_slave_vlog2xi(__A,__B,__C);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vlog2xi_i(intv16 __A,const int __B,const int __C)
{
	return (intv16)__builtin_sw_slave_vlog2xi_i(__A,__B,__C);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vlog3ri(intv16 __A,intv16 __B,intv16 __C,const int __D)
{
	return (intv16)__builtin_sw_slave_vlog3ri(__A,__B,__C,__D);
}

static __inline int512 __attribute__((__always_inline__))
simd_vlog2xx(int512 __A,int512 __B,const int __C)
{
	return (int512)__builtin_sw_slave_vlog2xx(__A,__B,__C);
}

static __inline int512 __attribute__((__always_inline__))
simd_vlog2xx_i(int512 __A,const int __B,const int __C)
{
	return (int512)__builtin_sw_slave_vlog2xx_i(__A,__B,__C);
}

static __inline int512 __attribute__((__always_inline__))
simd_vlog3rx(int512 __A,int512 __B,int512 __C,const int __D)
{
	return (int512)__builtin_sw_slave_vlog3rx(__A,__B,__C,__D);
}

static __inline int __attribute__((__always_inline__))
simd_reduc_plusw (intv16 __A)
{
        return (int) __builtin_sw_slave_reduc_plusw(__A);
}

static __inline float __attribute__((__always_inline__))
simd_reduc_pluss (floatv8 __A)
{
        return (float) __builtin_sw_slave_reduc_pluss(__A);
}

static __inline double __attribute__((__always_inline__))
simd_reduc_plusd (doublev8 __A)
{
        return (double) __builtin_sw_slave_reduc_plusd(__A);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_smaxs (floatv8 __A, floatv8 __B)
{
        return (floatv8) __builtin_sw_slave_smaxs(__A, __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_smaxd (doublev8 __A, doublev8 __B)
{
        return (doublev8) __builtin_sw_slave_smaxd(__A, __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_smins (floatv8 __A, floatv8 __B)
{
        return (floatv8) __builtin_sw_slave_smins(__A, __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_smind (doublev8 __A, doublev8 __B)
{
        return (doublev8) __builtin_sw_slave_smind(__A, __B);
}

static __inline float __attribute__((__always_inline__))
simd_reduc_smaxs (floatv8 __A)
{
        return (float) __builtin_sw_slave_reduc_smaxs(__A);
}

static __inline double __attribute__((__always_inline__))
simd_reduc_smaxd (doublev8 __A)
{
        return (double) __builtin_sw_slave_reduc_smaxd(__A);
}

static __inline float __attribute__((__always_inline__))
simd_reduc_smins (floatv8 __A)
{
        return (float) __builtin_sw_slave_reduc_smins(__A);
}

static __inline double __attribute__((__always_inline__))
simd_reduc_smind (doublev8 __A)
{
        return (double) __builtin_sw_slave_reduc_smind(__A);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vdivs (floatv8 __A, floatv8 __B)
{
	return (floatv8)(__A / __B);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vdivd (doublev8 __A, doublev8 __B)
{
	return (doublev8)(__A / __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vsqrts (floatv8 __x)
{
	return (floatv8) __builtin_sw_slave_vsqrts(__x);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vsqrtd (doublev8 __x)
{
	return (doublev8) __builtin_sw_slave_vsqrtd(__x);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfrecs (floatv8 __x)
{
	return (floatv8) __builtin_sw_slave_vfrecs(__x);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vfrecd (doublev8 __x)
{
	return (doublev8) __builtin_sw_slave_vfrecd(__x);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfrecps (floatv8 __x)
{
	return (floatv8) __builtin_sw_slave_vfrecps(__x);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vfrecpd (doublev8 __x)
{
	return (doublev8) __builtin_sw_slave_vfrecpd(__x);
}


/* QIANH20190304_9A*/
typedef _Float16 float16v32 __attribute__ ((__mode__(__V32HF__)));
static __inline float16v32
simd_set_float16v32 (_Float16 __a0, _Float16 __a1, _Float16 __a2, _Float16 __a3, _Float16 __a4, _Float16 __a5, _Float16 __a6, _Float16 __a7, _Float16 __a8, _Float16 __a9, _Float16 __a10, _Float16 __a11, _Float16 __a12, _Float16 __a13, _Float16 __a14, _Float16 __a15, _Float16 __a16, _Float16 __a17, _Float16 __a18, _Float16 __a19, _Float16 __a20, _Float16 __a21, _Float16 __a22, _Float16 __a23, _Float16 __a24, _Float16 __a25, _Float16 __a26, _Float16 __a27, _Float16 __a28, _Float16 __a29, _Float16 __a30, _Float16 __a31)
{
  union {
    _Float16 __a[32];
    float16v32 __v;
  } __u;
  __u.__a[0] = __a0;
  __u.__a[1] = __a1;
  __u.__a[2] = __a2;
  __u.__a[3] = __a3;
  __u.__a[4] = __a4;
  __u.__a[5] = __a5;
  __u.__a[6] = __a6;
  __u.__a[7] = __a7;
  __u.__a[8] = __a8;
  __u.__a[9] = __a9;
  __u.__a[10] = __a10;
  __u.__a[11] = __a11;
  __u.__a[12] = __a12;
  __u.__a[13] = __a13;
  __u.__a[14] = __a14;
  __u.__a[15] = __a15;
  __u.__a[16] = __a16;
  __u.__a[17] = __a17;
  __u.__a[18] = __a18;
  __u.__a[19] = __a19;
  __u.__a[20] = __a20;
  __u.__a[21] = __a21;
  __u.__a[22] = __a22;
  __u.__a[23] = __a23;
  __u.__a[24] = __a24;
  __u.__a[25] = __a25;
  __u.__a[26] = __a26;
  __u.__a[27] = __a27;
  __u.__a[28] = __a28;
  __u.__a[29] = __a29;
  __u.__a[30] = __a30;
  __u.__a[31] = __a31;
  return __u.__v;  
}

static __inline void __attribute__((__always_inline__))
simd_fprint_float16v32 (FILE *fp, float16v32 a)
{
  union {
    _Float16 __a[32];
    float16v32 __v;
  } __u;
  __u.__v = a;

  fprintf (fp, "[ %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e, %e ]\n", (float)__u.__a[31], (float)__u.__a[30], (float)__u.__a[29], (float)__u.__a[28], (float)__u.__a[27], (float)__u.__a[26], (float)__u.__a[25], (float)__u.__a[24], (float)__u.__a[23], (float)__u.__a[22], (float)__u.__a[21], (float)__u.__a[20], (float)__u.__a[19], (float)__u.__a[18], (float)__u.__a[17], (float)__u.__a[16], (float)__u.__a[15], (float)__u.__a[14], (float)__u.__a[13], (float)__u.__a[12], (float)__u.__a[11], (float)__u.__a[10], (float)__u.__a[9], (float)__u.__a[8], (float)__u.__a[7], (float)__u.__a[6], (float)__u.__a[5], (float)__u.__a[4], (float)__u.__a[3], (float)__u.__a[2], (float)__u.__a[1], (float)__u.__a[0]);
}

static __inline void __attribute__((__always_inline__))
simd_fprint_float16v32_X (FILE *fp, float16v32 a)
{
  union {
    short __a[32];
    float16v32 __v;
  } __u;
  __u.__v = a;

  fprintf (fp, "[ 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x ]\n", __u.__a[31], __u.__a[30], __u.__a[29], __u.__a[28], __u.__a[27], __u.__a[26], __u.__a[25], __u.__a[24], __u.__a[23], __u.__a[22], __u.__a[21], __u.__a[20], __u.__a[19], __u.__a[18], __u.__a[17], __u.__a[16], __u.__a[15], __u.__a[14], __u.__a[13], __u.__a[12], __u.__a[11], __u.__a[10], __u.__a[9], __u.__a[8], __u.__a[7], __u.__a[6], __u.__a[5], __u.__a[4], __u.__a[3], __u.__a[2], __u.__a[1], __u.__a[0]);
}
static __inline void __attribute__((__always_inline__)) simd_print_float16v32 (float16v32 a) { simd_fprint_float16v32 (stdout, a); }
static __inline void __attribute__((__always_inline__)) simd_print_float16v32_X (float16v32 a) { simd_fprint_float16v32_X (stdout, a); }

static __inline float16v32 __attribute__((__always_inline__))
simd_vaddh (float16v32 __A, float16v32 __B)
{
	return (float16v32)(__A + __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vsubh (float16v32 __A, float16v32 __B)
{
	return (float16v32)(__A - __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vmulh (float16v32 __A, float16v32 __B)
{
	return (float16v32)(__A * __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vmah (float16v32 __A, float16v32 __B, float16v32 __C) 
{
        return (__A*__B+__C);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vmsh (float16v32 __A, float16v32 __B, float16v32 __C) 
{
        return (__A*__B-__C);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vnmah (float16v32 __A, float16v32 __B, float16v32 __C) 
{
        return (-__A*__B+__C);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vnmsh (float16v32 __A, float16v32 __B, float16v32 __C) 
{
        return (-__A*__B-__C);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vinsh (_Float16 __A, float16v32 __B, const int __C) 
{
        return (float16v32)__builtin_sw_slave_vinsh(__A, __B, __C);
}

static __inline _Float16 __attribute__((__always_inline__))
simd_vexth (float16v32 __A, const int __B) 
{
        return (_Float16)__builtin_sw_slave_vexth(__A, __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vcpysh (float16v32 __A, float16v32 __B) 
{
        return (float16v32)__builtin_sw_slave_vcpysh(__A, __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vcpysnh (float16v32 __A, float16v32 __B) 
{
        return (float16v32)__builtin_sw_slave_vcpysnh(__A, __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vcpyseh (float16v32 __A, float16v32 __B) 
{
        return (float16v32)__builtin_sw_slave_vcpyseh(__A, __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vfcmpeqh (float16v32 __A, float16v32 __B)
{
        return (float16v32)(__A == __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vfcmpleh (float16v32 __A, float16v32 __B)
{
        return (float16v32)(__A <= __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vfcmplth (float16v32 __A, float16v32 __B)
{
        return (float16v32)(__A < __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vfcmpunh (float16v32 __A, float16v32 __B)
{
        return (float16v32)__builtin_sw_slave_vfcmpunh(__A, __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vseleqh (float16v32 __A, float16v32 __B, float16v32 __C) 
{
        return (float16v32)__builtin_sw_slave_vseleqh(__A, __B, __C);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vselleh (float16v32 __A, float16v32 __B, float16v32 __C) 
{
        return (float16v32)__builtin_sw_slave_vselleh(__A, __B, __C);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vsellth (float16v32 __A, float16v32 __B, float16v32 __C) 
{
        return (float16v32)__builtin_sw_slave_vsellth(__A, __B, __C);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vcmpeqw (intv16 __A, intv16 __B)
{
        return (intv16)__builtin_sw_slave_vcmpeqw(__A, __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vcmpeqwi (intv16 __A, const int __B)
{
        return (intv16)__builtin_sw_slave_vcmpeqwi(__A, __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vcmplew (intv16 __A, intv16 __B)
{
        return (intv16)__builtin_sw_slave_vcmplew(__A, __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vcmplewi (intv16 __A, const int __B)
{
        return (intv16)__builtin_sw_slave_vcmplewi(__A, __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vcmpltw (intv16 __A, intv16 __B)
{
        return (intv16)__builtin_sw_slave_vcmpltw(__A, __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vcmpltwi (intv16 __A, const int __B)
{
        return (intv16)__builtin_sw_slave_vcmpltwi(__A, __B);
}

static __inline uintv16 __attribute__((__always_inline__))
simd_vcmpulew (uintv16 __A, uintv16 __B)
{
        return (uintv16)__builtin_sw_slave_vcmpulew(__A, __B);
}

static __inline uintv16 __attribute__((__always_inline__))
simd_vcmpulewi (uintv16 __A, const unsigned int __B)
{
        return (uintv16)__builtin_sw_slave_vcmpulewi(__A, __B);
}

static __inline uintv16 __attribute__((__always_inline__))
simd_vcmpultw (uintv16 __A, uintv16 __B)
{
        return (uintv16)__builtin_sw_slave_vcmpultw(__A, __B);
}

static __inline uintv16 __attribute__((__always_inline__))
simd_vcmpultwi (uintv16 __A, const unsigned int __B)
{
        return (uintv16)__builtin_sw_slave_vcmpultwi(__A, __B);
}

static __inline intv16 __attribute__((__always_inline__))
simd_vshfw (intv16 __A, intv16 __B, intv16 __C)
{
        return (intv16)__builtin_sw_slave_vshfw(__A, __B, __C);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vshfh (float16v32 __A, float16v32 __B, intv16 __C)
{
        return (float16v32)__builtin_sw_slave_vshfh(__A, __B, __C);
}
static __inline floatv8 __attribute__((__always_inline__))
simd_vshfs (floatv8 __A, floatv8 __B, intv16 __C)
{
        return (floatv8)__builtin_sw_slave_vshfs(__A, __B, __C);
}
static __inline doublev8 __attribute__((__always_inline__))
simd_vshfd (doublev8 __A, doublev8 __B, intv16 __C)
{
        return (doublev8)__builtin_sw_slave_vshfd(__A, __B, __C);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vfcvtsh (floatv8 __A, const int __B)
{
        return (float16v32)__builtin_sw_slave_vfcvtsh(__A, __B);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vfcvths (float16v32 __A, const int __B)
{
        return (floatv8)__builtin_sw_slave_vfcvths(__A, __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vcpyh (_Float16 __A) 
{
        return (float16v32)__builtin_sw_slave_vcpyh(__A);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_sminh (float16v32 __A, float16v32 __B)
{
        return (float16v32) __builtin_sw_slave_sminh(__A, __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_smaxh (float16v32 __A, float16v32 __B)
{
        return (float16v32) __builtin_sw_slave_smaxh(__A, __B);
}

static __inline _Float16 __attribute__((__always_inline__))
simd_reduc_plush (float16v32 __A)
{
        return (_Float16) __builtin_sw_slave_reduc_plush(__A);
}

static __inline _Float16 __attribute__((__always_inline__))
simd_reduc_smaxh (float16v32 __A)
{
        return (_Float16) __builtin_sw_slave_reduc_smaxh(__A);
}

static __inline _Float16 __attribute__((__always_inline__))
simd_reduc_sminh (float16v32 __A)
{
        return (_Float16) __builtin_sw_slave_reduc_sminh(__A);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vdivh (float16v32 __A, float16v32 __B)
{
	return (float16v32)(__A / __B);
}

static __inline float16v32 __attribute__((__always_inline__))
simd_vsqrth (float16v32 __x)
{
	return (float16v32) __builtin_sw_slave_vsqrth(__x);
}

static __inline floatv8 __attribute__((__always_inline__))
simd_vrsqrts (floatv8 __x)
{
	return (floatv8) __builtin_sw_slave_vrsqrts(__x);
}

static __inline doublev8 __attribute__((__always_inline__))
simd_vrsqrtd (doublev8 __x)
{
	return (doublev8) __builtin_sw_slave_vrsqrtd(__x);
}

static inline float16v32 vfcvthh(float16v32 __a)
{
	return __builtin_sw_slave_vfcvthh(__a);
}

#ifndef fanxj20210112

#define simd_vnprsqrtd simd_vfsqrtrecd
static __inline doublev8 __attribute__((__always_inline__))
simd_vfsqrtrecd (doublev8 __X)
{
        return (doublev8) __builtin_sw_slave_vfsqrtrecd(__X);
}
static __inline doublev8 __attribute__((__always_inline__))
simd_vabsd (doublev8 __X) 
{
        return (doublev8)__builtin_sw_slave_vcpysd(0,__X);
}
static __inline floatv8 __attribute__((__always_inline__))
simd_vabss (floatv8 __X) 
{
        return (floatv8)__builtin_sw_slave_vcpyss(0,__X);
}
static __inline int512 __attribute__((__always_inline__))
simd_vcastdl(doublev8 __X)
{
        return (int512)__builtin_sw_slave_vcastdl(__X);
}
static __inline doublev8 __attribute__((__always_inline__))
simd_vcastld(int512 __X)
{
        return (doublev8)__builtin_sw_slave_vcastld(__X);
}
static __inline doublev8 __attribute__((__always_inline__))
simd_vallzerod(doublev8 __X)
{
        return (int)__builtin_sw_slave_vallzerod(__X);
}
static __inline doublev8 __attribute__((__always_inline__))
simd_vallzerol(int512 __X)
{
        return (int)__builtin_sw_slave_vallzerol(__X);
}

#endif
static inline void simd_convert_float16(_Float16 *dest, _Float16 *src, int num)
{
	int i;
	float16v32 vdest,vsrc;
	for(i=0;i<num/32;i++)
	{
		simd_load(vsrc,&src[i*32]);
		vdest=vfcvthh(vsrc);
		simd_store(vdest,&dest[i*32]);
	}
}

extern  __attribute__ ((section (".ldm"))) int512 __mask,__mask1;
static inline intv16 simd_vmulw(intv16 va, intv16 vb) 
{
	doublev8 a0d, a1d, bd, a0d_h, a1d_h, bd_h, r0d, r1d, r0d_h, r1d_h;
	intv16 a0, a1, a_h;
	int512 r0x, r1x, r0x_h, r1x_h;
	intv16 r0, r1, r, r0_h, r1_h;
	a_h   = ((int512)va >> 32) & __mask1;
	bd_h  = ((int512)vb >> 32) & __mask1;
	bd    = (int512)vb & __mask1;
	a1d_h = (int512)(a_h >> 16) & __mask;
	a0d_h = (int512)a_h & __mask;
	a1d   = (int512)(va >> 16) & __mask;
	a0d   = (int512)va & __mask;

	r1d_h = a1d_h * bd_h;
	r0d_h = a0d_h * bd_h;
	r1d = a1d * bd; 
	r0d = a0d * bd; 
	r1x_h = r1d_h;
	r0x_h = r0d_h;
	r1x = r1d;
	r0x = r0d;
	r1_h = (r1x_h & __mask1) << 32; 
	r0_h = (r0x_h & __mask1) << 32; 
	r0 = (intv16)(r0x & __mask1) | r0_h;
	r1 = (intv16)(r1x & __mask1) | r1_h;
	r = (r1 << 16) + r0; 

	return r;
}

static inline intv16 simd_vdivw(intv16 va, intv16 vb) 
{
	doublev8 a0d, a1d, bd, a0d_h, a1d_h, bd_h, r0d, r1d, r0d_h, r1d_h;
	intv16 a0, a1, a_h;
	int512 r0x, r1x, r0x_h, r1x_h ,v3;
	intv16 r0, r1, r, r0_h, r1_h;
	intv16 v1,v2;
	v1 = ((int512)va >> 32) & __mask1;
	v2 = v1 >> 31 ;
	v3 = (int512)v2 << 32;
	a0d_h   = ((int512)va >> 32) & __mask1 | v3;
	v1 = (int512)va & __mask1 ;
	v2 = v1 >> 31 ;
	v3 = (int512)v2 << 32;
	a0d  = (int512)va & __mask1 | v3;
	v1 = ((int512)vb >> 32) & __mask1;
	v2 = v1 >> 31 ;
	v3 = (int512)v2 << 32;
	bd_h  = (((int512)vb >> 32) & __mask1) | v3;
	v1 = (int512)vb & __mask1 ;
	v2 = v1 >> 31 ;
	v3 = (int512)v2 << 32;
	bd    = (int512)vb & __mask1 | v3;

	r0d_h = a0d_h / bd_h;
	r0d = a0d / bd;
	r0x_h = r0d_h;
	r0x = r0d;
	r0_h = (r0x_h & __mask1) << 32;
	r0 = (intv16)(r0x & __mask1) | r0_h;
	r = r0;

	return r;
}
#pragma ccc thread_code_end
#endif /*_SIMD_SLAVE*/

 
#endif /*_SIMD_H_INCLUDED*/
