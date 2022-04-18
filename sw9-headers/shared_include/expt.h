//===================================
//	Define the internal signo	
//===================================
enum {
        SW3_SIGSYSC = 1,        /*1: host side syscall agent */
        SW3_SIGHALT,            /*2: slave core group halt */
        SW3_SIGEND,             /*3: slave core end */
	SW3_SIGERR,		/*4: slave error report */
	SW3_SIGPRINT = 11,	/*11: for parallel*/
	SW3_SIGEXPT = 62,	/*62: for exception */
        SW3_SIGMAX = 63,        /*63: core signal max number */
};

#ifndef WANGF20110117
//===================================
//	Define the exception signo
//===================================
//===================================
//	Revised at 2012-11-21
//===================================

#define SIGEXPT 55

enum Exception_Kind {
        UNALIGN = 0,
        ILLEGAL,
        OVI,
	INE,
        UNF,
	OVF,
	DBZ,
        INV,
	DNO,
	CLASSE1,
	CLASSE2,
        SELLDWE,
	LDME,
	SDLBE,
	MABC,
	MATNO,
	RAMAR,
        IFLOWE,
        SBMDE1,
        SBMDE2,
        SYNE1,
        SYNE2,
	RCE,
	DMAE1,
	DMAE2,
	DMAE3,
	DMAE4,
	DMAE5,
        IOE1,
        IOE2,
        IOE3,
	OTHERE,
	_SYS_NSIG,
};

typedef struct slave_expt {
    int coreno;  //记录的从核号
    unsigned long expt_vector;	    //异常向量
    unsigned long expt_pc;	    //异常pc
    unsigned long dma_expt_type;    //dma异常类型
    unsigned long dma0;		    //dma描述符0
    unsigned long dma1;		    //dma描述符1
    unsigned long dma2;		    //dma描述符2
    unsigned long dma3;		    //dma描述符3
    unsigned long tc_sdlb_err_spot; //如果是sdlb异常的化，此位有效
    unsigned long reserve_data;	    //预留的数据位
} slave_expt;

#endif

