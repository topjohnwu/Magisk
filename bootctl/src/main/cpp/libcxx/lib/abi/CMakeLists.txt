
if (DEFINED TARGET_TRIPLE)
    if (TARGET_TRIPLE MATCHES "darwin")
        # Ignore the major, minor, and patchlevel versions of darwin targets.
        string(REGEX REPLACE "darwin[0-9]+\\.[0-9]+\\.[0-9]+" "darwin"
               GENERIC_TARGET_TRIPLE "${TARGET_TRIPLE}")
    elseif(TARGET_TRIPLE MATCHES "freebsd")
        # Ignore the major and minor versions of freebsd targets.
        string(REGEX REPLACE "freebsd[0-9]+\\.[0-9]+" "freebsd"
               GENERIC_TARGET_TRIPLE "${TARGET_TRIPLE}")
    else()
        set(GENERIC_TARGET_TRIPLE "${TARGET_TRIPLE}")
    endif()
endif()

# Detect if we are building in the same configuration used to generate
# the abilist files.
set(ABILIST_FILE "${CMAKE_CURRENT_LIST_DIR}/${GENERIC_TARGET_TRIPLE}.v${LIBCXX_ABI_VERSION}.abilist")
set(SYMDIFF_EXE "${LIBCXX_SOURCE_DIR}/utils/sym_diff.py")
if (EXISTS "${ABILIST_FILE}"
    AND TARGET cxx_shared
    AND ("${LIBCXX_CXX_ABI_LIBNAME}" STREQUAL "libcxxabi" OR
         (APPLE AND "${LIBCXX_CXX_ABI_LIBNAME}" STREQUAL "default"))
    AND NOT LIBCXX_ABI_UNSTABLE)
    add_custom_target(check-cxx-abilist
            ${SYMDIFF_EXE} --only-stdlib-symbols --strict ${ABILIST_FILE}
            $<TARGET_SONAME_FILE:cxx_shared>
            DEPENDS cxx_shared
            COMMENT "Testing ABI compatibility...")
else()
    message(STATUS "there is no pre-generated ABI list for the requested libc++ configuration. "
            "check-cxx-abilist target is not supported")
endif()
