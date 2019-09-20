#include <unistd.h>
#include <sys/mman.h>

extern "C" {
#include <libfdt.h>
}
#include <utils.h>
#include <bitset>

#include "magiskboot.h"
#include "format.h"

using namespace std;

constexpr int MAX_DEPTH = 32;
static bitset<MAX_DEPTH> depth_set;

static void pretty_node(int depth) {
	if (depth == 0)
		return;

	for (int i = 0; i < depth - 1; ++i) {
		if (depth_set[i])
			printf("│   ");
		else
			printf("    ");
	}
	if (depth_set[depth - 1])
		printf("├── ");
	else
		printf("└── ");
}

static void pretty_prop(int depth) {
	for (int i = 0; i < depth; ++i) {
		if (depth_set[i])
			printf("│   ");
		else
			printf("    ");
	}
	if (depth_set[depth])
		printf("│  ");
	else
		printf("   ");
}

static void print_node(const void *fdt, int node = 0, int depth = 0) {
	// Print node itself
	pretty_node(depth);
	printf("#%d: %s\n", node, fdt_get_name(fdt, node, nullptr));

	// Print properties
	depth_set[depth] = fdt_first_subnode(fdt, node) >= 0;
	int prop;
	fdt_for_each_property_offset(prop, fdt, node) {
		pretty_prop(depth);
		int size;
		const char *name;
		auto value = static_cast<const char *>(fdt_getprop_by_offset(fdt, prop, &name, &size));

		bool is_str = !(size > 1 && value[0] == 0);
		if (is_str) {
			// Scan through value to see if printable
			for (int i = 0; i < size; ++i) {
				char c = value[i];
				if (i == size - 1) {
					// Make sure null terminate
					is_str = c == '\0';
				} else if ((c > 0 && c < 32) || c >= 127) {
					is_str = false;
					break;
				}
			}
		}

		if (is_str) {
			printf("[%s]: [%s]\n", name, value);
		} else {
			printf("[%s]: <bytes>(%d)\n", name, size);
		}
	}

	// Recursive
	if (depth_set[depth]) {
		int child;
		int prev = -1;
		fdt_for_each_subnode(child, fdt, node) {
			if (prev >= 0)
				print_node(fdt, prev, depth + 1);
			prev = child;
		}
		depth_set[depth] = false;
		print_node(fdt, prev, depth + 1);
	}
}

static int find_fstab(const void *fdt, int parent = 0) {
	int node, fstab;
	fdt_for_each_subnode(node, fdt, parent) {
		if (strcmp(fdt_get_name(fdt, node, nullptr), "fstab") == 0)
			return node;
		fstab = find_fstab(fdt, node);
		if (fstab != -1)
			return fstab;
	}
	return -1;
}

static void dtb_print(const char *file, bool fstab) {
	size_t size ;
	uint8_t *dtb, *fdt;
	fprintf(stderr, "Loading dtbs from [%s]\n", file);
	mmap_ro(file, dtb, size);
	// Loop through all the dtbs
	int dtb_num = 0;
	for (int i = 0; i < size; ++i) {
		if (memcmp(dtb + i, DTB_MAGIC, 4) == 0) {
			fdt = dtb + i;
			if (fstab) {
				int node = find_fstab(fdt);
				if (node >= 0) {
					fprintf(stderr, "Found fstab in dtb.%04d\n", dtb_num);
					print_node(fdt, node);
				}
			} else {
				fprintf(stderr, "Printing dtb.%04d\n", dtb_num);
				print_node(fdt);
			}
			dtb_num++;
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
		mmap_rw(file, dtb, size);
	else
		mmap_ro(file, dtb, size);
	// Loop through all the dtbs
	int dtb_num = 0;
	bool found = false;
	for (int i = 0; i < size; ++i) {
		if (memcmp(dtb + i, DTB_MAGIC, 4) == 0) {
			fdt = dtb + i;
			int fstab = find_fstab(fdt, 0);
			if (fstab > 0) {
				fprintf(stderr, "Found fstab in dtb.%04d\n", dtb_num++);
				int block;
				fdt_for_each_subnode(block, fdt, fstab) {
					fprintf(stderr, "Found block [%s] in fstab\n", fdt_get_name(fdt, block, nullptr));
					uint32_t value_size;
					void *value = (void *) fdt_getprop(fdt, block, "fsmgr_flags", (int *)&value_size);
					if (patch) {
						void *dup = xmalloc(value_size);
						memcpy(dup, value, value_size);
						memset(value, 0, value_size);
						found |= patch_verity(&dup, &value_size);
						memcpy(value, dup, value_size);
						free(dup);
					} else {
						found |= patch_verity(&value, &value_size, false);
					}
				}
			}
		}
	}
	munmap(dtb, size);
	exit(!found);
}

int dtb_commands(int argc, char *argv[]) {
	char *dtb = argv[0];
	++argv;
	--argc;

	if (argv[0] == "print"sv) {
		dtb_print(dtb, argc > 1 && argv[1] == "-f"sv);
	} else if (argv[0] == "patch"sv) {
		dtb_patch(dtb, 1);
	} else if (argv[0] == "test"sv) {
		dtb_patch(dtb, 0);
	} else {
		return 1;
	}

	return 0;
}
