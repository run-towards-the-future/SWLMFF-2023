#ifndef __SW_SETUP_H
#define __SW_SETUP_H

/*#define COMMAND_LINE_SIZE	256*/
#define COMMAND_LINE_SIZE       512

/*
 * We leave one page for the initial stack page, and one page for
 * the initial process structure. Also, the console eats 3 MB for
 * the initial bootloader (one of which we can reclaim later).
 */
#define BOOT_PCB	0x20000000
#define BOOT_ADDR	0x20000000
/* Remove when official MILO sources have ELF support: */
#define BOOT_SIZE	(16*1024)

#define KERNEL_START_PHYS	0x900000 /* bootloaders hardcoded this.  */

#define KERNEL_START	(__START_KERNEL_map+KERNEL_START_PHYS)
#define SWAPPER_PGD	KERNEL_START
#define INIT_STACK	(__START_KERNEL_map+KERNEL_START_PHYS+0x02000)
#define EMPTY_PGT	(__START_KERNEL_map+KERNEL_START_PHYS+0x04000)
#define EMPTY_PGE	(__START_KERNEL_map+KERNEL_START_PHYS+0x08000)
#define ZERO_PGE	(__START_KERNEL_map+KERNEL_START_PHYS+0x0A000)

#define START_ADDR	(__START_KERNEL_map+KERNEL_START_PHYS+0x10000)

/*
 * This is setup by the secondary bootstrap loader.  Because
 * the zero page is zeroed out as soon as the vm system is
 * initialized, we need to copy things out into a more permanent
 * place.
 */
#define PARAM			ZERO_PGE
#define COMMAND_LINE		((char*)(KERNEL_START + 0x0B000))
/*#define COMMAND_LINE            ((char*)(__START_KERNEL_map+0x410000)) */
#define INITRD_START		(*(unsigned long *) (PARAM+0x100))
#define INITRD_SIZE		(*(unsigned long *) (PARAM+0x108))

#endif
