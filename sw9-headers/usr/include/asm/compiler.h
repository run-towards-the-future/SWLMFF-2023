#ifndef __SW_64_COMPILER_H
#define __SW_64_COMPILER_H

/* 
 * Herein are macros we use when describing various patterns we want to GCC.
 * In all cases we can get better schedules out of the compiler if we hide
 * as little as possible inside __inline__ assembly.  However, we want to be
 * able to know what we'll get out before giving up __inline__ assembly.  Thus
 * these tests and macros.
 */

# define __kernel_ins0b(val, shift)					\
  ({ unsigned long __kir;						\
     __asm__("ins0b %2,%1,%0" : "=r"(__kir) : "rI"(shift), "r"(val));	\
     __kir; })
# define __kernel_ins1b(val, shift)					\
  ({ unsigned long __kir;						\
     __asm__("ins1b %2,%1,%0" : "=r"(__kir) : "rI"(shift), "r"(val));	\
     __kir; })
# define __kernel_ins3b(val, shift)					\
  ({ unsigned long __kir;						\
     __asm__("ins3b %2,%1,%0" : "=r"(__kir) : "rI"(shift), "r"(val));	\
     __kir; })
# define __kernel_ins6b(val, shift)					\
  ({ unsigned long __kir;						\
     __asm__("ins6b %2,%1,%0" : "=r"(__kir) : "rI"(shift), "r"(val));	\
     __kir; })
# define __kernel_ext0b(val, shift)					\
  ({ unsigned long __kir;						\
     __asm__("ext0b %2,%1,%0" : "=r"(__kir) : "rI"(shift), "r"(val));	\
     __kir; })
# define __kernel_ext1b(val, shift)					\
  ({ unsigned long __kir;						\
     __asm__("ext1b %2,%1,%0" : "=r"(__kir) : "rI"(shift), "r"(val));	\
     __kir; })
# define __kernel_cmpgeb(a, b)						\
  ({ unsigned long __kir;						\
     __asm__("cmpgeb %r2,%1,%0" : "=r"(__kir) : "rI"(b), "rJ"(a));	\
     __kir; })

#  define __kernel_cttz(x)						\
   ({ unsigned long __kir;						\
      __asm__("cttz %1,%0" : "=r"(__kir) : "r"(x));			\
      __kir; })
#  define __kernel_ctlz(x)						\
   ({ unsigned long __kir;						\
      __asm__("ctlz %1,%0" : "=r"(__kir) : "r"(x));			\
      __kir; })
#  define __kernel_ctpop(x)						\
   ({ unsigned long __kir;						\
      __asm__("ctpop %1,%0" : "=r"(__kir) : "r"(x));			\
      __kir; })


/* 
 * Beginning with EGCS 1.1, GCC defines __sw_64_bwx__ when the BWX 
 * extension is enabled.  Previous versions did not define anything
 * we could test during compilation -- too bad, so sad.
 */

#if defined(__sw_64_bwx__)
#define __kernel_ldbu(mem)	(mem)
#define __kernel_ldhu(mem)	(mem)
#define __kernel_stb(val,mem)	((mem) = (val))
#define __kernel_sth(val,mem)	((mem) = (val))
#else
#define __kernel_ldbu(mem)				\
  ({ unsigned char __kir;				\
     __asm__(".arch sw2f;				\
	      ldbu %0,%1" : "=r"(__kir) : "m"(mem));	\
     __kir; })
#define __kernel_ldhu(mem)				\
  ({ unsigned short __kir;				\
     __asm__(".arch sw2f;				\
	      ldhu %0,%1" : "=r"(__kir) : "m"(mem));	\
     __kir; })
#define __kernel_stb(val,mem)				\
  __asm__(".arch sw2f;					\
	   stb %1,%0" : "=m"(mem) : "r"(val))
#define __kernel_sth(val,mem)				\
  __asm__(".arch sw2f;					\
	   sth %1,%0" : "=m"(mem) : "r"(val))
#endif


#endif /* __SW_64_COMPILER_H */
