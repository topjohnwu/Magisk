#!/usr/bin/env bash

set -ue

function usage() {
  cat <<EOM
$(basename ${0}) [-h|--help] --libcxx-root <LIBCXX-ROOT> --libcxxabi-root <LIBCXXABI-ROOT> --std <STD> --arch <ARCHITECTURE> [--lit-args <ARGS...>]

This script is used to continually test libc++ and libc++abi trunk on MacOS.

  --libcxx-root     Full path to the root of the libc++ repository to test.
  --libcxxabi-root  Full path to the root of the libc++abi repository to test.
  --std             Version of the C++ Standard to run the tests under (c++03, c++11, etc..).
  --arch            Architecture to build the tests for (32, 64).
  [--lit-args]      Additional arguments to pass to lit (optional). If there are multiple arguments, quote them to pass them as a single argument to this script.
  [--no-cleanup]    Do not cleanup the temporary directory that was used for testing at the end. This can be useful to debug failures. Make sure to clean up manually after.
  [-h, --help]      Print this help.
EOM
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --libcxx-root)
    LIBCXX_ROOT="${2}"
    if [[ ! -e "${LIBCXX_ROOT}" ]]; then
      echo "--libcxx-root '${LIBCXX_ROOT}' is not a valid directory"
      usage
      exit 1
    fi
    shift; shift
    ;;
    --libcxxabi-root)
    LIBCXXABI_ROOT="${2}"
    if [[ ! -e "${LIBCXXABI_ROOT}" ]]; then
      echo "--libcxxabi-root '${LIBCXXABI_ROOT}' is not a valid directory"
      usage
      exit 1
    fi
    shift; shift
    ;;
    --std)
    STD="${2}"
    shift; shift
    ;;
    --arch)
    ARCH="${2}"
    shift; shift
    ;;
    --lit-args)
    ADDITIONAL_LIT_ARGS="${2}"
    shift; shift
    ;;
    --no-cleanup)
    NO_CLEANUP=""
    shift
    ;;
    -h|--help)
    usage
    exit 0
    ;;
    *)
    echo "${1} is not a supported argument"
    usage
    exit 1
    ;;
  esac
done

if [[ -z ${LIBCXX_ROOT+x} ]]; then echo "--libcxx-root is a required parameter"; usage; exit 1; fi
if [[ -z ${LIBCXXABI_ROOT+x} ]]; then echo "--libcxxabi-root is a required parameter"; usage; exit 1; fi
if [[ -z ${STD+x} ]]; then echo "--std is a required parameter"; usage; exit 1; fi
if [[ -z ${ARCH+x} ]]; then echo "--arch is a required parameter"; usage; exit 1; fi
if [[ -z ${ADDITIONAL_LIT_ARGS+x} ]]; then ADDITIONAL_LIT_ARGS=""; fi


TEMP_DIR="$(mktemp -d)"
echo "Created temporary directory ${TEMP_DIR}"
function cleanup {
  if [[ -z ${NO_CLEANUP+x} ]]; then
    echo "Removing temporary directory ${TEMP_DIR}"
    rm -rf "${TEMP_DIR}"
  else
    echo "Temporary directory is at '${TEMP_DIR}', make sure to clean it up yourself"
  fi
}
trap cleanup EXIT


LLVM_ROOT="${TEMP_DIR}/llvm"
LIBCXX_BUILD_DIR="${TEMP_DIR}/libcxx-build"
LIBCXX_INSTALL_DIR="${TEMP_DIR}/libcxx-install"
LIBCXXABI_BUILD_DIR="${TEMP_DIR}/libcxxabi-build"
LIBCXXABI_INSTALL_DIR="${TEMP_DIR}/libcxxabi-install"

LLVM_TARBALL_URL="https://github.com/llvm-mirror/llvm/archive/master.tar.gz"
export CC="$(xcrun --find clang)"
export CXX="$(xcrun --find clang++)"


echo "@@@ Downloading LLVM tarball of master (only used for CMake configuration) @@@"
mkdir "${LLVM_ROOT}"
curl -L "${LLVM_TARBALL_URL}" | tar -xz --strip-components=1 -C "${LLVM_ROOT}"
echo "@@@@@@"


echo "@@@ Setting up LIT flags @@@"
LIT_FLAGS="-sv --param=std=${STD} ${ADDITIONAL_LIT_ARGS}"
if [[ "${ARCH}" == "32" ]]; then
  LIT_FLAGS+=" --param=enable_32bit=true"
fi
echo "@@@@@@"


echo "@@@ Configuring CMake for libc++ @@@"
mkdir -p "${LIBCXX_BUILD_DIR}"
(cd "${LIBCXX_BUILD_DIR}" &&
  xcrun cmake "${LIBCXX_ROOT}" -GNinja \
    -DLLVM_PATH="${LLVM_ROOT}" \
    -DCMAKE_INSTALL_PREFIX="${LIBCXX_INSTALL_DIR}" \
    -DLLVM_LIT_ARGS="${LIT_FLAGS}" \
    -DCMAKE_OSX_ARCHITECTURES="i386;x86_64" # Build a universal dylib
)
echo "@@@@@@"


echo "@@@ Configuring CMake for libc++abi @@@"
mkdir -p "${LIBCXXABI_BUILD_DIR}"
(cd "${LIBCXXABI_BUILD_DIR}" &&
  xcrun cmake "${LIBCXXABI_ROOT}" -GNinja \
    -DLIBCXXABI_LIBCXX_PATH="${LIBCXX_ROOT}" \
    -DLLVM_PATH="${LLVM_ROOT}" \
    -DCMAKE_INSTALL_PREFIX="${LIBCXXABI_INSTALL_DIR}" \
    -DLLVM_LIT_ARGS="${LIT_FLAGS}" \
    -DCMAKE_OSX_ARCHITECTURES="i386;x86_64" # Build a universal dylib
)
echo "@@@@@@"


echo "@@@ Building libc++.dylib and libc++abi.dylib from sources (just to make sure it works) @@@"
ninja -C "${LIBCXX_BUILD_DIR}" install-cxx
ninja -C "${LIBCXXABI_BUILD_DIR}" install-cxxabi
echo "@@@@@@"


echo "@@@ Running tests for libc++ @@@"
# TODO: We should run check-cxx-abilist too
ninja -C "${LIBCXX_BUILD_DIR}" check-cxx
echo "@@@@@@"


echo "@@@ Running tests for libc++abi @@@"
ninja -C "${LIBCXXABI_BUILD_DIR}" check-cxxabi
echo "@@@@@@"
