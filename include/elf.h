#ifndef __ELF_H__
#define __ELF_H__

#include <kernel/types.h>

typedef u32 elf32_addr;
typedef u16 elf32_half;
typedef u32 elf32_off;
typedef i32  elf32_sword;
typedef u32 elf32_word;

#define EI_NIDENT 16

typedef struct elf32_ehdr {
    unsigned char e_ident[EI_NIDENT];
    elf32_half    e_type;
    elf32_half    e_machine;
    elf32_word    e_version;
    elf32_addr    e_entry;
    elf32_off     e_phoff;
    elf32_off     e_shoff;
    elf32_word    e_flags;
    elf32_half    e_ehsize;
    elf32_half    e_phentsize;
    elf32_half    e_phnum;
    elf32_half    e_shentsize;
    elf32_half    e_shnum;
    elf32_half    e_shstrndx;
} elf32_ehdr;

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

typedef struct elf32_phdr {
    elf32_word p_type;
    elf32_off  p_offset;
    elf32_addr p_vaddr;
    elf32_addr p_paddr;
    elf32_word p_filesz;
    elf32_word p_memsz;
    elf32_word p_flags;
    elf32_word p_align;
} elf32_phdr;

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
