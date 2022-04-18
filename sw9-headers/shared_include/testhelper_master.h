/********************************************************************************

*

* �ļ�: trendtest.h

*

* �汾: 1.0 (2004)

* ��д: �ƴ�� 2004-01-15( 21:13:16 )

* ��Ȩ: �ƴ����������Ȩ���У�

*

* ����:��Դ����İ�Ȩ�ܡ��л����񹲺͹�����Ȩ�����Լ�������ط��ɺ���Լ�ı�����

*      �κ��л���õ���Դ����ĸ��˺ͻ�����δ��������ȷ��Ȩ�����ý���Դ��������

*      �κ���ҵĿ��(ֱ�ӵĻ��ӵ�)�����ڷ���ҵĿ�ĵ�ʹ�� (�������ơ�����������

*      ���޸�)�� ԭ����û���ر�������������������ĵ���˵������Դ��������ԭ

*      ���ߵ�����Ȩ��

*

* ��ҳ: http://soft.jn/  E-mail: tdg@soft.jn

*

***********************************************************************************/



#ifndef _TRENDTEST_H_

#define _TRENDTEST_H_


#ifndef TREND_NONETESTHELPER

#define TREND_TESTHELPER_SIGNAL


//enum{TTH_INDEPENDENT=0x2,TTH_EACH=0x4,TTH_AUTOZIP=0x8,TTH_PRINTINFO=0x10};
enum{TTH_SIGNAL=0x1,TTH_INDEPENDENT=0x2,TTH_EACH=0x4,TTH_AUTOZIP=0x8,TTH_PRINTINFO=0x10};;


//#define TTH_DEFAULT (TTH_SIGNAL|TTH_INDEPENDENT|TTH_AUTOZIP|TTH_PRINTINFO)
#define TTH_DEFAULT (TTH_INDEPENDENT|TTH_AUTOZIP|TTH_PRINTINFO)
#ifdef __cplusplus
extern "C" {
#endif

extern int testhelper_init_(int len);
extern int testhelper_init2_(int len,char* md5file);
extern int testhelper_init3_(int len,char* md5file,int flags);
extern int checksumvar_(void* data,int size,char* srcfile,int lineno);
extern int checkpointvar_(char* srcfile,int lineno);

extern int checksumvarori_(void* data,int size,char* srcfile,int lineno);
extern int checkpointvarori_(char* srcfile,int lineno);

extern int checkpointfile_(char * datafile,char* srcfile,int lineno);

extern void CHECKSUM_PRINTF_TOLOG(char *format, ...);
extern void PRINTF_TOLOG(char *format, ...);
extern void CHECKSUM_PRINTF(char *format, ...);

extern char* GetMD5FileNameFromArgv(int argc,char**argv);

#ifdef __cplusplus
}
#endif

	#define TESTHELPER_INIT(len)	{testhelper_init3_(len,NULL,TTH_DEFAULT);}
	#define TESTHELPER_INIT2(len,md5file)	{testhelper_init3_(len,md5file,TTH_DEFAULT);}
	#define TESTHELPER_INIT3(len,md5file,flags)	{testhelper_init3_(len,md5file,flags);}

	#define CHECKSUMVAR(data,size)	{checksumvar_(data,size,__FILE__,__LINE__);}
	#define CHECKSUMVARORI(data,size)	{checksumvarori_(data,size,__FILE__,__LINE__);}
	#define CHECKSUMINT(data)	       {checksumvar_(data,sizeof(int),__FILE__,__LINE__);}
	#define CHECKSUMLONG(data)		{checksumvar_(data,sizeof(long),__FILE__,__LINE__);}
	#define CHECKSUMFLOAT(data)		{checksumvar_(data,sizeof(float),__FILE__,__LINE__);}
	#define CHECKSUMDOUBLE(data)	{checksumvar_(data,sizeof(double),__FILE__,__LINE__);}
	#define CHECKSUMSTRING(data)	{checksumvar_(data,strlen(data),__FILE__,__LINE__);}

	#define CHECKPOINTFILE(file)	{checkpointfile_(file,__FILE__,__LINE__);}

	#define CHECKPOINTVAR()			checkpointvar_(__FILE__,__LINE__)
	#define CHECKPOINTVARORI()			checkpointvarori_(__FILE__,__LINE__)

	#define GETMD5FILEFROMARGV(argc,argv)		GetMD5FileNameFromArgv(argc,argv)

	#define ASSERT(exp,msg)				\
	{\
	if( (exp)==0 )\
		printf("assert failed(%s:%d):%s\n",__FILE__,__LINE__,msg);\
	}
	#define ASSERT2(exp,msg,filename,lineno)				\
	{\
	if( (exp)==0 )\
		printf("assert failed(%s:%d):%s\n",filename,lineno,msg);\
	}

#else
	#define TESTHELPER_INIT(len)			{}
	#define TESTHELPER_INIT2(len,md5file)		{}
	#define TESTHELPER_INIT3(len,md5file,flags)	{}
	#define CHECKSUMVAR(data,size)			{}
	#define CHECKSUMINT(data)			{}
	#define CHECKSUMLONG(data)			{}
	#define CHECKSUMFLOAT(data)			{}
	#define CHECKSUMDOUBLE(data)			{}
	#define CHECKSUMSTRING(data)			{}
	#define CHECKPOINTFILE(file)			{}
	#define CHECKPOINTVAR()				{}
	#define ASSERT(exp,msg)				{}
	#define ASSERT2(exp,msg,filename,lineno)	{}
	#define GETMD5FILEFROMARGV(argc,argv)		
#endif

#endif
