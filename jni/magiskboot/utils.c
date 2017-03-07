#include "magiskboot.h"
#include "elf.h"

char *SUP_EXT_LIST[SUP_NUM] = { "gz", "xz", "lzma", "bz2", "lz4" };
file_t SUP_TYPE_LIST[SUP_NUM] = { GZIP, XZ, LZMA, BZIP2, LZ4 };

void mmap_ro(const char *filename, unsigned char **buf, size_t *size) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		error(1, "Cannot open %s", filename);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = mmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
}

void mmap_rw(const char *filename, unsigned char **buf, size_t *size) {
	int fd = open(filename, O_RDWR);
	if (fd < 0)
		error(1, "Cannot open %s", filename);
	*size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	*buf = mmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
}

file_t check_type(const unsigned char *buf) {
	if (memcmp(buf, CHROMEOS_MAGIC, CHROMEOS_MAGIC_SIZE) == 0) {
		return CHROMEOS;
	} else if (memcmp(buf, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0) {
		return AOSP;
	} else if (memcmp(buf, ELF_MAGIC, ELF_MAGIC_SIZE) == 0) {
		return ELF;
	} else if (memcmp(buf, "\x1f\x8b\x08\x00", 4) == 0) {
		return GZIP;
	} else if (memcmp(buf, "\x89\x4c\x5a\x4f\x00\x0d\x0a\x1a\x0a", 9) == 0) {
		return LZOP;
	} else if (memcmp(buf, "\xfd""7zXZ\x00", 6) == 0) {
		return XZ;
	} else if (memcmp(buf, "\x5d\x00\x00", 3) == 0 
			&& (buf[12] == (unsigned char) '\xff' || buf[12] == (unsigned char) '\x00')) {
		return LZMA;
	} else if (memcmp(buf, "BZh", 3) == 0) {
		return BZIP2;
	} else if ( (  memcmp(buf, "\x04\x22\x4d\x18", 4) == 0 
				|| memcmp(buf, "\x03\x21\x4c\x18", 4) == 0) 
				|| memcmp(buf, "\x02\x21\x4c\x18", 4) == 0) {
		return LZ4;
	} else if (memcmp(buf, "\x88\x16\x88\x58", 4) == 0) {
		return MTK;
	} else if (memcmp(buf, "QCDT", 4) == 0) {
		return QCDT;
	} else {
		return UNKNOWN;
	}
}

void mem_align(size_t *pos, size_t align) {
	size_t mask = align - 1;
	if (*pos & mask) {
		*pos += align - (*pos & mask);
	}
}

void file_align(int fd, size_t align, int out) {
	size_t pos = lseek(fd, 0, SEEK_CUR);
	size_t mask = align - 1;
	if (pos & mask) {
		pos += align - (pos & mask);
		if (out) {
			ftruncate(fd, pos);
		}
		lseek(fd, pos, SEEK_SET);
	}
}

int open_new(const char *filename) {
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		error(1, "Unable to create %s", filename);
	return fd;
}

void print_info() {
	printf("KERNEL [%d] @ 0x%08x\n", hdr.kernel_size, hdr.kernel_addr);
	printf("RAMDISK [%d] @ 0x%08x\n", hdr.ramdisk_size, hdr.ramdisk_addr);
	printf("SECOND [%d] @ 0x%08x\n", hdr.second_size, hdr.second_addr);
	printf("DTB [%d] @ 0x%08x\n", hdr.dt_size, hdr.tags_addr);
	printf("PAGESIZE [%d]\n", hdr.page_size);
	if (hdr.os_version != 0) {
		int a,b,c,y,m = 0;
		int os_version, os_patch_level;
		os_version = hdr.os_version >> 11;
		os_patch_level = hdr.os_version & 0x7ff;
		
		a = (os_version >> 14) & 0x7f;
		b = (os_version >> 7) & 0x7f;
		c = os_version & 0x7f;
		printf("OS_VERSION [%d.%d.%d]\n", a, b, c);
		
		y = (os_patch_level >> 4) + 2000;
		m = os_patch_level & 0xf;
		printf("PATCH_LEVEL [%d-%02d]\n", y, m);
	}
	printf("NAME [%s]\n", hdr.name);
	printf("CMDLINE [%s]\n", hdr.cmdline);

	switch (ramdisk_type) {
		case GZIP:
			printf("COMPRESSION [%s]\n", "gzip");
			break;
		case XZ:
			printf("COMPRESSION [%s]\n", "xz");
			break;
		case LZMA:
			printf("COMPRESSION [%s]\n", "lzma");
			break;
		case BZIP2:
			printf("COMPRESSION [%s]\n", "bzip2");
			break;
		case LZ4:
			printf("COMPRESSION [%s]\n", "lz4");
			break;
		default:
			fprintf(stderr, "Unknown ramdisk format!\n");
	}
}

void cleanup() {
	printf("Cleaning up...\n");
	char name[PATH_MAX];
	unlink(KERNEL_FILE);
	unlink(RAMDISK_FILE);
	unlink(RAMDISK_FILE ".unsupport");
	unlink(SECOND_FILE);
	unlink(DTB_FILE);
	unlink(NEW_BOOT);
	for (int i = 0; i < SUP_NUM; ++i) {
		sprintf(name, "%s.%s", RAMDISK_FILE, SUP_EXT_LIST[i]);
		unlink(name);
	}
}

void vec_init(vector *v) {
	vec_size(v) = 0;
	vec_cap(v) = 1;
	vec_entry(v) = malloc(sizeof(void*));
}

void vec_push_back(vector *v, void *p) {
	if (v == NULL) return;
	if (vec_size(v) == vec_cap(v)) {
		vec_cap(v) *= 2;
		vec_entry(v) = realloc(vec_entry(v), sizeof(void*) * vec_cap(v));
	}
	vec_entry(v)[vec_size(v)] = p;
	++vec_size(v);
}

void vec_destroy(vector *v) {
	// Will not free each entry!
	// Manually free each entry, then call this function
	vec_size(v) = 0;
	vec_cap(v) = 0;
	free(v->data);
}
