#ifndef _ALPHA_PAGE_H
#define _ALPHA_PAGE_H

#include <linux/const.h>
#include <asm/pal.h>

/* PAGE_SHIFT determines the page size */
#define PAGE_SHIFT	13
#define PAGE_SIZE	(_AC(1,UL) << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define HPAGE_SHIFT             PMD_SHIFT
#define HPAGE_SIZE              (_AC(1,UL) << HPAGE_SHIFT)
#define HPAGE_MASK              (~(HPAGE_SIZE - 1))
#define HUGETLB_PAGE_ORDER      (HPAGE_SHIFT - PAGE_SHIFT)

#define HUGE_MAX_HSTATE 2


#endif /* _ALPHA_PAGE_H */
