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

static bool is_qcom_dtbtool(const uint8_t* dtimg) {
	if (memcmp(dtimg, QCDT_MAGIC, 4 * sizeof(uint8_t)) == 0) {
		return true;
	}
	return false;
}

static int get_qcom_hdr_version(const uint8_t* dtimg) {
	uint32_t version;
	memcpy(&version, reinterpret_cast<const uint32_t*>(dtimg + 4), sizeof(uint32_t));
	return version;
}

static size_t get_qcom_table_entry_size(const uint8_t* dtimg) {
	size_t size = 0;
	switch (get_qcom_hdr_version(dtimg)) {
		case 1:
			size = 20;
			break;
		case 2:
			size = 24;
			break;
		case 3:
			size = 40;
			break;
		default:
			// Error bad or unsupported dtimg
			break;
	}
	return size;
}

static size_t get_qcom_hdr_size(const uint8_t* dtimg) {
	int dtb_count;
	memcpy(&dtb_count, reinterpret_cast<const uint32_t*>(dtimg + 8), sizeof(uint32_t));
	return 	4 + 4 + 4 										+ 	// qcom magic + version + dtb's count
			get_qcom_table_entry_size(dtimg) * dtb_count 	+
			4;													// end of table indicator
}

static void set_qcom_hdr_dt_size_for(uint8_t* dtimg, int dt_num, uint32_t dt_size) {
	size_t offset = 4 + 4 + 4; // qcom magic + version + dtb's count
	size_t table_entry_size = get_qcom_table_entry_size(dtimg);
	size_t dt_size_offset = table_entry_size - 4; // dt_size is the last uint32 in the table, for any version
	offset += table_entry_size * dt_num + dt_size_offset;
	memcpy(dtimg + offset, &dt_size, sizeof(uint32_t));
}

static void set_qcom_hdr_dt_offset_for(uint8_t* dtimg, int dt_num, uint32_t dt_offset) {
	size_t offset = 4 + 4 + 4; // qcom magic + version + dtb's count
	size_t table_entry_size = get_qcom_table_entry_size(dtimg);
	size_t dt_offset_offset = table_entry_size - 8; // dt_offset is the penultimate uint32 in the table, for any version
	offset += table_entry_size * dt_num + dt_offset_offset;
	memcpy(dtimg + offset, &dt_offset, sizeof(uint32_t));
}

static void dtb_patch(const char *in, const char *out) {
	bool keepverity = check_env("KEEPVERITY");
	bool redirect = check_env("TWOSTAGEINIT");

	vector<uint8_t *> fdt_list;
	size_t dtb_sz ;
	uint8_t *dtb;
	fprintf(stderr, "Loading dtbs from [%s]\n", in);
	mmap_ro(in, dtb, dtb_sz);
	const bool m_is_qcom_dtbtool = is_qcom_dtbtool(dtb);
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
	if (modified) {
		if (!out)
			out = in;
		int fd = xopen(out, O_WRONLY | O_CREAT | O_CLOEXEC);

		int page_size;
		uint8_t* filler;
		if (m_is_qcom_dtbtool) {
			// Parse page size
			parse_prop_file(HEADER_FILE, [&](string_view key, string_view value) -> bool {
				if (key == "page_size")
					page_size = parse_int(value);
				return true;
			});
			filler = (uint8_t*)calloc(page_size, sizeof(uint8_t));

			// Copy the header
			size_t header_size = get_qcom_hdr_size(dtb);
			xwrite(fd, dtb, header_size);
			xwrite(fd, filler, page_size - header_size % page_size);
		}

		for (auto fdt : fdt_list) {
			fdt_pack(fdt);
			size_t dt_size = fdt_totalsize(fdt);
			xwrite(fd, fdt, dt_size);

			if(m_is_qcom_dtbtool)
				xwrite(fd, filler, page_size - dt_size % page_size);
		}
		close(fd);

		if (m_is_qcom_dtbtool) {
			// Correct sizes in qcom header
			uint8_t* dtimg;
			size_t dtimg_sz;
			size_t dt_offset = get_qcom_hdr_size(dtb);
			dt_offset += page_size - dt_offset % page_size;
			mmap_rw(out, dtimg, dtimg_sz);
			for (int i = 0; i < fdt_list.size(); ++i) {
				auto fdt = fdt_list[i];
				size_t dt_size = fdt_totalsize(fdt);
				dt_size += page_size - dt_size % page_size;
				set_qcom_hdr_dt_size_for(dtimg, i, dt_size);
				set_qcom_hdr_dt_offset_for(dtimg, i, dt_offset);

				dt_offset += dt_size;
			}
			munmap(dtimg, dtimg_sz);
		}
	}
	munmap(dtb, dtb_sz);
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
