#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

#include "bootimgtools.h"

void dump(uint8_t *ptr, size_t size, char* filename) {
	unlink(filename);
	int ofd = open(filename, O_WRONLY|O_CREAT, 0644);
	assert(ofd >= 0);
	int ret = write(ofd, ptr, size);
	assert(ret == size);
	close(ofd);
}

//TODO: Search for other header types
void dump_ramdisk(uint8_t *ptr, size_t size) {
	//GZip header
	if(memcmp(ptr, "\x1f\x8b\x08\x00", 4) == 0) {
		dump(ptr, size, "ramdisk.gz");
	//MTK header
	} else if(memcmp(ptr, "\x88\x16\x88\x58", 4) == 0) {
		dump(ptr, 0, "ramdisk-mtk"); //Create an mtk flag
		dump_ramdisk(ptr+512, size-512);
	} else {
		//Since our first aim is to extract/repack ramdisk
		//Stop if we can't find it
		//Still dump it for debug purposes
		dump(ptr, size, "ramdisk");

		fprintf(stderr, "Unknown ramdisk type\n");
		abort();
	}
}

void search_security_hdr(uint8_t *buf, size_t size) {
	if(memcmp(buf, "CHROMEOS", 8) == 0) {
		dump(buf, 0, "chromeos");
		return;
	}
}

int search_security(uint8_t *buf, size_t size, int pos) {
	//Rockchip signature
	if(memcmp(buf+1024, "SIGN", 4) == 0) {
		//Rockchip signature AT LEAST means the bootloader will check the crc
		dump(buf, 0, "rkcrc"); //Create an flag to tell it

		//And it's possible there is a security too
		return 1;
	}

	//If we didn't parse the whole file, it is highly likely there is a boot signature
	if(pos < size) {
		return 1;
	}

	return 0;
}

/*
 * TODO:
 *  - At the moment we dump kernel + ramdisk + second + DT, it's likely we only want ramdisk
 *  - Error-handling via assert() is perhaps not the best
 */
int extract(char *image) {

	int fd = open(image, O_RDONLY);
	off_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	uint8_t *orig = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	uint8_t *base = orig;
	assert(base);

	search_security_hdr(base, size);

	//We're searching for the header in the whole file, we could stop earlier.
	//At least HTC and nVidia have a signature header
	while(base<(orig+size)) {
		if(memcmp(base, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0)
			break;
		//We're searching every 256bytes, is it ok?
		base += 256;
	}
	assert(base < (orig+size));

	struct boot_img_hdr *hdr = (struct boot_img_hdr*) base;
	assert(
			hdr->page_size == 2048 ||
			hdr->page_size == 4096 ||
			hdr->page_size == 16384
			);

	long pos = hdr->page_size;
	dump(base+pos, hdr->kernel_size, "kernel");
	pos += hdr->kernel_size + hdr->page_size-1;
	pos &= ~(hdr->page_size-1L);

	dump_ramdisk(base+pos, hdr->ramdisk_size);
	pos += hdr->ramdisk_size + hdr->page_size-1;
	pos &= ~(hdr->page_size-1L);

	if(hdr->second_size) {
		assert( (pos+hdr->second_size) <= size);
		dump(base+pos, hdr->second_size, "second");
		pos += hdr->second_size + hdr->page_size-1;
		pos &= ~(hdr->page_size-1L);
	}

	//This is non-standard, so we triple check
	if( hdr->unused[0] &&
			pos < size &&
			(pos+hdr->unused[0]) <= size) {

		if(memcmp(base+pos, "QCDT", 4) == 0 ||
				memcmp(base+pos, "SPRD", 4) == 0 ||
				memcmp(base+pos, "DTBH", 4) == 0
				) {
			dump(base+pos, hdr->unused[0], "dt");
			pos += hdr->unused[0] + hdr->page_size-1;
			pos &= ~(hdr->page_size-1L);
		}
	}

	//If we think we find some security-related infos in the boot.img
	//create a "secure" flag to warn the user it is dangerous
	if(search_security(base, size, pos)) {
		dump(base, 0, "secure");
	}

	munmap(orig, size);
	close(fd);
	return 0;
}
