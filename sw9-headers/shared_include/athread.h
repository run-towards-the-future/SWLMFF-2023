#ifndef _ATHREAD_H
#define _ATHREAD_H 1

#ifdef __sw_host__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(CONFIG_SW_7) && !defined(CONFIG_SW_9) && !defined(CONFIG_SW_AI)
#define CONFIG_SW_9   //default to SW_9
#endif

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>

#define SLAVE_FUN(x)        slave_##x

#if defined(CONFIG_SW_7) || defined(CONFIG_SW_9)
#define SCORE_PRIVATE_VADDR           (0x40UL << 40)
#define SCORE_PRIVATE_VADDR_NC           ((0x40UL << 40)|(0x1UL<<45))
#define SCORE_SHARED_TEXT_VADDR         (0x4ffffUL << 28)
#define SCORE_SHARED_DATA_VADDR         (0x50UL << 40)
#define SCORE_CROSS_VADDR             (0x58UL << 40)
#endif

#define PAGE_SIZE 8192
#define PAGE_SHIFT 13

#ifdef CONFIG_SW_7
#define NR_SCORE_ARRAY_NUM         4
#define CG_NUM NR_SCORE_ARRAY_NUM
#define NR_SC_NUM_PER_ARRAY 	   16
#define NR_SCORE_NUM_PER_ARRAY     64
#endif

#ifdef CONFIG_SW_9
#define NR_SCORE_ARRAY_NUM         6
#define CG_NUM NR_SCORE_ARRAY_NUM
#define NR_SC_NUM_PER_ARRAY 	   16
#define NR_SCORE_NUM_PER_ARRAY     64
#define MAX_CORES NR_SCORE_NUM_PER_ARRAY
#endif

#define NR_SCORES               (NR_SCORE_ARRAY_NUM * NR_SCORE_NUM_PER_ARRAY)

#define AKERNEL_MAX_PENUM            (NR_SCORE_ARRAY_NUM*NR_SCORE_NUM_PER_ARRAY)

#define __min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define __max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })


#define BITS_PER_LONG 64

#define small_const_nbits(nbits) \
	(__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG)



enum score_ldm_shared_size {
	SCORE_LDM_SHARED_0KB,
	SCORE_LDM_SHARED_4KB,
	SCORE_LDM_SHARED_8KB,
	SCORE_LDM_SHARED_16KB,
	SCORE_LDM_SHARED_32KB,
	SCORE_LDM_SHARED_64KB,
	SCORE_LDM_SHARED_128KB,
        SCORE_LDM_SHARED_SIZE_ERR, /* for err checking, not a valid option */
};

enum score_ldm_shared_mode {
	SCORE_LDM_SHARED_SINGLE,
	SCORE_LDM_SHARED_DOUBLE,
	SCORE_LDM_SHARED_QUAD,
	SCORE_LDM_SHARED_ALL_CLUSTER,
	SCORE_LDM_SHARED_ALL_ROW,
        SCORE_LDM_SHARED_NONE,
        SCORE_LDM_SHARED_MODE_ERR, /* for err checking, not a valid option */
};

enum score_ldm_dcache_size {
	SCORE_LDM_DCACHE_0KB,
	SCORE_LDM_DCACHE_32KB,
	SCORE_LDM_DCACHE_128KB,
        SCORE_LDM_DCACHE_SIZE_ERR, /* for err checking, not a valid option */
};

enum score_mieee {
	SCORE_MIEEE_DEFAULT,
	SCORE_MIEEE_WITH_INEXACT,
	SCORE_MIEEE,
        SCORE_MIEEE_ERR, /* for err checking, not a valid option */
};

enum score_rouding {
        SCORE_ROUNDING_ZERO, 
        SCORE_ROUNDING_NEGATIVE_INF, 
        SCORE_ROUNDING_NEAREST, 
        SCORE_ROUNDING_POSITIVE_INF, 
        SCORE_ROUNDING_ERR, /* for err checking, not a valid option */
};


/* Score Stack start from LDM Bottom */
#define SCORE_PESTACK_MODE_LDM         0x1111
/* Score Stack start from private segment space */
#define SCORE_PESTACK_MODE_SEGPRI      0x2222

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d)) 
#define BITS_PER_BYTE       8
#define BITS_TO_LONGS(nr)   DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define DECLARE_BITMAP(name,bits) \
    unsigned long name[BITS_TO_LONGS(bits)]

typedef struct cpumask { DECLARE_BITMAP(bits, AKERNEL_MAX_PENUM); } cpumask_t; 
#define cpumask_bits(maskp) ((maskp)->bits)
#define nr_cpumask_bits AKERNEL_MAX_PENUM

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

/* For athread */
typedef struct s_thread_info{
    char valid;             //the thread info is valid or not
    int thread_id;
    int core_num;
    volatile  int state_flag;         //run :1 ; finish:0
    void * pc;
    void * arg;
    char fini_sig;          //finish type
    long gid;
    int team_size;
    int flush_slave_cache;	//flush slave cache or not
} thread_info_t;



static inline void
set_bit(unsigned long nr, volatile void * addr)
{
	unsigned long temp1, temp2, base;
	int *m = ((int *) addr) + (nr >> 5);

	__asm__ __volatile__(
	"	memb\n"
	"	ldi	%3, %5\n"
	"1:	lldw	%0, 0(%3)\n"
	"	ldi	%1, 1\n"
	"	wr_f	%1\n"
	"	bis	%0, %4, %0\n"
#ifdef CONFIG_SW_9
    "   .align 4\n"
    "   nop\n"
    "   memb\n"
#endif
	"	lstw	%0, 0(%3)\n"
	"	rd_f	%1\n"
	"	beq	%1, 2f\n"
	".subsection 2\n"
	"2:	br	1b\n"
	".previous"
	:"=&r" (temp1), "=&r" (temp2), "=m" (*m), "=&r" (base)
	:"Ir" (1UL << (nr & 31)), "m" (*m));
}

static inline void
clear_bit(unsigned long nr, volatile void * addr)
{
	unsigned long temp1, temp2, base;
	int *m = ((int *) addr) + (nr >> 5);

	__asm__ __volatile__(
	"	memb\n"
	"	ldi	%3, %5\n"
	"1:	lldw	%0, 0(%3)\n"
	"	ldi	%1, 1\n"
	"	wr_f	%1\n"
	"	bic	%0, %4, %0\n"
#ifdef CONFIG_SW_9
    "   .align 4\n"
    "   nop\n"
    "   memb\n"
#endif
	"	lstw	%0, 0(%3)\n"
	"	rd_f	%1\n"
	"	beq	%1, 2f\n"
	".subsection 2\n"
	"2:	br	1b\n"
	".previous"
	:"=&r" (temp1), "=&r" (temp2), "=m" (*m), "=&r" (base)
	:"Ir" (1UL << (nr & 31)), "m" (*m));
}

static inline int
test_bit(int nr, const volatile void * addr)
{
	return (1UL & (((const int *) addr)[nr >> 5] >> (nr & 31))) != 0UL;
}

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


static inline unsigned long __arch_hweight64(unsigned long w)
{
	return __kernel_ctpop(w);
}

static inline unsigned int __arch_hweight32(unsigned int w)
{
	return __arch_hweight64(w);
}

static inline unsigned int __arch_hweight16(unsigned int w)
{
	return __arch_hweight64(w & 0xffff);
}

static inline unsigned int __arch_hweight8(unsigned int w)
{
	return __arch_hweight64(w & 0xff);
}



#define __const_hweight8(w)		\
	((unsigned int)			\
	 ((!!((w) & (1ULL << 0))) +	\
	  (!!((w) & (1ULL << 1))) +	\
	  (!!((w) & (1ULL << 2))) +	\
	  (!!((w) & (1ULL << 3))) +	\
	  (!!((w) & (1ULL << 4))) +	\
	  (!!((w) & (1ULL << 5))) +	\
	  (!!((w) & (1ULL << 6))) +	\
	  (!!((w) & (1ULL << 7)))))

#define __const_hweight16(w) (__const_hweight8(w)  + __const_hweight8((w)  >> 8 ))
#define __const_hweight32(w) (__const_hweight16(w) + __const_hweight16((w) >> 16))
#define __const_hweight64(w) (__const_hweight32(w) + __const_hweight32((w) >> 32))

/*
 * Generic interface.
 */
#define hweight8(w)  (__builtin_constant_p(w) ? __const_hweight8(w)  : __arch_hweight8(w))
#define hweight16(w) (__builtin_constant_p(w) ? __const_hweight16(w) : __arch_hweight16(w))
#define hweight32(w) (__builtin_constant_p(w) ? __const_hweight32(w) : __arch_hweight32(w))
#define hweight64(w) (__builtin_constant_p(w) ? __const_hweight64(w) : __arch_hweight64(w))

static inline unsigned long ffz(unsigned long word)
{
	return __kernel_cttz(~word);
}

/*
 * __ffs = Find First set bit in word.  Undefined if no set bit exists.
 */
static inline unsigned long __ffs(unsigned long word)
{
	return __kernel_cttz(word);
}


static __always_inline unsigned long hweight_long(unsigned long w)
{
	return sizeof(w) == 4 ? hweight32(w) : hweight64(w);
}


extern int __bitmap_empty(const unsigned long *bitmap, unsigned int nbits);
extern int __bitmap_full(const unsigned long *bitmap, unsigned int nbits);
extern int __bitmap_equal(const unsigned long *bitmap1,
			  const unsigned long *bitmap2, unsigned int nbits);
extern void __bitmap_complement(unsigned long *dst, const unsigned long *src,
			unsigned int nbits);
extern void __bitmap_shift_right(unsigned long *dst, const unsigned long *src,
				unsigned int shift, unsigned int nbits);
extern void __bitmap_shift_left(unsigned long *dst, const unsigned long *src,
				unsigned int shift, unsigned int nbits);
extern int __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int nbits);
extern void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int nbits);
extern void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int nbits);
extern int __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int nbits);
extern int __bitmap_intersects(const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int nbits);
extern int __bitmap_subset(const unsigned long *bitmap1,
			const unsigned long *bitmap2, unsigned int nbits);
extern int __bitmap_weight(const unsigned long *bitmap, unsigned int nbits);

extern void bitmap_set(unsigned long *map, unsigned int start, int len);
extern void bitmap_clear(unsigned long *map, unsigned int start, int len);

extern unsigned long find_first_bit(const unsigned long *addr, unsigned long size);

extern unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size);

extern unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
        unsigned long offset);

static inline void bitmap_zero(unsigned long *dst, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = 0UL;
	else {
		unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memset(dst, 0, len);
	}
}

static inline void bitmap_fill(unsigned long *dst, unsigned int nbits)
{
	unsigned int nlongs = BITS_TO_LONGS(nbits);
	if (!small_const_nbits(nbits)) {
		unsigned int len = (nlongs - 1) * sizeof(unsigned long);
		memset(dst, 0xff,  len);
	}
	dst[nlongs - 1] = BITMAP_LAST_WORD_MASK(nbits);
}

static inline void bitmap_copy(unsigned long *dst, const unsigned long *src,
			unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = *src;
	else {
		unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memcpy(dst, src, len);
	}
}

static inline int bitmap_and(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return (*dst = *src1 & *src2 & BITMAP_LAST_WORD_MASK(nbits)) != 0;
	return __bitmap_and(dst, src1, src2, nbits);
}

static inline void bitmap_or(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = *src1 | *src2;
	else
		__bitmap_or(dst, src1, src2, nbits);
}

static inline void bitmap_xor(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = *src1 ^ *src2;
	else
		__bitmap_xor(dst, src1, src2, nbits);
}

static inline int bitmap_andnot(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return (*dst = *src1 & ~(*src2) & BITMAP_LAST_WORD_MASK(nbits)) != 0;
	return __bitmap_andnot(dst, src1, src2, nbits);
}

static inline void bitmap_complement(unsigned long *dst, const unsigned long *src,
			unsigned int nbits)
{
	if (small_const_nbits(nbits))
		*dst = ~(*src);
	else
		__bitmap_complement(dst, src, nbits);
}

static inline int bitmap_equal(const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return ! ((*src1 ^ *src2) & BITMAP_LAST_WORD_MASK(nbits));
	else
		return __bitmap_equal(src1, src2, nbits);
}

static inline int bitmap_intersects(const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return ((*src1 & *src2) & BITMAP_LAST_WORD_MASK(nbits)) != 0;
	else
		return __bitmap_intersects(src1, src2, nbits);
}

static inline int bitmap_subset(const unsigned long *src1,
			const unsigned long *src2, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return ! ((*src1 & ~(*src2)) & BITMAP_LAST_WORD_MASK(nbits));
	else
		return __bitmap_subset(src1, src2, nbits);
}

static inline int bitmap_empty(const unsigned long *src, unsigned nbits)
{
	if (small_const_nbits(nbits))
		return ! (*src & BITMAP_LAST_WORD_MASK(nbits));

	return find_first_bit(src, nbits) == nbits;
}

static inline int bitmap_full(const unsigned long *src, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return ! (~(*src) & BITMAP_LAST_WORD_MASK(nbits));

	return find_first_zero_bit(src, nbits) == nbits;
}

static __always_inline int bitmap_weight(const unsigned long *src, unsigned int nbits)
{
	if (small_const_nbits(nbits))
		return hweight_long(*src & BITMAP_LAST_WORD_MASK(nbits));
	return __bitmap_weight(src, nbits);
}


static inline unsigned int cpumask_check(unsigned int cpu)
{
    assert(cpu < AKERNEL_MAX_PENUM);
	return cpu;
}

/**
 * cpumask_weight - Count of bits in *srcp
 * @srcp: the cpumask to count bits (< nr_cpu_ids) in.
 */
static inline unsigned int cpumask_weight(const struct cpumask *srcp)
{
	return bitmap_weight(cpumask_bits(srcp), nr_cpumask_bits);
}


/**
 * cpumask_set_cpu - set a cpu in a cpumask
 * @cpu: cpu number (< nr_cpumask_bits)
 * @dstp: the cpumask pointer
 */
static inline void cpumask_set_cpu(unsigned int cpu, struct cpumask *dstp)
{
	set_bit(cpumask_check(cpu), cpumask_bits(dstp));
}

/**
 * cpumask_clear_cpu - clear a cpu in a cpumask
 * @cpu: cpu number (< nr_cpumask_bits)
 * @dstp: the cpumask pointer
 */
static inline void cpumask_clear_cpu(int cpu, struct cpumask *dstp)
{
	clear_bit(cpumask_check(cpu), cpumask_bits(dstp));
}

/**
 * cpumask_test_cpu - test for a cpu in a cpumask
 * @cpu: cpu number (< nr_cpumask_bits)
 * @cpumask: the cpumask pointer
 *
 * Returns 1 if @cpu is set in @cpumask, else returns 0
 */
static inline int cpumask_test_cpu(int cpu, const struct cpumask *cpumask)
{
	return test_bit(cpumask_check(cpu), cpumask_bits((cpumask)));
}

/**
 * cpumask_setall - set all cpus (< nr_cpumask_bits) in a cpumask
 * @dstp: the cpumask pointer
 */
static inline void cpumask_setall(struct cpumask *dstp)
{
	bitmap_fill(cpumask_bits(dstp), nr_cpumask_bits);
}

/**
 * cpumask_clear - clear all cpus (< nr_cpumask_bits) in a cpumask
 * @dstp: the cpumask pointer
 */
static inline void cpumask_clear(struct cpumask *dstp)
{
	bitmap_zero(cpumask_bits(dstp), nr_cpumask_bits);
}

/**
 * cpumask_and - *dstp = *src1p & *src2p
 * @dstp: the cpumask result
 * @src1p: the first input
 * @src2p: the second input
 *
 * If *@dstp is empty, returns 0, else returns 1
 */
static inline int cpumask_and(struct cpumask *dstp,
			       const struct cpumask *src1p,
			       const struct cpumask *src2p)
{
	return bitmap_and(cpumask_bits(dstp), cpumask_bits(src1p),
				       cpumask_bits(src2p), nr_cpumask_bits);
}

/**
 * cpumask_or - *dstp = *src1p | *src2p
 * @dstp: the cpumask result
 * @src1p: the first input
 * @src2p: the second input
 */
static inline void cpumask_or(struct cpumask *dstp, const struct cpumask *src1p,
			      const struct cpumask *src2p)
{
	bitmap_or(cpumask_bits(dstp), cpumask_bits(src1p),
				      cpumask_bits(src2p), nr_cpumask_bits);
}

/**
 * cpumask_xor - *dstp = *src1p ^ *src2p
 * @dstp: the cpumask result
 * @src1p: the first input
 * @src2p: the second input
 */
static inline void cpumask_xor(struct cpumask *dstp,
			       const struct cpumask *src1p,
			       const struct cpumask *src2p)
{
	bitmap_xor(cpumask_bits(dstp), cpumask_bits(src1p),
				       cpumask_bits(src2p), nr_cpumask_bits);
}

/**
 * cpumask_andnot - *dstp = *src1p & ~*src2p
 * @dstp: the cpumask result
 * @src1p: the first input
 * @src2p: the second input
 *
 * If *@dstp is empty, returns 0, else returns 1
 */
static inline int cpumask_andnot(struct cpumask *dstp,
				  const struct cpumask *src1p,
				  const struct cpumask *src2p)
{
	return bitmap_andnot(cpumask_bits(dstp), cpumask_bits(src1p),
					  cpumask_bits(src2p), nr_cpumask_bits);
}

/**
 * cpumask_next - get the next cpu in a cpumask
 * @n: the cpu prior to the place to search (ie. return will be > @n)
 * @srcp: the cpumask pointer
 *
 * Returns >= nr_cpumask_bits if no further cpus set.
 */
static inline unsigned int cpumask_next(int n, const struct cpumask *srcp)
{
	/* -1 is a legal arg here. */
	if (n != -1)
		cpumask_check(n);
	return find_next_bit(cpumask_bits(srcp), nr_cpumask_bits, n+1);
}

/**
 * cpumask_empty - *srcp == 0
 * @srcp: the cpumask to that all cpus < nr_cpu_ids are clear.
 */
static inline int cpumask_empty(const struct cpumask *srcp)
{
	return bitmap_empty(cpumask_bits(srcp), nr_cpumask_bits);
}

/**
 * cpumask_full - *srcp == 0xFFFFFFFF...
 * @srcp: the cpumask to that all cpus < nr_cpu_ids are set.
 */
static inline int cpumask_full(const struct cpumask *srcp)
{
	return bitmap_full(cpumask_bits(srcp), nr_cpumask_bits);
}

static inline void cpumask_sprint(const struct cpumask *srcp, char * str)
{
    int i;
    char *tmp = str;
    unsigned long * bits = (unsigned long *)cpumask_bits(srcp);
    for (i=0; i<(AKERNEL_MAX_PENUM/64); i++) {
        sprintf(tmp,"0x%016lx ",*bits);
        bits++;
        tmp += 19;
    }
}

static inline void cpumask_print(const struct cpumask *srcp)
{
    int i;
    unsigned long * bits = (unsigned long *)cpumask_bits(srcp);
    printf("%p: ",srcp);
    for (i=0; i<(AKERNEL_MAX_PENUM/64); i++) {
        printf("%#lx ",*bits);
        bits++;
    }
    printf("\n");
}

static inline int __spawn_mode(){
        char* mode = getenv("SPAWN_MODE");
        if(mode && (strcmp(mode,"auto")==0))
                return 1;
        else if(mode && (strcmp(mode,"local")==0))
                return 2;
        else    
               return 0;
}

/**
 * for_each_cpu - iterate over every cpu in a mask
 * @cpu: the (optionally unsigned) integer iterator
 * @mask: the cpumask pointer
 *
 * After the loop, cpu is >= nr_cpumask_bits.
 */
#define for_each_cpu(cpu, mask)				\
	for ((cpu) = -1;				\
		(cpu) = cpumask_next((cpu), (mask)),	\
		(cpu) < nr_cpumask_bits;)


typedef struct { union { unsigned int __ui[20]; int __i[20]; volatile int __vi[20]; unsigned long __s[10]; void *__p[10]; } __u; } athread_res_t;

int athread_init_resource(athread_res_t * resource);
int athread_init(void);
int athread_init_all(void);
int athread_init_cgs(void);
#define athread_init() ({athread_init();CRTS_init();(0);})
#define athread_init_cgs() ({athread_init_cgs();CRTS_init();(0);})
#define athread_init_all() ({athread_init_all();CRTS_init();(0);})
#define athread_init_resource(x) ({athread_init_resource(x);CRTS_init();(0);})
void athread_res_show(void);

int athread_res_init(athread_res_t * res);
int athread_res_getcurrent(athread_res_t * res);

int athread_res_getjobid(const athread_res_t *res, unsigned long *jobid);
int athread_res_setjobid(athread_res_t *res, unsigned long jobid);
int athread_res_getpenum(const athread_res_t *res, unsigned int * penum);
int athread_res_setpenum(athread_res_t *res, unsigned int penum);
int athread_res_getpemask(const athread_res_t *res, cpumask_t *pemask);
int athread_res_setpemask(athread_res_t *res, cpumask_t *pemask);
int athread_res_getsegpri(const athread_res_t *res, unsigned int * segpri);
int athread_res_setsegpri(athread_res_t *res, unsigned int segpri);
int athread_res_getsegtext(const athread_res_t *res, unsigned int * segtext);
int athread_res_setsegtext(athread_res_t *res, unsigned int segtext);
int athread_res_getsegdata(const athread_res_t *res, unsigned int * segdata);
int athread_res_setsegdata(athread_res_t *res, unsigned int segdata);
int athread_res_getsegcross(const athread_res_t *res, unsigned int * segcross);
int athread_res_setsegcross(athread_res_t *res, unsigned int segcross);
int athread_res_getdcache_mode(const athread_res_t *res, unsigned int *dcache_mode);
int athread_res_setdcache_mode(athread_res_t *res, unsigned int dcache_mode);
int athread_res_getldmshared_mode(const athread_res_t *res, unsigned int *ldmshared_mode);
int athread_res_setldmshared_mode(athread_res_t *res, unsigned int ldmshared_mode);
int athread_res_getldmshared_size(const athread_res_t *res, unsigned int *ldmshared_size);
int athread_res_setldmshared_size(athread_res_t *res, unsigned int ldmshared_size);
int athread_res_getpestack_mode(const athread_res_t *res, unsigned int *pestack_mode);
int athread_res_setpestack_mode(athread_res_t *res,  unsigned int pestack_mode);
int athread_res_getpestack_size(const athread_res_t *res, unsigned int *pestack_size);
int athread_res_setpestack_size(athread_res_t *res,  unsigned int pestack_size);
int athread_res_getmieee(const athread_res_t *res, unsigned int *mode);
int athread_res_setmieee(athread_res_t *res,  unsigned int mode);
int athread_res_getrounding(const athread_res_t *res, unsigned int *mode);
int athread_res_setrounding(athread_res_t *res,  unsigned int mode);

int athread_ctx_slave_fd();
int athread_ctx_sjob_fd();
int athread_ctx_stask_fd();

int get_stask_fd();
int get_sjob_fd();
int get_slave_fd();


typedef void start_routine(void *arg);

int athread_enter64(void);
int athread_enter64_arg(void);
int athread_leave64(void);
int athread_leave64_arg(void);
int athread_enter64_cgs();
int athread_leave64_cgs();

//#ifndef SLAVE_FUN
//#define SLAVE_FUN(x)        slave_##x
//#endif

int __real_athread_spawn(void *func, void *arg, int flush_slave_dcache);
int __real_athread_spawn_all(void *func, void *arg, int flush_slave_dcache);
int __real_athread_spawn_cgs(void *func, void *arg, int flush_slave_dcache);
int __real_athread_spawn_cgmask(int cgmask, void *func, void *arg, int flush_slave_dcache);
int __real_athread_spawn64(void *func);
int __real_athread_spawn64_arg(void *func, void *arg);
int __real_athread_spawn64_cgs(void *func);
int athread_join();
int athread_join_all();
int athread_join_cgs();
int athread_join_cgmask();
int athread_join64();
int athread_join64_arg();
int athread_join64_cgs();
int __real_athread_create(int id, void *func, void *arg);
int athread_wait(int id);
int athread_end(int id);
int athread_setaffinity_np(int cgmask);

#ifdef __cplusplus
#define athread_spawn(y,z) __real_athread_spawn(y,(void *)z,1)
#define athread_spawn_all(y,z) __real_athread_spawn_all(y,z,1)
#define athread_spawn_cgs(y,z) __real_athread_spawn_cgs(y,z,1)
#define athread_spawn_cgmask(x,y,z) __real_athread_spawn_cgmask(x,y,z,1)
#define athread_spawn_noflush(y,z) __real_athread_spawn(y,z,0)
#define athread_spawn_all_noflush(y,z) __real_athread_spawn_all(y,z,0)
#define athread_spawn_cgs_noflush(y,z) __real_athread_spawn_cgs(y,z,0)
#define athread_spawn_cgmask_noflush(x,y,z) __real_athread_spawn_cgmask(x,y,z,0) 
#define athread_spawn64(y) __real_athread_spawn64(y)
#define athread_spawn64_arg(x,y) __real_athread_spawn64_arg(x,y)
#define athread_spawn64_cgs(y) __real_athread_spawn64_cgs(y)
#define athread_create(id, func, arg) __real_athread_create(id, func, arg)
#else
#define athread_spawn(y,z) __real_athread_spawn(slave_##y,(void *)z,1)
#define athread_spawn_all(y,z) __real_athread_spawn_all(slave_##y,z,1)
#define athread_spawn_cgs(y,z) __real_athread_spawn_cgs(slave_##y,z,1)
#define athread_spawn_cgmask(x,y,z) __real_athread_spawn_cgmask(x,slave_##y,z,1)
#define athread_spawn_noflush(y,z) __real_athread_spawn(slave_##y,z,0)
#define athread_spawn_all_noflush(y,z) __real_athread_spawn_all(slave_##y,z,0)
#define athread_spawn_cgs_noflush(y,z) __real_athread_spawn_cgs(slave_##y,z,0)
#define athread_spawn_cgmask_noflush(x,y,z) __real_athread_spawn_cgmask(x,slave_##y,z,0) 
#define athread_spawn64(y) __real_athread_spawn64(slave_##y)
#define athread_spawn64_arg(x,y) __real_athread_spawn64_arg(slave_##x,y)
#define athread_spawn64_cgs(y) __real_athread_spawn64_cgs(slave_##y)
#define athread_create(id, func, arg) __real_athread_create(id, slave_##func, arg)
#endif

#ifndef WUW20200520_SPAWN64CROSS
#define athread_spawn64_cross(cgmask,y) __real_athread_spawn64_cross(cgmask,slave_##y)
extern int __real_athread_spawn64_cross(int cgmask, void* fpc);
extern int athread_join64_cross(int cgmask);
extern int athread_enter64_cross();
#endif

//akernel only
int __real_athread_spawn64_cgmask(int cgmask , void *func, void *arg);
int athread_join64_cgmask(int cgmask);
#define athread_spawn64_cgmask(m,y,z) __real_athread_spawn64_cgmask(m, slave_##y, z)

void athread_res_dump(athread_res_t *r);

#define athread_halt() do { } while(0)

int cross_init(size_t cross_size);
int cross_malloc_init(unsigned long start_addr, size_t size);
void *cross_malloc(size_t size);
void cross_free(void *pointer);
int _sw_xmalloc_init(unsigned long start_addr, size_t size);
void *_sw_xmalloc(size_t size);
void _sw_xfree(void *ptr);

int athread_set_num_threads(int num);
int athread_get_num_threads();
int athread_get_max_threads();
int athread_setaffinity_np(int cgmask);

void * athread_va_to_pa(void *va);

typedef void handler_t(int);

unsigned int athread_get_slave_fd();
unsigned int athread_get_sjob_fd();
unsigned int athread_get_stask_fd();
int athread_get_cross_size();
int athread_get_share_size();
int athread_get_priv_size();
int athread_get_cache_size();
int athread_get_pestack_size();
int athread_get_ldmshare_mode();
int athread_get_ldmshare_size();
unsigned long athread_get_pemask();
int athread_get_stack_in_ldm();
unsigned long athread_idle();

#ifndef ZHUQ20210125
#include "swckpt.h"
extern unsigned long __sw_memory_upper_bound;
extern unsigned long __sw_memory_lower_bound;
extern unsigned long __sw_xmemory_upper_bound;
extern unsigned long __sw_xmemory_lower_bound;

typedef struct __sw_memory_in_use{
  unsigned long start_addr;
  unsigned long size;
} __sw_memory_in_use_t;

#define SW_MEMORY_IN_USE_SLOT 1000
#define SW_MEMORY_FREE_SLOT 1000
extern __sw_memory_in_use_t __sw_memory_in_use_array[SW_MEMORY_IN_USE_SLOT];
extern __sw_memory_in_use_t __sw_xmemory_in_use_array[SW_MEMORY_IN_USE_SLOT];
extern void analysis_sw_memory_in_use(void);
extern void analysis_sw_xmemory_in_use(void);
extern void analysis_sw_memory_in_use_(void);
extern void analysis_sw_xmemory_in_use_(void);
#endif

//#define SPEIO_HW_BASE (0x1ULL<<47ULL | 0x0ULL<<36ULL )
#define SPEL1_HW_BASE (0x1ULL<<47ULL | 0x3ULL<<22ULL ) 
//#define LDM_HW_BASE   (0x1ULL<<47ULL | 0x2ULL<<22ULL )
#define SLAVE_HW_BASE  (0x1ULL<<47ULL)

#define SPEIO_REG_ADDR(cgid,speid,reg)\
          (unsigned long *)(SPEIO_HW_BASE + (reg) + ( (unsigned long)cgid<<40) + ((unsigned long)speid <<24) );
#define SPEIO_READ_REG(cgid,speid,reg)\
            *(unsigned long *)(SPEIO_HW_BASE + (reg) + ( (unsigned long)cgid<<40) + ((unsigned long)speid <<24) );
#define SPEIO_WRITE_REG(cgid,speid,reg,val)               \
            *(unsigned long *)(SPEIO_HW_BASE + (reg) + ( (unsigned long)cgid<<40) + ((unsigned long)speid <<24) ) = val;\
        asm("memb");

#define SPE_EBXIO_FPCR             0x100800    //浮点控制状态寄存器

#define RFPCR(x) {asm volatile("rfpcr %0":"=f"(x));}
#define WFPCR(x) {asm volatile("wfpcr %0"::"f"(x):"memory");}

#ifndef GML20180917
#define SET_round_mode(x)\
{\
	unsigned long val1, val2;\
	RFPCR(val1);\
	switch(x)\
	{\
		case 0:\
		{\
			val2 = val1 & 0xF3FFFFFFFFFFFFFF;\
			break;\
		}\
		case 1:\
		{\
			val2 = val1 & 0xF7FFFFFFFFFFFFFF;\
			val2 = val2 | 0x0400000000000000;\
			break;\
		}\
		case 2:\
		{\
			val2 = val1 & 0xFBFFFFFFFFFFFFFF;\
			val2 = val2 | 0x0800000000000000;\
			break;\
		}\
		case 3:\
		{\
			val2 = val1 | 0x0C00000000000000;\
			break;\
		}\
		default: \
			perror("incorrect round mode\n");\
	}\
	WFPCR(val2);\
}
#endif
#define SPEIO_LDM_HW_BASE   (0x1ULL<<47ULL | 0x2ULL<<22ULL )
#define SPEIO_LDM_ADDR(cgid,speid,disp)\
({\
	if(disp%4!=0){printf("Error:disp's align should be 4,8,etc\n");return;}\
	unsigned long * addr\
        = (unsigned long *)(LDM_HW_BASE + ( (unsigned long)cgid<<40) + ((unsigned long)speid <<24) + disp);\
	(addr);\
})
unsigned long athread_get_nodeid();

#ifndef current_array_id
#define current_array_id athread_get_arrayid
static inline int athread_get_arrayid(void)
{
    unsigned long cid; 
    asm volatile(
            "rcid %0\n"
            "sll  %0, 61, %0\n"
            "srl  %0, 61, %0\n"
            :"=r"(cid)
            :
            :"memory");
    return (int) cid;
}
#endif
#define  __atomic __uncached

/*
 * Standard way to access the cycle counter.
 */

typedef unsigned long cycles_t;

static inline cycles_t get_cycles (void)
{
	cycles_t ret;
	__asm__ __volatile__ ("rtc %0" : "=r"(ret));
	return ret;
}

#ifndef ZHUQ20210105
extern unsigned long tlsoffset_global;
extern unsigned long athread_get_tlsoffset_global();
#endif

#include "crts2ath.h"
#include<slaveio.h>
#ifdef __cplusplus
}
#endif



#endif /* __sw_host__ */

#endif /* _AKERNEL_H_ */
