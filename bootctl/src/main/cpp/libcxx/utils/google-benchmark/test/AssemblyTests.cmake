
include(split_list)

set(ASM_TEST_FLAGS "")
check_cxx_compiler_flag(-O3 BENCHMARK_HAS_O3_FLAG)
if (BENCHMARK_HAS_O3_FLAG)
  list(APPEND ASM_TEST_FLAGS -O3)
endif()

check_cxx_compiler_flag(-g0 BENCHMARK_HAS_G0_FLAG)
if (BENCHMARK_HAS_G0_FLAG)
  list(APPEND ASM_TEST_FLAGS -g0)
endif()

check_cxx_compiler_flag(-fno-stack-protector BENCHMARK_HAS_FNO_STACK_PROTECTOR_FLAG)
if (BENCHMARK_HAS_FNO_STACK_PROTECTOR_FLAG)
  list(APPEND ASM_TEST_FLAGS -fno-stack-protector)
endif()

split_list(ASM_TEST_FLAGS)
string(TOUPPER "${CMAKE_CXX_COMPILER_ID}" ASM_TEST_COMPILER)

macro(add_filecheck_test name)
  cmake_parse_arguments(ARG "" "" "CHECK_PREFIXES" ${ARGV})
  add_library(${name} OBJECT ${name}.cc)
  set_target_properties(${name} PROPERTIES COMPILE_FLAGS "-S ${ASM_TEST_FLAGS}")
  set(ASM_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${name}.s")
  add_custom_target(copy_${name} ALL
      COMMAND ${PROJECT_SOURCE_DIR}/tools/strip_asm.py
        $<TARGET_OBJECTS:${name}>
        ${ASM_OUTPUT_FILE}
      BYPRODUCTS ${ASM_OUTPUT_FILE})
  add_dependencies(copy_${name} ${name})
  if (NOT ARG_CHECK_PREFIXES)
    set(ARG_CHECK_PREFIXES "CHECK")
  endif()
  foreach(prefix ${ARG_CHECK_PREFIXES})
    add_test(NAME run_${name}_${prefix}
        COMMAND
          ${LLVM_FILECHECK_EXE} ${name}.cc
          --input-file=${ASM_OUTPUT_FILE}
          --check-prefixes=CHECK,CHECK-${ASM_TEST_COMPILER}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
endmacro()

