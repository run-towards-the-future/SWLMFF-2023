/********************************************************************
 * Author		: Qi ZHU
 * Date			: 2021/01/22
 * Name			: swckpt.h
 * Description	: Head file of sw light-weight checkpoint library. 
 * ******************************************************************/

#ifndef SWCKPT_H
#define SWCKPT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define CR_EMBED_CHKPT 1000
#define CR_EMBED_RESUME 1001
#define CR_SYNC_INIT 1002
#define CR_DIG_HOLE 1003
#define CR_RESTORE_FLAG 1009
#define CR_WRITE2FD 1004
#define CR_READ2FD 1005
#define CR_CKPT_ID 1006
#define CR_HANDLE_RESTORE 1007

#define SLAVEIO 0xAA
#define SLAVE_TEST_STABLE_CKPT                     _IOW(SLAVEIO,   0x0c, struct slave_test_stable_ckpt)

#define PAGE_POWER 13
#define PAGE_SIZE (1<<PAGE_POWER)

struct slave_test_stable_ckpt {
    int pid;    
    int arrayid;
    unsigned long coremask;
};

typedef struct ckpt_arg{
    int fd;
    int jobid;
    int rankid;
    char username[32];
}ckpt_arg;

struct fd2_struct{
    int ckpt_fd;
    int dev_fd;
};

typedef struct addr_noneedsave_struct{
    unsigned long start;
    unsigned long size; 
}addr_noneed_struct;

extern ckpt_arg _sw_ckpt_para;
extern int _sw_ckpt_dev_fd;
extern struct timeval tv_global;

extern int sw_ckpt_init();
extern int sw_ckpt_exclude_space(unsigned long, unsigned long);
extern int sw_ckpt_save_real();
extern int sw_ckpt_save(long, int);
extern int sw_ckpt_recover();

#endif
