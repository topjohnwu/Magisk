#include <unistd.h>
#include <sys/mman.h>

extern "C" {
#include <libfdt.h>
}
#include <utils.h>
#include <bitset>
#include <vector>
#include <map>

#include "magiskboot.h"

using namespace std;

#define DTB_MAGIC       "\xd0\x0d\xfe\xed"
#define QCDT_MAGIC      "QCDT"
#define DTBH_MAGIC      "DTBH"
#define PXADT_MAGIC     "PXA-DT"
#define PXA19xx_MAGIC   "PXA-19xx"
#define SPRD_MAGIC      "SPRD"

struct qcdt_hdr {
	char magic[4];          /* "QCDT" */
	uint32_t version;       /* QCDT version */
	uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct qctable_v1 {
	uint32_t cpu_info[3];   /* Some CPU info */
	uint32_t offset;        /* DTB offset in QCDT */
	uint32_t len;           /* DTB size */
} __attribute__((packed));

struct qctable_v2 {
	uint32_t cpu_info[4];   /* Some CPU info */
	uint32_t offset;        /* DTB offset in QCDT */
	uint32_t len;           /* DTB size */
} __attribute__((packed));

struct qctable_v3 {
	uint32_t cpu_info[8];   /* Some CPU info */
	uint32_t offset;        /* DTB offset in QCDT */
	uint32_t len;           /* DTB size */
} __attribute__((packed));

struct dtbh_hdr {
	char magic[4];          /* "DTBH" */
	uint32_t version;       /* DTBH version */
	uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct bhtable_v2 {
	uint32_t cpu_info[5];   /* Some CPU info */
	uint32_t offset;        /* DTB offset in DTBH */
	uint32_t len;           /* DTB size */
	uint32_t space;         /* 0x00000020 */
} __attribute__((packed));

struct pxadt_hdr {
	char magic[6];          /* "PXA-DT" */
	uint32_t version;       /* PXA-* version */
	uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct pxa19xx_hdr {
	char magic[8];          /* "PXA-19xx" */
	uint32_t version;       /* PXA-* version */
	uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct pxatable_v1 {
	uint32_t cpu_info[2];   /* Some CPU info */
	uint32_t offset;        /* DTB offset in PXA-* */
	uint32_t len;           /* DTB size */
} __attribute__((packed));

struct sprd_hdr {
	char magic[4];          /* "SPRD" */
	uint32_t version;       /* SPRD version */
	uint32_t num_dtbs;      /* Number of DTBs */
} __attribute__((packed));

struct sprdtable_v1 {
	uint32_t cpu_info[3];   /* Some CPU info */
	uint32_t offset;        /* DTB offset in SPRD */
	uint32_t len;           /* DTB size */
} __attribute__((packed));

struct dtb_blob {
	void *fdt;
	uint32_t offset;
	uint32_t len;
};

template <class Iter>
class fdt_map_iter {
public:
	typedef decltype(std::declval<typename Iter::value_type::second_type>().fdt) value_type;
	typedef value_type* pointer;
	typedef value_type& reference;

	explicit fdt_map_iter(Iter j) : i(j) {}
	fdt_map_iter& operator++() { ++i; return *this; }
	fdt_map_iter operator++(int) { auto tmp = *this; ++(*this); return tmp; }
	fdt_map_iter& operator--() { --i; return *this; }
	fdt_map_iter operator--(int) { auto tmp = *this; --(*this); return tmp; }
	bool operator==(fdt_map_iter j) const { return i == j.i; }
	bool operator!=(fdt_map_iter j) const { return !(*this == j); }
	reference operator*() { return i->second.fdt; }
	pointer operator->() { return &i->second.fdt; }
private:
	Iter i;
};

template<class Iter>
inline fdt_map_iter<Iter> make_iter(Iter j) { return fdt_map_iter<Iter>(j); }

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
			i += fdt_totalsize(fdt) - 1;
		}
	}
	fprintf(stderr, "\n");
	munmap(dtb, size);
}

template <typename Iter>
static bool fdt_patch(Iter first, Iter last) {
	bool keepverity = check_env("KEEPVERITY");
	bool redirect = check_env("TWOSTAGEINIT");
	bool modified = false;

	int idx = 0;
	for (auto it = first; it != last; ++it) {
		++idx;
		auto fdt = *it;
		int fstab = find_fstab(fdt);
		if (fstab < 0)
			continue;
		fprintf(stderr, "Found fstab in dtb.%04d\n", idx - 1);
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
	}
	return modified;
}

template <class Table, class Header>
static int dtb_patch(const Header *hdr, const char *in, const char *out) {
	map<uint32_t, dtb_blob> dtb_map;
	auto buf = reinterpret_cast<const uint8_t *>(hdr);
	auto tables = reinterpret_cast<const Table *>(hdr + 1);

	// Collect all dtbs
	for (int i = 0; i < hdr->num_dtbs; ++i) {
		if (dtb_map.find(tables[i].offset) == dtb_map.end()) {
			auto blob = buf + tables[i].offset;
			int size = fdt_totalsize(blob);
			auto fdt = xmalloc(size + 256);
			memcpy(fdt, blob, size);
			fdt_open_into(fdt, fdt, size + 256);
			dtb_map[tables[i].offset] = { fdt, tables[i].offset };
		}
	}
	if (dtb_map.empty())
		return 1;

	// Patch fdt
	if (!fdt_patch(make_iter(dtb_map.begin()), make_iter(dtb_map.end())))
		return 1;

	if (out == in)
		unlink(in);
	int fd = xopen(out, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);

	// Copy headers and tables
	xwrite(fd, buf, dtb_map.begin()->first);

	// mmap rw to patch table values retroactively
	auto mmap_sz = lseek(fd, 0, SEEK_CUR);
	auto addr = (uint8_t *) xmmap(nullptr, mmap_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	// Guess page size using gcd
	auto it = dtb_map.begin();
	uint32_t page_size = (it++)->first;
	for (; it != dtb_map.end(); ++it)
		page_size = binary_gcd(page_size, it->first);

	// Write dtbs
	for (auto &val : dtb_map) {
		val.second.offset = lseek(fd, 0, SEEK_CUR);
		auto fdt = val.second.fdt;
		fdt_pack(fdt);
		int size = fdt_totalsize(fdt);
		xwrite(fd, fdt, size);
		val.second.len = do_align(size, page_size);
		write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR), page_size));
		free(fdt);
	}

	// Patch tables
	auto tables_rw = reinterpret_cast<Table *>(addr + sizeof(Header));
	for (int i = 0; i < hdr->num_dtbs; ++i) {
		auto &blob = dtb_map[tables_rw[i].offset];
		tables_rw[i].offset = blob.offset;
		tables_rw[i].len = blob.len;
	}

	munmap(addr, mmap_sz);
	close(fd);

	return 0;
}

#define MATCH(s) (memcmp(dtb, s, sizeof(s) - 1) == 0)

static int dtb_patch(const char *in, const char *out) {
	if (!out)
		out = in;
	size_t dtb_sz ;
	uint8_t *dtb;
	fprintf(stderr, "Loading dtbs from [%s]\n", in);
	mmap_ro(in, dtb, dtb_sz);
	run_finally f([&]{ munmap(dtb, dtb_sz); });

	if (MATCH(QCDT_MAGIC)) {
		auto hdr = reinterpret_cast<qcdt_hdr*>(dtb);
		switch (hdr->version) {
			case 1:
				fprintf(stderr, "QCDT v1\n");
				return dtb_patch<qctable_v1>(hdr, in, out);
			case 2:
				fprintf(stderr, "QCDT v2\n");
				return dtb_patch<qctable_v2>(hdr, in, out);
			case 3:
				fprintf(stderr, "QCDT v3\n");
				return dtb_patch<qctable_v3>(hdr, in, out);
			default:
				return 1;
		}
	} else if (MATCH(DTBH_MAGIC)) {
		auto hdr = reinterpret_cast<dtbh_hdr *>(dtb);
		switch (hdr->version) {
			case 2:
				fprintf(stderr, "DTBH v2\n");
				return dtb_patch<bhtable_v2>(hdr, in, out);
			default:
				return 1;
		}
	} else if (MATCH(PXADT_MAGIC)) {
		auto hdr = reinterpret_cast<pxadt_hdr *>(dtb);
		switch (hdr->version) {
			case 1:
				fprintf(stderr, "PXA-DT v1\n");
				return dtb_patch<pxatable_v1>(hdr, in, out);
			default:
				return 1;
		}
	} else if (MATCH(PXA19xx_MAGIC)) {
		auto hdr = reinterpret_cast<pxa19xx_hdr *>(dtb);
		switch (hdr->version) {
			case 1:
				fprintf(stderr, "PXA-19xx v1\n");
				return dtb_patch<pxatable_v1>(hdr, in, out);
			default:
				return 1;
		}
	} else if (MATCH(SPRD_MAGIC)) {
		auto hdr = reinterpret_cast<sprd_hdr *>(dtb);
		switch (hdr->version) {
			case 1:
				fprintf(stderr, "SPRD v1\n");
				return dtb_patch<sprdtable_v1>(hdr, in, out);
			default:
				return 1;
		}
	} else {
		vector<uint8_t *> fdt_list;
		for (int i = 0; i < dtb_sz; ++i) {
			if (memcmp(dtb + i, DTB_MAGIC, 4) == 0) {
				int len = fdt_totalsize(dtb + i);
				auto fdt = static_cast<uint8_t *>(xmalloc(len + 256));
				memcpy(fdt, dtb + i, len);
				fdt_open_into(fdt, fdt, len + 256);
				fdt_list.push_back(fdt);
				i += len - 1;
			}
		}
		if (!fdt_patch(fdt_list.begin(), fdt_list.end()))
			return 1;
		int fd = xopen(out, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		for (auto fdt : fdt_list) {
			fdt_pack(fdt);
			xwrite(fd, fdt, fdt_totalsize(fdt));
			free(fdt);
		}
		close(fd);
	}
	return 0;
}

int dtb_commands(int argc, char *argv[]) {
	char *dtb = argv[0];
	++argv;
	--argc;

	if (argv[0] == "print"sv) {
		dtb_print(dtb, argc > 1 && argv[1] == "-f"sv);
		return 0;
	} else if (argv[0] == "patch"sv) {
		if (dtb_patch(dtb, argv[1]))
			exit(1);
		return 0;
	} else {
		return 1;
	}
}
