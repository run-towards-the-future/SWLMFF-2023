#ifndef _SW_64_TYPES_H
#define _SW_64_TYPES_H

/*
 * This file is never included by application software unless
 * explicitly requested (e.g., via linux/types.h) in which case the
 * application is Linux specific so (user-) name space pollution is
 * not a major issue.  However, for interoperability, libraries still
 * need to be careful to avoid a name clashes.
 */

#include <asm-generic/int-l64.h>

#endif /* _SW_64_TYPES_H */
