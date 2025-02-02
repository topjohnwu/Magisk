#pragma once

#include "../xwrap.hpp"
#include "../files.hpp"
#include "../misc.hpp"
#include "../logging.hpp"
#include "../base-rs.hpp"

using rust::xpipe2;
using rust::fd_path;

inline bool operator==(::rust::Utf8CStr const &s, std::string_view const &ss) {
    return std::string_view{s.data(), s.size()} == ss;
}

inline bool operator==(std::string_view const &ss, ::rust::Utf8CStr const &s) {
    return s == ss;
}

namespace rust {
inline Utf8CStr::Utf8CStr(std::string_view const &s) noexcept : Utf8CStr{s.data(), s.size()} {}
inline Utf8CStr::Utf8CStr(std::string const &s) noexcept : Utf8CStr{s.data(), s.size()} {}
}
