#include <sys/mount.h>

#include <magisk.hpp>
#include <utils.hpp>
#include <link.h>

#include "deny.hpp"

using namespace std;

static void lazy_unmount(const char* mountpoint) {
    if (umount2(mountpoint, MNT_DETACH) != -1)
        LOGD("denylist: Unmounted (%s)\n", mountpoint);
}

#define TMPFS_MNT(dir) (mentry->mnt_type == "tmpfs"sv && str_starts(mentry->mnt_dir, "/" #dir))

void revert_unmount() {
    vector<string> targets;

    // Unmount dummy skeletons and MAGISKTMP
    targets.push_back(MAGISKTMP);
    parse_mnt("/proc/self/mounts", [&](mntent *mentry) {
        if (TMPFS_MNT(system) || TMPFS_MNT(vendor) || TMPFS_MNT(product) || TMPFS_MNT(system_ext))
            targets.emplace_back(mentry->mnt_dir);
        return true;
    });

    for (auto &s : reversed(targets))
        lazy_unmount(s.data());
    targets.clear();

    // Unmount all Magisk created mounts
    parse_mnt("/proc/self/mounts", [&](mntent *mentry) {
        if (str_contains(mentry->mnt_fsname, BLOCKDIR))
            targets.emplace_back(mentry->mnt_dir);
        return true;
    });

    for (auto &s : reversed(targets))
        lazy_unmount(s.data());
}

void cleanup_preload() {
    char buff[256];
    off_t load_addr;
#if defined(__LP64__)
    std::string elf = "/linker64";
#else
    std::string elf = "/linker";
#endif
    bool found = false;
    FILE *maps = fopen("/proc/self/maps", "r");
    while (fgets(buff, sizeof(buff), maps)) {
        if ((strstr(buff, "r-xp") || strstr(buff, "r--p")) && strstr(buff, elf.data())) {
            std::string_view b = buff;
            if (auto begin = b.find_last_of(' '); begin != std::string_view::npos && b[++begin] == '/') {
                found = true;
                elf = b.substr(begin);
                if (elf.back() == '\n') elf.pop_back();
                break;
            }
        }
    }

    fclose(maps);
    char *next = buff;
    load_addr = strtoul(buff, &next, 16);
    if (!found || next == buff) {
        return;
    }

    int fd = open(elf.data(), O_RDONLY);

    if (fd < 0) {
        return;
    }
    size_t header_size = lseek(fd, 0, SEEK_END);

    if (header_size <= 0) {
        close(fd);
        return;
    }

    auto header_mmap = mmap(nullptr, header_size, PROT_READ, MAP_SHARED, fd, 0);
    auto *header = reinterpret_cast<ElfW(Ehdr) *>(header_mmap);
    auto header_diff = reinterpret_cast<uintptr_t>(header);

    close(fd);

    auto *section_header = reinterpret_cast<ElfW(Shdr) *>(header_diff + header->e_shoff);

    auto shoff = reinterpret_cast<uintptr_t>(section_header);
    char *section_str = reinterpret_cast<char *>(
            header_diff +section_header[header->e_shstrndx].sh_offset);

    off_t bias = -4396;

    ElfW(Shdr) *strtab = nullptr;
    ElfW(Shdr) *dynsym = nullptr;
    ElfW(Sym) *symtab_start = nullptr;
    ElfW(Off) symtab_count = 0;
    ElfW(Off) symstr_offset_for_symtab = 0;
    ElfW(Off) symtab_offset;
    ElfW(Off) symtab_size;

    for (int i = 0; i < header->e_shnum; i++, shoff += header->e_shentsize) {
        auto *section_h = (ElfW(Shdr) *) shoff;
        char *sname = section_h->sh_name + section_str;
        auto entsize = section_h->sh_entsize;
        switch (section_h->sh_type) {
            case SHT_DYNSYM: {
                if (bias == -4396) {
                    dynsym = section_h;
                }
                break;
            }
            case SHT_SYMTAB: {
                if (strcmp(sname, ".symtab") == 0) {
                    symtab_offset = section_h->sh_offset;
                    symtab_size = section_h->sh_size;
                    symtab_count = symtab_size / entsize;
                    symtab_start = reinterpret_cast<ElfW(Sym) *>(header_diff + symtab_offset);
                }
                break;
            }
            case SHT_STRTAB: {
                if (bias == -4396) {
                    strtab = section_h;
                }
                if (strcmp(sname, ".strtab") == 0) {
                    symstr_offset_for_symtab = section_h->sh_offset;
                }
                break;
            }
            case SHT_PROGBITS: {
                if (strtab == nullptr || dynsym == nullptr) break;
                if (bias == -4396) {
                    bias = (off_t) section_h->sh_addr - (off_t) section_h->sh_offset;
                }
                break;
            }
        }
    }

    if (symtab_start != nullptr && symstr_offset_for_symtab != 0) {
        for (ElfW(Off) i = 0; i < symtab_count; i++) {
            unsigned int st_type = ELF_ST_TYPE(symtab_start[i].st_info);
            const char *st_name = reinterpret_cast<const char *>(
                    header_diff + symstr_offset_for_symtab + symtab_start[i].st_name);
            if ((st_type == STT_FUNC || st_type == STT_OBJECT) && symtab_start[i].st_size) {
                if (strcmp(st_name, "__dl__ZL13g_ld_preloads") == 0) {
                    auto *preloadVector = reinterpret_cast<std::vector<void *> *>(
                            symtab_start[i].st_value + load_addr - bias);
                    if (preloadVector != nullptr && !preloadVector->empty()) {
                        preloadVector->clear();
                    }
                }
            }
        }
    }

    munmap(header_mmap, header_size);
}
