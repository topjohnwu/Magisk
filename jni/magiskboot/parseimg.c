#include "bootimg.h"
#include "elf.h"
#include "magiskboot.h"

unsigned char *kernel, *ramdisk, *second, *dtb, *extra;
boot_img_hdr hdr;
int mtk_kernel = 0, mtk_ramdisk = 0;
file_t boot_type, ramdisk_type, dtb_type;

static void check_headers() {
	// Check ramdisk compression type
	ramdisk_type = check_type(ramdisk);

	// Check MTK
	if (check_type(kernel) == MTK) {
		printf("MTK header found in kernel\n");
		mtk_kernel = 1;
	}
	if (ramdisk_type == MTK) {
		printf("MTK header found in ramdisk\n");
		mtk_ramdisk = 1;
		ramdisk_type = check_type(ramdisk + 512);
	}

	// Check dtb if ELF boot
	if (boot_type == ELF && hdr.dt_size) {
		dtb_type = check_type(dtb);
	}

	// Print info
	print_info();
}

static void elf_header_check(void *elf, int is64) {

	size_t e_size, mach, ver, p_size, p_num, s_size, s_num;
	size_t r_e_size, r_p_size, r_s_size;

	if (is64) {
		e_size = ((elf64_ehdr *) elf)->e_ehsize;
		mach = ((elf64_ehdr *) elf)->e_machine;
		ver = ((elf64_ehdr *) elf)->e_version;
		p_size = ((elf64_ehdr *) elf)->e_phentsize;
		p_num = ((elf64_ehdr *) elf)->e_phnum;
		s_size = ((elf64_ehdr *) elf)->e_shentsize;
		s_num = ((elf64_ehdr *) elf)->e_shnum;
		r_e_size = sizeof(elf64_ehdr);
		r_p_size = sizeof(elf64_phdr);
		r_s_size = sizeof(elf64_shdr);
	} else {
		e_size = ((elf32_ehdr *) elf)->e_ehsize;
		mach = ((elf32_ehdr *) elf)->e_machine;
		ver = ((elf32_ehdr *) elf)->e_version;
		p_size = ((elf32_ehdr *) elf)->e_phentsize;
		p_num = ((elf32_ehdr *) elf)->e_phnum;
		s_size = ((elf32_ehdr *) elf)->e_shentsize;
		s_num = ((elf32_ehdr *) elf)->e_shnum;
		r_e_size = sizeof(elf32_ehdr);
		r_p_size = sizeof(elf32_phdr);
		r_s_size = sizeof(elf32_shdr);
	}

	if (e_size != r_e_size)
		error(1, "Header size not %d", r_e_size);

	if (mach != EM_ARM)
		error(1, "ELF machine is not ARM");

	if (ver != 1)
		error(1, "Unknown ELF version");

	if (p_size != r_p_size)
		error(1, "Program header size not %d", r_p_size);

	if (p_num < 2 || p_num > 4)
		error(1, "Unexpected number of elements: %d", p_num);

	if (s_num && s_size != r_s_size)
		error(1, "Section header size not %d", r_s_size);

	if (s_num > 1)
		error(1, "More than one section header");
}

static void elf_set(int i, unsigned char *base, size_t size, size_t offset, size_t addr) {
	if (size <= 4096) {
		// Possible cmdline
		memset(hdr.cmdline, 0, BOOT_ARGS_SIZE);
		strncpy((char *) hdr.cmdline, (char *) (base + offset), BOOT_ARGS_SIZE);
		hdr.cmdline[strcspn((char*) hdr.cmdline, "\n")] = '\0';
		return;
	}
	switch(i) {
		case 0:
			// kernel
			kernel = base + offset;
			hdr.kernel_size = size;
			hdr.kernel_addr = addr;
			break;
		case 1:
			// ramdisk
			ramdisk = base + offset;
			hdr.ramdisk_size = size;
			hdr.ramdisk_addr = addr;
			break;
		case 2:
			// dtb
			dtb = base + offset;
			hdr.dt_size = size;
			hdr.tags_addr = addr;
			break;
	}
}

static void parse_elf(unsigned char *base) {

	// Reset boot image header
	memset(&hdr, 0, sizeof(hdr));

	// Hardcode header magic and pagesize
	memcpy(hdr.magic, BOOT_MAGIC, BOOT_MAGIC_SIZE);
	hdr.page_size = 4096;

	switch(base[EI_CLASS]) {

		case ELFCLASS32: {

			elf32_ehdr *elf32;
			elf32_phdr *ph32;
			elf32_shdr *sh32;

			printf("IMAGE [ELF32]\n");

			elf32 = (elf32_ehdr *) base;

			elf_header_check(elf32, 0);

			ph32 = (elf32_phdr *) (base + elf32->e_phoff);
			sh32 = (elf32_shdr *) (base + elf32->e_shoff);

			for (int i = 0; i < elf32->e_phnum; ++i) {
				elf_set(i, base, ph32[i].p_filesz, ph32[i].p_offset, ph32[i].p_paddr);
			}

			if (elf32->e_shnum) {
				// cmdline
				memset(hdr.cmdline, 0, BOOT_ARGS_SIZE);
				strncpy((char *) hdr.cmdline, (char *) (base + sh32->s_offset + 8), BOOT_ARGS_SIZE);
				hdr.cmdline[strcspn((char*) hdr.cmdline, "\n")] = '\0';
			}

			break;
		}

		case ELFCLASS64: {

			elf64_ehdr *elf64;
			elf64_phdr *ph64;
			elf64_shdr *sh64;

			printf("IMAGE [ELF64]\n");

			elf64 = (elf64_ehdr *) base;

			elf_header_check(elf64, 1);

			ph64 = (elf64_phdr *) (base + elf64->e_phoff);
			sh64 = (elf64_shdr *) (base + elf64->e_shoff);

			for (int i = 0; i < elf64->e_phnum; ++i) {
				elf_set(i, base, ph64[i].p_filesz, ph64[i].p_offset, ph64[i].p_paddr);
			}

			if (elf64->e_shnum) {
				// cmdline
				memset(hdr.cmdline, 0, BOOT_ARGS_SIZE);
				strncpy((char *) hdr.cmdline, (char *) (base + sh64->s_offset + 8), BOOT_ARGS_SIZE);
				hdr.cmdline[strcspn((char*) hdr.cmdline, "\n")] = '\0';
			}
			break;
		}

		default:
			error(1, "ELF format error!");
	}

	check_headers();
}

static void parse_aosp(unsigned char *base, size_t size) {

	printf("IMG [AOSP]\n");

	size_t pos = 0;

	// Read the header
	memcpy(&hdr, base, sizeof(hdr));
	pos += hdr.page_size;

	// Kernel position
	kernel = base + pos;
	pos += hdr.kernel_size;
	mem_align(&pos, hdr.page_size);

	// Ramdisk position
	ramdisk = base + pos;
	pos += hdr.ramdisk_size;
	mem_align(&pos, hdr.page_size);

	if (hdr.second_size) {
		// Second position
		second = base + pos;
		pos += hdr.second_size;
		mem_align(&pos, hdr.page_size);
	}

	if (hdr.dt_size) {
		// dtb position
		dtb = base + pos;
		pos += hdr.dt_size;
		mem_align(&pos, hdr.page_size);
	}

	if (pos < size) {
		extra = base + pos;
	}

	check_headers();
}

void parse_img(unsigned char *orig, size_t size) {
	unsigned char *base, *end;
	for(base = orig, end = orig + size; base < end; base += 256, size -= 256) {
		switch (check_type(base)) {
			case CHROMEOS:
				boot_type = CHROMEOS;
				continue;
			case AOSP:
				// Don't override CHROMEOS
				if (boot_type != CHROMEOS)
					boot_type = AOSP;
				parse_aosp(base, size);
				return;
			case ELF:
				boot_type = ELF;
				parse_elf(base);
				return;
			default:
				continue;
		}
	}
	error(1, "No boot image magic found!");
}
