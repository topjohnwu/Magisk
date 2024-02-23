#pragma once

#include <base.hpp>

namespace rust {
struct MagiskD;
}

struct MagiskD {
    // Make sure only references can exist
    ~MagiskD() = delete;

    // Binding to Rust
    static const MagiskD &get();

    // C++ implementation
    void reboot() const;
    bool post_fs_data() const;
    void late_start() const;
    void boot_complete() const;

    const rust::MagiskD *operator->() const;

private:
    const rust::MagiskD &as_rust() const;
};

const char *get_magisk_tmp();
static inline rust::Utf8CStr get_magisk_tmp_rs() { return get_magisk_tmp(); }
