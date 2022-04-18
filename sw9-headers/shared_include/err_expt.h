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
				"���Խ��쳣",
				"�Ƿ�ָ��:ָ��Ĳ�����Ϊ�����Ĳ�����",
				"�Ƿ�ָ��:ָ��Ĺ�����Ϊ�����Ĺ�����",
				"�Ƿ�ָ��:����ָ�����ñ����Ĵ����ļ�",
				"�Ƿ�ָ��:RCSR��WCSRָ���е����������Ϸ�",
				"�������",
				"�Ǿ�ȷ���",
				"����",
				"����",
				"����Ϊ0",
				"��Ч����",
				"�ǹ����",
				"�����ӷ����㣨ADDL��SUBL��ADDW��SUBW�������Ľ��������Ŀ��Ĵ���������ʾ��Χ",
				"�����˷����㣨MULW��MULL�������Ľ��������Ŀ��Ĵ���������ʾ��Χ",
				"512λ�����Ӽ�������Ľ���λ�Ĵ������,�����˽���λ�Ĵ���������ʾ��Χ��-128~+127��",
				"�Ӻ˵�24λȡָSPC��Ϊ��������ת,�������",
				"����ڼ���SPC+4ʱ����SPC���,��JMPָ���Ŀ��PC[47:24]�뱾ָ��GPC[47:24]��һ��,������PC����쳣",
				"�������������쳣��Class_stָ����з���ʱ,����������������ܱ�ʾ�ķ�Χʱ�������쳣",
				"������������쳣��Class_wrָ����з����������ʱ������Ӳ���޷�����ķǷ�����ֵʱ�������쳣",
				"HASH_BUF���ʲ����ڵĻ���,�������üĴ���������Ϊ���ۼӡ�������ָ������������üĴ����е����Ͳ�ƥ��",
				"�Ӻ�ͨ��vlenmasָ�����LDM������,������ַԽ",
				"�Ӻ�LD/STָ���CLASS_STָ�����LDM˽�пռ������,������ַԽ",
				"�Ӻ�ԭ�Ӳ���ָ���DCache����ָ��ԽȨ����LDM˽�п�",
				"�Ӻ˷�LDM˽�пռ���ʵ����ַ[47:45]�Ƿ�",
				"�Ӻ�LD/STָ�����LDM����ռ��ַԽ��",
				"�Ӻ�ԭ�Ӳ���ָ��ԽȨ����LDM����ռ�������Cache�ռ�",
				"�Ӻ�DCacheδʹ��ʱ���������Cache�ռ�",
				"�Ӻ�1B/2B����LD/ST��дԽȨ����LDM����ռ�",
				"DCacheװ�����ڷ�DCache�ռ�",
				"DMA/RMA����LDM�ռ��ַԽ��",
				"Զ��RLD/RST���ʱ��Ӻ�LDM�ռ��ַԽ��",
				"�Ӻ˷����棨LD/ST��DMA��ָ���ѰУ�������SLB����ʱ����ԽȨ��Խ��",
				"�Ӻ˷����棨LD/ST��DMA��������,��������ĵ�ַ��������ʵ�����õ��������������ʱ�����������쳣",
				"DCacheװ�д�����Ͷ����������в������쳣",
				"ϵͳ�ӿ�����Ľ���ַԽ���������ƺ��ķ��Ʋ�����Խ��",
				"SLB����ʱ����ԽȨ��Խ��,�������ַ�ռ����Բ��Ϸ�",
				"��������ĵ�ַ��������ʵ�����õ��������������ʱ�����������쳣",
				"ԭ��ȡ����1����ָ����ִ�й�����,����������������ʾ��Χ",
				"ԭ�Ӹ��£�����ԭ�Ӽ�n��ԭ�Ӽ�n������ָ����ִ�й�����,����������������ʾ��Χ",
				"DMA/RMA�ش�������1�쳣",
				"�Ӻ˷������С���ͬ�������в��������Ӻ�",
				"��ο�dma����쳣��",
				"�Ӻ����л��ش��ڲ��ɱ����ʵ�״̬ʱ,��IO���󷢸���Ӧ����",
				"���ʱ�����IO�ռ��ַ",
				"����IO�ռ�������쳣,��ֻ����ֻд��IOR������ԽȨ����",
				"���û����򣨻���ͨ��IO���е�΢�����ô����󣩵�Rank��ַ������Ԥ�����õ����Rank��",
				"�Ѱ�װ��Я���쳣��ʶ",
				"�Ӻ˴ع����������쳣,��tbox,rbox,scmt�������쳣"
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

