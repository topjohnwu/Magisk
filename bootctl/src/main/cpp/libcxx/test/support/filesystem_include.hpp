#ifndef TEST_SUPPORT_FILESYSTEM_INCLUDE_HPP
#define TEST_SUPPORT_FILESYSTEM_INCLUDE_HPP

#include <filesystem>
#include "test_macros.h"

#if defined(_LIBCPP_VERSION) && TEST_STD_VER < 17
namespace fs = std::__fs::filesystem;
#else
namespace fs = std::filesystem;
#endif

#endif
