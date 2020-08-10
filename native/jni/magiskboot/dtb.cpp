#include <unistd.h>
#include <sys/mman.h>
#include <bitset>
#include <vector>
#include <map>

#include <utils.hpp>

#include "magiskboot.hpp"
#include "dtb.hpp"
extern "C" {
#include <libfdt.h>
}

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
		if (memcmp(dtb + i, FDT_MAGIC_STR, 4) == 0) {
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

static bool dtb_patch(const char *file) {
	bool keepverity = check_env("KEEPVERITY");
	bool patched = false;
	size_t size;
	uint8_t *dtb;
	fprintf(stderr, "Loading dtbs from [%s]\n", file);
	mmap_rw(file, dtb, size);
	// Loop through all the dtbs
	int dtb_num = 0;
	for (int i = 0; i < size; ++i) {
		if (memcmp(dtb + i, FDT_MAGIC_STR, 4) == 0) {
			auto fdt = dtb + i;
			fprintf(stderr, "Loading dtb.%04d\n", dtb_num);
			if (int fstab = find_fstab(fdt); fstab >= 0) {
				int node;
				fdt_for_each_subnode(node, fdt, fstab) {
					const char *name = fdt_get_name(fdt, node, nullptr);
					fprintf(stderr, "Found fstab entry [%s]\n", name);
					if (!keepverity) {
						int len;
						auto value = fdt_getprop(fdt, node, "fsmgr_flags", &len);
						patched |= patch_verity(const_cast<void *>(value), len) != len;
					}
				}
			}
			++dtb_num;
			i += fdt_totalsize(fdt) - 1;
		}
	}
	fprintf(stderr, "\n");
	munmap(dtb, size);
	return patched;
}

int dtb_commands(int argc, char *argv[]) {
	char *dtb = argv[0];
	++argv;
	--argc;

	if (argv[0] == "print"sv) {
		dtb_print(dtb, argc > 1 && argv[1] == "-f"sv);
		return 0;
	} else if (argv[0] == "patch"sv) {
		if (!dtb_patch(dtb))
			exit(1);
		return 0;
	} else {
		return 1;
	}
}

namespace {
	// Unused, but keep these precious code as they took TONs of effort to write

	struct fdt_blob {
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
					int size;
					auto value = fdt_getprop(fdt, block, "fsmgr_flags", &size);
					char *copy = static_cast<char *>(memcpy(malloc(size), value, size));
					if (patch_verity(copy, size) != size) {
						modified = true;
						fdt_setprop_string(fdt, block, "fsmgr_flags", copy);
					}
					free(copy);
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

#define MAX_FDT_GROWTH 256

	template <class Table, class Header>
	static int dt_table_patch(const Header *hdr, const char *out) {
		map<uint32_t, fdt_blob> dtb_map;
		auto buf = reinterpret_cast<const uint8_t *>(hdr);
		auto tables = reinterpret_cast<const Table *>(hdr + 1);

		constexpr bool is_dt_table = std::is_same_v<Header, dt_table_header>;

		using endian_conv = uint32_t (*)(uint32_t);
		endian_conv be_to_le;
		endian_conv le_to_be;
		if constexpr (is_dt_table) {
			be_to_le = fdt32_to_cpu;
			le_to_be = cpu_to_fdt32;
		} else {
			be_to_le = le_to_be = [](uint32_t x) -> auto { return x; };
		}

		// Collect all dtbs
		auto num_dtb = be_to_le(hdr->num_dtbs);
		for (int i = 0; i < num_dtb; ++i) {
			auto offset = be_to_le(tables[i].offset);
			if (dtb_map.count(offset) == 0) {
				auto blob = buf + offset;
				uint32_t size = fdt_totalsize(blob);
				auto fdt = xmalloc(size + MAX_FDT_GROWTH);
				memcpy(fdt, blob, size);
				fdt_open_into(fdt, fdt, size + MAX_FDT_GROWTH);
				dtb_map[offset] = { fdt, offset };
			}
		}
		if (dtb_map.empty())
			return 1;

		// Patch fdt
		if (!fdt_patch(make_iter(dtb_map.begin()), make_iter(dtb_map.end())))
			return 1;

		unlink(out);
		int fd = xopen(out, O_RDWR | O_CREAT | O_CLOEXEC, 0644);

		uint32_t total_size = 0;

		// Copy headers and tables
		total_size += xwrite(fd, buf, dtb_map.begin()->first);

		// mmap rw to patch table values retroactively
		auto mmap_sz = lseek(fd, 0, SEEK_CUR);
		auto addr = (uint8_t *) xmmap(nullptr, mmap_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		// Guess alignment using gcd
		uint32_t align = 1;
		if constexpr (!is_dt_table) {
			auto it = dtb_map.begin();
			align = (it++)->first;
			for (; it != dtb_map.end(); ++it)
				align = binary_gcd(align, it->first);
		}

		// Write dtbs
		for (auto &val : dtb_map) {
			val.second.offset = lseek(fd, 0, SEEK_CUR);
			auto fdt = val.second.fdt;
			fdt_pack(fdt);
			auto size = fdt_totalsize(fdt);
			total_size += xwrite(fd, fdt, size);
			val.second.len = do_align(size, align);
			write_zero(fd, align_off(lseek(fd, 0, SEEK_CUR), align));
			// total_size += align_off(lseek(fd, 0, SEEK_CUR), align);  /* Not needed */
			free(fdt);
		}

		// Patch headers
		if constexpr (is_dt_table) {
			auto hdr_rw = reinterpret_cast<Header *>(addr);
			hdr_rw->total_size = le_to_be(total_size);
		}
		auto tables_rw = reinterpret_cast<Table *>(addr + sizeof(Header));
		for (int i = 0; i < num_dtb; ++i) {
			auto &blob = dtb_map[be_to_le(tables_rw[i].offset)];
			tables_rw[i].offset = le_to_be(blob.offset);
			tables_rw[i].len = le_to_be(blob.len);
		}

		munmap(addr, mmap_sz);
		close(fd);

		return 0;
	}

	static int blob_patch(uint8_t *dtb, size_t dtb_sz, const char *out) {
		vector<uint8_t *> fdt_list;
		vector<uint32_t> padding_list;
		for (int i = 0; i < dtb_sz; ++i) {
			if (memcmp(dtb + i, FDT_MAGIC_STR, 4) == 0) {
				auto len = fdt_totalsize(dtb + i);
				auto fdt = static_cast<uint8_t *>(xmalloc(len + MAX_FDT_GROWTH));
				memcpy(fdt, dtb + i, len);
				fdt_pack(fdt);
				uint32_t padding = len - fdt_totalsize(fdt);
				padding_list.push_back(padding);
				fdt_open_into(fdt, fdt, len + MAX_FDT_GROWTH);
				fdt_list.push_back(fdt);
				i += len - 1;
			}
		}

		if (!fdt_patch(fdt_list.begin(), fdt_list.end()))
			return 1;

		unlink(out);
		int fd = xopen(out, O_WRONLY | O_CREAT | O_CLOEXEC, 0644);

		for (int i = 0; i < fdt_list.size(); ++i) {
			auto fdt = fdt_list[i];
			fdt_pack(fdt);
			// Only add padding back if it is anything meaningful
			if (padding_list[i] > 4) {
				auto len = fdt_totalsize(fdt);
				fdt_set_totalsize(fdt, len + padding_list[i]);
			}
			xwrite(fd, fdt, fdt_totalsize(fdt));
			free(fdt);
		}
		close(fd);

		return 0;
	}

#define MATCH(s) (memcmp(dtb, s, sizeof(s) - 1) == 0)

	[[maybe_unused]] static int dtb_patch(const char *in, const char *out) {
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
					return dt_table_patch<qctable_v1>(hdr, out);
				case 2:
					fprintf(stderr, "QCDT v2\n");
					return dt_table_patch<qctable_v2>(hdr, out);
				case 3:
					fprintf(stderr, "QCDT v3\n");
					return dt_table_patch<qctable_v3>(hdr, out);
				default:
					return 1;
			}
		} else if (MATCH(DTBH_MAGIC)) {
			auto hdr = reinterpret_cast<dtbh_hdr *>(dtb);
			switch (hdr->version) {
				case 2:
					fprintf(stderr, "DTBH v2\n");
					return dt_table_patch<bhtable_v2>(hdr, out);
				default:
					return 1;
			}
		} else if (MATCH(PXADT_MAGIC)) {
			auto hdr = reinterpret_cast<pxadt_hdr *>(dtb);
			switch (hdr->version) {
				case 1:
					fprintf(stderr, "PXA-DT v1\n");
					return dt_table_patch<pxatable_v1>(hdr, out);
				default:
					return 1;
			}
		} else if (MATCH(PXA19xx_MAGIC)) {
			auto hdr = reinterpret_cast<pxa19xx_hdr *>(dtb);
			switch (hdr->version) {
				case 1:
					fprintf(stderr, "PXA-19xx v1\n");
					return dt_table_patch<pxatable_v1>(hdr, out);
				default:
					return 1;
			}
		} else if (MATCH(SPRD_MAGIC)) {
			auto hdr = reinterpret_cast<sprd_hdr *>(dtb);
			switch (hdr->version) {
				case 1:
					fprintf(stderr, "SPRD v1\n");
					return dt_table_patch<sprdtable_v1>(hdr, out);
				default:
					return 1;
			}
		} else if (MATCH(DT_TABLE_MAGIC)) {
			auto hdr = reinterpret_cast<dt_table_header *>(dtb);
			switch (hdr->version) {
				case 0:
					fprintf(stderr, "DT_TABLE v0\n");
					return dt_table_patch<dt_table_entry>(hdr, out);
				default:
					return 1;
			}
		} else {
			return blob_patch(dtb, dtb_sz, out);
		}
	}
}
