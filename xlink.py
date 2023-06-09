import traceback
import struct
import sys
import re
import collections

ELF64_HDR_FMT = "10sHHIQQQIHHHHHH"
ELF64_Hdr = collections.namedtuple('ELF64_Hdr', ['e_ident', 'e_type', 'e_machine', 'e_version',
                                                 'e_entry', 'e_phoff', 'e_shoff', 'e_flags',
                                                 'e_ehsize', 'e_phentsize', 'e_phnum', 'e_shentsize',
                                                  'e_shnum', 'e_shstrndx'])
ELF64_SHDR_FMT = "IIQQQQIIQQ"
ELF64_Shdr = collections.namedtuple('ELF64_Shdr', ['sh_name', 'sh_type', 'sh_flags', 'sh_addr',
                                                   'sh_offset', 'sh_size', 'sh_link', 'sh_info',
                                                   'sh_addralign', 'sh_entsize'])
ELF64_ST_BIND = lambda x: (x & 0xff) >> 4
ELF64_ST_TYPE = lambda x: x & 0xf
ELF64_ST_INFO = lambda bind, type: bind << 4 | type & 0xf

STB_LOCAL      = 0               # Local symbol */
STB_GLOBAL     = 1               # Global symbol */
STB_WEAK       = 2               # Weak symbol */
STB_NUM        = 3               # Number of defined types.  */
STB_LOOS       = 10              # Start of OS-specific */
STB_GNU_UNIQUE = 10              # Unique symbol.  */
STB_HIOS       = 12              # End of OS-specific */
STB_LOPROC     = 13              # Start of processor-specific */
STB_HIPROC     = 15              # End of processor-specific */

STT_NOTYPE     = 0               # Symbol type is unspecified */
STT_OBJECT     = 1               # Symbol is a data object */
STT_FUNC       = 2               # Symbol is a code object */
STT_SECTION    = 3               # Symbol associated with a section */
STT_FILE       = 4               # Symbol's name is file name */
STT_COMMON     = 5               # Symbol is a common data object */
STT_TLS        = 6               # Symbol is thread-local data object*/
STT_NUM        = 7               # Number of defined types.  */
STT_LOOS       = 10              # Start of OS-specific */
STT_GNU_IFUNC  = 10              # Symbol is indirect code object */
STT_HIOS       = 12              # End of OS-specific */
STT_LOPROC     = 13              # Start of processor-specific */
STT_HIPROC     = 15		# End of processor-specific */


#define STN_UNDEF	0		/* End of a chain.  */

ELF64_SYM_FMT = "IBBHQQ"
ELF64_Sym = collections.namedtuple('ELF64_Sym', ['st_name', 'st_info', 'st_other', 'st_shndx', 'st_value', 'st_size'])

class ELF64:
    def __init__(self, bin):
        bin = bytearray(bin)
        ehdr = ELF64_Hdr(*struct.unpack(ELF64_HDR_FMT, bin[0:64]))
        self.bin = bin
        self.ehsize    = ehdr.e_ehsize
        self.phoff     = ehdr.e_phoff
        self.phnum     = ehdr.e_phnum
        self.phentsize = ehdr.e_phentsize
        self.shoff     = ehdr.e_shoff
        self.shnum     = ehdr.e_shnum
        self.shentsize = ehdr.e_shentsize

        shdr_end = self.shoff + self.shnum * self.shentsize
        shdr = list(map(lambda x: ELF64_Shdr(*x), struct.iter_unpack(ELF64_SHDR_FMT, bin[self.shoff : shdr_end])))
        shstrtabhdr = shdr[ehdr.e_shstrndx]
        #shstrdict = ELF64.split_strtab(
        shstrtab = ELF64.get_section_bin(bin, shstrtabhdr)
        self.shstrtab = shstrtab
        #print(shstrtab[341:351], len(shstrtab))
        #print("TTT:", shstrtab[341:20])
        self.shdrs = {}
        self.ishdrs = []
        for hdr in shdr:
            # print(hdr, hdr.sh_name, ELF64.get_str_from_tab(shstrtab, hdr.sh_name))
            self.shdrs[ELF64.get_str_from_tab(shstrtab, hdr.sh_name)] = hdr
            self.ishdrs.append(hdr)
        symtabhdr = self.shdrs['.symtab']
        symtabbin = ELF64.get_section_bin(bin, symtabhdr)
        syms = list(map(lambda x: ELF64_Sym(*x), struct.iter_unpack(ELF64_SYM_FMT, symtabbin)))
        strtabhdr = self.shdrs['.strtab']
        #strdict = ELF64.split_strtab(
        
        strtab = ELF64.get_section_bin(bin, strtabhdr)
        self.strtab = strtab
        #self.sym_name = {}
        self.syms = list(map(lambda sym: sym._replace(st_name = ELF64.get_str_from_tab(strtab, sym.st_name)), syms))
        self.global_syms = {}
        for sym in self.syms:
            if ELF64_ST_BIND(sym.st_info) in [STB_GLOBAL, STB_WEAK]:
                self.global_syms[sym.st_name] = sym
    @staticmethod
    def get_str_from_tab(tab, index):
        return tab[index:].split(b"\0", 1)[0].decode()

    @staticmethod
    def get_section_bin(elf_bin, sec_hdr):
        return elf_bin[sec_hdr.sh_offset : sec_hdr.sh_offset + sec_hdr.sh_size]
import enum
class SW5Rel(enum.Enum):
    NONE       = 0  # /* No reloc */
    REFLONG    = 1  # /* Direct 32 bit */
    REFQUAD    = 2  # /* Direct 64 bit */
    GPREL32    = 3  # /* GP relative 32 bit */
    LITERAL    = 4  # /* GP relative 16 bit w/optimization */
    LITUSE     = 5  # /* Optimization hint for LITERAL */
    GPDISP     = 6  # /* Add displacement to GP */
    BRADDR     = 7  # /* PC+4 relative 23 bit shifted */
    HINT       = 8  # /* PC+4 relative 16 bit shifted */
    SREL16     = 9  # /* PC relative 16 bit */
    SREL32     = 10 # /* PC relative 32 bit */
    SREL64     = 11 # /* PC relative 64 bit */
    GPRELHIGH  = 17 # /* GP relative 32 bit, high 16 bits */
    GPRELLOW   = 18 # /* GP relative 32 bit, low 16 bits */
    GPREL16    = 19 # /* GP relative 16 bit */
    COPY       = 24 # /* Copy symbol at runtime */
    GLOB_DAT   = 25 # /* Create GOT entry */
    JMP_SLOT   = 26 # /* Create PLT entry */
    RELATIVE   = 27 # /* Adjust by program base */
    TLS_GD_HI  = 28
    TLSGD      = 29
    TLS_LDM    = 30
    DTPMOD64   = 31
    GOTDTPREL  = 32
    DTPREL64   = 33
    DTPRELHI   = 34
    DTPRELLO   = 35
    DTPREL16   = 36
    GOTTPREL   = 37
    TPREL64    = 38
    TPRELHI    = 39
    TPRELLO    = 40
    TPREL16    = 41
    LDMLO      = 42
    LDMHI      = 43
RA = 26
T12 = 27
GP = 29
LDI = 0x3e
LDIH = 0x3f
LDL = 0x23
MANGLE_RE = re.compile(r"_Z(?P<ns>N?)(?P<len>\d+)slave_(?P<rest>.*)")
def remangle_slave(mangled_name):
    m = MANGLE_RE.match(mangled_name)
    if m:
        remangle = "slave__Z%s%d%s" % (m["ns"], int(m["len"]) - 6, m["rest"])
        return remangle
    else:
        return None
import subprocess
import os

def remangle_defs(elf):
  ret = []
  if '.rela.text' in elf.shdrs:
    relatext1_offset = elf.shdrs[".rela.text"].sh_offset
    relatext1_size = elf.shdrs[".rela.text"].sh_size
    for i in range(relatext1_offset, relatext1_offset + relatext1_size, 24):
        addr, info, off = struct.unpack("<QQQ", elf.bin[i:i+24])
        target = info >> 32
        target_sym = elf.syms[target]
        if target_sym.st_shndx < len(elf.ishdrs) and elf.ishdrs[target_sym.st_shndx].sh_type == 0:
            if remangle_slave(elf.syms[target].st_name):
                ret.append('--defsym=%s=%s' % (elf.syms[target].st_name, remangle_slave(elf.syms[target].st_name)))
  return ret
def run(argv_link, add_args=[], **kwargs):
  if os.path.basename(argv_link[0]) != 'sw5ld':
    add_args = list(map(lambda x: "-Wl,"+x, add_args))
  if verbose:
      print(argv_link + add_args)
  subprocess.run(argv_link + add_args, **kwargs)
def get_output(argv):
  for i, arg in enumerate(argv):
    if arg == '-o':
      return argv[i+1]
  return "./a.out"
def write_elf(path, binary):
  open(path, "wb").write(binary)
  import os
  import stat
  st = os.stat(path)
  os.chmod(path, st.st_mode | stat.S_IEXEC)
if __name__ == '__main__':
  import sys
  import tempfile
  if len(sys.argv) == 1 or sys.argv[1] == '-h':
    print("xlink.py [-v] <original link command>", file=sys.stderr)
    sys.exit(1)
  verbose = False
  if sys.argv[1] == '-v':
    verbose = True
    sys.argv = sys.argv[1:]
  tempdir = tempfile.mkdtemp(prefix='xlink-')
  try:
    linker = sys.argv[1]
    argv_link = sys.argv[1:].copy()
    dest = get_output(argv_link)
    if os.path.exists(dest):
      os.remove(dest)
    pass1_elf = os.path.join(tempdir, 'pass1_elf')
    add_args_pass0 = ['-q', '--unresolved-symbols=ignore-all', '-o', pass1_elf]
    run(argv_link, add_args_pass0)
    elf1 = ELF64(open(pass1_elf, "rb").read())

    pass2_elf = os.path.join(tempdir, 'pass2_elf')
    add_args_pass2 = ['-o', dest] + remangle_defs(elf1)
    run(argv_link, add_args_pass2)
  finally:
    import shutil
    shutil.rmtree(tempdir)
