#include <unistd.h>
#include <sys/mman.h>

extern "C" {
#include <libfdt.h>
}
#include <utils.h>
#include <bitset>
#include <vector>

#include "magiskboot.h"
#include "format.h"

using namespace std;

constexpr int MAX_DEPTH = 32;
static bitset<MAX_DEPTH> depth_set;

static void pretty_node(int depth) {
	if (depth == 0)
		return;

	for (int i = 0; i < depth - 1; ++i)
		printf(depth_set[i] ? "│   " : "    ");

	printf(depth_set[depth - 1] ? "├── " : "└── ");
}

static void pretty_prop(int depth) {
	for (int i = 0; i < depth; ++i)
		printf(depth_set[i] ? "│   " : "    ");

	printf(depth_set[depth] ? "│  " : "   ");
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

static int find_fstab(const void *fdt, int node = 0) {
	if (fdt_get_name(fdt, node, nullptr) == "fstab"sv)
		return node;
	int child;
	fdt_for_each_subnode(child, fdt, node) {
		int fstab = find_fstab(fdt, child);
		if (fstab >= 0)
			return fstab;
	}
	return -1;
}

static void dtb_print(const char *file, bool fstab) {
	size_t size;
	uint8_t *dtb;
	fprintf(stderr, "Loading dtbs from [%s]\n", file);
	mmap_ro(file, dtb, size);
	// Loop through all the dtbs
	int dtb_num = 0;
	for (int i = 0; i < size; ++i) {
		if (memcmp(dtb + i, DTB_MAGIC, 4) == 0) {
			auto fdt = dtb + i;
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
			++dtb_num;
		}
	}
	fprintf(stderr, "\n");
	munmap(dtb, size);
	exit(0);
}

static void dtb_patch(const char *in, const char *out) {
	bool keepverity = check_env("KEEPVERITY");
	bool redirect = check_env("TWOSTAGEINIT");

	vector<uint8_t *> fdt_list;
	size_t dtb_sz ;
	uint8_t *dtb;
	fprintf(stderr, "Loading dtbs from [%s]\n", in);
	mmap_ro(in, dtb, dtb_sz);
	bool modified = false;
	for (int i = 0; i < dtb_sz; ++i) {
		if (memcmp(dtb + i, DTB_MAGIC, 4) == 0) {
			int len = fdt_totalsize(dtb + i);
			auto fdt = static_cast<uint8_t *>(xmalloc(redirect ? len + 256 : len));
			memcpy(fdt, dtb + i, len);
			if (redirect)
				fdt_open_into(fdt, fdt, len + 256);

			int fstab = find_fstab(fdt);
			if (fstab < 0)
				continue;
			fprintf(stderr, "Found fstab in dtb.%04d\n", fdt_list.size());
			int block;
			fdt_for_each_subnode(block, fdt, fstab) {
				const char *name = fdt_get_name(fdt, block, nullptr);
				fprintf(stderr, "Found entry [%s] in fstab\n", name);
				if (!keepverity) {
					uint32_t size;
					auto value = static_cast<const char *>(
							fdt_getprop(fdt, block, "fsmgr_flags", reinterpret_cast<int *>(&size)));
					char *pval = patch_verity(value, size);
					if (pval) {
						modified = true;
						fdt_setprop_string(fdt, block, "fsmgr_flags", pval);
					}
				}
				if (redirect && name == "system"sv) {
					modified = true;
					fprintf(stderr, "Changing mnt_point to /system_root\n");
					fdt_setprop_string(fdt, block, "mnt_point", "/system_root");
				}
			}
			fdt_list.push_back(fdt);
		}
	}
	munmap(dtb, dtb_sz);
	if (modified) {
		if (!out)
			out = in;
		int fd = xopen(out, O_WRONLY | O_CREAT | O_CLOEXEC);
		for (auto fdt : fdt_list) {
			fdt_pack(fdt);
			xwrite(fd, fdt, fdt_totalsize(fdt));
		}
		close(fd);
	}
	exit(!modified);
}

int dtb_commands(int argc, char *argv[]) {
	char *dtb = argv[0];
	++argv;
	--argc;

	if (argv[0] == "print"sv) {
		dtb_print(dtb, argc > 1 && argv[1] == "-f"sv);
	} else if (argv[0] == "patch"sv) {
		dtb_patch(dtb, argv[1]);
	} else {
		return 1;
	}

	return 0;
}
