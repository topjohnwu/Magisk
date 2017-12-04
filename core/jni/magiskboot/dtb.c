#include <libfdt.h>
#include <unistd.h>
#include <sys/mman.h>

#include "magiskboot.h"
#include "utils.h"

extern int check_verity_pattern(const char *s);

static void print_props(const void *fdt, int node, int depth) {
	int prop;
	fdt_for_each_property_offset(prop, fdt, node) {
		for (int i = 0; i < depth; ++i) printf("    ");
		printf("  ");
		int size;
		const char *name;
		const char *value = fdt_getprop_by_offset(fdt, prop, &name, &size);
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
	void *dtb, *fdt;
	fprintf(stderr, "Loading dtbs from [%s]\n", file);
	mmap_ro(file, &dtb, &size);
	// Loop through all the dtbs
	int dtb_num = 0;
	for (int i = 0; i < size; ++i) {
		if (memcmp(dtb + i, DTB_MAGIC, 4) == 0) {
			fdt = dtb + i;
			fprintf(stderr, "Dumping dtb.%04d\n\n", dtb_num++);
			print_subnode(fdt, 0, 0);
		}
	}
	fprintf(stderr, "\n");
	munmap(dtb, size);
	exit(0);
}

static void dtb_patch(const char *file, int patch) {
	size_t size ;
	void *dtb, *fdt;
	fprintf(stderr, "Loading dtbs from [%s]\n\n", file);
	if (patch)
		mmap_rw(file, &dtb, &size);
	else
		mmap_ro(file, &dtb, &size);
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
					int skip, value_size;
					char *value = (char *) fdt_getprop(fdt, block, "fsmgr_flags", &value_size);
					for (int i = 0; i < value_size; ++i) {
						if ((skip = check_verity_pattern(value + i)) > 0) {
							if (patch) {
								fprintf(stderr, "Remove pattern [%.*s] in [fsmgr_flags]\n", skip, value + i);
								memcpy(value + i, value + i + skip, value_size - i - skip);
								memset(value + value_size - skip, '\0', skip);
							} else {
								fprintf(stderr, "Found pattern [%.*s] in [fsmgr_flags]\n", skip, value + i);
								i += skip - 1;
							}
							found = 1;
						}
					}
				}
			}
		}
	}
	fprintf(stderr, "\n");
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


