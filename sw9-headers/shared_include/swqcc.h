/*****************************************************************
 * Author	: Qi ZHUQ
 * Date		: 2021/01/27
 * Description	: Interface definition for sw quantum program
 * Name		: swqcc.h
 *****************************************************************/
#ifndef SWQCC_H
#define SWQCC_H
#include <stdlib.h>
#include <string.h>

#define SIG_QSIM_START "sig_qsim_start"
#define SIG_QSIM_END "sig_qsim_end"
#define OUTPUT_FILE "output.qres"
#define GRID_FILE "grid.txt"
#define ORDERING_FILE "ordering.txt"

typedef unsigned int swqreg;
typedef float swangle;

typedef struct swqout{
  char * state;
  double probability;
}swqout_t;

typedef struct swqprog{
  char * ptr;
  unsigned int layer;
  swqout_t * qout;
}swqprog_t;

extern int __logicqubit;
extern int * __cregarray;
extern int * gridMatrix;
extern int * swqubitMap;
extern int gridxn, gridyn;
extern int gridQcount;
extern int activeQcount;
extern int * activeMap;
extern int cutNum;

extern int  swqprog_init(swqprog_t* p, int num);
extern int  swqprog_finish(swqprog_t* p);
extern void swqprog_dump(swqprog_t* p);
extern void swqprog_spawn(swqprog_t* p, char* arch_name, char * out_name);
extern void swqprog_join(swqprog_t* p, int * creg);
extern void swqprog_map(swqprog_t* p, char* arch_name, char * out_name);
extern char * swqprog_get_state(swqprog_t* p, int index);
extern double swqprog_get_probability(swqprog_t* p, int index);

extern void gate_h(swqprog_t* p, swqreg q);
extern void gate_t(swqprog_t* p, swqreg q);
extern void gate_s(swqprog_t* p, swqreg q);
extern void gate_x(swqprog_t* p, swqreg q);
extern void gate_y(swqprog_t* p, swqreg q);
extern void gate_z(swqprog_t* p, swqreg q);

extern void gate_rx(swqprog_t* p, swqreg q, swangle a); 
extern void gate_ry(swqprog_t* p, swqreg q, swangle a); 
extern void gate_rz(swqprog_t* p, swqreg q, swangle a); 

extern void gate_cx(swqprog_t* p, swqreg q1, swqreg q2);
extern void gate_cz(swqprog_t* p, swqreg q1, swqreg q2);
extern void gate_swap(swqprog_t* p, swqreg q1, swqreg q2);

extern void gate_p(swqprog_t* p, swqreg q1, swqreg q2, swangle a);

extern void gate_measure(swqprog_t* p, swqreg q1, int c);
#endif
