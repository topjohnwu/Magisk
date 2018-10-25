#include <unistd.h>
#include <sys/mman.h>
extern "C" {
#include <libfdt.h>
}

#include "magiskboot.h"
#include "utils.h"

static void print_props(const void *fdt, int node, int depth) {
	int prop;
	fdt_for_each_property_offset(prop, fdt, node) {
		for (int i = 0; i < depth; ++i) printf("    ");
		printf("  ");
		int size;
		const char *name;
		const char *value = (char *) fdt_getprop_by_offset(fdt, prop, &name, &size);
		printf("[%s]: [%s]\n", name, value);
	}
}

static void print_subnode(const void *fdt, int parent, int depth) {
	int node;
	fdt_for_each_subnode(node, fdt, parent) {
		for (int i = 0; i < depth; ++i) printf("    ");
		printf("#%d: %s\n", node, fdt_get_name(fdt, node, NULL));
		print_props(fdt, node, depth);
		print_subnode(fdt, node, depth + 1);
	}
}

static int find_fstab(const void *fdt, int parent) {
	int node, fstab;
	fdt_for_each_subnode(node, fdt, parent) {
		if (strcmp(fdt_get_name(fdt, node, NULL), "fstab") == 0)
			return node;
		fstab = find_fstab(fdt, node);
		if (fstab != -1)
			return fstab;
	}
	return -1;
}

static void dtb_dump(const char *file) {
	size_t size ;
	uint8_t *dtb, *fdt;
	fprintf(stderr, "Loading dtbs from [%s]\n", file);
	mmap_ro(file, (void **) &dtb, &size);
	// Loop through all the dtbs
	int dtb_num = 0;
	for (int i = 0; i < size; ++i) {
		if (memcmp(dtb + i, DTB_MAGIC, 4) == 0) {
			fdt = dtb + i;
			fprintf(stderr, "Dumping dtb.%04d\n", dtb_num++);
			print_subnode(fdt, 0, 0);
		}
	}
	fprintf(stderr, "\n");
	munmap(dtb, size);
	exit(0);
}

static void dtb_patch(const char *file, int patch) {
	size_t size ;
	uint8_t *dtb, *fdt;
	fprintf(stderr, "Loading dtbs from [%s]\n", file);
	if (patch)
		mmap_rw(file, (void **) &dtb, &size);
	else
		mmap_ro(file, (void **) &dtb, &size);
	// Loop through all the dtbs
	int dtb_num = 0, found = 0;
	for (int i = 0; i < size; ++i) {
		if (memcmp(dtb + i, DTB_MAGIC, 4) == 0) {
			fdt = dtb + i;
			int fstab = find_fstab(fdt, 0);
			if (fstab > 0) {
				fprintf(stderr, "Found fstab in dtb.%04d\n", dtb_num++);
				int block;
				fdt_for_each_subnode(block, fdt, fstab) {
					fprintf(stderr, "Found block [%s] in fstab\n", fdt_get_name(fdt, block, NULL));
					uint32_t value_size;
					void *value = (void *) fdt_getprop(fdt, block, "fsmgr_flags", (int *)&value_size);
					if (patch) {
						void *dup = xmalloc(value_size);
						memcpy(dup, value, value_size);
						memset(value, 0, value_size);
						found |= patch_verity(&dup, &value_size, 1);
						memcpy(value, dup, value_size);
						free(dup);
					} else {
						found |= patch_verity(&value, &value_size, 0);
					}
				}
			}
		}
	}
	munmap(dtb, size);
	exit(!found);
}

int dtb_commands(const char *cmd, int argc, char *argv[]) {
	if (argc == 0) return 1;
	if (strcmp(cmd, "dump") == 0)
		dtb_dump(argv[0]);
	else if (strcmp(cmd, "patch") == 0)
		dtb_patch(argv[0], 1);
	else if (strcmp(cmd, "test") == 0)
		dtb_patch(argv[0], 0);
	return 0;
}


