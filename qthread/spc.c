#ifdef __sw_host__
#include <athread.h>
#include <signal.h>
#include <unistd.h>
extern int sys_m_cgid();
extern void sys_m_get_expt_inf(int cgid, void* user_addr, int coreno, int io_reg, int flags);
extern long *__qstat_addr;
static char fopen_mode[2] = {'w', '\0'};
#define BITS(in, start, end) ((in) >> start & ((1L << ((end) - (start))) - 1))
static void print_banner(FILE *spotfile, char *s){
  int len = strlen(s);
  int left_spacing = 39 - (len >> 1);
  int right_spacing = 39 - (len + 1 >> 1);
  fprintf(spotfile, "+------------------------------------------------------------------------------+\n");
  fprintf(spotfile, "|%*s%s%*s|\n", left_spacing, "", s, right_spacing, "");
  fprintf(spotfile, "+------------------------------------------------------------------------------+\n");
}

long spc_cgid;
extern long *__qstat_addr;;
#define IO_GET(addr) (*(long*)(addr))
#define CPE_BASE_ADDR(cpe_id) (0x8003000000L | spc_cgid << 36L | cpe_id << 16L)
#define CPE_REG_ADDR(rnum, quater) (0x4000L | (rnum << 7L) | (quater << 12L))
//#define extract_bits(long )
#define CPE_PC_ADDR 0x2000L
#define CPE_RUN     0x2080L
#define CPE_P0RO    0x0580L
#define CPE_P1RO    0x0600L

inline long get_cpe_pc(int cpe_id){
  return IO_GET(CPE_BASE_ADDR(cpe_id) | CPE_PC_ADDR);
}
#define TC_BASE_ADDR      (0x8004000000L | spc_cgid << 36L)
#define TC_SDLB_ERR_PEVEC      0x308000L
#define TC_SDLB_ERR_SPOT       0x308080L
#define TC_DDMA_SCAN           0x200100L

#define GC_BASE_ADDR      (0x8005000000L | spc_cgid << 36L)
#define GC_DMACHK_TYPE         0x200700L
#define GC_DMACHK_FIELD0       0x200780L
#define GC_DMACHK_FIELD1       0x200800L
#define GC_DMACHK_FIELD2       0x200880L
#define GC_EX                  0x200000L

//cuts bits from in, right exclusive.

char *BOOL_STR[] = {"no", "yes"};
char *SDLB_REQ_TYPE_STR[] = {"read", "write", "faa", "updt+", "updt-"};
char *SDLB_REQ_SRC_STR[] = {"cpe", "ibox", "dma"};

void decode_sdlb_err(FILE *spotfile){
  long spot = IO_GET(TC_BASE_ADDR | TC_SDLB_ERR_SPOT);
  long reqtype = BITS(spot, 0, 4);
  long reqaddr = BITS(spot, 4, 39);
  long src_pe = BITS(spot, 40, 46);
  long grain = BITS(spot, 46, 48);
  long src_id = BITS(spot, 59, 61);
  long out_of_range = BITS(spot, 61, 62);
  long out_of_perm = BITS(spot, 62, 63);

  fprintf(spotfile, "TC_SDLB_ERR_SPOT: %lx\n", spot);
  fprintf(spotfile, "REQ_TYPE: %s\n", SDLB_REQ_TYPE_STR[reqtype]);
  fprintf(spotfile, "TC_SDLB_REQ_ADDR: %lx\n", reqaddr << 4);
  fprintf(spotfile, "SRC_PE: %d\n", src_pe);
  fprintf(spotfile, "GRAIN: %d\n", 1 << grain);
  fprintf(spotfile, "SRC_TYPE: %s\n", SDLB_REQ_SRC_STR[src_id - 1]);
  fprintf(spotfile, "OUT_OF_RANGE: %s\n", BOOL_STR[out_of_range]);
  fprintf(spotfile, "OUT_OF_PERM: %s\n", BOOL_STR[out_of_perm]);
}

char *DMA_ERR_STR[] = {
  "LDM unaligned",
  "MEM unaligned",
  "size unaligned",
  "bsize unaligned",
  "stride unaligned",
  "GET_P/PUT_P occur",
  "size non positive",
  "",
  "invalid OP",
  "invalid MODE",
  "invalid OP+MODE",
  "reply overflow/unaligned",
  "LDM overflow",
  "bcast vec=0",
  "not connected"
};

char *DMA_OP_STR[] = {
  "DMA_PUT",
  "DMA_GET",
  "DMA_PUT_P",
  "DMA_GET_P",
  "",
  "DMA_BARRIER"
};

char *DMA_MODE_STR[] = {
  "PE_MODE",
  "BCAST_MODE",
  "ROW_MODE",
  "BROW_MODE",
  "RANK_MODE"
};


void decode_dma_err(FILE *spotfile){
  long chk = IO_GET(GC_BASE_ADDR | GC_DMACHK_TYPE);
  long src_pe = BITS(chk, 16, 22);
  long field0 = IO_GET(GC_BASE_ADDR | GC_DMACHK_FIELD0);
  long field1 = IO_GET(GC_BASE_ADDR | GC_DMACHK_FIELD1);
  long field2 = IO_GET(GC_BASE_ADDR | GC_DMACHK_FIELD2);
  long size = BITS(field0, 0, 24);
  long bsize = BITS(field0, 24, 44);
  long op = BITS(field0, 44, 48);
  long mode = BITS(field0, 48, 52);
  long reply_addr = BITS(field0, 52, 64) | BITS(field1, 0, 4) << 12;
  long bcast_mask = BITS(field1, 4, 12);
  long stride = BITS(field1, 12, 44);
  long mem_addr = BITS(field1, 44, 64) | BITS(field2, 0, 20) << 20;
  long ldm_addr = BITS(field2, 20, 36);
  long pc = BITS(field2, 36, 58);

  fprintf(spotfile, "DMACHK_FIELD0: %x\n", field0);
  fprintf(spotfile, "DMACHK_FIELD1: %x\n", field1);
  fprintf(spotfile, "DMACHK_FIELD2: %x\n", field2);
  long type;
  for (type = 0; type <= 14; type ++)
    if (1 << type & chk)
      fprintf(spotfile, "ERR_DETECTED: %s\n", DMA_ERR_STR[type]);
  fprintf(spotfile, "SRC_PE: %d\n", src_pe);
  fprintf(spotfile, "TRASFER_SIZE: %d (0x%x)\n", size, size);
  fprintf(spotfile, "BSIZE: %d (0x%x)\n", bsize, bsize);
  fprintf(spotfile, "OP: %s\n", DMA_OP_STR[op]);
  fprintf(spotfile, "MODE: %s\n", DMA_MODE_STR[mode]);
  fprintf(spotfile, "REPLY_ADDR: %x\n", reply_addr);
  fprintf(spotfile, "BCAST_MASK: %x\n", bcast_mask);
  fprintf(spotfile, "STRIDE: %d (0x%x)\n", stride, stride);
  fprintf(spotfile, "MEM_ADDR: %lx\n", mem_addr);
  fprintf(spotfile, "LDM_ADDR: %x\n", ldm_addr);
  fprintf(spotfile, "DMA_PC[24:2]: 0x%x\n", pc);
}

char *P0_BLOCK_REASON_STR[] = {
  "src not ready or dst not OK",
  "GPR write port conflict",
  "instruction barrier",
  "channel full"
};

char *P1_BLOCK_REASON_STR[] = {
  "src not ready or dst not OK",
  "GPR write port conflict",
  "instruction barrier",
  "SBMD buffer full",
  "memory access blocked",
  "MEMB barrier",
  "GETC no data",
  "GETR no data",
  "PUTR/PUTC buffer full",
  "RCSR/WCSR blocked"
};

void decode_reg_comm(FILE *spotfile){
  int i, j;
  for (i = 0; i < 8; i ++){
    for (j = 0; j < 8; j ++){
      long p1ro = IO_GET(CPE_BASE_ADDR(i * 8 + j) | CPE_P1RO);
      char *stuck_str = "";
      if (BITS(p1ro, 56, 57) && BITS(p1ro, 62, 64) == 3){
        stuck_str = "GETC";
      }
      if (BITS(p1ro, 57, 58) && BITS(p1ro, 62, 64) == 3){
        stuck_str = "GETR";
      }
      if (BITS(p1ro, 58, 59) && BITS(p1ro, 62, 64) == 3){
        stuck_str = "PUT";
      }
      fprintf(spotfile, "%3d: %4s", i * 8 + j, stuck_str);
    }
    fprintf(spotfile, "\n");
  }
}
char *REG_NAME_STR[] = {" $0(  v0  )",
                        " $1(  t0  )",
                        " $2(  t1  )",
                        " $3(  t2  )",
                        " $4(  t3  )",
                        " $5(  t4  )",
                        " $6(  t5  )",
                        " $7(  t6  )",
                        " $8(  t7  )",
                        " $9(  s0  )",
                        "$10(  s1  )",
                        "$11(  s2  )",
                        "$12(  s3  )",
                        "$13(  s4  )",
                        "$14(  s5  )",
                        "$15(s6/fp )",
                        "$16(  a0  )",
                        "$17(  a1  )",
                        "$18(  a2  )",
                        "$19(  a3  )",
                        "$20(  a4  )",
                        "$21(  a5  )",
                        "$22(  t8  )",
                        "$23(  t9  )",
                        "$24( t10  )",
                        "$25( t11  )",
                        "$26(  ra  )",
                        "$27(t12/pv)",
                        "$28(  at  )",
                        "$29(  gp  )",
                        "$30(  sp  )",
                        "$31( zero )"};
typedef union reg_data_t{
  double f[4];
  long l[4];
  int i[4];
} reg_data_t;

void decode_cpe_regs(FILE *spotfile){
  int i, j;
  for (i = 0; i < 64; i ++){
    fprintf(spotfile, "CPE[%02d]:\n", i);
    for (j = 0; j < 32; j ++){
      reg_data_t r;
      IO_GET(CPE_BASE_ADDR(i) | CPE_RUN) = 3;
      r.l[0] = IO_GET(CPE_BASE_ADDR(i) | CPE_REG_ADDR(j, 0));
      r.l[1] = IO_GET(CPE_BASE_ADDR(i) | CPE_REG_ADDR(j, 1));
      r.l[2] = IO_GET(CPE_BASE_ADDR(i) | CPE_REG_ADDR(j, 2));
      r.l[3] = IO_GET(CPE_BASE_ADDR(i) | CPE_REG_ADDR(j, 3));
      IO_GET(CPE_BASE_ADDR(i) | CPE_RUN) = 0;
      fprintf(spotfile, "  %s:\n", REG_NAME_STR[j]);
      fprintf(spotfile, "    hex           : %016lx%016lx%016lx%016lx\n"
          , r.l[3], r.l[2], r.l[1], r.l[0]);
      fprintf(spotfile, "    4B mark       : %8d%8d%8d%8d%8d%8d%8d%8d\n", 7, 6, 5, 4, 3, 2, 1, 0);
      fprintf(spotfile, "    8B mark       : %16d%16d%16d%16d\n", 3, 2, 1, 0);
      fprintf(spotfile, "    doublev4_value: %e, %e, %e, %e\n", r.f[0], r.f[1], r.f[2], r.f[3]);
      fprintf(spotfile, "    longv4_value  : %ld, %ld, %ld, %ld\n", r.l[0], r.l[1], r.l[2], r.l[3]);
      fprintf(spotfile, "    intv8_value   : %d, %d, %d, %d, %d, %d, %d, %d\n"
          , r.i[0], r.i[1], r.i[2], r.i[3]
          , r.i[4], r.i[5], r.i[6], r.i[7]);
    }
    fflush(spotfile);
  }
}

int inst_is_halt(int inst){
  return (inst >> 26) == 0x6 && (inst & 0xff) == 0x80;
}

void wake_cpes(){
  int i;
  for (i = 0; i < 64; i ++){
    long pc = IO_GET(CPE_BASE_ADDR(i) | CPE_PC_ADDR);
    int inst = IO_GET(pc);
    if (inst_is_halt(inst)){
      printf("Waking up CPE %d!\n", i);
      IO_GET(CPE_BASE_ADDR(i) | CPE_PC_ADDR) = pc + 4;
      IO_GET(CPE_BASE_ADDR(i) | CPE_RUN) = 0;
    }
  }
}

char *MESSAGES[] = {
  "0. ",
  "1. ",
  "2. ",
  "3. "
};

void show_messages(FILE *spotfile){
  int i;
  for (i = 0; i < 4; i ++){
    fprintf(spotfile, "%s\n", MESSAGES[i]);
    fflush(spotfile);
  }
}

enum enabled_decode {
  DECODE_PC   = 1,
  DECODE_RC   = 2,
  DECODE_SDLB = 4,
  DECODE_DMA  = 8,
  DECODE_REG  = 16,
  DECODE_ALL  = 31
};

void decode_cpe_spot(FILE *spot, int things){
  /* print_banner(spot, "Donate this project to put your messages here. Contact: dxhisboy@126.com"); */
  /* show_messages(spot); */

  print_banner(spot, "Decoding CPE spots");
  fprintf(spot, "Last function spawned %llx\n", __qstat_addr[0]);

  if (DECODE_PC & things) {
    print_banner(spot, "Decode of CPE PCs");
    int i, j;
    for (i = 0; i < 8; i ++) {
      for (j = 0; j < 8; j ++)
        fprintf(spot, "%3d: %11llx ", i * 8 + j, IO_GET(CPE_BASE_ADDR(i * 8 + j) | CPE_PC_ADDR));
      fprintf(spot, "\n");
    }
    fflush(spot);
  }

  if (DECODE_RC & things) {
    print_banner(spot, "Decode of register commiuncation status");
    decode_reg_comm(spot);
  }

  if (DECODE_SDLB & things) {
    print_banner(spot, "Decode of SDLB error spot");
    decode_sdlb_err(spot);
  }

  if (DECODE_DMA & things){
    print_banner(spot, "Decode of DMA error spot");
    decode_dma_err(spot);
  }

  if (DECODE_REG & things){
    print_banner(spot, "Decode of CPE registers");
    decode_cpe_regs(spot);
  }
  fprintf(spot, "\n\n");
}
void pc_on_sig(int sig){
  char fname[1024];
  char hname[1024];
  char cname[1024];
  getcwd(cname, 1024);
  gethostname(hname, 1024);
  sprintf(fname, "%s/%s_cg%d_cpe_spot.txt", cname, hname, spc_cgid);
  printf("\x1b[33m(NODE=%s, CG=%d) wrote CPE spots to %s due to signal %d\x1b[0m\n", hname, spc_cgid, fname, sig);
  FILE *spot = fopen(fname, fopen_mode);
  if (fopen_mode[0] == 'w') fopen_mode[0] = 'a';
  decode_cpe_spot(spot, DECODE_ALL);

  if (sig == 31)
    wake_cpes();
  fclose(spot);
}

#include <ucontext.h>
static void detect_expt(char *ret, long expt, int try, const char *expt_str) {
  if (expt & (1L << try)) {
    if (*ret){
      strcat(ret, ", ");
    }
    strcat(ret, expt_str);
  }
}
void stringify_exceptions(char *ret, long expt){
  *ret = 0;
  detect_expt(ret, expt, UNALIGN, "Unaligned Access" );
  detect_expt(ret, expt, ILLEGAL, "Illegal Instruction" );
  detect_expt(ret, expt, OVI    , "Integer Overflow");
  detect_expt(ret, expt, INE    , "Inexact Result");
  detect_expt(ret, expt, UNF    , "Float Underflow");
  detect_expt(ret, expt, OVF    , "Float Overflow");
  detect_expt(ret, expt, DBZ    , "Divided by Zero");
  detect_expt(ret, expt, INV    , "Invalid Operation");
  detect_expt(ret, expt, DNO    , "Denormalized Number");
  detect_expt(ret, expt, CLASSE1, "Classify Counter Overflow");
  detect_expt(ret, expt, CLASSE2, "Classify Argument Exception");
  detect_expt(ret, expt, SELLDWE, "SELLDW Exception");
  detect_expt(ret, expt, LDME   , "LDM Access Exception");
  detect_expt(ret, expt, SDLBE  , "SDLB Transform Exception");
  detect_expt(ret, expt, MABC   , "Memory Access Out of Range");
  detect_expt(ret, expt, MATNO  , "Target CG not Present");
  detect_expt(ret, expt, RAMAR  , "Error Memory Access Response");
  detect_expt(ret, expt, IFLOWE , "Reserved0");
  detect_expt(ret, expt, SBMDE1 , "SBMD Excpetion (Match/Query)");
  detect_expt(ret, expt, SBMDE2 , "SBMD Exception (Other)");
  detect_expt(ret, expt, SYNE1  , "Sync Exception (No Self)");
  detect_expt(ret, expt, SYNE2  , "Sync Exception (Demotion Disalbed)");
  detect_expt(ret, expt, RCE    , "Communication Exception");
  detect_expt(ret, expt, DMAE1  , "DMA Descriptor Examination Error");
  detect_expt(ret, expt, DMAE2  , "DMA Descriptor Examination Warning");
  detect_expt(ret, expt, DMAE3  , "Reserved1");
  detect_expt(ret, expt, DMAE4  , "Rank mode DMA out of LDM Range");
  detect_expt(ret, expt, DMAE5  , "Reserved2");
  detect_expt(ret, expt, IOE1   , "IO of Reserved Address");
  detect_expt(ret, expt, IOE2   , "IO Out of Range");
  detect_expt(ret, expt, IOE3   , "IO Unaccessable");
  detect_expt(ret, expt, OTHERE , "Other Excpetion");

}
void cpe_err_handler(int sig, siginfo_t *info, void *vcontext){
  ucontext_t *context = vcontext;
  char fname[1024];
  char hname[1024];
  char cname[1024];
  getcwd(cname, 1024);
  gethostname(hname, 1024);
  sprintf(fname, "%s/%s_cg%d_cpe_spot.txt", cname, hname, spc_cgid);
  //printf("\x1b[33m(NODE=%s, CG=%d) wrote CPE spots to %s due to signal %d\x1b[0m\n", hname, cgid, fname, sig);

  long coremask = info->si_errno | (long)info->si_pid << 32L;
  long things = DECODE_PC;
  char expt_str[1024];
  long expt_vector_total = 0;
  struct slave_expt cg_expt, pe_expt[64];
  if (info->si_uid == 0xbad){
    sys_m_get_expt_inf(sys_m_cgid(), &cg_expt, 0xff, 0, 0);
    expt_vector_total |= cg_expt.expt_vector;
    if (cg_expt.expt_vector & (31 << DMAE1)) things |= DECODE_DMA;
    if (cg_expt.expt_vector & (31 << SDLBE)) things |= DECODE_SDLB;
  }
  if (coremask) {
    int i;
    for (i = 0; i < 64; i ++)
      if ((1L << i) & coremask) {
        sys_m_get_expt_inf(sys_m_cgid(), pe_expt + i, i, 0, 0);
        expt_vector_total |= pe_expt[i].expt_vector;
      }
  }
  stringify_exceptions(expt_str, expt_vector_total);
  printf("\x1b[33m(NODE=%s, CG=%d) wrote CPE spots to %s due to signal %d (%s)\x1b[0m\n", hname, spc_cgid, fname, sig, expt_str);

  FILE *spot = fopen(fname, fopen_mode);
  if (fopen_mode[0] == 'w') fopen_mode[0] = 'a';
  print_banner(spot, "CPE Exception Encountered");
  if (info->si_uid == 0xbad){
    if (cg_expt.expt_vector & (31 << DMAE1)) fprintf(spot, "DMA Exception Happened\n");
    if (cg_expt.expt_vector & (31 << SDLBE)) fprintf(spot, "SDLB Exception Happended\n");
  }

  if (coremask) {
    int i;
    for (i = 0; i < 64; i ++)
      if ((1L << i) & coremask) {
        char pe_expt_str[1024];
        stringify_exceptions(pe_expt_str, pe_expt[i].expt_vector);
        fprintf(spot, "CPE=%d, PC=%p: %s\n", i, pe_expt[i].expt_pc, pe_expt_str);
      }
  }
  decode_cpe_spot(spot, things | DECODE_REG);
  fclose(spot);
  //puts("done");
}

// extern int __real_athread_init();
// int athread_inited = 0, athread_init_ret = -1;;
// int __wrap_athread_init(){
//   if (!athread_inited){
//     athread_init_ret = __real_athread_init();
//   }
//   athread_inited = 1;
//   spc_cgid = sys_m_cgid();
//   signal(SIGUSR1, pc_on_sig);
//   signal(SIGUSR2, pc_on_sig);
//   struct sigaction sa;
//   sigaction(55, NULL, &sa);
//   sa.sa_sigaction = cpe_err_handler;
//   sigaction(55, &sa, NULL);
//   //puts("after sigaction");
//   return athread_init_ret;
// }
// /* extern void __real___expt_handler(int sig, void *b, void *c); */
// /* void __wrap___expt_handler(int sig, long *b, long *c){ */
// /*   pc_on_sig(55); */
// /*   __real___expt_handler(sig, b, c); */
// /* } */
// extern void __real___expt_handler(int sig, void *b, void *c);
// void __wrap___expt_handler(int sig, long *b, long *c){
//   pc_on_sig(55);
//   __real___expt_handler(sig, b, c);
// }

// extern int __real___real_athread_spawn(void *a, void *b);
// void __wrap___real_athread_spawn(void *a, void *b){
//   last_enterance = a;
//   __real___real_athread_spawn(a, b);
// }
#endif
