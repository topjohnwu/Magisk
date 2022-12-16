#include <bitset>
#include <vector>
#include <map>
#include <libfdt.h>

#include <base.hpp>

#include "magiskboot.hpp"
#include "dtb.hpp"
#include "format.hpp"

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
    if (auto name = fdt_get_name(fdt, node, nullptr); name && name == "fstab"sv)
        return node;
    int child;
    fdt_for_each_subnode(child, fdt, node) {
        int fstab = find_fstab(fdt, child);
        if (fstab >= 0)
            return fstab;
    }
    return -1;
}

template<typename Func>
static void for_each_fdt(const char *file, bool rw, Func fn) {
    auto m = mmap_data(file, rw);
    uint8_t *end = m.buf + m.sz;
    for (uint8_t *fdt = m.buf; fdt < end;) {
        fdt = static_cast<uint8_t*>(memmem(fdt, end - fdt, DTB_MAGIC, sizeof(fdt32_t)));
        if (fdt == nullptr)
            break;
        fn(fdt);
        fdt += fdt_totalsize(fdt);
    }
}

static void dtb_print(const char *file, bool fstab) {
    fprintf(stderr, "Loading dtbs from [%s]\n", file);
    int dtb_num = 0;
    for_each_fdt(file, false, [&](uint8_t *fdt) {
        if (fstab) {
            if (int node = find_fstab(fdt); node >= 0) {
                fprintf(stderr, "Found fstab in dtb.%04d\n", dtb_num);
                print_node(fdt, node);
            }
        } else {
            fprintf(stderr, "Printing dtb.%04d\n", dtb_num);
            print_node(fdt);
        }
        ++dtb_num;
    });
    fprintf(stderr, "\n");
}

static bool dtb_patch(const char *file) {
    fprintf(stderr, "Loading dtbs from [%s]\n", file);

    bool keep_verity = check_env("KEEPVERITY");
    bool patched = false;
    for_each_fdt(file, true, [&](uint8_t *fdt) {
        int node;
        // Patch the chosen node for bootargs
        fdt_for_each_subnode(node, fdt, 0) {
            if (auto name = fdt_get_name(fdt, node, nullptr); !name || name != "chosen"sv)
                continue;
            int len;
            if (auto value = fdt_getprop(fdt, node, "bootargs", &len)) {
                if (void *skip = memmem(value, len, "skip_initramfs", 14)) {
                    fprintf(stderr, "Patch [skip_initramfs] -> [want_initramfs]\n");
                    memcpy(skip, "want", 4);
                    patched = true;
                }
            }
            break;
        }
        if (!keep_verity) {
            if (int fstab = find_fstab(fdt); fstab >= 0) {
                fdt_for_each_subnode(node, fdt, fstab) {
                    int len;
                    char *value = (char *) fdt_getprop(fdt, node, "fsmgr_flags", &len);
                    patched |= patch_verity(value, len) != len;
                }
            }
        }
    });
    return patched;
}

[[noreturn]]
static void dtb_test(const char *file) {
    for_each_fdt(file, false, [&](uint8_t *fdt) {
        // Find the system node in fstab
        if (int fstab = find_fstab(fdt); fstab >= 0) {
            int node;
            fdt_for_each_subnode(node, fdt, fstab) {
                if (auto name = fdt_get_name(fdt, node, nullptr); !name || name != "system"sv)
                    continue;
                int len;
                if (auto value = fdt_getprop(fdt, node, "mnt_point", &len)) {
                    // If mnt_point is set to /system_root, abort!
                    if (strncmp(static_cast<const char *>(value), "/system_root", len) == 0) {
                        exit(1);
                    }
                }
            }
        }
    });
    exit(0);
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
    } else if (argv[0] == "test"sv) {
        dtb_test(dtb);
    } else {
        return 1;
    }
}

// The following code is unused, left here for historical purpose. Since the code is
// extremely complicated, I won't want to rewrite this whole thing if somehow we need
// to use it in the future...

namespace {

struct fdt_blob {
    void *fdt;
    uint32_t offset;
    uint32_t len;
};

static bool fdt_patch(void *fdt) {
    int fstab = find_fstab(fdt);
    if (fstab < 0)
        return false;
    bool modified = false;
    int node;
    fdt_for_each_subnode(node, fdt, fstab) {
        const char *name = fdt_get_name(fdt, node, nullptr);
        // Force remove AVB for 2SI since it may bootloop some devices
        int len;
        auto value = (const char *) fdt_getprop(fdt, node, "fsmgr_flags", &len);
        string copy(value, len);
        uint32_t new_len = patch_verity(copy.data(), len);
        if (new_len != len) {
            modified = true;
            fdt_setprop(fdt, node, "fsmgr_flags", copy.data(), new_len);
        }
        if (name == "system"sv) {
            fprintf(stderr, "Setting [mnt_point] to [/system_root]\n");
            fdt_setprop_string(fdt, node, "mnt_point", "/system_root");
            modified = true;
        }
    }
    return modified;
}

#define MAX_FDT_GROWTH 256

template <class Table, class Header>
static bool dt_table_patch(const Header *hdr, const char *out) {
    map<uint32_t, fdt_blob> dtb_map;
    auto buf = reinterpret_cast<const uint8_t *>(hdr);
    auto tables = reinterpret_cast<const Table *>(buf + sizeof(Header));

    constexpr bool is_aosp = std::is_same_v<Header, dt_table_header>;

    // AOSP DTB store ints in big endian
    using endian_conv = uint32_t (*)(uint32_t);
    endian_conv be_to_le;
    endian_conv le_to_be;
    if constexpr (is_aosp) {
        be_to_le = fdt32_to_cpu;
        le_to_be = cpu_to_fdt32;
    } else {
        be_to_le = le_to_be = [](uint32_t x) { return x; };
    }

    // Collect all dtbs
    auto num_dtb = be_to_le(hdr->num_dtbs);
    for (int i = 0; i < num_dtb; ++i) {
        auto offset = be_to_le(tables[i].offset);
        if (dtb_map.count(offset) == 0) {
            auto blob = buf + offset;
            uint32_t size = fdt_totalsize(blob);
            auto fdt = malloc(size + MAX_FDT_GROWTH);
            memcpy(fdt, blob, size);
            fdt_open_into(fdt, fdt, size + MAX_FDT_GROWTH);
            dtb_map[offset] = { fdt, offset };
        }
    }
    if (dtb_map.empty())
        return false;

    // Patch fdt
    bool modified = false;
    for (auto &[_, blob] : dtb_map)
        modified |= fdt_patch(blob.fdt);
    if (!modified)
        return false;

    unlink(out);
    int fd = xopen(out, O_RDWR | O_CREAT | O_CLOEXEC, 0644);

    // This value is only used if AOSP DTB
    uint32_t total_size = 0;

    // Copy headers and tables
    total_size += xwrite(fd, buf, dtb_map.begin()->first);

    // mmap rw to patch table values retroactively
    auto mmap_sz = lseek(fd, 0, SEEK_CUR);
    auto addr = (uint8_t *) xmmap(nullptr, mmap_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Guess alignment using gcd
    uint32_t align = 1;
    if constexpr (!is_aosp) {
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
        if constexpr (!is_aosp) {
            val.second.len = align_to(size, align);
            write_zero(fd, align_padding(lseek(fd, 0, SEEK_CUR), align));
        }
        free(fdt);
    }

    // Patch headers
    if constexpr (is_aosp) {
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

    return true;
}

static bool blob_patch(uint8_t *dtb, size_t dtb_sz, const char *out) {
    vector<uint8_t *> fdt_list;
    vector<uint32_t> padding_list;

    uint8_t * const end = dtb + dtb_sz;
    for (uint8_t *curr = dtb; curr < end;) {
        curr = static_cast<uint8_t*>(memmem(curr, end - curr, DTB_MAGIC, sizeof(fdt32_t)));
        if (curr == nullptr)
            break;
        auto len = fdt_totalsize(curr);
        auto fdt = static_cast<uint8_t *>(malloc(len + MAX_FDT_GROWTH));
        memcpy(fdt, curr, len);
        fdt_pack(fdt);
        uint32_t padding = len - fdt_totalsize(fdt);
        padding_list.push_back(padding);
        fdt_open_into(fdt, fdt, len + MAX_FDT_GROWTH);
        fdt_list.push_back(fdt);
        curr += len;
    }

    bool modified = false;
    for (auto fdt : fdt_list)
        modified |= fdt_patch(fdt);
    if (!modified)
        return false;

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

    return true;
}

#define DTB_MATCH(s) BUFFER_MATCH(dtb, s)

[[maybe_unused]]
static bool dtb_patch_rebuild(uint8_t *dtb, size_t dtb_sz, const char *file) {
    if (DTB_MATCH(QCDT_MAGIC)) {
        auto hdr = reinterpret_cast<qcdt_hdr*>(dtb);
        switch (hdr->version) {
            case 1:
                fprintf(stderr, "QCDT v1\n");
                return dt_table_patch<qctable_v1>(hdr, file);
            case 2:
                fprintf(stderr, "QCDT v2\n");
                return dt_table_patch<qctable_v2>(hdr, file);
            case 3:
                fprintf(stderr, "QCDT v3\n");
                return dt_table_patch<qctable_v3>(hdr, file);
            default:
                return false;
        }
    } else if (DTB_MATCH(DTBH_MAGIC)) {
        auto hdr = reinterpret_cast<dtbh_hdr *>(dtb);
        switch (hdr->version) {
            case 2:
                fprintf(stderr, "DTBH v2\n");
                return dt_table_patch<bhtable_v2>(hdr, file);
            default:
                return false;
        }
    } else if (DTB_MATCH(PXADT_MAGIC)) {
        auto hdr = reinterpret_cast<pxadt_hdr *>(dtb);
        switch (hdr->version) {
            case 1:
                fprintf(stderr, "PXA-DT v1\n");
                return dt_table_patch<pxatable_v1>(hdr, file);
            default:
                return false;
        }
    } else if (DTB_MATCH(PXA19xx_MAGIC)) {
        auto hdr = reinterpret_cast<pxa19xx_hdr *>(dtb);
        switch (hdr->version) {
            case 1:
                fprintf(stderr, "PXA-19xx v1\n");
                return dt_table_patch<pxatable_v1>(hdr, file);
            default:
                return false;
        }
    } else if (DTB_MATCH(SPRD_MAGIC)) {
        auto hdr = reinterpret_cast<sprd_hdr *>(dtb);
        switch (hdr->version) {
            case 1:
                fprintf(stderr, "SPRD v1\n");
                return dt_table_patch<sprdtable_v1>(hdr, file);
            default:
                return false;
        }
    } else if (DTB_MATCH(DT_TABLE_MAGIC)) {
        auto hdr = reinterpret_cast<dt_table_header *>(dtb);
        switch (hdr->version) {
            case 0:
                fprintf(stderr, "DT_TABLE v0\n");
                return dt_table_patch<dt_table_entry>(hdr, file);
            default:
                return false;
        }
    } else {
        return blob_patch(dtb, dtb_sz, file);
    }
}

} // namespace
