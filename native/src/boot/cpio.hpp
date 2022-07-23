#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <string_view>

struct cpio_newc_header;

struct cpio_entry {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t filesize;
    void *data;

    explicit cpio_entry(uint32_t mode = 0);
    explicit cpio_entry(const cpio_newc_header *h);
    ~cpio_entry() { free(data); }
};

class cpio {
public:
    struct StringCmp {
        using is_transparent = void;
        bool operator()(std::string_view a, std::string_view b) const {
            return a < b;
        }
    };
    using entry_map = std::map<std::string, std::unique_ptr<cpio_entry>, StringCmp>;

    void load_cpio(const char *file);
    void dump(const char *file);
    void rm(const char *name, bool r = false);
    void extract();
    bool extract(const char *name, const char *file);
    bool exists(const char *name);
    void add(mode_t mode, const char *name, const char *file);
    void mkdir(mode_t mode, const char *name);
    void ln(const char *target, const char *name);
    bool mv(const char *from, const char *to);

protected:
    entry_map entries;

    static void extract_entry(const entry_map::value_type &e, const char *file);
    void rm(entry_map::iterator it);
    void mv(entry_map::iterator it, const char *name);

private:
    void dump(FILE *out);
    void insert(std::string_view name, cpio_entry *e);
    void load_cpio(const char *buf, size_t sz);
};
