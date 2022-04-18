/* Copyright (C) 1997-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_STATFS_H
# error "Never include <bits/statfs.h> directly; use <sys/statfs.h> instead."
#endif

#include <bits/types.h>  /* for __fsid_t and __fsblkcnt_t.  */

#ifdef HUANGLM20161111
struct statfs
  {
    int f_type;
    int f_bsize;
#ifndef __USE_FILE_OFFSET64
    __fsblkcnt_t f_blocks;
    __fsblkcnt_t f_bfree;
    __fsblkcnt_t f_bavail;
    __fsfilcnt_t f_files;
    __fsfilcnt_t f_ffree;
#else
    __fsblkcnt64_t f_blocks;
    __fsblkcnt64_t f_bfree;
    __fsblkcnt64_t f_bavail;
    __fsfilcnt64_t f_files;
    __fsfilcnt64_t f_ffree;
#endif
    __fsid_t f_fsid;
    int f_namelen;
    int f_frsize;
    int f_flags;
    int f_spare[4];
  };
#else
#ifndef __statfs_word
#define __statfs_word long
#endif
struct statfs
  {
    __statfs_word f_type;
    __statfs_word f_bsize;
    __statfs_word f_blocks;
    __statfs_word f_bfree;
    __statfs_word f_bavail;
    __statfs_word f_files;
    __statfs_word f_ffree;
    __fsid_t f_fsid;
    __statfs_word f_namelen;
    __statfs_word f_frsize;
    __statfs_word f_flags;
    __statfs_word f_spare[4];
};

#endif

#ifdef HUANGLM20161111
#ifdef __USE_LARGEFILE64
struct statfs64
  {
    int f_type;
    int f_bsize;
    __fsblkcnt64_t f_blocks;
    __fsblkcnt64_t f_bfree;
    __fsblkcnt64_t f_bavail;
    __fsfilcnt64_t f_files;
    __fsfilcnt64_t f_ffree;
    __fsid_t f_fsid;
    int f_namelen;
    int f_frsize;
    int f_flags;
    int f_spare[4];
  };
#endif
#else
#ifdef __USE_LARGEFILE64
struct statfs64
  {
    long f_type;
    long f_bsize;
    __fsblkcnt64_t f_blocks;
    __fsblkcnt64_t f_bfree;
    __fsblkcnt64_t f_bavail;
    __fsfilcnt64_t f_files;
    __fsfilcnt64_t f_ffree;
    __fsid_t f_fsid;
    long f_namelen;
    long f_frsize;
    long f_flags;
    long f_spare[4];
  };
#endif
#endif

/* Tell code we have this member.  */
#define _STATFS_F_NAMELEN
#define _STATFS_F_FRSIZE
