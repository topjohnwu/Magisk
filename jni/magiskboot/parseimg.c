#include "bootimg.h"
#include "magiskboot.h"

unsigned char *kernel, *ramdisk, *second, *dtb, *extra;
boot_img_hdr hdr;
int mtk_kernel = 0, mtk_ramdisk = 0;
file_t ramdisk_type;

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

	// Print info
	print_info();
}

static void parse_aosp(unsigned char *base, size_t size) {

	// printf("IMG [AOSP]\n");

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
			// The caller should know it's chromeos, as it needs additional signing
			close(open_new("chromeos"));
			continue;
		case ELF32:
			exit(2);
			return;
		case ELF64:
			exit(3);
			return;
		case AOSP:
			parse_aosp(base, size);
			return;
		default:
			continue;
		}
	}
	error(1, "No boot image magic found!");
}
