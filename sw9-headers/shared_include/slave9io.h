#ifndef __SLAVE9_IO__
#define __SLAVE9_IO__

#define IO_MARK_BIT     47
#define IO_BITS         (0x1UL<<IO_MARK_BIT)

#define SPEIO_HW_BASE   (0x1UL<<47)
#define SPEIO_HW_SIZE   (0x1UL<<32)  //映射4G长度，将从核、从核簇以及PAIU的寄存器全部映射，并且包含广播地址

#define get_rowid_from_coreid(coreid)  		((coreid >> 3) & 0x7)
#define get_colid_from_coreid(coreid)  		(coreid & 0x7)
#define get_spc_rowid_from_coreid(coreid) 	(get_rowid_from_coreid(coreid) >> 1)
#define get_spc_colid_from_coreid(coreid) 	(get_colid_from_coreid(coreid) >> 1)
#define get_spcrid_from_spcid(spcid)		((spcid >> 2) & 0x3)
#define get_spccid_from_spcid(spcid)  		(spcid & 0x3)
#define get_coreid_from_spcid(spcid, logicno)	( (get_spcrid_from_spcid(spcid) << 4) | ((logicno >> 1) <<3) | (get_spccid_from_spcid(spcid) << 1) | (logicno & 0x1) )
#define coreid_to_spcid(coreid) 	(((coreid>>4)<<2)|((coreid>>1)&3))
#define spcid_to_coreid(spcid, logicno) (((((spcid>>2)<<1)|((logicno>>1)&1))<<3) | (((spcid&3)<<1)|(logicno&1)))

#define ARRAYID_BITS_SHIFT	40 			

/* 运算核心中的IO寄存器 SPEIO 0x0UL<<36 */
#define LDM_HW_BASE     			((0x2UL<<22) | (0x1UL<<47))
#define LDM_HW_SIZE     			0x1000000UL	// 256KB * 64
#define LDM_BROADCAST_HW_BASE 	(LDM_HW_BASE | BROADCAST_WRITE_FLAG)
#define SCORE_LDM_SIZE  			(256 << 10)	// 256KB
#define SCORE_LDM_PADDR(arrayid,coreno)		(LDM_HW_BASE + ( (unsigned long)arrayid<<ARRAYID_BITS_SHIFT) + ( (unsigned long)coreno<<24) )
#define SCORE_LDM_KADDR(arrayid,coreno)		((unsigned long)SCORE_LDM_PADDR(arrayid, coreno) | PAGE_OFFSET)
#define SCORE_LDM_UADDR(arrayid, coreno)	SCORE_LDM_PADDR(arrayid, coreno)

#define L1IC_HW_BASE	(SPEIO | (0x3UL<<22) | (0x1UL<<47))
#define L1IC_HW_SIZE	(16 << 10)		// 16KB
 
#define __iomem
static inline uint64_t readq(const volatile void __iomem *addr)
{
    uint64_t ret = * (uint64_t *)(addr);
    asm volatile("memb");
    return ret;
}

static inline uint64_t writeq(uint64_t val, volatile void __iomem *addr)
{
    * (uint64_t *)(addr) = val;
    asm volatile("memb");
    return val;
}

#define SWIO_READ_REG(reg) readq((void *) __iomem reg)
#define SWIO_WRITE_REG(reg,val)  writeq(val,(void *) __iomem reg)

/*
 * 	从核阵列io地址编址：
 * 		[47] 	： IO空间标志；
 * 		[46:45] :  保留
 * 		[44]	:  从核阵列Io空间和全芯片空间io标志
 * 		[43] 	:  保留
 * 		[42:40]	:  从核阵列号
 * 		[39:36] = 0000 : 从核簇、从核和paiu
 * 		        [31] : 阵列内广播
 * 		        [30] : 1为PAIU，0为从核、从核簇
 * 		        [29:24] : 部件编号，当为从核时，表示行列编号；当为从核簇时，簇号={PA[29：28], PA[26:25]};目标为PAIU时，
 * 		        	  PAIU编号 ：[26,25]
 * 		[39:36] = 0010 :  RIUS0
 * 		        = 0011 :  RIUS1
 * 		        = 0100 :  SRI0
 * 		        = 0101 :  SRI1
 * 		        = 0111 :  PAMU
 *
 */
#define SPE_IOBITS 	(IO_BITS)
#define SPE_PAIU_IOBITS (SPE_IOBITS | (0x1UL<<30))
#define SPE_BROADCAST_INDEX 	(0x1UL<<31)
#define RIUS0_IOBITS 	(IO_BITS | (0x2UL<<36)) 		//RIU从核阵列环网站台部件，[42:40] 表示阵列号
#define RIUS1_IOBITS 	(IO_BITS | (0x3UL<<36))
#define RIUM_IOBITS	(IO_BITS | (0x1UL<<44) | (0x4UL<<36))  //RIU主核簇环网站台部件，[42:40] 表示阵列号
#define RIUX_IOBITS	(IO_BITS | (0x1UL<<44) | (0xcUL<<40) | (0x2UL<<36))  //RIU系统接口站台部件
#define SRI0_IOBITS 	(IO_BITS | (0x4UL<<36)) 		
#define SRI1_IOBITS 	(IO_BITS | (0x5UL<<36)) 		
#define PAMU_IOBITS 	(IO_BITS | (0x7UL<<36)) 		

//从核阵列寄存器广播写
#define SPEIO_WRITE_BROADCAST_REG(arrayid, index, val)  \
	SWIO_WRITE_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | SPE_BROADCAST_INDEX | (index)), val)
//从核内部寄存器读写
#define SPEIO_READ_SINGLE_REG(arrayid, scoreid, index)  \
	SWIO_READ_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | ((long)scoreid<<24) | (index)))
#define SPEIO_WRITE_SINGLE_REG(arrayid, scoreid, index, val)  \
	SWIO_WRITE_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | ((long)scoreid<<24) | (index)), val)
//从核簇内部寄存器读写
#define SPCIO_READ_SINGLE_REG(arrayid, clusterid, index)  \
	SWIO_READ_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | ((long)(clusterid&3)<<25) |((long)(clusterid>>2)<<28) | (index)))
#define SPCIO_WRITE_SINGLE_REG(arrayid, clusterid, index, val)  \
	SWIO_WRITE_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | ((long)(clusterid&3)<<25) |((long)(clusterid>>2)<<28) | (index)), val)
//PAIU内部寄存器读写
#define PAIU_READ_SINGLE_REG(arrayid, paiuid, index)  \
	SWIO_READ_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | ((long)(paiuid)<<25) | (index)))
#define PAIU_WRITE_SINGLE_REG(arrayid, paiuid, index, val)  \
	SWIO_WRITE_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | ((long)(paiuid)<<25) | (index)), val)
#define scoreid_to_paiuid(scoreid) 	((scoreid>>1)&0x3)

enum SlaveCoreEboxRegisters
{
	SPE_EBXIO_CID           =    SPE_IOBITS | 0x100000 ,    //从核唯一ID                      
	SPE_EBXIO_TC            =    SPE_IOBITS | 0x100080 ,    //周期计数器                      
	SPE_EBXIO_LOG3RL        =    SPE_IOBITS | 0x100100 ,    //LOG3R真值表低段                 
	SPE_EBXIO_LOG3RH        =    SPE_IOBITS | 0x100180 ,    //LOG3R真值表高段                 
	SPE_EBXIO_512CA         =    SPE_IOBITS | 0x100200 ,    //512位加减法进借位               
	SPE_EBXIO_VLENMAS       =    SPE_IOBITS | 0x100280 ,    //VLENMAS配置                     
#ifdef CONFIG_SW_7
    SPE_EBXIO_AES           =    SPE_IOBITS | 0x100300 ,    //AES配置                         
#endif
	SPE_EBXIO_LRF           =    SPE_IOBITS | 0x100380 ,    //GPR临时变量标志位               
	SPE_EBXIO_DEF0          =    SPE_IOBITS | 0x100400 ,    //用户自定义项0                   
	SPE_EBXIO_DEF1          =    SPE_IOBITS | 0x100480 ,    //用户自定义项1                   
	SPE_EBXIO_DEF2          =    SPE_IOBITS | 0x100500 ,    //用户自定义项2                   
	SPE_EBXIO_DEF3          =    SPE_IOBITS | 0x100580 ,    //用户自定义项3                   
	SPE_EBXIO_PCRC          =    SPE_IOBITS | 0x100780 ,    //性能计数器控制          
	SPE_EBXIO_FPCRL         =    SPE_IOBITS | 0x100800 ,    //FPCR低段                        
	SPE_EBXIO_FPCRH         =    SPE_IOBITS | 0x100880 ,    //FPCR高段                        
    SPE_EBXIO_CTRL          =    SPE_IOBITS | 0x100900 ,    //EBOX控制                        
	SPE_EBXIO_GATER         =    SPE_IOBITS | 0x100980 ,    //门控时钟控制（待定）            
	SPE_EBXIO_STAT          =    SPE_IOBITS | 0x100a00 ,    //EBOX状态                        
	SPE_EBXIO_SBOK          =    SPE_IOBITS | 0x100a80 ,    //GPR记分板OK位                   
	SPE_EBXIO_LAST_JMP      =    SPE_IOBITS | 0x100b00 ,    //最近一次JMP操作的状态（待定）   
#ifdef CONFIG_SW_9
	SPE_EBXIO_FRAC_ZERO     =    SPE_IOBITS | 0x100b80 ,    //内部调试寄存器   
#endif
    SPE_EBXIO_EXCP          =    SPE_IOBITS | 0x101000 ,    //EBOX异常                        
	SPE_EBXIO_EXCP_MASK     =    SPE_IOBITS | 0x101080 ,    //EBOX异常屏蔽                    
	SPE_EBXIO_EXCP_W1S      =    SPE_IOBITS | 0x101100 ,    //EBOX异常写1置                   
    SPE_EBXIO_DESC_EN       =    SPE_IOBITS | 0x101180 ,    //描述符异常现场登记使能
    SPE_EBXIO_DESC_0SRCL    =    SPE_IOBITS | 0x101200 ,    //描述符异常现场：操作数0低段
    SPE_EBXIO_DESC_0SRCH    =    SPE_IOBITS | 0x101280 ,    //描述符异常现场：操作数0高段
    SPE_EBXIO_DESC_1SRC     =    SPE_IOBITS | 0x101300 ,    //描述符异常现场：操作数1
    SPE_EBXIO_DESC_2SRC     =    SPE_IOBITS | 0x101380 ,    //描述符异常现场：操作数2与PC	
    SPE_EBXIO_ERR           =    SPE_IOBITS | 0x101400 ,    //EBOX错误                        
	SPE_EBXIO_ERR_W1S       =    SPE_IOBITS | 0x101500 ,    //EBOX错误写1置                   
	SPE_EBXIO_ERR_PC        =    SPE_IOBITS | 0x101600 ,    //EBOX错误现场（PC值）            
	SPE_EBXIO_ERR_SYN       =    SPE_IOBITS | 0x101680 ,    //EBOX错误现场（ECC纠错码）       
	SPE_EBXIO_FAULT         =    SPE_IOBITS | 0x101800 ,    //EBOX故障                        
	SPE_EBXIO_FAULT_W1S     =    SPE_IOBITS | 0x101900 ,    //EBOX故障写1置	
															//保留[63:59][55:0], 信号故障位[58:56]                  
#ifdef CONFIG_SW_9
	SPE_EBXIO_FLAG_WGPR     =    SPE_IOBITS | 0x101c00 ,    //标记：IO写GPR                        
	SPE_EBXIO_FLAG_WIOR     =    SPE_IOBITS | 0x101c80 ,    //标记：IO写EBX_IOR                        
#endif

#ifdef CONFIG_SLAVE_DCACHE
	SPE_EBXIO_HASH_V0       =    SPE_IOBITS | 0x102000 ,    //HASH_BUF分量0                   
	SPE_EBXIO_HASH_V1       =    SPE_IOBITS | 0x102080 ,    //HASH_BUF分量1                   
	SPE_EBXIO_HASH_V2       =    SPE_IOBITS | 0x102100 ,    //HASH_BUF分量2                   
	SPE_EBXIO_HASH_V3       =    SPE_IOBITS | 0x102180 ,    //HASH_BUF分量3                   
	SPE_EBXIO_HASH_V4       =    SPE_IOBITS | 0x102200 ,    //HASH_BUF分量4                   
	SPE_EBXIO_HASH_V5       =    SPE_IOBITS | 0x102280 ,    //HASH_BUF分量5                   
	SPE_EBXIO_HASH_V6       =    SPE_IOBITS | 0x102300 ,    //HASH_BUF分量6                   
	SPE_EBXIO_HASH_V7       =    SPE_IOBITS | 0x102380 ,    //HASH_BUF分量7                  
	SPE_EBXIO_HASHIDX       =    SPE_IOBITS | 0x102400 ,    //HASH_BUF访问地址               
#endif
	SPE_EBXIO_OPQ_INFO0     =    SPE_IOBITS | 0x102800 ,    //OPQ条目内容低段                
	SPE_EBXIO_OPQ_INFO1     =    SPE_IOBITS | 0x102880 ,    //OPQ条目内容高段                
	SPE_EBXIO_OPQ_INFO2     =    SPE_IOBITS | 0x102900 ,    //OPQ条目状态                    
	SPE_EBXIO_OPQIDX        =    SPE_IOBITS | 0x102c00 ,    //OPQ条目访问地址                
    SPE_EBXIO_PCR0          =    SPE_IOBITS | 0x103000 ,    //性能计数器0             
	SPE_EBXIO_PCR1          =    SPE_IOBITS | 0x103080 ,    //性能计数器1             
	SPE_EBXIO_PCR2          =    SPE_IOBITS | 0x103100 ,    //性能计数器2             
	SPE_EBXIO_PCR3          =    SPE_IOBITS | 0x103180 ,    //性能计数器3             
	SPE_EBXIO_PCR4          =    SPE_IOBITS | 0x103200 ,    //性能计数器4             
	SPE_EBXIO_PCR5          =    SPE_IOBITS | 0x103280 ,    //性能计数器5             
	SPE_EBXIO_PCR6          =    SPE_IOBITS | 0x103300 ,    //性能计数器6             
	SPE_EBXIO_PCR7          =    SPE_IOBITS | 0x103380 ,    //性能计数器7             
#ifdef CONFIG_SW_7
    SPE_EBXIO_GPR_ECC       =    SPE_IOBITS | 0x118000 ,     //GPR0~GPR63的ECC码              
#endif
#ifdef CONFIG_SW_9
    SPE_EBXIO_GPR_ECC       =    SPE_IOBITS | 0x108000 ,     //GPR0~GPR63的ECC码              
#endif
	SPE_EBXIO_GPR           =    SPE_IOBITS | 0x300000 ,     //GPR0~GPR63的数据               
};

enum SlaveCoreIboxRegisters
{
	SPE_IBXIO_GPC           =    SPE_IOBITS | 0x008000,       //全局PC            
	SPE_IBXIO_RUN           =    SPE_IOBITS | 0x008100,       //运行控制          
	SPE_IBXIO_STAT          =    SPE_IOBITS | 0x008200,       //IBOX状态          
	SPE_IBXIO_ERR           =    SPE_IOBITS | 0x008300,       //IBOX错误及异常    
	SPE_IBXIO_BP            =    SPE_IOBITS | 0x008400,       //断点设置          
	SPE_IBXIO_ICU           =    SPE_IOBITS | 0x008500,       //ICU模块状态       
	SPE_IBXIO_LBCWR         =    SPE_IOBITS | 0x008600,       //LBCwr模块状态     
	SPE_IBXIO_LB0           =    SPE_IOBITS | 0x008700,       //L0 IC循环缓冲状态0
	SPE_IBXIO_LB1           =    SPE_IOBITS | 0x008800,       //L0 IC循环缓冲状态1
	SPE_IBXIO_LBCRD         =    SPE_IOBITS | 0x008900,       //LBCrd模块状态     
	SPE_IBXIO_IDU0          =    SPE_IOBITS | 0x008a00,       //IDU模块状态0      
	SPE_IBXIO_IDU1          =    SPE_IOBITS | 0x008b00,       //IDU模块状态1      
	SPE_IBXIO_SP            =    SPE_IOBITS | 0x008c00,       //同步指令暂停模式  
	SPE_IBXIO_BRP_HINT      =    SPE_IOBITS | 0x008d00,       //转移预测器提示标记
	SPE_IBXIO_BRP_PWS       =    SPE_IOBITS | 0x008e00,       //转移预测器预测方向
    SPE_IBXIO_LB_ACT        =    SPE_IOBITS | 0x008f00,       //l0 ic 循环缓冲开关`
};

enum SlaveCoreMboxRegisters {
	SPE_MBXIO_EXCP                          =      SPE_IOBITS | 0x180000,           //MBOX异常                                  
	SPE_MBXIO_EXCP_MASK                     =      SPE_IOBITS | 0x180080,           //MBOX异常屏蔽                              
	SPE_MBXIO_EXCP_W1S                      =      SPE_IOBITS | 0x180100,           //MBOX异常写1置                             
	SPE_MBXIO_ERR                           =      SPE_IOBITS | 0x180180,           //MBOX错误                                  
	SPE_MBXIO_ERR_W1S                       =      SPE_IOBITS | 0x180200,           //MBOX错误写1置                             
	SPE_MBXIO_ERR_SYN0                      =      SPE_IOBITS | 0x180280,           //错误现场0：校正子                         
	SPE_MBXIO_ERR_SYN1                      =      SPE_IOBITS | 0x180300,           //错误现场1：校正子                         
	SPE_MBXIO_FAULT                         =      SPE_IOBITS | 0x180380,           //MBOX故障                                  
	SPE_MBXIO_FAULT_W1S                     =      SPE_IOBITS | 0x180400,           //MBOX故障写1置                             
	SPE_MBXIO_DA_INT                        =      SPE_IOBITS | 0x180480,           //MBOX数据流中断和现场                      
	SPE_MBXIO_DA_MATCH0                     =      SPE_IOBITS | 0x180500,           //数据流地址匹配设置0                       
	SPE_MBXIO_DA_MATCH1                     =      SPE_IOBITS | 0x180580,           //数据流地址匹配设置1                       
	SPE_MBXIO_DA_MASK0                      =      SPE_IOBITS | 0x180600,           //数据流地址屏蔽0                           
	SPE_MBXIO_DA_MASK1                      =      SPE_IOBITS | 0x180680,           //数据流地址屏蔽1                           
#ifdef CONFIG_SW_7
	SPE_MBXIO_CLASOVP                       =      SPE_IOBITS | 0x188000,           //分类存储总体参数                          
#endif
#ifdef CONFIG_SW_9
	SPE_MBXIO_GGQ_MAXCNT                    =      SPE_IOBITS | 0x1a8000,           //GGQ最大计数                         
	SPE_MBXIO_GGQ_STAT                      =      SPE_IOBITS | 0x1a8800,           //GGQ状态                         
#endif
#ifdef CONFIG_SLAVE_DCACHE
	SPE_MBXIO_DCC_CRQ_VALIDANDLOCK          =      SPE_IOBITS | 0x1a0000,           //CRQ有效位向量和锁                         
	SPE_MBXIO_DCC_CRQ_NUM0_CONTENT0         =      SPE_IOBITS | 0x1a0080,           //CRQ条目0内容，不包括主存地址和写数据      
	SPE_MBXIO_DCC_CRQ_NUM0_CONTENT1         =      SPE_IOBITS | 0x1a0100,           //CRQ条目0内容中的主存地址                  
	SPE_MBXIO_DCC_CRQ_NUM1_CONTENT0         =      SPE_IOBITS | 0x1a0180,           //CRQ条目1内容，不包括主存地址和写数据      
	SPE_MBXIO_DCC_CRQ_NUM1_CONTENT1         =      SPE_IOBITS | 0x1a0200,           //CRQ条目1内容中的主存地址                  
	SPE_MBXIO_DCC_CRQ_NUM2_CONTENT0         =      SPE_IOBITS | 0x1a0280,           //CRQ条目2内容，不包括主存地址和写数据      
	SPE_MBXIO_DCC_CRQ_NUM2_CONTENT1         =      SPE_IOBITS | 0x1a0300,           //CRQ条目2内容中的主存地址                  
	SPE_MBXIO_DCC_CRQ_NUM3_CONTENT0         =      SPE_IOBITS | 0x1a0380,           //CRQ条目3内容，不包括主存地址和写数据      
	SPE_MBXIO_DCC_CRQ_NUM3_CONTENT1         =      SPE_IOBITS | 0x1a0400,           //CRQ条目3内容中的主存地址                  
	SPE_MBXIO_DCC_CRQ_ACTPTR                =      SPE_IOBITS | 0x1a0480,           //CRQ中的激活指针                           
	SPE_MBXIO_DCC_CRQ_PRI0START             =      SPE_IOBITS | 0x1a0500,           //私有段0的起始地址                         
	SPE_MBXIO_DCC_CRQ_PRI0END               =      SPE_IOBITS | 0x1a0580,           //私有段0的结束地址                         
	SPE_MBXIO_DCC_CRQ_PRI1START             =      SPE_IOBITS | 0x1a0600,           //私有段1的起始地址                         
	SPE_MBXIO_DCC_CRQ_PRI1END               =      SPE_IOBITS | 0x1a0680,           //私有段1的结束地址                         
	SPE_MBXIO_DCC_CRQ_PRI2START             =      SPE_IOBITS | 0x1a0700,           //私有段2的起始地址                         
	SPE_MBXIO_DCC_CRQ_PRI2END               =      SPE_IOBITS | 0x1a0780,           //私有段2的结束地址                         
	SPE_MBXIO_DCC_CRQ_PRI3START             =      SPE_IOBITS | 0x1a0800,           //私有段3的起始地址                         
	SPE_MBXIO_DCC_CRQ_PRI3END               =      SPE_IOBITS | 0x1a0880,           //私有段3的结束地址                         
	SPE_MBXIO_DCC_TAU_V_WAY0_LOW            =      SPE_IOBITS | 0x1a1000,           //0路有效位的低段部分                       
	SPE_MBXIO_DCC_TAU_V_WAY0_HIGH           =      SPE_IOBITS | 0x1a1080,           //0路有效位的高段部分                       
	SPE_MBXIO_DCC_TAU_V_WAY1_LOW            =      SPE_IOBITS | 0x1a1100,           //1路有效位的低段部分                       
	SPE_MBXIO_DCC_TAU_V_WAY1_HIGH           =      SPE_IOBITS | 0x1a1180,           //1路有效位的高段部分                       
	SPE_MBXIO_DCC_TAU_V_WAY2_LOW            =      SPE_IOBITS | 0x1a1200,           //2路有效位的低段部分                       
	SPE_MBXIO_DCC_TAU_V_WAY2_HIGH           =      SPE_IOBITS | 0x1a1280,           //2路有效位的高段部分                       
	SPE_MBXIO_DCC_TAU_V_WAY3_LOW            =      SPE_IOBITS | 0x1a1300,           //3路有效位的低段部分                       
	SPE_MBXIO_DCC_TAU_V_WAY3_HIGH           =      SPE_IOBITS | 0x1a1380,           //3路有效位的高段部分                       
	SPE_MBXIO_DCC_TAU_E_H_63T0              =      SPE_IOBITS | 0x1a1400,           //淘汰位高位的63至0                         
	SPE_MBXIO_DCC_TAU_E_H_127T64            =      SPE_IOBITS | 0x1a1480,           //淘汰位高位的127至64                       
	SPE_MBXIO_DCC_TAU_E_L_63T0              =      SPE_IOBITS | 0x1a1500,           //淘汰位低位的63至0                         
	SPE_MBXIO_DCC_TAU_E_L_127T64            =      SPE_IOBITS | 0x1a1580,           //淘汰位低位的127至64                       
	SPE_MBXIO_DCC_CLEAR_V                   =      SPE_IOBITS | 0x1a1600,           //有效位清0                                 
	SPE_MBXIO_DCC_CLEAR_E                   =      SPE_IOBITS | 0x1a1680,           //淘汰位清0                                 
	SPE_MBXIO_DCC_MMU_MB0_CONTENT0          =      SPE_IOBITS | 0x1a2000,           //脱靶缓冲条目0的内容寄存器0                
	SPE_MBXIO_DCC_MMU_MB0_CONTENT1          =      SPE_IOBITS | 0x1a2080,           //脱靶缓冲条目0的内容寄存器1                
	SPE_MBXIO_DCC_MMU_MB0_CONTENT2          =      SPE_IOBITS | 0x1a2100,           //脱靶缓冲条目0的内容寄存器2                
	SPE_MBXIO_DCC_MMU_MB1_CONTENT0          =      SPE_IOBITS | 0x1a2180,           //脱靶缓冲条目1的内容寄存器0                
	SPE_MBXIO_DCC_MMU_MB1_CONTENT1          =      SPE_IOBITS | 0x1a2200,           //脱靶缓冲条目1的内容寄存器1                
	SPE_MBXIO_DCC_MMU_MB1_CONTENT2          =      SPE_IOBITS | 0x1a2280,           //脱靶缓冲条目1的内容寄存器2                
	SPE_MBXIO_DCC_MMU_MB2_CONTENT0          =      SPE_IOBITS | 0x1a2300,           //脱靶缓冲条目2的内容寄存器0                
	SPE_MBXIO_DCC_MMU_MB2_CONTENT1          =      SPE_IOBITS | 0x1a2380,           //脱靶缓冲条目2的内容寄存器1                
	SPE_MBXIO_DCC_MMU_MB2_CONTENT2          =      SPE_IOBITS | 0x1a2400,           //脱靶缓冲条目2的内容寄存器2                
	SPE_MBXIO_DCC_MMU_MB3_CONTENT0          =      SPE_IOBITS | 0x1a2480,           //脱靶缓冲条目3的内容寄存器0                
	SPE_MBXIO_DCC_MMU_MB3_CONTENT1          =      SPE_IOBITS | 0x1a2500,           //脱靶缓冲条目3的内容寄存器1                
	SPE_MBXIO_DCC_MMU_MB3_CONTENT2          =      SPE_IOBITS | 0x1a2580,           //脱靶缓冲条目3的内容寄存器2                
	SPE_MBXIO_DCC_MMU_TB                    =      SPE_IOBITS | 0x1a2600,           //临时缓冲条目的内容                        
	SPE_MBXIO_DCC_MMU_STATE                 =      SPE_IOBITS | 0x1a2680,           //脱靶缓冲条目当前状态                      
	SPE_MBXIO_DCC_TAG                       =      SPE_IOBITS | 0x1C0000,           //DCache的Tag阵列条目内容                   
	SPE_MBXIO_DCC_MASK0                     =      SPE_IOBITS | 0x1E0000,           //DCache的Mask阵列条目内容中的mask值        
	SPE_MBXIO_DCC_MASK1                     =      SPE_IOBITS | 0x1E0080,           //DCache的Mask阵列条目内容中的独占位和脏位值
#endif
};

enum SPEL1Icache {
	SPE_L1ICIO_TV          =       SPE_IOBITS | 0x080000 ,          //TAG有效位    
	SPE_L1ICIO_TVC         =       SPE_IOBITS | 0x080100 ,          //TAG有效位清除
	SPE_L1ICIO_ERC         =       SPE_IOBITS | 0x080200 ,          //L1 ICache错误
	SPE_L1ICIO_TAG         =       SPE_IOBITS | 0x084000 ,          //L1 ICache Tag
};

enum SlaveCoreSboxRegisters {
	SBOX_ERR            =       SPE_IOBITS |  0x200000,      //SBOX校错寄存器        
	SBOX_ERR_MSK        =       SPE_IOBITS |  0x200080,      //SBOX错误屏蔽寄存器    
	SBOX_ERR_TEST       =       SPE_IOBITS |  0x200100,      //SBOX错误测试寄存器    
	SPE_ISOLATE_EN      =       SPE_IOBITS |  0x200180,      //从核自主隔离使能寄存器
	SBOX_DMA_ST         =       SPE_IOBITS |  0x200200,      //SBOX DMA状态寄存器 
	SBOX_MEM_ST         =       SPE_IOBITS |  0x200280,      //SBOX存储访问状态寄存器
	SBOX_SYN_ST         =       SPE_IOBITS |  0x200300,      //SBOX同步状态寄存器
	SBOX_SYNP_ST        =       SPE_IOBITS |  0x200380,      //SBOX管理行同步向量寄存器
};

enum SlaveCoreIOboxRegisters {
	SPE_ERR                 =  SPE_IOBITS | 0x280000,     //SPE总错      
	SPE_IOBXIO_IOERR        =  SPE_IOBITS | 0x281000,     //IOBOX错误登记
	SPE_IOBXIO_MASK         =  SPE_IOBITS | 0x281080,     //IOBOX错误屏蔽
	SPE_IOBXIO_DBUF         =  SPE_IOBITS | 0x281100,     //IOBOX数据站台
#ifdef CONFIG_SW_9
	SPE_IOBXIO_BIST         =  SPE_IOBITS | 0x282000,     //从核BIST状态
	SPE_IOBXIO_BIST_LDM     =  SPE_IOBITS | 0x282080,     //从核BIST状态：LDM
#endif
};


/* 从核簇管理路由部件RBOX */
enum SlaveCoreRboxRegisters {
	RBX_ERR             =  SPE_IOBITS | 0x400000,       //RBOX校错寄存器               
	RBX_ERR_MASK        =  SPE_IOBITS | 0x400080,       //RBOX错误屏蔽寄存器           
	RBX_ERR_SET         =  SPE_IOBITS | 0x400100,       //RBOX错误设置寄存器           
	RBX_TRACERR         =  SPE_IOBITS | 0x400180,       //RBOX登记错寄存器             
#ifdef CONFIG_SW_9
    RBX_CFG_WEIGHT      =  SPE_IOBITS | 0x400200,       //RBOX权重配置寄存器             
	RBX_CFG_CHK         =  SPE_IOBITS | 0x400280,       //RBOX检查配置寄存器             
#endif
	RBX_SCUST1          =  SPE_IOBITS | 0x410000,       //SCU状态寄存器1               
	RBX_SCUST0          =  SPE_IOBITS | 0x410080,       //SCU状态寄存器0               
	RBX_SCUERR          =  SPE_IOBITS | 0x410100,       //SCU校错寄存器                
	RBX_STN1FIFOST      =  SPE_IOBITS | 0x420000,       //STN1 fifo状态寄存器          
	RBX_STN0FIFOST      =  SPE_IOBITS | 0x420080,       //STN0 fifo状态寄存器          
	RBX_STN1CREDIT      =  SPE_IOBITS | 0x420100,       //STN1信用状态寄存器           
	RBX_STN0CREDIT      =  SPE_IOBITS | 0x420180,       //STN0信用状态寄存器           
	RBX_STN1CTRL        =  SPE_IOBITS | 0x420200,       //STN1控制状态寄存器           
	RBX_STN0CTRL        =  SPE_IOBITS | 0x420280,       //STN0控制状态寄存器           
#ifdef CONFIG_SW_7
	RBX_VCREXCEP        =  SPE_IOBITS | 0x420300,       //STN 异常寄存器               
#endif
	RBX_STN1CFAULT      =  SPE_IOBITS | 0x420380,       //STN1控制故障寄存器           
	RBX_STN0CFAULT      =  SPE_IOBITS | 0x420400,       //STN0控制故障寄存器           
	RBX_STN1FFAULT      =  SPE_IOBITS | 0x420480,       //STN1FIFO故障寄存器           
	RBX_STN0FFAULT      =  SPE_IOBITS | 0x420500,       //STN0FIFO故障寄存器           
	RBX_SYNST1          =  SPE_IOBITS | 0x430000,       //SYN状态寄存器1               
	RBX_SYNST0          =  SPE_IOBITS | 0x430080,       //SYN状态寄存器0               
	RBX_SYNERR          =  SPE_IOBITS | 0x430100,       //SYN校错寄存器                
	RBX_RCUST1          =  SPE_IOBITS | 0x440000,       //RCU状态寄存器1               
	RBX_RCUST0          =  SPE_IOBITS | 0x440080,       //RCU状态寄存器0               
	RBX_RCUERR          =  SPE_IOBITS | 0x440100,       //RCU校错寄存器                
	RBX_BUSERR          =  SPE_IOBITS | 0x450000,       //BUS校错寄存器                
	
    RBX_PCRC            =  SPE_IOBITS | 0x460000,       //RBOX计数器控制寄存器         
    RBX_SYN_PCR         =  SPE_IOBITS | 0x460080,       //RBOX同步网性能计数器
    RBX_STN_LPCR1       =  SPE_IOBITS | 0x460100,       //RBOX存储网本地端口性能计数器1         
    RBX_STN_LPCR0       =  SPE_IOBITS | 0x460180,       //RBOX存储网本地端口性能计数器0        
    RBX_STN_SPCR1       =  SPE_IOBITS | 0x460200,       //RBOX存储网S方向性能计数器1
    RBX_STN_SPCR0       =  SPE_IOBITS | 0x460280,       //RBOX存储网S方向性能计数器0        
    RBX_STN_NPCR1       =  SPE_IOBITS | 0x460300,       //RBOX存储网N方向性能计数器1
    RBX_STN_NPCR0       =  SPE_IOBITS | 0x460380,       //RBOX存储网N方向性能计数器0        
    RBX_STN_WPCR1       =  SPE_IOBITS | 0x460400,       //RBOX存储网W方向性能计数器1
    //RBX_STN_WPCR0       =  SPE_IOBITS | 0x460480,       //RBOX存储网W方向性能计数器0, 保留        
    RBX_STN_EPCR1       =  SPE_IOBITS | 0x460500,       //RBOX存储网E方向性能计数器1
    //RBX_STN_EPCR0       =  SPE_IOBITS | 0x460580,       //RBOX存储网E方向性能计数器0，保留  
#ifdef CONFIG_SW_7
	RBX_PCRERR          =  SPE_IOBITS | 0x460600,       //PCR校错寄存器                
#endif
};


/* 从核簇管理传输部件TBOX */
enum SlaveCoreTboxRegisters {
	TBOX_ERR_SUM               =  SPE_IOBITS | 0x480000 ,       //TBOX总错                                
	TBOX_FAULT                 =  SPE_IOBITS | 0x480080 ,       //TBOX故障                                
	TBOX_MERR                  =  SPE_IOBITS | 0x480100 ,       //TBOX多错                                
 	TBOX_EXCP                  =  SPE_IOBITS | 0x480180 ,       //TBOX异常                                
	TBOX_BISI_START            =  SPE_IOBITS | 0x480300 ,       //TBOX缓冲初始化启动                      
	TBOX_BISI_DONE             =  SPE_IOBITS | 0x480380 ,       //TBOX缓冲初始化结束                      
#ifdef CONFIG_SW_7
	TBOX_DELAY_CONFIG          =  SPE_IOBITS | 0x480480 ,       //TBOX延迟配置寄存器                      
#endif
#ifdef CONFIG_SW_9
	TBOX_CFG                   =  SPE_IOBITS | 0x480480 ,       //TBOX配置寄存器                      
#endif
	TBOX_IO_EXCP_ST            =  SPE_IOBITS | 0x480500 ,       //TBOXIO异常现场                          
	TBOX_FAULT_MASK            =  SPE_IOBITS | 0x480880 ,       //TBOX故障校错屏蔽寄存器                  
	TBOX_MERR_MASK             =  SPE_IOBITS | 0x480900 ,       //TBOX多错屏蔽寄存器                      
	TBOX_EXCP_MASK             =  SPE_IOBITS | 0x480980 ,       //TBOX异常屏蔽寄存器                      
#ifdef CONFIG_SW_7
	TBOX_GL_FLOW_CTRL          =  SPE_IOBITS | 0x480a00 ,       //TBOX全局流量控制寄存器                  
	TBOX_DMA_FLOW_CTRL         =  SPE_IOBITS | 0x480a80 ,       //TBOXDMA流量控制寄存器                   
#endif
	TBOX_FAULT_SET             =  SPE_IOBITS | 0x481080 ,       //TBOX故障造错寄存器                      
	TBOX_MERR_SET              =  SPE_IOBITS | 0x481100 ,       //TBOX多错造错寄存器                      
	TBOX_EXCP_SET              =  SPE_IOBITS | 0x481180 ,       //TBOX异常造错寄存器                      
    TBOX_PCRC                  =  SPE_IOBITS | 0x481200 ,       //TBOX性能计数控制寄存器
    TBOX_PCR0                  =  SPE_IOBITS | 0x481280 ,       //TBOX性能计数寄存器0	
    TBOX_PCR1                  =  SPE_IOBITS | 0x481300 ,       //TBOX性能计数寄存器1	
    TBOX_PCR2                  =  SPE_IOBITS | 0x481380 ,       //TBOX性能计数寄存器2	
    TBOX_PCR3                  =  SPE_IOBITS | 0x481400 ,       //TBOX性能计数寄存器3	
    TBOX_PCR4                  =  SPE_IOBITS | 0x481480 ,       //TBOX性能计数寄存器4	
    TBOX_PCR5                  =  SPE_IOBITS | 0x481500 ,       //TBOX性能计数寄存器5	
    TBOX_PCR6                  =  SPE_IOBITS | 0x481580 ,       //TBOX性能计数寄存器6	

    TBOX_DMA_BUF_V             =  SPE_IOBITS | 0x482000 ,       //TBOXDMA命令缓存占用                     
	TBOX_BARRIERx_ST           =  SPE_IOBITS | 0x482080 ,       //TBOX栏栅管理部件状态x4                  
	TBOX_BAR_ERR_ST            =  SPE_IOBITS | 0x482280 ,       //TBOX栏栅管理部件错误现场                
#ifdef CONFIG_SW_7
	TBOX_MSGS                  =  SPE_IOBITS | 0x484000 ,       //TBOX消息状态x32                         
#endif
#ifdef CONFIG_SW_9
	TBOX_MSGST                 =  SPE_IOBITS | 0x484000 ,       //TBOX消息状态x32                         
#endif
	TBOX_MSG_V                 =  SPE_IOBITS | 0x485000 ,       //TBOX消息占用状态                        
	TBOX_MSG_EXCP              =  SPE_IOBITS | 0x485080 ,       //TBOX消息异常                            
	TBOX_MSG_ANS_ACT           =  SPE_IOBITS | 0x485100 ,       //TBOX消息回答字激活                      
	TBOX_FLY_CNT               =  SPE_IOBITS | 0x485180 ,       //TBOX飞行计数器                          
	TBOX_DMA_FLY_CNT           =  SPE_IOBITS | 0x485200 ,       //TBOX DMA飞行计数器                      
	TBOX_ENG0_RQ_SEND_CREDIT   =  SPE_IOBITS | 0x486000 ,       //TBOX引擎0/1请求发送信用                 
	TBOX_ENG1_RQ_SEND_CREDIT   =  SPE_IOBITS | 0x488000 ,       //                                        
	TBOX_ENG0_RQ_SEND_ST       =  SPE_IOBITS | 0x486080 ,       //TBOX引擎0/1请求发送状态                 
	TBOX_ENG1_RQ_SEND_ST       =  SPE_IOBITS | 0x488080 ,       //                                        
	TBOX_ENG0_RQ_SEND_STAGE    =  SPE_IOBITS | 0x486100 ,       //TBOX引擎0/1请求发送站台                 
	TBOX_ENG1_RQ_SEND_STAGE    =  SPE_IOBITS | 0x488100 ,       //                                        
	TBOX_ENG0_ACK_SEND_CREDIT  =  SPE_IOBITS | 0x486180 ,       //TBOX引擎0/1响应发送信用                 
	TBOX_ENG1_ACK_SEND_CREDIT  =  SPE_IOBITS | 0x488180 ,       //                                        
	TBOX_ENG0_ACK_SEND_ST      =  SPE_IOBITS | 0x486200 ,       //TBOX引擎0/1响应发送状态                 
	TBOX_ENG1_ACK_SEND_ST      =  SPE_IOBITS | 0x488200 ,       //                                        
	TBOX_ENG0_ACK_SEND_STAGE   =  SPE_IOBITS | 0x486280 ,       //TBOX引擎0/1响应发送站台                 
	TBOX_ENG1_ACK_SEND_STAGE   =  SPE_IOBITS | 0x488280 ,       //                                        
	TBOX_ENG0_RCV_QUEUE_ST     =  SPE_IOBITS | 0x486300 ,       //TBOX引擎0/1接收队列状态                 
	TBOX_ENG1_RCV_QUEUE_ST     =  SPE_IOBITS | 0x488300 ,       //                                        
	TBOX_ENG0_RCV_RQ_CTRL_ST   =  SPE_IOBITS | 0x486380 ,       //TBOX引擎0/1接收请求控制状态             
	TBOX_ENG1_RCV_RQ_CTRL_ST   =  SPE_IOBITS | 0x488380 ,       //                                        
	TBOX_ENG0_RCV_ACK_CTRL_ST  =  SPE_IOBITS | 0x486400 ,       //TBOX引擎0/1接收响应控制状态             
	TBOX_ENG1_RCV_ACK_CTRL_ST  =  SPE_IOBITS | 0x488400 ,       //                                        
	TBOX_ENG0_WRBUF_V          =  SPE_IOBITS | 0x486480 ,       //TBOX引擎0/1写数据缓冲占用               
	TBOX_ENG1_WRBUF_V          =  SPE_IOBITS | 0x488480 ,       //                                        
	TBOX_ENG0_DMA_DIVP_ST0     =  SPE_IOBITS | 0x486500 ,       //TBOX引擎0/1拆包站台0状态0               
	TBOX_ENG1_DMA_DIVP_ST0     =  SPE_IOBITS | 0x488500 ,       //                                        
	TBOX_ENG0_DMA_DIVP0_ST1    =  SPE_IOBITS | 0x486580 ,       //TBOX引擎0/1拆包站台0状态1               
	TBOX_ENG1_DMA_DIVP0_ST1    =  SPE_IOBITS | 0x488580 ,       //                                        
	TBOX_ENG0_DMA_DIVP0_ST2    =  SPE_IOBITS | 0x486600 ,       //TBOX引擎0/1拆包站台0状态2               
	TBOX_ENG1_DMA_DIVP0_ST2    =  SPE_IOBITS | 0x488600 ,       //                                        
	TBOX_ENG0_DMA_DIVP0_ST0    =  SPE_IOBITS | 0x486680 ,       //TBOX引擎0/1拆包站台0状态0               
	TBOX_ENG1_DMA_DIVP0_ST0    =  SPE_IOBITS | 0x488680 ,       //                                        
	TBOX_ENG0_DMA_DIVP1_ST1    =  SPE_IOBITS | 0x486700 ,       //TBOX引擎0/1拆包站台0状态1               
	TBOX_ENG1_DMA_DIVP1_ST1    =  SPE_IOBITS | 0x488700 ,       //                                        
	TBOX_ENG0_DMA_DIVP1_ST2    =  SPE_IOBITS | 0x486780 ,       //TBOX引擎0/1拆包站台0状态2               
	TBOX_ENG1_DMA_DIVP1_ST2    =  SPE_IOBITS | 0x488780 ,       //                                        
#ifdef CONFIG_SW_7
    TBOX_ENG0_LDM_DIVP_SEND_S  =  SPE_IOBITS | 0x486900 ,       //TBOX引擎0/1LDM拆分部件发送状态          
	TBOX_ENG1_LDM_DIVP_SEND_S  =  SPE_IOBITS | 0x488900 ,       //                                        
	TBOX_ENG0_LDM_DIVP_EXCP_S  =  SPE_IOBITS | 0x486980 ,       //TBOX引擎0/1LDM拆分部件异常现场          
	TBOX_ENG1_LDM_DIVP_EXCP_S  =  SPE_IOBITS | 0x488980 ,       //                                        
#endif
#ifdef CONFIG_SW_9
    TBOX_ENG0_LDM_DIVP_SEND_ST =  SPE_IOBITS | 0x486900 ,       //TBOX引擎0/1LDM拆分部件发送状态          
	TBOX_ENG1_LDM_DIVP_SEND_ST =  SPE_IOBITS | 0x488900 ,       //                                        
	TBOX_ENG0_LDM_DIVP_EXCP_ST =  SPE_IOBITS | 0x486980 ,       //TBOX引擎0/1LDM拆分部件异常现场          
	TBOX_ENG1_LDM_DIVP_EXCP_ST =  SPE_IOBITS | 0x488980 ,       //                                        
#endif
	TBOX_ENG0_LDM_DIVP_ERR_ST  =  SPE_IOBITS | 0x486a00 ,       //TBOX引擎0/1LDM拆分部件错误现场          
	TBOX_ENG1_LDM_DIVP_ERR_ST  =  SPE_IOBITS | 0x488a00 ,       //                                        
	TBOX_ENG0_RDBUF_V          =  SPE_IOBITS | 0x486a80 ,       //TBOX引擎0/1读数据缓冲占用               
	TBOX_ENG1_RDBUF_V          =  SPE_IOBITS | 0x488a80 ,       //                                        
	TBOX_ENG0_ACKP             =  SPE_IOBITS | 0x486b00 ,       //TBOX引擎0/1响应悬挂X6                   
	TBOX_ENG1_ACKP             =  SPE_IOBITS | 0x488b00 ,       //                                        
	TBOX_ENG0_ACKP_V           =  SPE_IOBITS | 0x486e00 ,       //TBOX引擎0/1响应悬挂占用                 
	TBOX_ENG1_ACKP_V           =  SPE_IOBITS | 0x488e00 ,       //                                        
	TBOX_ENG0_ACKP_ST          =  SPE_IOBITS | 0x486e80 ,       //TBOX引擎0/1响应悬挂状态                 
	TBOX_ENG1_ACKP_ST          =  SPE_IOBITS | 0x488e80 ,       //                                        
	TBOX_ENG0_GPQ_ST           =  SPE_IOBITS | 0x486f00 ,       //TBOX引擎0 /1GET PUT队列状态             
	TBOX_ENG1_GPQ_ST           =  SPE_IOBITS | 0x488f00 ,       //                                        
	TBOX_ANS_BUF_V             =  SPE_IOBITS | 0x48a000 ,       //TBOX回答字占用状态                      
	TBOX_ANS_ERR_ST            =  SPE_IOBITS | 0x48a080 ,       //TBOX回答字错误现场                      
#ifdef CONFIG_SW_7
	TBOX_MBOX_INF0_SEND_S      =  SPE_IOBITS | 0x48c000 ,       //TBOX_MBOX接口0/1/2/3发送状态            
	TBOX_MBOX_INF1_SEND_S      =  SPE_IOBITS | 0x48c100 ,       //                                        
	TBOX_MBOX_INF2_SEND_S      =  SPE_IOBITS | 0x48c200 ,       //                                        
	TBOX_MBOX_INF3_SEND_S      =  SPE_IOBITS | 0x48c300 ,       //                                        
#endif
#ifdef CONFIG_SW_9
	TBOX_MBOX_INF0_SEND_ST     =  SPE_IOBITS | 0x48c000 ,       //TBOX_MBOX接口0/1/2/3发送状态            
	TBOX_MBOX_INF1_SEND_ST     =  SPE_IOBITS | 0x48c100 ,       //                                        
	TBOX_MBOX_INF2_SEND_ST     =  SPE_IOBITS | 0x48c200 ,       //                                        
	TBOX_MBOX_INF3_SEND_ST     =  SPE_IOBITS | 0x48c300 ,       //                                        
#endif
	TBOX_MBOX_INF0_RCV_ST      =  SPE_IOBITS | 0x48c080 ,       //TBOX_MBOX接口0/1/2/3接收状态            
	TBOX_MBOX_INF1_RCV_ST      =  SPE_IOBITS | 0x48c180 ,       //                                        
	TBOX_MBOX_INF2_RCV_ST      =  SPE_IOBITS | 0x48c280 ,       //                                        
	TBOX_MBOX_INF3_RCV_ST      =  SPE_IOBITS | 0x48c380 ,       //                                        
	TBOX_WEND0                 =  SPE_IOBITS | 0x48e000 ,       //TBOX写结束悬挂0 X8                      
	TBOX_WEND1                 =  SPE_IOBITS | 0x48e400 ,       //TBOX写结束悬挂1 X8                      
	TBOX_WEND_ST               =  SPE_IOBITS | 0x48e800 ,       //TBOX写结束管理状态                      
};


/* 从核簇维护部件SCMT */
enum SlaveCoreSCMTRegisters {
	SPC_ERR            =  SPE_IOBITS | 0x500000,     //SPC校错寄存器               
	SPC_ERR_MSK        =  SPE_IOBITS | 0x500080,     //SPC总错屏蔽寄存器           
	SCMT_ERR           =  SPE_IOBITS | 0x500100,     //SCMT校错寄存器              
	SCMT_ERR_MSK       =  SPE_IOBITS | 0x500180,     //SCMT错误屏蔽寄存器          
	SCMT_ERR_TEST      =  SPE_IOBITS | 0x500200,     //SCMT错误测试寄存器          
#ifdef CONFIG_SW_7
    SCMT_BCTIME_TH     =  SPE_IOBITS | 0x500280,     //SCMT广播超时阈值            
#endif
	SCMT_CFG           =  SPE_IOBITS | 0x500280,     //SCMT配置寄存器            
	SCMT_ST            =  SPE_IOBITS | 0x500300,     //SCMT状态寄存器              
	SPC_STNMODE        =  SPE_IOBITS | 0x500380,     //SPC存储网使用模式           
	SCMT_ADVWARN_CNT   =  SPE_IOBITS | 0x500400,     //SPC预警部件计数值           
	SCMT_ADVWARN_TH    =  SPE_IOBITS | 0x500480,     //SPC预警部件阈值             
	SCMT_PCR           =  SPE_IOBITS | 0x500500,     //SCMT性能计数器           
#ifdef CONFIG_SW_9
    SCMT_PRE_IORQ      =  SPE_IOBITS | 0x500580,     //SCMT最近访问IOR           
	SCMT_ERR_ST        =  SPE_IOBITS | 0x500600,     //SCMT错误现场           
#endif
};


/* 从核阵列接口PAIU */
enum PAIURegisters {
	PAIU_ERR           =  SPE_PAIU_IOBITS | 0x00000,       //PAIU校错寄存器               
	PAIU_ERR_MASK      =  SPE_PAIU_IOBITS | 0x00080,       //PAIU错误屏蔽寄存器           
	PAIU_ERR_SET       =  SPE_PAIU_IOBITS | 0x00100,       //PAIU错误设置寄存器           
#ifdef CONFIG_SW_7
	PAIU_RESERV        =  SPE_PAIU_IOBITS | 0x00180,       //保留     
#endif
	PAIU_ST2           =  SPE_PAIU_IOBITS | 0x00300,       //PAIU状态寄存器2              
	PAIU_ST1           =  SPE_PAIU_IOBITS | 0x00380,       //PAIU状态寄存器1              
	PAIU_ST0           =  SPE_PAIU_IOBITS | 0x00400,       //PAIU状态寄存器0              
	PAIU_BWCFG         =  SPE_PAIU_IOBITS | 0x00480,       //PAIU广播写配置寄存器         
	PAIU_SLB_EXCPSPOT  =  SPE_PAIU_IOBITS | 0x00500,       //地址代换异常现场             
	PAIU_SLB_EXCPVEC   =  SPE_PAIU_IOBITS | 0x00580,       //地址代换异常从核向量         
    PAIU_PCRC          =  SPE_PAIU_IOBITS | 0x00600,       //PAIU计数控制器寄存器
    PAIU_PCR2          =  SPE_PAIU_IOBITS | 0x00680,       //PAIU性能计数器2
    PAIU_PCR1          =  SPE_PAIU_IOBITS | 0x00700,       //PAIU性能计数器1
    PAIU_PCR0          =  SPE_PAIU_IOBITS | 0x00780,       //PAIU性能计数器0

	PAIU_SLBL          =  SPE_PAIU_IOBITS | 0x04000,       //SLB低段                      
	PAIU_SLBH          =  SPE_PAIU_IOBITS | 0x04080,       //SLB高段                      
};

enum SlaveSdlbPermissions
{
    SDLB_PERMISSION_RO  = 0x1,       
    SDLB_PERMISSION_WR  = 0x2,
    SDLB_PERMISSION_EX  = 0x4,
};

/* RIUS0 */
/* RIUS1 */
//从核阵列环网站台读写
#define RIUS_READ_REG(arrayid, RIUID, index)  \
	SWIO_READ_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | RIUS0_IOBITS | ((long)RIUID<<36) | (index)))
#define RIUS_WRITE_REG(arrayid, RIUID, index, val)  \
	SWIO_WRITE_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | RIUS0_IOBITS | ((long)RIUID<<36) | (index)), val)
//从核簇环网站台读写
#define RIUM_READ_REG(arrayid, index) 		\
	SWIO_READ_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | RIUM_IOBITS | (index)))
#define RIUM_WRITE_REG(arrayid, index, val) 		\
	SWIO_WRITE_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | RIUM_IOBITS | (index)), val)
//系统接口环网站台读写
#define RIUX_READ_REG(xid, index) 		\
	SWIO_READ_REG((RIUX_IOBITS | ((long)xid<<36) | (index)))
#define RIUX_WRITE_REG(xid, index, val) 		\
	SWIO_WRITE_REG((RIUX_IOBITS | ((long)xid<<36) | (index)), val)
enum RIUSRegisters {
#ifdef CONFIG_SW_9
    RIU_SUB0_CW_ERR                 	=	0x0000, 	//子模块0（顺）错误寄存器
    RIU_SUB0_CW_ERRSET  	            =	0x0080, 	//子模块0（顺）错误设置寄存器
    RIU_SUB0_CW_ERRMASK    	            =	0x0100, 	//子模块0（顺）错误屏蔽寄存器
    RIU_SUB0_AN_ERR    	                =	0x0180, 	//子模块0（逆）错误寄存器
    RIU_SUB0_AN_ERRSET              	=	0x0200, 	//子模块0（逆）错误设置寄存器
    RIU_SUB0_AN_ERRMASK             	=	0x0280, 	//子模块0（逆）错误屏蔽寄存器
    RIU_SUB1_CW_ERR    	                =	0x0300, 	//子模块1（顺）错误寄存器
    RIU_SUB1_CW_ERRSET              	=	0x0380, 	//子模块1（顺）错误设置寄存器
    RIU_SUB1_CW_ERRMASK    	            =	0x0400, 	//子模块1（顺）错误屏蔽寄存器
    RIU_SUB1_AN_ERR    	                =	0x0480, 	//子模块1（逆）错误寄存器
    RIU_SUB1_AN_ERRSET  	            =	0x0500, 	//子模块1（逆）错误设置寄存器
    RIU_SUB1_AN_ERRMASK    	            =	0x0580, 	//子模块1（逆）错误屏蔽寄存器
    RIU_IOERR  	                        =	0x0600, 	//IO错误寄存器
    RIU_IOERR_SET   	                =	0x0680, 	//IO错误设置寄存器
    RIU_IOERR_MASK                  	=	0x0700, 	//IO错误屏蔽寄存器
    RIU_CLR    	                        =	0x0780, 	//清错误现场寄存器
    RIU_CONFIG0   	                    =	0x0800, 	//配置寄存器0
    RIU_CONFIG1    	                    =	0x0880, 	//配置寄存器1
    RIU_ADVW_ECCSERR_CNT            	=	0x0900, 	//单错预警计数器
    RIU_ECCSERR_ADVWARN    	            =	0x0980, 	//单错预警配置寄存器
    RIU_FCTRL_FLYCNT_M 	                =	0x0a00, 	//流量控制飞行请求计数器（主核访存/从核取指）
    RIU_FCTRL_FLYCNT_M_WR  	            =	0x0a80, 	//流量控制飞行请求计数器（主核访存写）
    RIU_FCTRL_FLYCNT_S 	                =	0x0b00, 	//流量控制飞行请求计数器（从核数据访存）
    RIU_FCTRL_FLYCNT_S_WR  	            =	0x0b80, 	//流量控制飞行请求计数器（从核访存写）
    RIU_FCTRL_FLYCNT_X              	=	0x0c00, 	//流量控制飞行请求计数器（系统接口访存）
    RIU_FCTRL_FLYCNT_X_WR  	            =	0x0c80, 	//流量控制飞行请求计数器（系统接口访存写）
    RIU_SUB0_CW_FIFOST  	            =	0x8000, 	//子模块0（顺）队列状态寄存器
    RIU_SUB0_CW_RCV_TAGST           	=	0x8080, 	//子模块0（顺）接收标记状态寄存器
    RIU_SUB0_CW_RCV_FLYFIFO_CRDST   	=	0x8100, 	//子模块0（顺）接收飞行队列信用状态寄存器
    RIU_SUB0_CW_ERR_SCENE0	            =	0x8180, 	//子模块0（顺）错误现场寄存器0
    RIU_SUB0_CW_ERR_SCENE1 	            =	0x8200, 	//子模块0（顺）错误现场寄存器1
    RIU_SUB0_CW_SND_RQ1FLUXCNT 	        =	0x8280, 	//子模块0（顺）发送一次请求流量计数器
    RIU_SUB0_CW_SND_ACKFLUXCNT      	=	0x8300, 	//子模块0（顺）发送响应流量计数器
    RIU_SUB0_CW_SND_RQ2FLUXCNT      	=	0x8380, 	//子模块0（顺）发送二次请求流量计数器
    RIU_SUB0_CW_SND_RSPFLUXCNT 	        =	0x8400, 	//子模块0（顺）发送回答流量计数器
    RIU_SUB0_CW_RCV_RQ1FLUXCNT 	        =	0x8480, 	//子模块0（顺）接收一次请求流量计数器
    RIU_SUB0_CW_RCV_ACKFLUXCNT 	        =	0x8500, 	//子模块0（顺）接收响应流量计数器
    RIU_SUB0_CW_RCV_RQ2FLUXCNT      	=	0x8580, 	//子模块0（顺）接收二次请求流量计数器
    RIU_SUB0_CW_RCV_RSPFLUXCNT 	        =	0x8600, 	//子模块0（顺）接收回答流量计数器
    RIU_SUB0_AN_FIFOST              	=	0xa000, 	//子模块0（逆）队列状态寄存器
    RIU_SUB0_AN_RCV_TAGST   	        =	0xa080, 	//子模块0（逆）接收标记状态寄存器
    RIU_SUB0_AN_RCV_FLYFIFO_CRDST   	=	0xa100, 	//子模块0（逆）接收飞行队列信用状态寄存器
    RIU_SUB0_AN_ERR_SCENE0	            =	0xa180, 	//子模块0（逆）错误现场寄存器0
    RIU_SUB0_AN_ERR_SCENE1 	            =	0xa200, 	//子模块0（逆）错误现场寄存器1
    RIU_SUB0_AN_SND_RQ1FLUXCNT 	        =	0xa280, 	//子模块0（逆）发送一次请求流量计数器
    RIU_SUB0_AN_SND_ACKFLUXCNT      	=	0xa300, 	//子模块0（逆）发送响应流量计数器
    RIU_SUB0_AN_SND_RQ2FLUXCNT 	        =	0xa380, 	//子模块0（逆）发送二次请求流量计数器
    RIU_SUB0_AN_SND_RSPFLUXCNT 	        =	0xa400, 	//子模块0（逆）发送回答流量计数器
    RIU_SUB0_AN_RCV_RQ1FLUXCNT      	=	0xa480, 	//子模块0（逆）接收一次请求流量计数器
    RIU_SUB0_AN_RCV_ACKFLUXCNT 	        =	0xa500, 	//子模块0（逆）接收响应流量计数器
    RIU_SUB0_AN_RCV_RQ2FLUXCNT 	        =	0xa580, 	//子模块0（逆）接收二次请求流量计数器
    RIU_SUB0_AN_RCV_RSPFLUXCNT 	        =	0xa600, 	//子模块0（逆）接收回答流量计数器
    RIU_SUB1_CW_FIFOST  	            =	0xc000, 	//子模块1（顺）队列状态寄存器
    RIU_SUB1_CW_RCV_TAGST   	        =	0xc080, 	//子模块1（顺）接收标记状态寄存器
    RIU_SUB1_CW_RCV_FLYFIFO_CRDST   	=	0xc100, 	//子模块1（顺）接收飞行队列信用状态寄存器
    RIU_SUB1_CW_ERR_SCENE0	            =	0xc180, 	//子模块1（顺）错误现场寄存器0
    RIU_SUB1_CW_ERR_SCENE1 	            =	0xc200, 	//子模块1（顺）错误现场寄存器1
    RIU_SUB1_CW_SND_RQ1FLUXCNT      	=	0xc280, 	//子模块1（顺）发送一次请求流量计数器
    RIU_SUB1_CW_SND_ACKFLUXCNT 	        =	0xc300, 	//子模块1（顺）发送响应流量计数器
    RIU_SUB1_CW_SND_RQ2FLUXCNT 	        =	0xc380, 	//子模块1（顺）发送二次请求流量计数器
    RIU_SUB1_CW_SND_RSPFLUXCNT       	=	0xc400, 	//子模块1（顺）发送回答流量计数器
    RIU_SUB1_CW_RCV_RQ1FLUXCNT 	        =	0xc480, 	//子模块1（顺）接收一次请求流量计数器
    RIU_SUB1_CW_RCV_ACKFLUXCNT 	        =	0xc500, 	//子模块1（顺）接收响应流量计数器
    RIU_SUB1_CW_RCV_RQ2FLUXCNT 	        =	0xc580, 	//子模块1（顺）接收二次请求流量计数器
    RIU_SUB1_CW_RCV_RSPFLUXCNT 	        =	0xc600, 	//子模块1（顺）接收回答流量计数器
    RIU_SUB1_AN_FIFOST  	            =	0xe000, 	//子模块1（逆）队列状态寄存器
    RIU_SUB1_AN_RCV_TAGST           	=	0xe080, 	//子模块1（逆）接收标记状态寄存器
    RIU_SUB1_AN_RCV_FLYFIFO_CRDST   	=	0xe100, 	//子模块1（逆）接收飞行队列信用状态寄存器
    RIU_SUB1_AN_ERR_SCENE0	            =	0xe180, 	//子模块1（逆）错误现场寄存器0
    RIU_SUB1_AN_ERR_SCENE1 	            =   0xe200, 	//子模块1（逆）错误现场寄存器1
    RIU_SUB1_AN_SND_RQ1FLUXCNT      	=	0xe280, 	//子模块1（逆）发送一次请求流量计数器
    RIU_SUB1_AN_SND_ACKFLUXCNT 	        =	0xe300, 	//子模块1（逆）发送响应流量计数器
    RIU_SUB1_AN_SND_RQ2FLUXCNT 	        =	0xe380, 	//子模块1（逆）发送二次请求流量计数器
    RIU_SUB1_AN_SND_RSPFLUXCNT      	=	0xe400, 	//子模块1（逆）发送回答流量计数器
    RIU_SUB1_AN_RCV_RQ1FLUXCNT 	        =	0xe480, 	//子模块1（逆）接收一次请求流量计数器
    RIU_SUB1_AN_RCV_ACKFLUXCNT      	=	0xe500, 	//子模块1（逆）接收响应流量计数器
    RIU_SUB1_AN_RCV_RQ2FLUXCNT 	        =	0xe580, 	//子模块1（逆）接收二次请求流量计数器
    RIU_SUB1_AN_RCV_RSPFLUXCNT      	=	0xe600, 	//子模块1（逆）接收回答流量计数器

#endif
#ifdef CONFIG_SW_7
	RIU_ADVW_ECCSERR_CNT                  =  0x0000,     //单错预警计数器                             
	RIU_ECCSERR_ADVWARN                   =  0x0080,     //单错预警配置寄存器                         
	RIU_SUB0_ERR                          =  0x0100,     //子模块0错误寄存器                          
	RIU_SUB0_ERRSET                       =  0x0180,     //子模块0错误设置寄存器                      
	RIU_SUB0_ERRMASK                      =  0x0200,     //子模块0错误屏蔽寄存器                      
	RIU_SUB1_ERR                          =  0x0280,     //子模块1错误寄存器                          
	RIU_SUB1_ERRSET                       =  0x0300,     //子模块1错误设置寄存器                      
	RIU_SUB1_ERRMASK                      =  0x0380,     //子模块1错误屏蔽寄存器                      
	RIU_SUB2_ERR                          =  0x0400,     //子模块2错误寄存器                          
	RIU_SUB2_ERRSET                       =  0x0480,     //子模块2错误设置寄存器                      
	RIU_SUB2_ERRMASK                      =  0x0500,     //子模块2错误屏蔽寄存器                      
	RIU_SUB3_ERR                          =  0x0580,     //子模块3错误寄存器                          
	RIU_SUB3_ERRSET                       =  0x0600,     //子模块3错误设置寄存器                      
	RIU_SUB3_ERRMASK                      =  0x0680,     //子模块3错误屏蔽寄存器                      
	RIU_IOERR                             =  0x0700,     //IO错误寄存器                               
	RIU_IOERR_SET                         =  0x0780,     //IO错误设置寄存器                           
	RIU_IOERR_MASK                        =  0x0800,     //IO错误屏蔽寄存器                           
	RIU_RIU_CLR                           =  0x0880,     //清错误现场寄存器                           
	RIU_SUBMIT_CRD_MAX                    =  0x0900,     //最大信用申请数量寄存器                     
	RIU_OVERTIME_THRES                    =  0x0980,     //超时阈值寄存器                             
	RIU_RIGHT_TURN_THRES                  =  0x0a00,     //右转跳数阈值寄存器                         
	RIU_SUB0_FIFOST                       =  0x8000,     //子模块0的队列状态寄存器                    
	RIU_SUB0_SEND_REQ_ROUFIFOST           =  0x8080,     //子模块0的发送模块一次请求路由队列状态寄存器
	RIU_SUB0_SEND_ACK_ROUFIFOST           =  0x8100,     //子模块0的发送模块响应路由队列状态寄存器    
	RIU_SUB0_SEND_RESP_ROUFIFOST          =  0x8180,     //子模块0的发送模块回答路由队列状态寄存器    
	RIU_SUB0_RECV_REQ_ROUFIFOST           =  0x8200,     //子模块0的接收模块一次请求路由队列状态寄存器
	RIU_SUB0_RECV_ACK_ROUFIFOST           =  0x8280,     //子模块0的接收模块响应路由队列状态寄存器    
	RIU_SUB0_RECV_RESP_ROUFIFOST          =  0x8300,     //子模块0的接收模块回答路由队列状态寄存器    
	RIU_SUB0_RECV_TAGST                   =  0x8380,     //子模块0接收模块TAG状态寄存器               
	RIU_SUB0_RECV_FLYFIFO_CRDST           =  0x8400,     //子模块0的接收模块飞行FIFO信用状态寄存器    
	RIU_SUB0_ERR_SCENE0                   =  0x8480,     //子模块0的发送模块错误现场寄存器            
	RIU_SUB0_ERR_SCENE1                   =  0x8500,     //子模块0的接收模块错误现场寄存器            
	RIU_SUB0_SEND_RQ1_FLUX_CNT            =  0x8580,     //子模块0的发送模块一次请求流量计数器        
	RIU_SUB0_SEND_ACK_FLUX_CNT            =  0x8600,     //子模块0的发送模块响应流量计数器            
	RIU_SUB0_SEND_RQ2_FLUX_CNT            =  0x8680,     //子模块0的发送模块二次请求流量计数器        
	RIU_SUB0_SEND_RESP_FLUX_CNT           =  0x8700,     //子模块0的发送模块回答流量计数器            
	RIU_SUB0_RECV_RQ1_FLUX_CNT            =  0x8780,     //子模块0的接收模块一次请求流量计数器        
	RIU_SUB0_RECV_ACK_FLUX_CNT            =  0x8800,     //子模块0的接收模块响应流量计数器            
	RIU_SUB0_RECV_RQ2_FLUX_CNT            =  0x8880,     //子模块0的接收模块二次请求流量计数器        
	RIU_SUB0_RECV_RESP_FLUX_CNT           =  0x8900,     //子模块0的接收模块回答流量计数器            
	RIU_SUB1_FIFOST                       =  0xa000,     //子模块1的队列状态寄存器                    
	RIU_SUB1_SEND_REQ_ROUFIFOST           =  0xa080,     //子模块1的发送模块一次请求路由队列状态寄存器
	RIU_SUB1_SEND_ACK_ROUFIFOST           =  0xa100,     //子模块1的发送模块响应路由队列状态寄存器    
	RIU_SUB1_SEND_RESP_ROUFIFOST          =  0xa180,     //子模块1的发送模块回答路由队列状态寄存器    
	RIU_SUB1_RECV_REQ_ROUFIFOST           =  0xa200,     //子模块1的接收模块一次请求路由队列状态寄存器
	RIU_SUB1_RECV_ACK_ROUFIFOST           =  0xa280,     //子模块1的接收模块响应路由队列状态寄存器    
	RIU_SUB1_RECV_RESP_ROUFIFOST          =  0xa300,     //子模块1的接收模块回答路由队列状态寄存器    
	RIU_SUB1_RECV_TAGST                   =  0xa380,     //子模块1接收模块TAG状态寄存器               
	RIU_SUB1_RECV_FLYFIFO_CRDST           =  0xa400,     //子模块1的接收模块飞行FIFO信用状态寄存器    
	RIU_SUB1_ERR_SCENE0                   =  0xa480,     //子模块1的发送模块错误现场寄存器            
	RIU_SUB1_ERR_SCENE1                   =  0xa500,     //子模块1的接收模块错误现场寄存器            
	RIU_SUB1_SEND_RQ1_FLUX_CNT            =  0xa580,     //子模块1的发送模块一次请求流量计数器        
	RIU_SUB1_SEND_ACK_FLUX_CNT            =  0xa600,     //子模块1的发送模块响应流量计数器            
	RIU_SUB1_SEND_RQ2_FLUX_CNT            =  0xa680,     //子模块1的发送模块二次请求流量计数器        
	RIU_SUB1_SEND_RESP_FLUX_CNT           =  0xa700,     //子模块1的发送模块回答流量计数器            
	RIU_SUB1_RECV_RQ1_FLUX_CNT            =  0xa780,     //子模块1的接收模块一次请求流量计数器        
	RIU_SUB1_RECV_ACK_FLUX_CNT            =  0xa800,     //子模块1的接收模块响应流量计数器            
	RIU_SUB1_RECV_RQ2_FLUX_CNT            =  0xa880,     //子模块1的接收模块二次请求流量计数器        
	RIU_SUB1_RECV_RESP_FLUX_CNT           =  0xa900,     //子模块1的接收模块回答流量计数器            
	RIU_SUB2_FIFOST                       =  0xc000,     //子模块2的队列状态寄存器                    
	RIU_SUB2_SEND_REQ_ROUFIFOST           =  0xc080,     //子模块2的发送模块一次请求路由队列状态寄存器
	RIU_SUB2_SEND_ACK_ROUFIFOST           =  0xc100,     //子模块2的发送模块响应路由队列状态寄存器    
	RIU_SUB2_SEND_RESP_ROUFIFOST          =  0xc180,     //子模块2的发送模块回答路由队列状态寄存器    
	RIU_SUB2_RECV_REQ_ROUFIFOST           =  0xc200,     //子模块2的接收模块一次请求路由队列状态寄存器
	RIU_SUB2_RECV_ACK_ROUFIFOST           =  0xc280,     //子模块2的接收模块响应路由队列状态寄存器    
	RIU_SUB2_RECV_RESP_ROUFIFOST          =  0xc300,     //子模块2的接收模块回答路由队列状态寄存器    
	RIU_SUB2_RECV_TAGST                   =  0xc380,     //子模块2接收模块TAG状态寄存器               
	RIU_SUB2_RECV_FLYFIFO_CRDST           =  0xc400,     //子模块2的接收模块飞行FIFO信用状态寄存器    
	RIU_SUB2_ERR_SCENE0                   =  0xc480,     //子模块2的发送模块错误现场寄存器            
	RIU_SUB2_ERR_SCENE1                   =  0xc500,     //子模块2的接收模块错误现场寄存器            
	RIU_SUB2_SEND_RQ1_FLUX_CNT            =  0xc580,     //子模块2的发送模块一次请求流量计数器        
	RIU_SUB2_SEND_ACK_FLUX_CNT            =  0xc600,     //子模块2的发送模块响应流量计数器            
	RIU_SUB2_SEND_RQ2_FLUX_CNT            =  0xc680,     //子模块2的发送模块二次请求流量计数器        
	RIU_SUB2_SEND_RESP_FLUX_CNT           =  0xc700,     //子模块2的发送模块回答流量计数器            
	RIU_SUB2_RECV_RQ1_FLUX_CNT            =  0xc780,     //子模块2的接收模块一次请求流量计数器        
	RIU_SUB2_RECV_ACK_FLUX_CNT            =  0xc800,     //子模块2的接收模块响应流量计数器            
	RIU_SUB2_RECV_RQ2_FLUX_CNT            =  0xc880,     //子模块2的接收模块二次请求流量计数器        
	RIU_SUB2_RECV_RESP_FLUX_CNT           =  0xc900,     //子模块2的接收模块回答流量计数器            
	RIU_SUB3_FIFOST                       =  0xe000,     //子模块3的队列状态寄存器                    
	RIU_SUB3_SEND_REQ_ROUFIFOST           =  0xe080,     //子模块3的发送模块一次请求路由队列状态寄存器
	RIU_SUB3_SEND_ACK_ROUFIFOST           =  0xe100,     //子模块3的发送模块响应路由队列状态寄存器    
	RIU_SUB3_SEND_RESP_ROUFIFOST          =  0xe180,     //子模块3的发送模块回答路由队列状态寄存器    
	RIU_SUB3_RECV_REQ_ROUFIFOST           =  0xe200,     //子模块3的接收模块一次请求路由队列状态寄存器
	RIU_SUB3_RECV_ACK_ROUFIFOST           =  0xe280,     //子模块3的接收模块响应路由队列状态寄存器    
	RIU_SUB3_RECV_RESP_ROUFIFOST          =  0xe300,     //子模块3的接收模块回答路由队列状态寄存器    
	RIU_SUB3_RECV_TAGST                   =  0xe380,     //子模块3接收模块TAG状态寄存器               
	RIU_SUB3_RECV_FLYFIFO_CRDST           =  0xe400,     //子模块3的接收模块飞行FIFO信用状态寄存器    
	RIU_SUB3_ERR_SCENE0                   =  0xe480,     //子模块3的发送模块错误现场寄存器            
	RIU_SUB3_ERR_SCENE1                   =  0xe500,     //子模块3的接收模块错误现场寄存器            
	RIU_SUB3_SEND_RQ1_FLUX_CNT            =  0xe580,     //子模块3的发送模块一次请求流量计数器        
	RIU_SUB3_SEND_ACK_FLUX_CNT            =  0xe600,     //子模块3的发送模块响应流量计数器            
	RIU_SUB3_SEND_RQ2_FLUX_CNT            =  0xe680,     //子模块3的发送模块二次请求流量计数器        
	RIU_SUB3_SEND_RESP_FLUX_CNT           =  0xe700,     //子模块3的发送模块回答流量计数器            
	RIU_SUB3_RECV_RQ1_FLUX_CNT            =  0xe780,     //子模块3的接收模块一次请求流量计数器        
	RIU_SUB3_RECV_ACK_FLUX_CNT            =  0xe800,     //子模块3的接收模块响应流量计数器            
	RIU_SUB3_RECV_RQ2_FLUX_CNT            =  0xe880,     //子模块3的接收模块二次请求流量计数器        
	RIU_SUB3_RECV_RESP_FLUX_CNT           =  0xe900,     //子模块3的接收模块回答流量计数器            
#endif
};

/* SRI0 */
/* SRI1 */
#define SRI_READ_REG(arrayid, SRIID, index)  \
	SWIO_READ_REG((((long)arrayid<<40) | SRI0_IOBITS | ((long)SRIID<<36) | (index)))
#define SRI_WRITE_REG(arrayid, SRIID, index, val)  \
	SWIO_WRITE_REG((((long)arrayid<<40) | SRI0_IOBITS | ((long)SRIID<<36) | (index)), val)
enum SRIRegisters {
#ifdef CONFIG_SW_9
    SRI_SND_PAIUINT_ERR    	        =	0x0000, 	//发送模块PAIU接口错误寄存器
    SRI_SND_PAIUINT_ERRSET      	=	0x0080, 	//发送模块PAIU接口错误设置寄存器
    SRI_SND_PAIUINT_ERRMASK    	    =	0x0100, 	//发送模块PAIU接口错误屏蔽寄存器
    SRI_SND_PAMUINT_ERR          	=	0x0180, 	//发送模块PAMU接口错误寄存器
    SRI_SND_PAMUINT_ERRSET  	    =	0x0200, 	//发送模块PAMU接口错误设置寄存器
    SRI_SND_PAMUINT_ERRMASK    	    =	0x0280, 	//发送模块PAMU接口错误屏蔽寄存器
    SRI_SND_XBAR_ERR   	            =	0x0300, 	//发送模块交叉开关错误寄存器
    SRI_SND_XBAR_ERRSET         	=	0x0380, 	//发送模块交叉开关错误设置寄存器
    SRI_SND_XBAR_ERRMASK   	        =	0x0400, 	//发送模块交叉开关错误屏蔽寄存器
    SRI_RCV_PAIUINT_ERR    	        =	0x0480, 	//接收模块PAIU接口错误寄存器
    SRI_RCV_PAIUINT_ERRSET  	    =	0x0500, 	//接收模块PAIU接口错误设置寄存器
    SRI_RCV_PAIUINT_ERRMASK    	    =	0x0580, 	//接收模块PAIU接口错误屏蔽寄存器
    SRI_RCV_PAMUINT_ERR    	        =	0x0600, 	//接收模块PAMU接口错误寄存器
    SRI_RCV_PAMUINT_ERRSET  	    =	0x0680, 	//接收模块PAMU接口错误设置寄存器
    SRI_RCV_PAMUINT_ERRMASK    	    =	0x0700, 	//接收模块PAMU接口错误屏蔽寄存器
    SRI_RCV_XBAR_ERR   	            =	0x0780, 	//接收模块交叉开关错误寄存器
    SRI_RCV_XBAR_ERRSET 	        =	0x0800, 	//接收模块交叉开关错误设置寄存器
    SRI_RCV_XBAR_ERRMASK   	        =	0x0880, 	//接收模块交叉开关错误屏蔽寄存器
    SRI_IOERR  	                    =	0x0900, 	//IO错误寄存器
    SRI_IOERR_SET   	            =	0x0980, 	//IO错误设置寄存器
    SRI_IOERR_MASK              	=	0x0a00, 	//IO错误屏蔽寄存器
    SRI_CLR    	                    =	0x0a80, 	//清错误现场寄存器
    SRI_CONFIG  	                =	0x0b00, 	//配置寄存器
    SRI_ADVW_ECCSERR_CNT        	=	0x0b80, 	//单错预警计数器
    SRI_ECCSERR_ADVWARN         	=	0x0c00, 	//单错预警配置寄存器
    SRI_FLYRQ1_CNT0             	=	0x0c80, 	//飞行请求计数器0
    SRI_FLYRQ1_CNT1    	            =	0x0d00, 	//飞行请求计数器1
    SRI_ACKTHROW_CNT0           	=	0x0d80, 	//响应吞包计数器0
    SRI_ACKTHROW_CNT1  	            =	0x0e00, 	//响应吞包计数器1
    SRI_SND_INTFIFOST   	        =	0x8000, 	//发送模块接口队列状态寄存器
    SRI_SND_RIUCRDST    	        =	0x8080, 	//发送模块RIU信用状态寄存器
    SRI_SND_XBARST  	            =	0x8100, 	//发送模块交叉开关状态寄存器
    SRI_SND_ERR_SCENE0	            =	0x8180, 	//发送模块错误现场寄存器0
    SRI_SND_ERR_SCENE1 	            =	0x8200, 	//发送模块错误现场寄存器1
    SRI_SND_PAIUINT_RQ1FLUXCNT0   	=	0x8280, 	//发送模块PAIU接口一次请求流量计数器0
    SRI_SND_PAIUINT_RQ1FLUXCNT1    	=	0x8300, 	//发送模块PAIU接口一次请求流量计数器1
    SRI_SND_PAMUINT_ACKFLUXCNT 	    =	0x8380, 	//发送模块PAMU接口响应流量计数器
    SRI_RCV_INTFIFOST   	        =	0xc000, 	//接收模块接口队列状态寄存器
    SRI_RCV_XBARST  	            =	0xc080, 	//接收模块交叉开关状态寄存器
    SRI_RCV_ERR_SCENE0	            =	0xc100, 	//接收模块错误现场寄存器0
    SRI_RCV_ERR_SCENE1 	            =	0xc180, 	//接收模块错误现场寄存器1
    SRI_RCV_PAIUINT_ACKFLUXCNT0   	=	0xc200, 	//接收模块PAIU接口响应流量计数器0
    SRI_RCV_PAIUINT_ACKFLUXCNT1    	=	0xc280, 	//接收模块PAIU接口响应流量计数器1
    SRI_RCV_PAMUINT_RQ1FLUXCNT  	=	0xc300, 	//接收模块PAMU接口一次请求流量计数器

#endif
#ifdef CONFIG_SW_7
	SRI_ADVW_ECCSERR_CNT             =   0x0000,        //单错预警计数器                       
	SRI_ECCSERR_ADVWARN              =   0x0080,        //单错预警配置寄存器                   
	SRI_REQSEND_ERR                  =   0x0100,        //请求发送模块错误寄存器               
	SRI_REQSEND_ERRSET               =   0x0180,        //请求发送模块错误设置寄存器           
	SRI_REQSEND_ERRMASK              =   0x0200,        //请求发送模块错误屏蔽寄存器           
	SRI_ACKSEND_ERR                  =   0x0280,        //响应发送模块错误寄存器               
	SRI_ACKSEND_ERRSET               =   0x0300,        //响应发送模块错误设置寄存器           
	SRI_ACKSEND_ERRMASK              =   0x0380,        //响应发送模块错误屏蔽寄存器           
	SRI_REQRECV_ERR                  =   0x0400,        //请求接收模块错误寄存器               
	SRI_REQRECV_ERRSET               =   0x0480,        //请求接收模块错误设置寄存器           
	SRI_REQRECV_ERRMASK              =   0x0500,        //请求接收模块错误屏蔽寄存器           
	SRI_ACKRECV_ERR                  =   0x0580,        //响应接收模块错误寄存器               
	SRI_ACKRECV_ERRSET               =   0x0600,        //响应接收模块错误设置寄存器           
	SRI_ACKRECV_ERRMASK              =   0x0680,        //响应接收模块错误屏蔽寄存器           
	SRI_IOERR                        =   0x0700,        //IO错误寄存器                         
	SRI_IOERR_SET                    =   0x0780,        //IO错误设置寄存器                     
	SRI_IOERR_MASK                   =   0x0800,        //IO错误屏蔽寄存器                     
	SRI_SRI_CLR                      =   0x0880,        //清错误现场寄存器                     
	SRI_COMBINE_EN                   =   0x0900,        //请求发送合并使能寄存器               
	SRI_FLYREQ_CNT0                  =   0x0980,        //飞行请求计数器0                      
	SRI_FLYREQ_CNT1                  =   0x0a00,        //飞行请求计数器1                      
	SRI_REQSEND_FIFOST               =   0x8000,        //请求发送模块队列状态寄存器           
	SRI_REQSEND_CRDST                =   0x8080,        //请求发送模块信用寄存器               
	SRI_REQSEND_XBAR_PORTST          =   0x8100,        //请求发送模块交叉开关端口状态寄存器   
	SRI_REQSEND_ERR_SCENE0           =   0x8180,        //请求发送模块错误现场寄存器0          
	SRI_REQSEND_ERR_SCENE1           =   0x8200,        //请求发送模块错误现场寄存器1          
	SRI_REQSEND_FLUX_CNT0            =   0x8280,        //请求发送模块接口0流量计数器          
	SRI_REQSEND_FLUX_CNT1            =   0x8300,        //请求发送模块接口1流量计数器          
	SRI_ACKSEND_FIFOST               =   0xa000,        //响应发送模块队列状态寄存器           
	SRI_ACKSEND_CRDST                =   0xa080,        //响应发送模块信用寄存器               
	SRI_ACKSEND_XBAR_PORTST          =   0xa100,        //响应发送模块交叉开关端口状态寄存器   
	SRI_ACKSEND_ERR_SCENE0           =   0xa180,        //响应发送模块错误现场寄存器0          
	SRI_ACKSEND_ERR_SCENE1           =   0xa200,        //响应发送模块错误现场寄存器1          
	SRI_ACKSEND_FLUX_CNT             =   0xa280,        //响应发送模块接口流量计数器           
	SRI_REQRECV_FIFOST               =   0xc000,        //请求接收模块队列状态寄存器           
	SRI_REQRECV_CRDST                =   0xc080,        //请求接收模块信用寄存器               
	SRI_REQRECV_XBAR_PORTST          =   0xc100,        //请求接收模块交叉开关端口状态寄存器   
	SRI_REQRECV_ERR_SCENE0           =   0xc180,        //请求接收模块错误现场寄存器0          
	SRI_REQRECV_ERR_SCENE1           =   0xc200,        //请求接收模块错误现场寄存器1          
	SRI_REQRECV_FLUX_CNT             =   0xc280,        //请求接收模块接口流量计数器           
	SRI_ACKRECV_FIFOST               =   0xe000,        //响应接收模块队列状态寄存器           
	SRI_ACKRECV_CRDST                =   0xe080,        //响应接收模块信用寄存器               
	SRI_ACKRECV_XBAR_PORTST          =   0xe100,        //响应接收模块交叉开关端口状态寄存器   
	SRI_ACKRECV_ERR_SCENE0           =   0xe180,        //响应接收模块错误现场寄存器0          
	SRI_ACKRECV_ERR_SCENE1           =   0xe200,        //响应接收模块错误现场寄存器1          
	SRI_ACKRECV_FLUX_CNT0            =   0xe280,        //响应接收模块接口0流量计数器          
	SRI_ACKRECV_FLUX_CNT1            =   0xe300,        //响应接收模块接口1流量计数器          
#endif
};

/* PAMU */
#define PAMU_READ_REG(arrayid, index)  \
	SWIO_READ_REG((((long)arrayid<<ARRAYID_BITS_SHIFT)|(index)))
#define PAMU_WRITE_REG(arrayid, index, val)  \
	SWIO_WRITE_REG((((long)arrayid<<ARRAYID_BITS_SHIFT) | (index)), val)
enum PAMURegisters {
	PAMU_ERR                     =   PAMU_IOBITS | 0x0000,     //PAMU错误寄存器                                 
	PAMU_ERR_MASK                =   PAMU_IOBITS | 0x0080,     //PAMU错误屏蔽寄存器                             
	PAMU_ERR_SET                 =   PAMU_IOBITS | 0x0100,     //PAMU错误设置寄存器                             
	PAMU_TIMEOUT_CFG             =   PAMU_IOBITS | 0x0180,     //PAMU超时配置寄存器                             
	PA_EXCPT                     =   PAMU_IOBITS | 0x0200,     //PA异常寄存器                                   
	PA_ERR                       =   PAMU_IOBITS | 0x0280,     //PA不可纠错寄存器                               
	PA_FAULT                     =   PAMU_IOBITS | 0x0300,     //PA故障寄存器                                   
	PA_RERR                      =   PAMU_IOBITS | 0x0380,     //PA可纠错寄存器                                 
	PA_WARN                      =   PAMU_IOBITS | 0x0400,     //PA预警寄存器                                   
	PA_SINT                      =   PAMU_IOBITS | 0x0480,     //PA系统中断寄存器                               
    PA_SOFTRST                   =   PAMU_IOBITS | 0x0500,     //PA阵列软复位寄存器                             
	PA_SPECLKEN                  =   PAMU_IOBITS | 0x0580,     //PA时钟切换使能寄存器                           
	PA_BREAK                     =   PAMU_IOBITS | 0x0600,     //PA从核断连寄存器                               
	SPE_ONLINE                   =   PAMU_IOBITS | 0x0680,     //SPE从核在位寄存器                              
	SPE_BREAK                    =   PAMU_IOBITS | 0x0700,     //SPE从核断连寄存器                              
	PAMU_TEST                    =   PAMU_IOBITS | 0x0780,     //PAMU测试寄存器                                 
	PAMU_MPEIO_ST                =   PAMU_IOBITS | 0x0800,     //PAMU主核访IO接口状态寄存器                     
	PAMU_MPEIO_HEAD0             =   PAMU_IOBITS | 0x0880,     //PAMU主核访IO包头寄存器0，包头[63:0]位          
	PAMU_MPEIO_HEAD1             =   PAMU_IOBITS | 0x0900,     //PAMU主核访IO包头寄存器1，包头[127:64]位        
	PAMU_MPEIO_MISC              =   PAMU_IOBITS | 0x0980,     //PAMU主核访IO维护信息寄存器                     
	PAMU_MPEIO_EXCEPINF0         =   PAMU_IOBITS | 0x0a00,     //PAMU主核访IO异常信息寄存器0，异常信息[63:0]位  
	PAMU_MPEIO_EXCEPINF1         =   PAMU_IOBITS | 0x0a80,     //PAMU主核访IO异常信息寄存器1，异常信息[127:64]位
	PAMU_MTC_REQ                 =   PAMU_IOBITS | 0x0b00,     //PAMU中MTC访IO请求信息寄存器                    
	PAMU_MTC_MISC                =   PAMU_IOBITS | 0x0b80,     //PAMU中MTC访IO维护信息寄存器                    
	PAMU_SCANIO_REQDATA0L        =   PAMU_IOBITS | 0x0c00,     //PAMU中扫描批量转IO第0个访问低64位寄存器        
	PAMU_SCANIO_REQDATA0H        =   PAMU_IOBITS | 0x0c80,     //PAMU中扫描批量转IO第0个访问高64位寄存器        
	PAMU_SCANIO_REQDATA1L        =   PAMU_IOBITS | 0x0d00,     //PAMU中扫描批量转IO第1个访问低64位寄存器        
	PAMU_SCANIO_REQDATA1H        =   PAMU_IOBITS | 0x0d80,     //PAMU中扫描批量转IO第1个访问高64位寄存器        
	PAMU_SCANIO_REQDATA2L        =   PAMU_IOBITS | 0x0e00,     //PAMU中扫描批量转IO第2个访问低64位寄存器        
	PAMU_SCANIO_REQDATA2H        =   PAMU_IOBITS | 0x0e80,     //PAMU中扫描批量转IO第2个访问高64位寄存器        
	PAMU_SCANIO_REQDATA3L        =   PAMU_IOBITS | 0x0f00,     //PAMU中扫描批量转IO第3个访问低64位寄存器        
	PAMU_SCANIO_REQDATA3H        =   PAMU_IOBITS | 0x0f80,     //PAMU中扫描批量转IO第3个访问高64位寄存器        
	PAMU_SCANIO_REQDATA4L        =   PAMU_IOBITS | 0x1000,     //PAMU中扫描批量转IO第4个访问低64位寄存器        
	PAMU_SCANIO_REQDATA4H        =   PAMU_IOBITS | 0x1080,     //PAMU中扫描批量转IO第4个访问高64位寄存器        
	PAMU_SCANIO_REQDATA5L        =   PAMU_IOBITS | 0x1100,     //PAMU中扫描批量转IO第5个访问低64位寄存器        
	PAMU_SCANIO_REQDATA5H        =   PAMU_IOBITS | 0x1180,     //PAMU中扫描批量转IO第5个访问高64位寄存器        
	PAMU_SCANIO_REQDATA6L        =   PAMU_IOBITS | 0x1200,     //PAMU中扫描批量转IO第6个访问低64位寄存器        
	PAMU_SCANIO_REQDATA6H        =   PAMU_IOBITS | 0x1280,     //PAMU中扫描批量转IO第6个访问高64位寄存器        
	PAMU_SCANIO_REQDATA7L        =   PAMU_IOBITS | 0x1300,     //PAMU中扫描批量转IO第7个访问低64位寄存器        
	PAMU_SCANIO_REQDATA7H        =   PAMU_IOBITS | 0x1380,     //PAMU中扫描批量转IO第7个访问高64位寄存器        
	PAMU_SCANIO_MISC             =   PAMU_IOBITS | 0x1400,     //PAMU中扫描批量转IO访问维护信息寄存器           
	PAMU_REQARB                  =   PAMU_IOBITS | 0x1480,     //PAMU访问请求仲裁维护信息寄存器                 
	PAMU_IO_ALLOCATE0            =   PAMU_IOBITS | 0x1500,     //PAMU访问请求分配维护信息寄存器0（从核时钟阈）  
	PAMU_IO_ALLOCATE1            =   PAMU_IOBITS | 0x1580,     //PAMU访问请求分配维护信息寄存器1（维护时钟阈）  
	PAMU_ALLOCATE_EXCEPINF       =   PAMU_IOBITS | 0x1600,     //PAMU访问请求分配异常信息寄存器                 
	PAMU_ALLOCATE_LAST1IO        =   PAMU_IOBITS | 0x1680,     //PAMU当前IO访问信息寄存器                       
	PAMU_ALLOCATE_LAST2IO        =   PAMU_IOBITS | 0x1700,     //PAMU最近第1次IO访问信息寄存器                  
	PAMU_ALLOCATE_LAST3IO        =   PAMU_IOBITS | 0x1780,     //PAMU最近第2次IO访问信息寄存器                  
	PAMU_ALLOCATE_LAST4IO        =   PAMU_IOBITS | 0x1800,     //PAMU最近第3次IO访问信息寄存器                  
	PAMU_ALLOCATE_NACKINF        =   PAMU_IOBITS | 0x1880,     //PAMU（访问发生外部异常响应）响应错误现场寄存器 
	PAMU_BISTINF                 =   PAMU_IOBITS | 0x1900,     //PAMU BIST信息寄存器                            
};
#define LDM_IOBITS (LDM_HW_BASE | PAGE_OFFSET)

/* master read/write slave ldm, use for debug only */
#define LDM_READ_8B(cgid,speid,offset)               \
	SWIO_READ_REG((LDM_IOBITS | ((long)cgid << ARRAYID_BITS_SHIFT) | ((long)speid << 24) | offset))
#define LDM_READ_4B(cgid, speid, offset)	\
	readw((void*)(LDM_IOBITS | ((long)cgid << ARRAYID_BITS_SHIFT) | ((long)speid << 24) | offset))

#define LDM_WRITE_8B(cgid,speid,offset,val)	\
	SWIO_WRITE_REG((LDM_IOBITS | ((long)cgid << ARRAYID_BITS_SHIFT) | ((long)speid << 24) | offset), val)
#define LDM_WRITE_4B(cgid,speid,offset,val)	\
	writew(val, (void*)(LDM_IOBITS | ((long)cgid << ARRAYID_BITS_SHIFT) | ((long)speid << 24) | offset))

#define LDM_BROADCAST_WRITE_8B(cgid,speid,offset,val)     \
	SWIO_READ_REG((LDM_IOBITS | ((long)cgid << ARRAYID_BITS_SHIFT) | ((long)speid << 24) | offset | SPE_BROADCAST_INDEX))
#define LDM_BROADCAST_WRITE_4B(cgid,speid,offset,val)	\
	writew(val, (void*)(LDM_IOBITS | ((long)cgid << ARRAYID_BITS_SHIFT) | ((long)speid << 24) | offset | SPE_BROADCAST_INDEX))


#endif
