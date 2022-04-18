#ifndef _SW_64_STATFS_H
#define _SW_64_STATFS_H

#include <linux/types.h>

/* SW-64 is the only 64-bit platform with 32-bit statfs. And doesn't
   even seem to implement statfs64 */
#ifdef CONFIG_SW64_WITH_32BIT_STATFS 
#define __statfs_word __u32
#endif

#include <asm-generic/statfs.h>

#endif
