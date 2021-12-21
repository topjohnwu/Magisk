macro(find_llvm_parts)
# Rely on llvm-config.
  set(CONFIG_OUTPUT)
  if(NOT LLVM_CONFIG_PATH)
    find_program(LLVM_CONFIG_PATH "llvm-config")
  endif()
  if(DEFINED LLVM_PATH)
    set(LLVM_INCLUDE_DIR ${LLVM_INCLUDE_DIR} CACHE PATH "Path to llvm/include")
    set(LLVM_PATH ${LLVM_PATH} CACHE PATH "Path to LLVM source tree")
    set(LLVM_MAIN_SRC_DIR ${LLVM_PATH})
    set(LLVM_CMAKE_PATH "${LLVM_PATH}/cmake/modules")
  elseif(LLVM_CONFIG_PATH)
    message(STATUS "Found LLVM_CONFIG_PATH as ${LLVM_CONFIG_PATH}")
    set(LIBCXXABI_USING_INSTALLED_LLVM 1)
    set(CONFIG_COMMAND ${LLVM_CONFIG_PATH}
      "--includedir"
      "--prefix"
      "--src-root")
    execute_process(
      COMMAND ${CONFIG_COMMAND}
      RESULT_VARIABLE HAD_ERROR
      OUTPUT_VARIABLE CONFIG_OUTPUT
    )
    if(NOT HAD_ERROR)
      string(REGEX REPLACE
        "[ \t]*[\r\n]+[ \t]*" ";"
        CONFIG_OUTPUT ${CONFIG_OUTPUT})
    else()
      string(REPLACE ";" " " CONFIG_COMMAND_STR "${CONFIG_COMMAND}")
      message(STATUS "${CONFIG_COMMAND_STR}")
      message(FATAL_ERROR "llvm-config failed with status ${HAD_ERROR}")
    endif()

    list(GET CONFIG_OUTPUT 0 INCLUDE_DIR)
    list(GET CONFIG_OUTPUT 1 LLVM_OBJ_ROOT)
    list(GET CONFIG_OUTPUT 2 MAIN_SRC_DIR)

    set(LLVM_INCLUDE_DIR ${INCLUDE_DIR} CACHE PATH "Path to llvm/include")
    set(LLVM_BINARY_DIR ${LLVM_OBJ_ROOT} CACHE PATH "Path to LLVM build tree")
    set(LLVM_MAIN_SRC_DIR ${MAIN_SRC_DIR} CACHE PATH "Path to LLVM source tree")

    # --cmakedir is supported since llvm r291218 (4.0 release)
    execute_process(
      COMMAND ${LLVM_CONFIG_PATH} --cmakedir
      RESULT_VARIABLE HAD_ERROR
      OUTPUT_VARIABLE CONFIG_OUTPUT
      ERROR_QUIET)
    if(NOT HAD_ERROR)
      string(STRIP "${CONFIG_OUTPUT}" LLVM_CMAKE_PATH_FROM_LLVM_CONFIG)
      file(TO_CMAKE_PATH "${LLVM_CMAKE_PATH_FROM_LLVM_CONFIG}" LLVM_CMAKE_PATH)
    else()
      file(TO_CMAKE_PATH "${LLVM_BINARY_DIR}" LLVM_BINARY_DIR_CMAKE_STYLE)
      set(LLVM_CMAKE_PATH "${LLVM_BINARY_DIR_CMAKE_STYLE}/lib${LLVM_LIBDIR_SUFFIX}/cmake/llvm")
    endif()
  else()
    set(LLVM_FOUND OFF)
    message(WARNING "UNSUPPORTED LIBCXXABI CONFIGURATION DETECTED: "
                    "llvm-config not found and LLVM_PATH not defined.\n"
                    "Reconfigure with -DLLVM_CONFIG_PATH=path/to/llvm-config "
                    "or -DLLVM_PATH=path/to/llvm-source-root.")
    return()
  endif()

  if (EXISTS "${LLVM_CMAKE_PATH}")
    list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_PATH}")
  elseif (EXISTS "${LLVM_MAIN_SRC_DIR}/cmake/modules")
    list(APPEND CMAKE_MODULE_PATH "${LLVM_MAIN_SRC_DIR}/cmake/modules")
  else()
    set(LLVM_FOUND OFF)
    message(WARNING "Neither ${LLVM_CMAKE_PATH} nor ${LLVM_MAIN_SRC_DIR}/cmake/modules found")
    return()
  endif()

  set(LLVM_FOUND ON)
endmacro(find_llvm_parts)

macro(configure_out_of_tree_llvm)
  message(STATUS "Configuring for standalone build.")
  set(LIBCXXABI_STANDALONE_BUILD 1)

  find_llvm_parts()

  # Add LLVM Functions --------------------------------------------------------
  if (LLVM_FOUND AND LIBCXXABI_USING_INSTALLED_LLVM)
    include(LLVMConfig) # For TARGET_TRIPLE
  else()
    if (WIN32)
      set(LLVM_ON_UNIX 0)
      set(LLVM_ON_WIN32 1)
    else()
      set(LLVM_ON_UNIX 1)
      set(LLVM_ON_WIN32 0)
    endif()
  endif()
  if (LLVM_FOUND)
    include(AddLLVM OPTIONAL)
    include(HandleLLVMOptions OPTIONAL)
  endif()

  # LLVM Options --------------------------------------------------------------
  if (NOT DEFINED LLVM_INCLUDE_TESTS)
    set(LLVM_INCLUDE_TESTS ${LLVM_FOUND})
  endif()
  if (NOT DEFINED LLVM_INCLUDE_DOCS)
    set(LLVM_INCLUDE_DOCS ${LLVM_FOUND})
  endif()
  if (NOT DEFINED LLVM_ENABLE_SPHINX)
    set(LLVM_ENABLE_SPHINX OFF)
  endif()

  # In a standalone build, we don't have llvm to automatically generate the
  # llvm-lit script for us.  So we need to provide an explicit directory that
  # the configurator should write the script into.
  set(LLVM_LIT_OUTPUT_DIR "${libcxxabi_BINARY_DIR}/bin")

  if (LLVM_INCLUDE_TESTS)
    # Required LIT Configuration ------------------------------------------------
    # Define the default arguments to use with 'lit', and an option for the user
    # to override.
    set(LLVM_EXTERNAL_LIT "${LLVM_MAIN_SRC_DIR}/utils/lit/lit.py")
    set(LIT_ARGS_DEFAULT "-sv --show-xfail --show-unsupported")
    if (MSVC OR XCODE)
      set(LIT_ARGS_DEFAULT "${LIT_ARGS_DEFAULT} --no-progress-bar")
    endif()
    set(LLVM_LIT_ARGS "${LIT_ARGS_DEFAULT}" CACHE STRING "Default options for lit")
  endif()

  # Required doc configuration
  if (LLVM_ENABLE_SPHINX)
    find_package(Sphinx REQUIRED)
  endif()

  if (LLVM_ON_UNIX AND NOT APPLE)
    set(LLVM_HAVE_LINK_VERSION_SCRIPT 1)
  else()
    set(LLVM_HAVE_LINK_VERSION_SCRIPT 0)
  endif()
endmacro(configure_out_of_tree_llvm)

configure_out_of_tree_llvm()
