#ifndef __ELF_H__
#define __ELF_H__

#include <kernel/types.h>

typedef uint32_t elf32_Addr;
typedef uint16_t elf32_Half;
typedef uint32_t elf32_Off;
typedef int32_t  elf32_Sword;
typedef uint32_t elf32_Word;

#define EI_NIDENT 16

typedef struct elf32_Ehdr {
    unsigned char e_ident[EI_NIDENT];
    elf32_Half    e_type;
    elf32_Half    e_machine;
    elf32_Word    e_version;
    elf32_Addr    e_entry;
    elf32_Off     e_phoff;
    elf32_Off     e_shoff;
    elf32_Word    e_flags;
    elf32_Half    e_ehsize;
    elf32_Half    e_phentsize;
    elf32_Half    e_phnum;
    elf32_Half    e_shentsize;
    elf32_Half    e_shnum;
    elf32_Half    e_shstrndx;
} elf32_Ehdr;

#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

#define EM_NONE  0
#define EM_M32   1
#define EM_SPARC 2
#define EM_386   3
#define EM_68K   4
#define EM_88K   5
#define EM_860   7
#define EM_MIPS  8

#define EV_NONE    0
#define EV_CURRENT 1

#define EI_MAG0    0
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4
#define EI_DATA    5
#define EI_VERSION 6
#define EI_PAD     7
#define EI_NIDENT  16

typedef struct elf32_Phdr {
    elf32_Word p_type;
    elf32_Off  p_offset;
    elf32_Addr p_vaddr;
    elf32_Addr p_paddr;
    elf32_Word p_filesz;
    elf32_Word p_memsz;
    elf32_Word p_flags;
    elf32_Word p_align;
} elf32_Phdr;

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff

#endif
