#ifndef __SLAVE_IO__
#define __SLAVE_IO__


#ifdef CONFIG_SW_7
#include <slave7io.h>
#endif

#ifdef CONFIG_SW_9
#include <slave9io.h>
#endif

#ifdef CONFIG_SW_AI
#include <slaveaiio.h>
#endif

#endif
