// clang-format off
/* ----------------------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   https://www.lammps.org/, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

#include "memory.h"
#include "comm.h"
#include "error.h"

#if defined(LMP_INTEL) && \
  ((defined(__INTEL_COMPILER) || defined(__INTEL_LLVM_COMPILER)))
#ifndef LMP_INTEL_NO_TBB
#define LMP_USE_TBB_ALLOCATOR
#include "tbb/scalable_allocator.h"
#else
#include <cstring>
#if defined(__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#endif
#endif

#if defined(LMP_INTEL) && !defined(LAMMPS_MEMALIGN) && !defined(_WIN32)
#define LAMMPS_MEMALIGN 64
#endif
//#define __sw_logmem__
using namespace LAMMPS_NS;

/* ---------------------------------------------------------------------- */

Memory::Memory(LAMMPS *lmp) : Pointers(lmp) {}

/* ----------------------------------------------------------------------
   safe malloc
------------------------------------------------------------------------- */

void *Memory::smalloc(bigint nbytes, const char *name)
{
  if (nbytes == 0) return nullptr;

#if defined(LAMMPS_MEMALIGN)
  void *ptr;

#if defined(LMP_USE_TBB_ALLOCATOR)
  ptr = scalable_aligned_malloc(nbytes, LAMMPS_MEMALIGN);
#elif defined(__sw_64__)
  ptr = malloc(nbytes + LAMMPS_MEMALIGN+2);//posix_memalign(&ptr, LAMMPS_MEMALIGN, nbytes);
  if (ptr){
    long iptr = (long)ptr;
    iptr = (iptr + LAMMPS_MEMALIGN+2) / LAMMPS_MEMALIGN * LAMMPS_MEMALIGN;
    *((void**)iptr - 1) = ptr;
    *((long*)iptr - 2) = nbytes;
#ifdef __sw_logmem__
    if (comm && comm->me == 0)
      printf("%s %p %p %d %d\n", name, ptr, iptr, (long)iptr - (long)ptr, nbytes);
#endif
    return (void*)iptr;
  }
#else
  int retval = posix_memalign(&ptr, LAMMPS_MEMALIGN, nbytes);
  if (retval) ptr = nullptr;
#endif

#else
  void *ptr = malloc(nbytes);
#endif
  if (ptr == nullptr)
    error->one(FLERR,"Failed to allocate {} bytes for array {}",
	       nbytes,name);
  return ptr;
}

/* ----------------------------------------------------------------------
   safe realloc
------------------------------------------------------------------------- */

void *Memory::srealloc(void *ptr, bigint nbytes, const char *name)
{
  if (nbytes == 0) {
    destroy(ptr);
    return nullptr;
  }
#if defined(__sw_64__) && defined(LAMMPS_MEMALIGN)
  void *newptr = smalloc(nbytes, name);
  if (ptr != nullptr){
    void *oldptr = *((void**)ptr - 1);
    bigint oldsize = *((long*)ptr - 2);
    #ifdef __sw_logmem__
    if (comm && comm->me == 0)
      printf("%s %p %d\n", name, oldptr, oldsize);
    #endif
    memcpy(newptr, ptr, oldsize);
    sfree(ptr);
  }
  return newptr;
#endif
#if defined(LMP_USE_TBB_ALLOCATOR)
  ptr = scalable_aligned_realloc(ptr, nbytes, LAMMPS_MEMALIGN);
#elif defined(LMP_INTEL_NO_TBB) && defined(LAMMPS_MEMALIGN) &&	\
  (defined(__INTEL_COMPILER) || defined(__INTEL_LLVM_COMPILER))

  ptr = realloc(ptr, nbytes);
  uintptr_t offset = ((uintptr_t)(const void *)(ptr)) % LAMMPS_MEMALIGN;
  if (offset) {
    void *optr = ptr;
    ptr = smalloc(nbytes, name);
#if defined(__APPLE__)
    memcpy(ptr, optr, MIN(nbytes,malloc_size(optr)));
#elif defined(_WIN32) || defined(__MINGW32__)
    memcpy(ptr, optr, MIN(nbytes,_msize(optr)));
#else
    memcpy(ptr, optr, MIN(nbytes,malloc_usable_size(optr)));
#endif
    free(optr);
  }
#else
  ptr = realloc(ptr,nbytes);
#endif
  if (ptr == nullptr)
    error->one(FLERR,"Failed to reallocate {} bytes for array {}",
	       nbytes,name);
  return ptr;
}

/* ----------------------------------------------------------------------
   safe free
------------------------------------------------------------------------- */

void Memory::sfree(void *ptr)
{
  if (ptr == nullptr) return;
  #if defined(LMP_USE_TBB_ALLOCATOR)
  scalable_aligned_free(ptr);
#elif defined(__sw_64__) && defined(LMP_MEMALIGN)
  void *orig = *((void**)ptr - 1);
  free(orig);
  #else
  free(ptr);
  #endif
}

/* ----------------------------------------------------------------------
   erroneous usage of templated create/grow functions
------------------------------------------------------------------------- */

void Memory::fail(const char *name)
{
  error->one(FLERR,"Cannot create/grow a vector/array of "
                               "pointers for {}",name);
}
