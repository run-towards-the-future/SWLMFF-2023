#ifndef _INTC_H
#define _INTC_H

#define TRUE 1
#define FALSE 0


# define FmtAssert(Cond,ParmList) \
    ( Cond ? (int) 1 : \
    ( printf( "AHTREAD-LIB:Assertion Failed at file:%s line:%d \n::", __FILE__, __LINE__), \
      printf ParmList, \
      printf( "\n"), \
      exit(0) ) )


# define Is_True(Cond,ParmList) \
    ( Cond ? (int) 1 : \
    ( printf( "AHTREAD-LIB:Check Failed at file:%s line:%d \n::", __FILE__, __LINE__), \
      printf ParmList, \
      printf( "\n"),\
      (int) 0 ) )

#define DBG_printf( ParmList )\
    ( printf( "ATHREAD-DBG: %s %d :: ",__FUNCTION__, __LINE__), \
      printf ParmList )

/*
#define  athread_get_core( id ) \
({char __core__;__core__=__id_core_map[id];__core__;})

#define athread_get_id( core )\
({char __id__;__id__=__core_id_map[core];__id__;})
*/
#define HLINE "==================================================================\n"




typedef void handler_t(int);
void (*sw3_sig_handler[SW3_SIGMAX+1])();
extern void *Signal(int signum, void * handler);
extern void __sig_handler(int sig, siginfo_t *sinfo, struct sigcontext *sigcontext);
extern void __halt_handler(int sig, siginfo_t *sinfo, struct sigcontext *sigcontext);
extern void __end_handler(int sig, siginfo_t *sinfo, struct sigcontext *sigcontext);
extern void __err_handler(int sig, siginfo_t *sinfo, struct sigcontext *sigcontext);
extern void __sysc_handler(int sig, siginfo_t *sinfo, struct sigcontext *sigcontext);

#ifndef WANGF20110117
void (*sw3_exp_handler[_SYS_NSIG])();
extern void __expt_handler(int sig, siginfo_t *sinfo, struct sigcontext *sigcontext);
#endif

#endif //_INTC_H
