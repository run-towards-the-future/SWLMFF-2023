#ifndef __ASM_SW_64_ELF_H
#define __ASM_SW_64_ELF_H
/* Special values for the st_other field in the symbol table.  */

#define STO_SW_64_NOPV		0x80
#define STO_SW_64_STD_GPLOAD	0x88

/*
 * SW-64 ELF relocation types
 */
#define R_SW_64_NONE            0       /* No reloc */
#define R_SW_64_REFLONG         1       /* Direct 32 bit */
#define R_SW_64_REFQUAD         2       /* Direct 64 bit */
#define R_SW_64_GPREL32         3       /* GP relative 32 bit */
#define R_SW_64_LITERAL         4       /* GP relative 16 bit w/optimization */
#define R_SW_64_LITUSE          5       /* Optimization hint for LITERAL */
#define R_SW_64_GPDISP          6       /* Add displacement to GP */
#define R_SW_64_BRADDR          7       /* PC+4 relative 23 bit shifted */
#define R_SW_64_HINT            8       /* PC+4 relative 16 bit shifted */
#define R_SW_64_SREL16          9       /* PC relative 16 bit */
#define R_SW_64_SREL32          10      /* PC relative 32 bit */
#define R_SW_64_SREL64          11      /* PC relative 64 bit */
#define R_SW_64_GPRELHIGH       17      /* GP relative 32 bit, high 16 bits */
#define R_SW_64_GPRELLOW        18      /* GP relative 32 bit, low 16 bits */
#define R_SW_64_GPREL16         19      /* GP relative 16 bit */
#define R_SW_64_COPY            24      /* Copy symbol at runtime */
#define R_SW_64_GLOB_DAT        25      /* Create GOT entry */
#define R_SW_64_JMP_SLOT        26      /* Create PLT entry */
#define R_SW_64_RELATIVE        27      /* Adjust by program base */
#define R_SW_64_BRSGP		28
#define R_SW_64_TLSGD           29
#define R_SW_64_TLS_LDM         30
#define R_SW_64_DTPMOD64        31
#define R_SW_64_GOTDTPREL       32
#define R_SW_64_DTPREL64        33
#define R_SW_64_DTPRELHI        34
#define R_SW_64_DTPRELLO        35
#define R_SW_64_DTPREL16        36
#define R_SW_64_GOTTPREL        37
#define R_SW_64_TPREL64         38
#define R_SW_64_TPRELHI         39
#define R_SW_64_TPRELLO         40
#define R_SW_64_TPREL16         41

#define SHF_SW_64_GPREL		0x10000000

/* Legal values for e_flags field of Elf64_Ehdr.  */

#define EF_SW_64_32BIT		1	/* All addresses are below 2GB */

/*
 * ELF register definitions..
 */

/*
 * The OSF/1 version of <sys/procfs.h> makes gregset_t 46 entries long.
 * I have no idea why that is so.  For now, we just leave it at 33
 * (32 general regs + processor status word). 
 */
#define ELF_NGREG	33
#define ELF_NFPREG	32


typedef unsigned long elf_greg_t;
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef double elf_fpreg_t;
typedef elf_fpreg_t elf_fpregset_t[ELF_NFPREG];

/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(x) (((x)->e_machine == EM_SW_64) ||((x)->e_machine == EM_ALPHA))

/*
 * These are used to set parameters in the core dumps.
 */
#define ELF_CLASS	ELFCLASS64
#define ELF_DATA	ELFDATA2LSB
#define ELF_ARCH	EM_SW_64

#define ELF_EXEC_PAGESIZE	8192

/* This is the location that an ET_DYN program is loaded if exec'ed.  Typical
   use of this is to invoke "./ld.so someprog" to test out a new version of
   the loader.  We need to make sure that it is out of the way of the program
   that it will "exec", and that there is sufficient room for the brk.  */

#define ELF_ET_DYN_BASE		(TASK_UNMAPPED_BASE + 0x1000000)

/* $0 is set by ld.so to a pointer to a function which might be 
   registered using atexit.  This provides a mean for the dynamic
   linker to call DT_FINI functions for shared libraries that have
   been loaded before the code runs.

   So that we can use the same startup file with static executables,
   we start programs with a value of 0 to indicate that there is no
   such function.  */

#define ELF_PLAT_INIT(_r, load_addr)	_r->r0 = 0

/* The registers are laid out in pt_regs for PAL and syscall
   convenience.  Re-order them for the linear elf_gregset_t.  */

#endif /* __ASM_SW_64_ELF_H */
