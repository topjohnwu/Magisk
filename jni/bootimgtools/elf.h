
#ifndef _ELF_H_
#define _ELF_H_

#include <stdint.h>

/*
** ELF structure
**
** +-----------------+ 
** | ELF magic |     | 4 bytes
** +------------     +
** | ELF class |     | 1 byte  
** +------------     +
** |   ELF header    |
** +-----------------+
**          ~
** +-----------------+
** | program header  | kernel info
** +-----------------+
** | program header  | ramdisk info
** +-----------------+
** | program header  | dtb info
** +-----------------+
**          ~
** +-----------------+
** | section header  | cmdline info
** +-----------------+
**          ~
** +-----------------+
** |                 |
** |      Data       |  
** |                 |
** +-----------------+

*/

typedef uint32_t	elf32_addr;
typedef uint16_t	elf32_half;
typedef uint32_t	elf32_off;
typedef uint32_t	elf32_word;

typedef uint64_t	elf64_addr;
typedef uint16_t	elf64_half;
typedef uint64_t	elf64_off;
typedef uint32_t	elf64_word;
typedef uint64_t	elf64_xword;

#define ELF_MAGIC 		"\x7f""ELF"
#define ELF_MAGIC_SIZE	4

#define EI_CLASS		4
#define EI_DATA			5
#define EI_VERSION		6
#define EI_OSABI		7
#define EI_PAD			8

#define ELFCLASSNONE	0
#define ELFCLASS32		1
#define ELFCLASS64		2
#define ELFCLASSNUM		3

#define ET_EXEC			2
#define EM_ARM			40
#define EI_NIDENT		16

typedef struct elf32_ehdr{
	unsigned char	e_ident[EI_NIDENT];
	elf32_half		e_type;
	elf32_half		e_machine;
	elf32_word		e_version;
	elf32_addr		e_entry;	/* Entry point */
	elf32_off		e_phoff;
	elf32_off		e_shoff;
	elf32_word		e_flags;
	elf32_half		e_ehsize;
	elf32_half		e_phentsize;
	elf32_half		e_phnum;
	elf32_half		e_shentsize;
	elf32_half		e_shnum;
	elf32_half		e_shstrndx;
} elf32_ehdr;

typedef struct elf64_ehdr {
	unsigned char	e_ident[EI_NIDENT];	/* ELF "magic number" */
	elf64_half		e_type;
	elf64_half		e_machine;
	elf64_word		e_version;
	elf64_addr		e_entry;	/* Entry point virtual address */
	elf64_off		e_phoff;	/* Program header table file offset */
	elf64_off		e_shoff;	/* Section header table file offset */
	elf64_word		e_flags;
	elf64_half		e_ehsize;
	elf64_half		e_phentsize;
	elf64_half		e_phnum;
	elf64_half		e_shentsize;
	elf64_half		e_shnum;
	elf64_half		e_shstrndx;
} elf64_ehdr;

typedef struct elf32_phdr{
	elf32_word		p_type;
	elf32_off		p_offset;
	elf32_addr		p_vaddr;
	elf32_addr		p_paddr;
	elf32_word		p_filesz;
	elf32_word		p_memsz;
	elf32_word		p_flags;
	elf32_word		p_align;
} elf32_phdr;

typedef struct elf64_phdr {
	elf64_word		p_type;
	elf64_word		p_flags;
	elf64_off		p_offset;		/* Segment file offset */
	elf64_addr		p_vaddr;		/* Segment virtual address */
	elf64_addr		p_paddr;		/* Segment physical address */
	elf64_xword		p_filesz;		/* Segment size in file */
	elf64_xword		p_memsz;		/* Segment size in memory */
	elf64_xword		p_align;		/* Segment alignment, file & memory */
} elf64_phdr;

typedef struct elf32_shdr {
	elf32_word		s_name;
	elf32_word		s_type;
	elf32_word		s_flags;
	elf32_addr		s_addr;
	elf32_off		s_offset;
	elf32_word		s_size;
	elf32_word		s_link;
	elf32_word		s_info;
	elf32_word		s_addralign;
	elf32_word		s_entsize;
} elf32_shdr;

typedef struct elf64_shdr {
	elf64_word		s_name;			/* Section name, index in string tbl */
	elf64_word		s_type;			/* Type of section */
	elf64_xword		s_flags;		/* Miscellaneous section attributes */
	elf64_addr		s_addr;			/* Section virtual addr at execution */
	elf64_off		s_offset;		/* Section file offset */
	elf64_xword		s_size;			/* Size of section in bytes */
	elf64_word		s_link;			/* Index of another section */
	elf64_word		s_info;			/* Additional section information */
	elf64_xword		s_addralign;	/* Section alignment */
	elf64_xword		s_entsize;		/* Entry size if section holds table */
} elf64_shdr;

#endif
