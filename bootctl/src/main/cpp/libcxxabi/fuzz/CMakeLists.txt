# See http://llvm.org/docs/LibFuzzer.html
if( LLVM_USE_SANITIZE_COVERAGE )
  add_executable(cxa_demangle_fuzzer
    cxa_demangle_fuzzer.cpp
    ../src/cxa_demangle.cpp
    )

  target_link_libraries(cxa_demangle_fuzzer
    LLVMFuzzer
    )
endif()
