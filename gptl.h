/*
** $Id: gptl.h.template,v 1.3 2011-03-28 20:55:19 rosinski Exp $
**
** Author: Jim Rosinski
**
** GPTL header file to be included in user code
*/

#ifndef GPTL_H
#define GPTL_H

/*
** Options settable by a call to GPTLsetoption() (default in parens)
** These numbers need to be small integers because GPTLsetoption can
** be passed PAPI counters, and we need to avoid collisions in that
** integer space. PAPI presets are big negative integers, and PAPI
** native events are big positive integers.
*/

typedef enum {
  GPTLsync_mpi        = 0,  /* Synchronize before certain MPI calls (PMPI-mode only) */
  GPTLwall            = 1,  /* Collect wallclock stats (true) */
  GPTLcpu             = 2,  /* Collect CPU stats (false)*/
  GPTLabort_on_error  = 3,  /* Abort on failure (false) */
  GPTLoverhead        = 4,  /* Estimate overhead of underlying timing routine (true) */
  GPTLdepthlimit      = 5,  /* Only print timers this depth or less in the tree (inf) */
  GPTLverbose         = 6,  /* Verbose output (false) */
  GPTLnarrowprint     = 7,  /* Print PAPI and derived stats in 8 columns not 16 (true) */
  GPTLpercent         = 9,  /* Add a column for percent of first timer (false) */
  GPTLpersec          = 10, /* Add a PAPI column that prints "per second" stats (true) */
  GPTLmultiplex       = 11, /* Allow PAPI multiplexing (false) */
  GPTLdopr_preamble   = 12, /* Print preamble info (true) */
  GPTLdopr_threadsort = 13, /* Print sorted thread stats (true) */
  GPTLdopr_multparent = 14, /* Print multiple parent info (true) */
  GPTLdopr_collision  = 15, /* Print hastable collision info (true) */
  GPTLdopr_memusage   = 27, /* Call GPTLprint_memusage when auto-instrumented */
  GPTLprint_method    = 16, /* Tree print method: first parent, last parent
			       most frequent, or full tree (most frequent) */
  GPTLtablesize       = 50, /* per-thread size of hash table */
  GPTLmaxthreads      = 51, /* maximum number of threads */
  /*
  ** These are derived counters based on PAPI counters. All default to false
  */
  GPTL_IPC           = 17, /* Instructions per cycle */
  GPTL_CI            = 18, /* Computational intensity */
  GPTL_FPC           = 19, /* FP ops per cycle */
  GPTL_FPI           = 20, /* FP ops per instruction */
  GPTL_LSTPI         = 21, /* Load-store instruction fraction */
  GPTL_DCMRT         = 22, /* L1 miss rate (fraction) */
  GPTL_LSTPDCM       = 23, /* Load-stores per L1 miss */
  GPTL_L2MRT         = 24, /* L2 miss rate (fraction) */
  GPTL_LSTPL2M       = 25, /* Load-stores per L2 miss */
  GPTL_L3MRT         = 26  /* L3 read miss rate (fraction) */
} Option;

/*
** Underlying wallclock timer: optimize for best granularity with least overhead.
** These numbers need not be distinct from the above because these are passed
** to GPTLsetutr() and the above are passed to GPTLsetoption()
*/

typedef enum {
  GPTLgettimeofday   = 1, /* the default */
  GPTLnanotime       = 2, /* only available on x86 */
  GPTLmpiwtime       = 4, /* MPI_Wtime */
  GPTLclockgettime   = 5, /* clock_gettime */
  GPTLpapitime       = 6,  /* only if PAPI is available */
  GPTLplacebo        = 7,  /* do-nothing */
  GPTLread_real_time = 3  /* AIX only */
} Funcoption;

/*
** How to report parent/child relationships at print time (for children with multiple parents)
*/

typedef enum {
  GPTLfirst_parent  = 1,  /* first parent found */
  GPTLlast_parent   = 2,  /* last parent found */
  GPTLmost_frequent = 3,  /* most frequent parent (default) */
  GPTLfull_tree     = 4   /* complete call tree */
} Method;

/*
** Function prototypes
*/

#ifdef __cplusplus
extern "C" {
#endif

extern int GPTLsetoption (const int, const int);
extern int GPTLinitialize (void);
extern int GPTLstart (const char *);
extern int GPTLinit_handle (const char *, int *);
extern int GPTLstart_handle (const char *, int *);
extern int GPTLstop (const char *);
extern int GPTLstop_handle (const char *, int *);
extern int GPTLstamp (double *, double *, double *);
extern int GPTLpr (const int);
extern int GPTLpr_file (const char *);

/*
** Use K&R prototype for these 3 because they require MPI
** C++ compilers can encounter problems
*/
extern int GPTLpr_summary (const int);
extern int GPTLpr_summary_file (const int, const char *);
extern int GPTLbarrier ();

extern int GPTLreset (void);
extern int GPTLreset_timer (char *);
extern int GPTLfinalize (void);
extern int GPTLget_memusage (int *, int *, int *, int *, int *);
extern int GPTLprint_memusage (const char *);
extern int GPTLprint_rusage (const char *);
extern int GPTLenable (void);
extern int GPTLdisable (void);
extern int GPTLsetutr (const int);
extern int GPTLquery (const char *, int, int *, int *, double *, double *, double *,
		      long long *, const int);
extern int GPTLquerycounters (const char *, int, long long *);
extern int GPTLget_wallclock (const char *, int, double *);
extern int GPTLget_wallclock_latest (const char *, int, double *);
extern int GPTLget_threadwork (const char *, double *, double *);
extern int GPTLstartstop_val (const char *, double);
extern int GPTLget_eventvalue (const char *, const char *, int, double *);
extern int GPTLget_nregions (int, int *);
extern int GPTLget_regionname (int, int, char *, int);
extern int GPTL_PAPIlibraryinit (void);
extern int GPTLevent_name_to_code (const char *, int *);
extern int GPTLevent_code_to_name (const int, char *);
extern int GPTLnum_errors (void);
extern int GPTLnum_warn (void);
extern int GPTLget_count (const char *, int, int *);

#ifdef __cplusplus
};
#endif

#endif
