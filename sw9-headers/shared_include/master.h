#ifdef FPGA_TEST
#include <athread.h>
#else
#include <pthread.h>
#endif
#include "common_para.h"
#include <time.h>
#include "slave_kapi.h"
#include <sys/types.h>
#include <unistd.h>

/*
void cur_time()
{
        char *wday[]={
                "星期天","星期一","星期二","星期三","星期四","星期五","星期六"
        };
        time_t timep;
        struct tm *p;
        time(&timep);
        p = localtime(&timep);
        printf("%d年%02d月%02d日",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday);
        printf("%s%02d:%02d:%02d\n",wday[p->tm_wday],(p->tm_hour),p->tm_min,p->tm_sec);
        fflush(NULL);
}
*/
void cur_time()
{
        char *wday[]={
              "??????","????һ","???ڶ?","??????","??????","??????","??????"                     
        };
        time_t timep;
        struct tm *p;
        time(&timep);
        p = gmtime(&timep);
        printf("%d??%02d??%02d??",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday);
        printf("%s%02d:%02d:%02d\n",wday[p->tm_wday],(p->tm_hour+8),p->tm_min,p->tm_sec);
        fflush(NULL);
}
#ifdef SW7
void atomic_add(volatile unsigned int *addr)
{
#ifdef FPGA_TEST
        volatile unsigned int i;
        __asm__ __volatile__(
                                        "ldw_inc %0,0(%1)\n"
                                        :"=&r"(i)
                                        :"r"(addr)
        );
#endif
}
#endif
void test_atomic_add(unsigned long *addr)
{
	unsigned long master_core_log_id = current_array_id();
	if(master_core_log_id!=0)
	{
		atomic_add(&BEGIN_FLAG[0]);
		while(1){
			if(!BEGIN_FLAG[0])
				break;
		}
	}
	else{
		while(1){
			if(BEGIN_FLAG[0]==cg_num-1)
				break;
		}
		BEGIN_FLAG[0] = 0;
	}
}
unsigned long rtc()
{
#ifdef FPGA_TEST
        unsigned long time,tmp0;
        asm("rtc %0,%1" : "=r"(time),"=r"(tmp0) : );
        return time;
#endif
}
int get_master_id()
{
    int i;
#ifdef FPGA_TEST
    //i=sys_m_cgid();
    i=current_array_id();
#else
    i=0;
#endif

    return i;
}
int get_master_log_id()
{
    int i,j,tmp_cg;
    unsigned long long master_list;
    unsigned long long ret;
#ifdef FPGA_TEST
    //master_list = sys_m_read_cgstat(1);
    struct slave_get_cgstat cgstat;
    memset(&cgstat, 0 ,sizeof(cgstat));
    ret  = ioctl(slave_fd, SLAVE_GET_CGSTAT, &cgstat);
	if (ret != 0) {
        printf("get cgstat faild, errno = %d.\n",errno);
        exit(-1);
	}
	master_list = cgstat.cgstat;
#else
    master_list = 0xf;
#endif

#ifndef MCG
	return 0;
#else
#ifdef FULL
    j=0;
    for(i=0;i<MAX_CGNUM;i++)
	{
		masterlist[i].phy_id = i;
		masterlist[i].log_id = i;
		masterlist[i].valid = 0;
	    if( (master_list>>i)&0x1)
	    {
				masterlist[i].phy_id = i;
				masterlist[i].log_id = j;
				masterlist[i].valid = 1;
				j++;
#ifdef DEBUG
			printf("masterlist[%d].log_id = %d\n",i,masterlist[i].log_id);
			fflush(NULL);
#endif
	    }
    }

    if(masterlist[master_core_id].phy_id == master_core_id)
	i = masterlist[master_core_id].log_id;
#endif
#ifdef UNFULL
    j=0;
    for(i=0;i<MAX_CGNUM;i++)
	{
		*ADDR_MASTERLIST[i][M_PHY_ID] = i;
		*ADDR_MASTERLIST[i][M_LOG_ID] = i;
		*ADDR_MASTERLIST[i][M_VALID] = 0;
	    if( (master_list>>i)&0x1)
	    {
				*ADDR_MASTERLIST[i][M_PHY_ID] = i;
				*ADDR_MASTERLIST[i][M_LOG_ID] = j;
				*ADDR_MASTERLIST[i][M_VALID] = 1;
				j++;
#ifdef DEBUG
			printf("*ADDR_MASTERLIST[%d][M_LOG_ID] = %d\n",i,*ADDR_MASTERLIST[i][M_LOG_ID]);
			fflush(NULL);
#endif
	    }
    }

    if(*ADDR_MASTERLIST[master_core_id][M_PHY_ID] == master_core_id)
	i = *ADDR_MASTERLIST[master_core_id][M_LOG_ID];
#endif
    master_core_log_id=i;
#endif
#ifdef DEBUG
	printf("<<TEST>>:in get_master_log_id,master_core_log_id = %d\n",master_core_log_id);
	fflush(NULL);
#endif
	return i;
}

static inline void mb()
{
#ifdef FPGA_TEST
	asm volatile("memb");
#endif
}

void master_sync(unsigned int master_core)
{
   	volatile static int count = 0;
    int i,j;
    count++;
    mb();
#ifdef FULL
    SYN_M_BEGIN[master_core]++;
#ifdef DEBUG
    for(i=0;i<MAX_CGNUM;i++)
    {
    printf("before master_sync,&count = %#lx,count = %ld,0x%lx:SYN_M_BEGIN[%d] = %d\n",&count,count,&SYN_M_BEGIN[i],i,SYN_M_BEGIN[i]);
    fflush(NULL);
    }
#endif
    while(1){
		j=0;
            for(i = 0; i < MAX_CGNUM; i++){
                    if(SYN_M_BEGIN[i] >= count)
                            j++;
            } 
            if(j == cg_num)
                    break;
    } 
#endif
#ifdef UNFULL
    (*ADDR_SYN_M_BEGIN[master_core])++; 
#ifdef DEBUG
    for(i=0;i<MAX_CGNUM;i++)
    {
    printf("<<TEST>>:in master_sync,count = %ld,0x%lx:*ADDR_SYN_M_BEGIN[%d] = %d\n",count,ADDR_SYN_M_BEGIN[i],i,*ADDR_SYN_M_BEGIN[i]);
    fflush(NULL);
    }
#endif
    while(1){
		j=0;
            for(i = 0; i < MAX_CGNUM; i++){
                    if((*ADDR_SYN_M_BEGIN[i]) >= count)
                            j++;
            } 
            if(j == cg_num)
                    break;
    }
#endif
#ifdef DEBUG
    for(i=0;i<MAX_CGNUM;i++)
    {
    printf("after master_sync,count = %ld,0x%lx:SYN_M_BEGIN[%d] = %d\n",count,&SYN_M_BEGIN[i],i,SYN_M_BEGIN[i]);
    fflush(NULL);
    }
#endif
}
void ms_sync(int id)
{
    volatile static __thread int count = 0;
    int i,j;

    count++;
    mb();
#ifdef FULL
#ifdef DEBUG
    printf("<<TEST>>:in ms_sync(),count = %d,%#lx:SYN_MS_BEGIN[%d] = %d\n",count,&SYN_MS_BEGIN[id],id,SYN_MS_BEGIN[id]);
    fflush(NULL);
#endif
    SYN_MS_BEGIN[id]++;
    mb();
#ifdef DEBUG
    printf("<<TEST>>:in ms_sync(),count = %d,%#lx:SYN_MS_BEGIN[%d] = %d\n",count,&SYN_MS_BEGIN[id],id,SYN_MS_BEGIN[id]);
    fflush(NULL);
#endif
    while(1){
		j=0;
            for(i = 0; i < (MAX_PENUM+MAX_CGNUM); i++){
                    if(SYN_MS_BEGIN[i] >= count)
                            j++;
            }
            if(j == 2)
                    break;
    }
#endif
#ifdef UNFULL
    (*ADDR_SYN_MS_BEGIN[id])++;
#ifdef DEBUG
    printf("<<TEST>>:in ms_sync(),count = %d,%#lx:*ADDR_SYN_MS_BEGIN[%d] = %d\n",count,ADDR_SYN_MS_BEGIN[id],id,*ADDR_SYN_MS_BEGIN[id]);
    fflush(NULL);
#endif
    while(1){
		j=0;
            for(i = 0; i < (MAX_PENUM+MAX_CGNUM); i++){
                    if((*ADDR_SYN_MS_BEGIN[i]) >= count)
                            j++;
            }
            if(j == 2)
                    break;
    }
#endif
}
unsigned long get_master_corestart(void)
{
    unsigned long cg_start;
    unsigned long long ret;
#ifdef FPGA_TEST
    //cg_start = sys_m_read_cgstat(1);
    struct slave_get_cgstat cgstat;
    memset(&cgstat, 0 ,sizeof(cgstat));
    ret  = ioctl(slave_fd, SLAVE_GET_CGSTAT, &cgstat);
	if (ret != 0) {
        printf("get cgstat faild, errno = %d.\n",errno);
        exit(-1);
	}
    cg_start = cgstat.cgstat;
#else
    cg_start = 0xf;
#endif

    return cg_start;    
}
int get_num_groups(void)
{
    int i,count,j,k;
    unsigned long long master_list;
    unsigned long long ret;
#ifdef FPGA_TEST
    struct slave_get_cgstat cgstat;
#endif
    count=0;
#ifdef FPGA_TEST
//    master_list=sys_m_read_cgstat(1);
    memset(&cgstat, 0 ,sizeof(cgstat));
    ret = ioctl(slave_fd, SLAVE_GET_CGSTAT, &cgstat);
	if (ret != 0) {
        printf("get cgstat faild, errno = %d.\n",errno);
        exit(-1);
	}
    master_list = cgstat.cgstat;
#else
    master_list=0xf;
#endif
#ifdef DEBUG
    printf("<<TEST>>:in get_num_groups,master_list = 0x%lx\n",master_list);
    fflush(NULL);
#endif
    for(i=0;i<MAX_CGNUM;i++)
    {
	    if( (master_list>>i)&0x1)
	    {
				count++;
	    }
    }
    
    return count;
}
inline int log_id_to_phy_id_in_cg(unsigned int dimindx,unsigned int id)
{
    int i,j,k;

    k=0;
#ifdef FULL
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if((corelist[dimindx][j].valid == 1) && (corelist[dimindx][j].local_id == id))
            {
	        k=(corelist[dimindx][j].row_id)*MAX_COL_NUM+corelist[dimindx][j].col_id;
                break;
            }
	}
#endif
#ifdef UNFULL
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if(((*ADDR_CORELIST[dimindx][j][VALID])== 1) && ((*ADDR_CORELIST[dimindx][j][LOCAL_ID])== id))
            {
	        k=((*ADDR_CORELIST[dimindx][j][ROW_ID]))*MAX_COL_NUM+(*ADDR_CORELIST[dimindx][j][COL_ID]);
                break;
            }
	}
#endif

    return k;
}
#if 1
void get_phy_addr(int dimindx)
{
	unsigned long slave_phy_id;
#ifdef FPGA_TEST
	struct stask_vaddr_to_paddr vtp;
	unsigned long long ret;
#endif
        slave_phy_id = log_id_to_phy_id_in_cg(dimindx,0);
#ifdef DEBUG
	printf("in get_phy_addr:cgid = %d,slave_phy_id = %d\n",dimindx,slave_phy_id);
	printf("SHARED_BEGIN[%d] = 0x%llx\n",dimindx,SHARED_BEGIN[dimindx]);
#ifdef MCG
	printf("ALL_SHARED_BEGIN[%d] = 0x%llx\n",dimindx,ALL_SHARED_BEGIN[dimindx]);
#endif
#endif
#ifdef FPGA_TEST
#ifdef FULL
	//SHARED_BEGIN_PHY[dimindx] = sys_m_getaddr(dimindx,1,slave_phy_id,SHARED_BEGIN[dimindx]);
    	//memset(&vtp, 0 ,sizeof(vtp));
    	memset(&vtp, 0 , sizeof(struct stask_vaddr_to_paddr));
	vtp.vaddr = (unsigned long)SHARED_BEGIN[dimindx];
	//vtp.pid = getpid();
	vtp.pid = 0;//??ȡ?????̵?paddr?????԰?pid????0
	ret  = ioctl(stask_fd, STASK_VADDR_TO_PADDR, &vtp);
	if (ret == -1 || vtp.paddr == 0) {
        printf("vaddr_to paddr failed, errno = %d.\n",errno);
        exit(-1);
	}
	SHARED_BEGIN_PHY[dimindx] = (unsigned long *)(vtp.paddr);
#ifdef DEBUG
	printf("SHARED_BEGIN_PHY[%d] = 0x%llx\n",dimindx,SHARED_BEGIN_PHY[dimindx]);
#endif
	//ALL_SHARED_BEGIN_PHY[dimindx] = sys_m_getaddr(dimindx,1,slave_phy_id,ALL_SHARED_BEGIN[dimindx]);
#ifdef MCG
	vtp.vaddr = (unsigned long)ALL_SHARED_BEGIN[dimindx];
	//vtp.pid = getpid();
	vtp.pid = 0;//??ȡ?????̵?paddr?????԰?pid????0
	ret  = ioctl(stask_fd, STASK_VADDR_TO_PADDR, &vtp);
	if (ret == -1 || vtp.paddr == 0) {
        printf("vaddr_to paddr failed, errno = %d.\n",errno);
        exit(-1);
	}
	ALL_SHARED_BEGIN_PHY[dimindx] = (unsigned long *)vtp.paddr;
//#ifdef DEBUG
	printf("ALL_SHARED_BEGIN_PHY[%d] = 0x%llx\n",dimindx,ALL_SHARED_BEGIN_PHY[dimindx]);
	fflush(NULL);
//#endif
#endif
#endif
#else
#ifdef FULL
	SHARED_BEGIN_PHY[dimindx] = SHARED_BEGIN[dimindx];
#endif
	ALL_SHARED_BEGIN_PHY[dimindx] = ALL_SHARED_BEGIN[dimindx];
#endif
}
#endif
void get_ldm_addr(unsigned int dimindx)
{
	int i,j,k;
	int slave_id;
	#ifdef MCG
	for(i=0;i<MAX_CGNUM;i++)
	#endif
	#ifndef MCG
	i=dimindx;
	#endif
	{
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
		if(corelist[i][j].valid == 1)	
		{
			k=corelist[i][j].local_id;
			slave_id=MAX_COL_NUM*(corelist[i][j].row_id)+corelist[i][j].col_id;
			ldm_addr[i][k]=SPEIO_LDM_ADDR(i,slave_id,0);	
		}
	}
	}

}
#if 0
void get_ldm_addr()
{
        unsigned long long master_corestart;
        int i,j,k,l,m,n;
        unsigned long long ret;
	int slave_id;
#ifdef FPGA_TEST
        //master_corestart=sys_m_read_cgstat(1);
    	struct slave_get_cgstat cgstat;
    	ret  = ioctl(slave_fd, SLAVE_GET_CGSTAT, &cgstat);
	if (ret != 0) {
        printf("get cgstat faild, errno = %d.\n",errno);
        exit(-1);
	}
    	master_corestart = cgstat.cgstat;
	printf("master_corestart:%x\n",master_corestart);
#else
        master_corestart=0xf;
#endif
        for(i=0;i<MAX_CGNUM;i++)
        {
                if((master_corestart>>i)&0x1)
                {
			for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
			{
#ifdef FULL
			    if(corelist[i][j].valid == 1)
			    {
				printf("i: %d\t j: %d\n", i, j);
				k=corelist[i][j].local_id;
				printf("k: %d\n", k);
				slave_id=MAX_COL_NUM*(corelist[i][j].row_id)+corelist[i][j].col_id;
				printf("slave_id: %d\n", slave_id);
#ifdef FPGA_TEST
				//ldm_addr[i][k]=sys_m_ldm(i,slave_id,-1);
				ldm_addr[i][k]=SPEIO_LDM_ADDR(i,slave_id,0);
#else
				ldm_addr[i][k]=malloc(LDM_SIZE);
#endif
		#ifdef DEBUG
				printf("ldm_addr[%d][%d] = 0x%lx\n",i,k,ldm_addr[i][k]);fflush(NULL);
		#endif

			    }
#endif
#ifdef UNFULL
			    if(*(ADDR_CORELIST[i][j][VALID]) == 1)
			    {
				k=*ADDR_CORELIST[i][j][LOCAL_ID];
				slave_id=MAX_COL_NUM*(*ADDR_CORELIST[i][j][ROW_ID])+(*ADDR_CORELIST[i][j][COL_ID]);
		#ifdef DEBUG
				printf("slave_id = %d\n",slave_id);fflush(NULL);
		#endif
#ifdef FPGA_TEST
				//ldm_addr[i][k]=sys_m_ldm(i,slave_id,-1);
				ldm_addr[i][k]=SPEIO_LDM_ADDR(i,slave_id,0);
#else
				ldm_addr[i][k]=malloc(LDM_SIZE);
#endif
		#ifdef DEBUG
				printf("ldm_addr[%d][%d] = 0x%lx,0x%lx\n",i,k,ldm_addr[i][k]);fflush(NULL);
		#endif

			    }
#endif
			}
                }
        }

}
#endif
inline int get_phy_core_id_based_on_log_id(unsigned int id)
{
    int i,j,k;
#ifdef FULL
	for(i=0;i<MAX_CGNUM;i++)
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if((corelist[i][j].valid == 1) && (corelist[i][j].global_id == id))
            {
	        k=(corelist[i][j].row_id)*MAX_COL_NUM+corelist[i][j].col_id;
                break;
            }
	}
#endif
#ifdef UNFULL
	for(i=0;i<MAX_CGNUM;i++)
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if(((*ADDR_CORELIST[i][j][VALID])== 1) && ((*ADDR_CORELIST[i][j][GLOBAL_ID])== id))
            {
	        k=((*ADDR_CORELIST[i][j][ROW_ID]))*MAX_COL_NUM+(*ADDR_CORELIST[i][j][COL_ID]);
                break;
            }
	}
#endif
    return k;
}
inline int get_phy_cg_id_based_on_log_id(unsigned int id)
{
    int i,j,k;
#ifdef FULL
	for(i=0;i<MAX_CGNUM;i++)
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if((corelist[i][j].valid == 1) && (corelist[i][j].global_id == id))
            {
	        k=i;
                break;
            }
	}
#endif
#ifdef UNFULL
	for(i=0;i<MAX_CGNUM;i++)
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if(((*ADDR_CORELIST[i][j][VALID])== 1) && ((*ADDR_CORELIST[i][j][GLOBAL_ID])== id))
            {
	        k=i;
                break;
            }
	}
#endif

    return k;
}
inline int get_core_num(unsigned int dimindx)
{
    int i,j,k;

    k=0;
#ifdef FULL
    for(i=0;i<MAX_CGNUM;i++)
    {
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if(corelist[i][j].valid == 1)
	    k++;
	}
    }
#endif
#ifdef UNFULL
    for(i=0;i<MAX_CGNUM;i++)
    {
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if((*ADDR_CORELIST[i][j][VALID])== 1)
	    k++;
	}
    }
#endif
    return k;
}

inline int get_core_num_in_cg(unsigned int dimindx)
{
    int i,j,k;

    k=0;
#ifdef FULL
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if(corelist[dimindx][j].valid == 1)
	    k++;
	}
#endif
#ifdef UNFULL
	for(j=0;j<MAX_ROW_NUM*MAX_COL_NUM;j++)
	{
	    if((*ADDR_CORELIST[dimindx][i][VALID])== 1)
	    k++;
	}
#endif

    return k;
}
inline int get_core_num_in_row(unsigned int dimindx,int r_id)
{
    int i,j,k;

    j=0;
#ifdef FULL
    for(i=0;i<MAX_ROW_NUM*MAX_COL_NUM;i++)
    {
	if((corelist[dimindx][i].row_id == r_id) && (corelist[dimindx][i].valid == 1))
	    j++;
    }
#endif
#ifdef UNFULL
    for(i=0;i<MAX_ROW_NUM*MAX_COL_NUM;i++)
    {
	if((*ADDR_CORELIST[dimindx][i][ROW_ID]== r_id) && (*ADDR_CORELIST[dimindx][i][VALID]== 1))
	    j++;
    }
#endif

    return j;
}
inline int get_core_num_in_col(unsigned int dimindx,int c_id)
{
    int i,j,k;

    j=0;
#ifdef FULL
    for(i=0;i<MAX_ROW_NUM*MAX_COL_NUM;i++)
    {
	if((corelist[dimindx][i].col_id == c_id) && (corelist[dimindx][i].valid == 1))
	    j++;
    }
#endif
#ifdef UNFULL
    for(i=0;i<MAX_ROW_NUM*MAX_COL_NUM;i++)
    {
	if((*ADDR_CORELIST[dimindx][i][COL_ID]== c_id) && (*ADDR_CORELIST[dimindx][i][VALID]== 1))
	    j++;
    }
#endif

    return j;
}
unsigned int get_master_log_id_after_modify()
{
    int i,j,tmp_cg;
    unsigned long long master_list;


	tmp_cg = 0;
	master_list = 0;
	for(i=0;i<MAX_CGNUM;i++)
	{
		if(get_core_num_in_cg(i) != 0 )
		{
			tmp_cg++;
			master_list+=(1<<i);
		}
	}

	cg_num = tmp_cg;
#ifdef FULL
    j=0;
    for(i=0;i<MAX_CGNUM;i++)
	{
		masterlist[i].phy_id = i;
		masterlist[i].log_id = i;
		masterlist[i].valid = 0;
	    if( (master_list>>i)&0x1)
	    {
				masterlist[i].phy_id = i;
				masterlist[i].log_id = j;
				masterlist[i].valid = 1;
				j++;
#ifdef DEBUG
			printf("masterlist[%d].log_id = %d\n",i,masterlist[i].log_id);
			fflush(NULL);
#endif
	    }
    }

    if(masterlist[master_core_id].phy_id == master_core_id)
	i = masterlist[master_core_id].log_id;
#endif
#ifdef UNFULL
    j=0;
    for(i=0;i<MAX_CGNUM;i++)
	{
		*ADDR_MASTERLIST[i][M_PHY_ID] = i;
		*ADDR_MASTERLIST[i][M_LOG_ID] = i;
		*ADDR_MASTERLIST[i][M_VALID] = 0;
	    if( (master_list>>i)&0x1)
	    {
				*ADDR_MASTERLIST[i][M_PHY_ID] = i;
				*ADDR_MASTERLIST[i][M_LOG_ID] = j;
				*ADDR_MASTERLIST[i][M_VALID] = 1;
				j++;
#ifdef DEBUG
			printf("*ADDR_MASTERLIST[%d][M_LOG_ID] = %d\n",i,*ADDR_MASTERLIST[i][M_LOG_ID]);
			fflush(NULL);
#endif
	    }
    }

    if(*ADDR_MASTERLIST[master_core_id][M_PHY_ID] == master_core_id)
	i = *ADDR_MASTERLIST[master_core_id][M_LOG_ID];
#endif
#ifdef DEBUG
	printf("<<TEST>>:in get_master_list_after_modify,cg_num = %d,master_list = 0x%x\n",cg_num,master_list);
	fflush(NULL);
#endif
    return i;
}
void write_check_m(int num)
{
    m_check = num;
#ifdef DEBUG
    printf("<<TEST>>:num = %d,&m_check = 0x%lx\n",num,&m_check);fflush(NULL);
#endif
}

inline void write_check_s(int num)
{
    s_check = num;
#ifdef DEBUG
    printf("<<TEST>>:num = %d,&s_check = 0x%lx\n",num,&s_check);fflush(NULL);
#endif
}

static inline void cccr_spin_lock(volatile arch_spinlock_t *lock)
{
	unsigned long tmp,addr;
#ifdef DEBUG
	printf("before cccr_spin_lock:address of lock is 0x%lx, lock->lock = %ld\n",lock,lock->lock);
	fflush(NULL);
#endif
#ifdef FPGA_TEST
	__asm__ __volatile__(
			"	ldi %2,%1\n"
			"1:	lldl %0,0(%2)\n"
			"	cmpeq %0,0,%0\n"
			"	wr_f %0\n"
			"	ldi %0,1\n"
			"	memb\n"
			"	lstl %0,0(%2)\n"
			"	rd_f %0\n"
			"   beq %0, 1b\n"
			:	"=&r" (tmp),"=m"(lock->lock),"=&r"(addr)
			:"m"(lock->lock)	 
			: "memory");
	if(lock->lock == 0){
		printf("go in default ,but lock-> lock == 0\n");
		while(1);
	}
#endif
#ifdef DEBUG
	printf("after cccr_spin_lock:address of lock is 0x%lx, lock->lock = %ld\n",lock,lock->lock);
	fflush(NULL);
#endif
}

static inline void cccr_spin_unlock(volatile arch_spinlock_t *lock)
{
#ifdef DEBUG
	printf("before spin_unlock,address of lock is 0x%lx,lock->lock = %ld\n",lock,lock->lock);
	fflush(NULL);
#endif

	mb();
	lock->lock = 0;
	mb();
#ifdef DEBUG
	printf("after spin_unlock,address of lock is 0x%lx,lock->lock = %ld\n",lock,lock->lock);
	fflush(NULL);
#endif
}

static inline void arch_read_lock(arch_rwlock_t *rw) 
{ 
        long regx, regy, addr; 
        __asm__ __volatile__( 
        "       ldi     %3, %0\n" 
	"1:	lldw    %1, 0(%3)\n"    //?? 
        "       cmpeq     %1, 0, %2\n"    //??????0????????????? 
        "       wr_f    %2\n"           //?lock_flag?? 
        "       subw    %1, 2, %1\n"    //???,???1 
        "       memb\n"    
        "       lstw    %1, 0(%3)\n"    //??? 
        "       rd_f    %2\n"           //?lock_success?? 
        "       beq     %2,1b\n"        //????????? 
        : "=m" (rw->lock), "=&r" (regx), "=&r" (regy), "=&r" (addr) 
        : "m" (rw->lock) : "memory"); 
} 

static inline void arch_write_lock(arch_rwlock_t *rw) 
{ 
        long regx, addr; 
 
        __asm__ __volatile__( 
        "       ldi     %2, %0\n" 
	"1:	lldw    %1, 0(%2)\n"            //??lock? 
        "       seleq   %1, 1, $31, %1\n"       //??lock=0??lock_flag=1,??lock_flag=0 
        "       wr_f    %1\n"                   //?lock_flag?? 
        "       ldi     %1, 1\n" 
        "       memb\n"    
        "       lstw    %1, 0(%2)\n"            //?????lock=1??? 
        "       rd_f    %1\n" 
        "       beq     %1, 1b\n"               //???????? 
        : "=m" (rw->lock), "=&r" (regx), "=&r" (addr) 
        : "m" (rw->lock) : "memory"); 
} 
 
static inline int arch_read_trylock(arch_rwlock_t *rw) 
{ 
        long regx, addr; 
        int success; 
 
        __asm__ __volatile__( 
        "       ldi     %3, %0\n" 
	"1:	lldw    %1, 0(%3)\n"            //??lock? 
        "       sellbc  %1, 1, $31, %2\n"       //????write_locked: ????success=lock_flag=0;??success=lock_flag=1 
        "       wr_f    %2\n"                   //?lock_flag 
        "       subw    %1, 2, %1\n"            //????1 
        "       memb\n"    
        "       lstw    %1, 0(%3)\n"            //??? 
        "       rd_f    %1\n"                   //?lock_success?? 
        "       bne     %1, 2f\n"               //??lock_success=1???? 
        "       bne     %2, 6f\n"               //??success=1???????? 
        "2:     \n"
        ".subsection 2\n" 
        "6:     br      1b\n" 
        ".previous" 
        : "=m" (rw->lock), "=&r" (regx), "=&r" (success), "=&r" (addr) 
        : "m" (rw->lock) : "memory"); 
 
        return success; 
} 

static inline int arch_write_trylock(arch_rwlock_t *rw) 
{ 
        long regx, addr; 
        int success; 
 
        __asm__ __volatile__( 
        "       ldi     %3, %0\n" 
	"1:	lldw    %1, 0(%3)\n"            //??lock? 
        "       seleq   %1, 1, $31, %2\n"       //??????????: ????success=lock_flag=1;??success=lock_flag=0 
        "       wr_f    %2\n"                   //?lock_flag?? 
        "       ldi     %1, 1\n" 
        "       memb\n"    
        "       lstw    %1, 0(%3)\n"            //???lock?1 
        "       rd_f    %1\n"                   //?lock_success?? 
        "       bne     %1, 2f\n"               //??lock_success?????? 
        "       bne     %2, 6f\n"               //??success=1???????? 
        "2:     \n"
        ".subsection 2\n" 
        "6:     br      1b\n" 
        ".previous" 
        : "=m" (rw->lock), "=&r" (regx), "=&r" (success), "=&r" (addr) 
        : "m" (rw->lock) : "memory"); 
 
        return success; 
} 

static inline void arch_read_unlock(arch_rwlock_t *rw) 
{ 
        long regx, regy, addr; 
 
        __asm__ __volatile__( 
        "       ldi     %3, %0\n" 
	"1:	lldw    %1, 0(%3)\n"    //??lock? 
        "       ldi     %2, 1\n" 
        "       wr_f    %2\n"           //?lock_flag=1 
        "       addw    %1, 2, %1\n"    //??????1 
        "       memb\n"    
        "       lstw    %1, 0(%3)\n"    //??? 
        "       rd_f    %1\n"           //?lock_success?? 
        "       beq     %1, 1b\n"       //?lock_success=0????unlock 
        : "=m" (rw->lock), "=&r" (regx), "=&r" (regy), "=&r" (addr) 
        : "m" (rw->lock) : "memory"); 
} 

static inline void arch_write_unlock(volatile arch_rwlock_t *rw) 
{ 
        mb(); 
        rw->counter = 0;
        cccr_spin_unlock(&rw->lock);
} 



int m_lock_node() // lock inside cg/node
{
        unsigned int ret;
        volatile int i;
        volatile arch_spinlock_t *p;
	
#ifdef FULL
	p = addr_master_lock;
#endif
#ifdef UNFULL
	p = ADDR_MASTER_LOCK[0][M_LOCK];
#endif


#ifdef DEBUG
	printf("in m_lock_node:addr of m_lock_node =   0x%lx, *p = %d\n", p, *p);
	fflush(NULL);
#endif	
        cccr_spin_lock(p);
        return 0;
}
int mm_lock_node() // lock inside cg/node
{
        unsigned int ret;
        volatile int i;
        volatile arch_spinlock_t *p;
	
#ifdef FULL
	p = &LOCK_ADDR;
#endif
#ifdef UNFULL
	p = ADDR_MASTER_LOCK[0][M_LOCK];
#endif


#ifdef DEBUG
	printf("in m_lock_node:addr of m_lock_node =   0x%lx, *p = %d\n", p, *p);
	fflush(NULL);
#endif	
        cccr_spin_lock(p);
        return 0;
}

int m_unlock_node()
{
#ifdef FULL
#ifdef DEBUG
	printf("in m_unlock_node:addr_master_lock= 0x%lx, *addr_master_lock =  %d\n",addr_master_lock, *addr_master_lock);
	fflush(NULL);
#endif
        cccr_spin_unlock(addr_master_lock);
#endif
#ifdef UNFULL
#ifdef DEBUG
	printf("in m_unlock_node:ADDR_MASTER_LOCK[0]= 0x%lx, *ADDR_MASTER_LOCK[0] =  %d\n",ADDR_MASTER_LOCK[0], *ADDR_MASTER_LOCK[0]);
	fflush(NULL);
#endif
        cccr_spin_unlock(ADDR_MASTER_LOCK[0][M_LOCK]);
#endif
        return 0;
}
int mm_unlock_node()
{
        volatile arch_spinlock_t *p;
#ifdef FULL
#ifdef DEBUG
	printf("in m_unlock_node:&LOCK_ADDR= 0x%lx, LOCK_ADDR =  %d\n",&LOCK_ADDR, LOCK_ADDR);
	fflush(NULL);
#endif
	p = &LOCK_ADDR;
        cccr_spin_unlock(p);
#endif
#ifdef UNFULL
#ifdef DEBUG
	printf("in m_unlock_node:ADDR_MASTER_LOCK[0]= 0x%lx, *ADDR_MASTER_LOCK[0] =  %d\n",ADDR_MASTER_LOCK[0], *ADDR_MASTER_LOCK[0]);
	fflush(NULL);
#endif
        cccr_spin_unlock(ADDR_MASTER_LOCK[0][M_LOCK]);
#endif
        return 0;
}
inline void unlock(volatile arch_spinlock_t *p)
{
	mb();
	p->lock = 0;
	mb();
}
inline void ms_unlock()
{
        unsigned long tmp;
        __asm__ __volatile__(
                        "ldi %0,0\n"
                        "stl %0,0(%1)\n"
                        "memb\n"
                        :"=&r"(tmp)
                        :"r"(&(addr_ms_lock->lock))
                        :"memory");
}
inline void ms_lock()
{
        unsigned long tmp;
        __asm__ __volatile__(
#if 0
			"	rtc %0\n"
			"	and %0,0xff,%0\n"
			"4:	beq %0,5f\n"
			"       subl %0,1,%0\n"
                        "       br 4b\n"
			"5:	nop\n"
#endif
                        "1:     ldi %0,1\n"
                        "       stl %0,0(%1)\n"
                        "       memb\n"
                        "       ldl %0,0(%2)\n"
                        "       bne %0,2f\n"
                        ".subsection 2\n"
                        "2:     ldi %0,0\n"
                        "       stl %0,0(%1)\n"
                        "       memb\n"
                        "       rtc %0\n"
                        "       and %0,0xff,%0\n"
                        "3:     beq %0,1b\n"
                        "       subl %0,1,%0\n"
                        "       br 3b\n"
                        ".previous"
                        :"=&r"(tmp)
                        :"r"(&(addr_ms_lock->lock)),"r"(&(addr_ms_lock1->lock))
                        :"memory");
}
#if 0
inline void lock(volatile arch_spinlock_t *p)
{
        unsigned long tmp;
#ifdef DEBUG
	printf("in lock,before faal:  value of lock is %ld\n",p->lock);
	fflush(NULL);
#endif
#ifdef FPGA_TEST
		   __asm__ __volatile__(
				   				"       memb\n"
								"1:     ldl_inc %0,0(%1)\n"
								"       bne %0,1b\n"
								"       memb\n"
								:       "=&r" (tmp)
								:       "r"(&(p->lock)) : "memory");
#endif
#ifdef DEBUG
	printf("the value before lock is %ld\n",tmp);
	fflush(NULL);
#endif
}
#endif
inline void lock(volatile arch_spinlock_t *lock)
{
	unsigned long tmp,addr;
#ifdef DEBUG
	printf("before ms_lock:address of lock is 0x%lx, lock->lock = %ld\n",lock,lock->lock);
	fflush(NULL);
#endif
#ifdef FPGA_TEST
	__asm__ __volatile__(
			"	ldi %2,%1\n"
			"1:	lldl %0,0(%2)\n"
		//	"	xor %0,1,%0\n"
			"	cmpeq %0,0,%0\n"
			"	wr_f %0\n"
			"	ldi %0,1\n"
        		"       memb\n"    
			"	lstl %0,0(%2)\n"
			"	rd_f %0\n"
			"   beq %0, 1b\n"
			:	"=&r" (tmp),"=m"(lock->lock),"=&r"(addr)
			:"m"(lock->lock)	 
			: "memory");
	if(lock->lock == 0){
		printf("go in default ,but lock-> lock == 0\n");
		while(1);
	}
#endif
#ifdef DEBUG
	printf("after ms_lock:address of lock is 0x%lx, lock->lock = %ld\n",lock,lock->lock);
	fflush(NULL);
#endif
}
int
ms_lock_node() // lock inside cg/node
{
        unsigned int ret;
        volatile int i;
	volatile arch_spinlock_t *p;
	
#ifdef FULL
	p = addr_m_lock;
#endif
#ifdef UNFULL
	p = ADDR_M_LOCK[0];
#endif

        lock(p);
        return 0;
}
int
ms_unlock_node()
{
#ifdef FULL
    unlock(addr_m_lock);
#endif
#ifdef UNFULL
    unlock(ADDR_M_LOCK[0]);
#endif
        return 0;
}
