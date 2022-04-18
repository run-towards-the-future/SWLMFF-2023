#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#ifdef FPGA_TEST
#include <athread.h>
#else
#include <pthread.h>
#endif
#define SIG_NUM 48
extern unsigned long __core_spawn_mask[CG_NUM];
//#define  __uncached __attribute__ ((section (".uncached_rw"))) 
extern thread_info_t __v_core_info[CG_NUM][MAX_CORES];
//extern unsigned long __still_living_mask;
typedef void (*sighandler_t)(int);

void myprintf(char* fmt,...)
{

	va_list args;
        char    buffer[2048];

#ifdef USE_MPI
	if(m_id != 0)	return;
#else
	if(phy_cgid != 0)   return;
#endif
	va_start(args,fmt);
        vsprintf(buffer,fmt,args);
	va_end(args);
        printf(buffer);
        fflush(stdout);
}



#ifdef FPGA_TEST
void Dismiss_Core(int coreid)
{
    	unsigned long  id_mask,ret;
//	unsigned long cur_still_living_mask = __still_living_mask;
	unsigned long cur_still_living_mask[phy_cgid];
	//sys_m_dismiss(-1,1ULL<<coreid,0,-1);
    
	#if 0
		athread_cancel( coreid );
//	#else
		id_mask = 1ULL<<coreid;
		ret=sys_m_stop((int)-1,id_mask,NULL);
		//printf("ret of sys_m_stop == %d\n",ret);
		
		ret=sys_m_reset((int)-1,id_mask,NULL);
		//printf("ret of sys_m_reset == %d\n",ret);
		
		ret=sys_m_run((int)-1,id_mask,&slave_start,NULL);
		//printf("ret of sys_m_run == %d\n",ret);
	#endif
	struct scoremask mask;
	struct stask_run_score run;
	memset(&mask,0,sizeof (mask));
	memset(&run,0,sizeof (run));
	id_mask = 1ULL<<coreid;
	mask.coremask[phy_cgid]=id_mask;
	run.scoremask.coremask[phy_cgid]=id_mask;
	ret  = ioctl(stask_fd, STASK_HALT_SCORE, &mask);
	if (ret != 0) {
	        printf("halt score failed, errno = %d.\n",errno);
		exit(-1);
	}
	ret  = ioctl(slave_fd, SLAVE_RESET_SCORE, &mask);
	if (ret != 0) {
	        printf("reset score failed, errno = %d.\n",errno);
		exit(-1);
	}
	ret  = ioctl(stask_fd, STASK_RUN_SCORE, &run);
	if (ret != 0) {
	        printf("run stask score failed, errno = %d.\n",errno);
		exit(-1);
	}
	cur_still_living_mask[phy_cgid] = __core_spawn_mask[phy_cgid];
	//printf("__core_spawn_mask[0]=%#lx\n",__core_spawn_mask[0]);fflush(NULL);
	cur_still_living_mask[phy_cgid] &=  ~(1ULL<<coreid);
//	g_coremask_adjust=cur_still_living_mask;

//	__set_still_living_mask ( cur_still_living_mask );
	__core_spawn_mask[phy_cgid] = cur_still_living_mask[phy_cgid];
	//printf("__core_spawn_mask[0]=%#lx\n",__core_spawn_mask[0]);fflush(NULL);
    
	//exit(0);
}
#endif


void TimeOut_handler(int signum)
{
    unsigned long  i,id,id_mask,ret;
#ifdef FPGA_TEST
    for(i=0;i<MAX_PENUM_CG;i++)
    {
		//#define BitGet(mask,num) ( ((unsigned long)mask >>(num)) & 1ULL )
	    //if( ( BitGet(__id_spawn_mask,i) == 1 ) && ( __v_thread_info[i].state_flag == 1) ) // 1:run, 0:finish
	    if( ( BitGet(__core_spawn_mask[phy_cgid],i) == 1 ) && ( __v_core_info[phy_cgid][i].state_flag == 1) )
	    {
		id =i;
		exception_num++;
		printf("<<EXCEPTION>>:CPU %d,CG %d,CORE %d is EXCEPTION, exception=TIMEOUT\n",phy_cpuid,phy_cgid,id);
		fflush(stdout);
		//Dismiss_Core( id );
            }
     }
#else
    printf("Entering TimeOut_handler......\n");
    fflush(NULL);
    for(i=0;i<MAX_PENUM_CG;i++)
    {
	if(pthread_cancel(thread[i])==0)
	{
	    id = i;
	    printf("<<EXCEPTION>>:CPU %d,CG %d,CORE %d is EXCEPTION, exception=TIMEOUT\n",phy_cpuid,phy_cgid,id);
	}
    }
#endif
    alarm(thresholdtime);
    return;
}

#ifdef FPGA_TEST
handler_t * handler(int signum, siginfo_t *sinfo, struct sigcontext *sigcontext)
{
	char *expt_type[SIG_NUM]=	{
	"UNALIGN",
	"OPCODE0",
	"OPCODE1",
	"OPCODE2",
	"OPCODE3",
	"FPE_IOV",
	"FPE_INE",
	"FPE_OVF",
	"FPE_DZE",
	"FPE_INV",
	"FPE_DNO",
	"OVI0",
	"OVI1",
	"OVI2",
	"OVPC0",
	"OVPC1",
	"CLASS0",
	"CLASS1",
	"HASH_EXPT",
	"ACV0",
	"ACV1",
	"ACV2",
	"ACV3",
	"ACV4",
	"ACV5",
	"ACV6",
	"ACV7",
	"ACV8",
	"ACV9",
	"ACVA",
	"ACVB",
	"ACVC",
	"ACVD",
	"ACVE",
	"IACV0",
	"IACV1",
	"ATOMIC0",
	"ATOMIC1",
	"ATOMIC2",
	"SYN",
	"DMA_EXPT",
	"IOEXPT0",
	"IOEXPT1",
	"IOEXPT2",
	"RANK",
	"OTHER",
	"SPC_EXPT"
				};
	
	char *sw3_exp_sysmsgs[SIG_NUM] = {
				"不对界异常",
				"非法指令:指令的操作码为保留的操作码",
				"非法指令:指令的功能码为保留的功能码",
				"非法指令:向量指令引用标量寄存器文件",
				"非法指令:RCSR、WCSR指令中的立即数不合法",
				"整数溢出",
				"非精确结果",
				"下溢",
				"上溢",
				"除数为0",
				"无效操作",
				"非规格化数",
				"整数加法运算（ADDL、SUBL、ADDW、SUBW）产生的结果超出了目标寄存器的最大表示范围",
				"整数乘法运算（MULW、MULL）产生的结果超出了目标寄存器的最大表示范围",
				"512位整数加减法运算的进借位寄存器结果,超出了进借位寄存器的最大表示范围（-128~+127）",
				"从核的24位取指SPC因为递增或跳转,导致溢出",
				"如果在计算SPC+4时发生SPC溢出,或JMP指令的目标PC[47:24]与本指令GPC[47:24]不一致,将发生PC溢出异常",
				"分类计数器溢出异常：Class_st指令进行分类时,分类计数器超出所能表示的范围时发生该异常",
				"分类参数设置异常：Class_wr指令进行分类参数设置时出现了硬件无法处理的非法设置值时发生该异常",
				"HASH_BUF访问不存在的缓冲,或者配置寄存器的类型为“累加”；编码指令的类型与配置寄存器中的类型不匹配",
				"从核通过vlenmas指令访问LDM过程中,发生地址越",
				"从核LD/ST指令或CLASS_ST指令访问LDM私有空间过程中,发生地址越",
				"从核原子操作指令或DCache管理指令越权访问LDM私有空",
				"从核非LDM私有空间访问的虚地址[47:45]非法",
				"从核LD/ST指令访问LDM共享空间地址越界",
				"从核原子操作指令越权访问LDM共享空间或主存可Cache空间",
				"从核DCache未使能时访问主存可Cache空间",
				"从核1B/2B粒度LD/ST读写越权访问LDM共享空间",
				"DCache装填落在非DCache空间",
				"DMA/RMA访问LDM空间地址越界",
				"远程RLD/RST访问本从核LDM空间地址越界",
				"从核访主存（LD/ST或DMA和指令脱靶）过程中SLB代换时发生越权或越界",
				"从核访主存（LD/ST、DMA）过程中,访问主存的地址超出主存实际配置的容量或主存访问时发生界标溢出异常",
				"DCache装填、写操作和读操作过程中产生的异常",
				"系统接口请求的界标地址越界或运算控制核心非推测请求越界",
				"SLB代换时发生越权或越界,包括虚地址空间属性不合法",
				"访问主存的地址超出主存实际配置的容量或主存访问时发生界标溢出异常",
				"原子取并加1操作指令在执行过程中,运算结果超出了最大表示范围",
				"原子更新（包括原子加n和原子减n）操作指令在执行过程中,运算结果超出了最大表示范围",
				"DMA/RMA回答字自增1异常",
				"从核发出的行、列同步向量中不包含本从核",
				"请参考dma检查异常表",
				"从核阵列或存控处于不可被访问的状态时,有IO请求发给对应部件",
				"访问保留的IO空间地址",
				"访问IO空间的属性异常,对只读或只写的IOR进行了越权操作",
				"当用户程序（或者通过IO进行的微操作访存请求）的Rank地址超过了预先配置的最大Rank号",
				"脱靶装填携带异常标识",
				"从核簇公共管理部件异常,如tbox,rbox,scmt部件的异常"
				};

#if 1
	int size_sp;
	char expt_buf[256];

	memset(expt_buf,0,sizeof(expt_buf));
	size_sp=sprintf(expt_buf,"<<EXCEPTION>> CPU=%d, CG=%d, core=%d, exception=%s(%s),pc:=%#lx,address=%#lx\n",phy_cpuid,sinfo->si_uid,sinfo->si_pid,expt_type[signum],sw3_exp_sysmsgs[signum],sigcontext->sc_pc,sinfo->si_ptr);
	//write(1,expt_buf,sizeof(expt_buf));
	write(1,expt_buf,size_sp);
	fflush(stdout);

#endif	
#if 0
	expt_printf("<<EXCEPTION>> CPU=%d, ",phy_cpuid);
	expt_printf("CG=%d, ",sinfo->si_uid);
	expt_printf("core=%d, ",sinfo->si_pid);
	expt_printf("exception=%s",expt_type[signum]);
	expt_printf("(%s), ",sw3_exp_sysmsgs[signum]);
	expt_printf("pc:=%#lx, ",sigcontext->sc_pc);
	expt_printf("address=%#lx \n",sinfo->si_ptr);
#endif 

	//printf("<<EXCEPTION>> CPU=%d, CG=%d, core=%d, exception=%s(%s),pc:=%#lx,address=%#lx\n",phy_cpuid,sinfo->si_uid,sinfo->si_pid,expt_type[signum],sw3_exp_sysmsgs[signum],sigcontext->sc_pc,sinfo->si_ptr);

	exception_num++;
	//Dismiss_Core(sinfo->si_pid);	
	
}
#endif
#if 0
void *read_slave_expt_info()
{
	int i;
	slave_expt_t *s_expt;
	s_expt = (slave_expt_t *)calloc(sizeof(slave_expt_t),1);
	for(i=0;i<MAX_PENUM_CG;i++)
		sys_m_get_expt_inf(master_core_id,s_expt,i,0,0);	
	
	return;
}
#endif

#ifdef FPGA_TEST
/*illegal signal handler here*/
void signal_handler(int signum,siginfo_t *sinfo, void *sigcontext)
{
	struct sigcontext *sc_p = (struct sigcontext *)sigcontext;

	printf("<<EXCEPTION>> CPU=%d, CG=%d, MASTER ",phy_cpuid,phy_cgid);
	exception_num++;
	switch(signum){
	case	SIGILL:
		printf("exception=SIGILL,pc:=%#lx\n\n",sc_p->sc_pc);
		exit(1);

	case	SIGFPE:
		printf("exception=SIGFPE,pc:=%#lx\n\n",sc_p->sc_pc);
		exit(1);

	case	SIGSYS:
		printf("exception=SIGSYS,pc:=%#lx\n\n",sc_p->sc_pc);
		exit(1);

	case	SIGSEGV:
		printf("exception=SIGSEGV,pc:=%#lx\n\n",sc_p->sc_pc);
		exit(1);

	case	SIGBUS:
		printf("exception=SIGBUS,pc:=%#lx\n\n",sc_p->sc_pc);
		exit(1);

	default:
		printf("exception=other-EXCEPTION,pc:=%#lx\n\n",sc_p->sc_pc);
		exit(1);
	}
}

/*init signal*/
void signal_catch()
{
	if(signal(SIGILL,(sighandler_t)signal_handler)==SIG_ERR)
	{
		perror("can't set handler for SIGILL\n");
		exit(1);
	}
	if(signal(SIGFPE,(sighandler_t)signal_handler)==SIG_ERR)
	{
		perror("can't set handler for SIGFPE\n");
		exit(1);
	}
	if(signal(SIGSYS,(sighandler_t)signal_handler)==SIG_ERR)
	{
		perror("can't set handler for SIGSYS\n");
		exit(1);
	}
	if(signal(SIGSEGV,(sighandler_t)signal_handler)==SIG_ERR)
	{
		perror("can't set handler for SIGSEGV\n");
		exit(1);
		return;
	}
	if(signal(SIGBUS,(sighandler_t)signal_handler)==SIG_ERR)
	{
		perror("can't set handler for SIGBUS\n");
		exit(1);
	}
}

/*master core exception catch*/
void master_expt()
{
	signal_catch();
}
#endif 

