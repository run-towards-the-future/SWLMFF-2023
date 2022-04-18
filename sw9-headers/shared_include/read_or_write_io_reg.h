#include <stdio.h>
#include <stdlib.h>

unsigned long get_cg_id()
{
#ifdef FPGA_TEST
    unsigned long myid;
    __asm__ __volatile__(
                        "rcid %0\n"
                        :"=r"(myid)
                        );
    printf("cgid is %d\n",(myid&0xff)>>2);
    return ((myid&0xff)>>2);
#else
    return 0;
#endif
}
unsigned long  sw_wrpa(int read_write, unsigned long  pa, unsigned long val)
{
	register unsigned long __r0 __asm__("$0");
	register unsigned long __r16 __asm__("$16") = read_write;
	register unsigned long __r17 __asm__("$17") = pa;
	register unsigned long __r18 __asm__("$18") = val;

	__asm__ __volatile__(
			"sys_call %4"
			: "=r"(__r0), "=r"(__r16), "=r"(__r17), "=r"(__r18)
			: "i"(0xB3), "0"(__r16), "1"(__r17), "2"(__r18)
			: "$1", "$22", "$23", "$24", "$25");

	return __r0;
}
#if 0
void main()
{
	printf("从核阵列IO空间访问\n");
	scan_array_io();
	printf("全芯片IO空间访问\n");
	//scan_chip_io();

}
#endif
/************************************************************************/
/*					read io				*/
/************************************************************************/
unsigned long read_CORE_ONLINE()
{
	int read_write = 1; //0 write,1 read
	unsigned long  pa;
	unsigned long val;

	pa = (((unsigned long)1<<47)|((unsigned long)1<<44)|((unsigned long)1<<43)|((unsigned long)1<<36)|(0x000000fUL<<7));
	val = sw_wrpa(read_write,pa,val);
	printf("%#lx : 0x%lx\n",pa,val);fflush(NULL);
	return val;
}
