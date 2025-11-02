#!/usr/bin/env bash
# generate_prebuilts.sh
# RAFAELIA: compile liblzma/liblz4 for Android NDK ABIs and copy to native/prebuilt/<abi>
#
# Notes:
# - This is a convenience helper. Building these libraries for Android properly
#   requires correct configure/build steps for each library and ABI.
# - You must have the NDK installed and NDK_PATH set (or default path chosen below).
# - You must check out/build the lzma (xz-utils/liblzma) and lz4 sources into
#   directories referenced below (LZMA_SRC and LZ4_SRC).
#
# Usage:
#   export NDK_PATH=/path/to/android/ndk
#   ./scripts/generate_prebuilts.sh
#
set -euo pipefail

# Default NDK path (adjust if needed)
NDK_PATH=${NDK_PATH:-"$HOME/Android/Sdk/ndk/25.2.9519653"}
PREBUILT_DIR="native/prebuilt"

# Adjust these to where you have sources for liblzma (xz) and lz4
LZMA_SRC="${PWD}/third_party/xz"
LZ4_SRC="${PWD}/third_party/lz4"

ABIS=("armeabi-v7a" "arm64-v8a" "x86" "x86_64")

if [ ! -d "${NDK_PATH}" ]; then
  echo "NDK not found at ${NDK_PATH}. Set NDK_PATH env var to your Android NDK location."
  exit 1
fi

if [ ! -d "${LZMA_SRC}" ]; then
  echo "Please clone/build liblzma sources under ${LZMA_SRC}"
  echo "Example: git clone https://git.tukaani.org/xz.git ${LZMA_SRC}"
  exit 1
fi

if [ ! -d "${LZ4_SRC}" ]; then
  echo "Please clone/build lz4 sources under ${LZ4_SRC}"
  echo "Example: git clone https://github.com/lz4/lz4.git ${LZ4_SRC}"
  exit 1
fi

mkdir -p "${PREBUILT_DIR}"

for ABI in "${ABIS[@]}"; do
  ABI_DIR="${PREBUILT_DIR}/${ABI}"
  mkdir -p "${ABI_DIR}"
  echo "[*] Building prebuilts for ABI: ${ABI}"

  case "${ABI}" in
    "armeabi-v7a")
      TARGET_HOST="armv7a-linux-androideabi"
      API=19
      CLANG_TRIPLE="armv7a-linux-androideabi${API}-clang"
      ;;
    "arm64-v8a")
      TARGET_HOST="aarch64-linux-android"
      API=21
      CLANG_TRIPLE="aarch64-linux-android${API}-clang"
      ;;
    "x86")
      TARGET_HOST="i686-linux-android"
      API=19
      CLANG_TRIPLE="i686-linux-android${API}-clang"
      ;;
    "x86_64")
      TARGET_HOST="x86_64-linux-android"
      API=21
      CLANG_TRIPLE="x86_64-linux-android${API}-clang"
      ;;
    *)
      echo "Unknown ABI ${ABI}"; exit 1
      ;;
  esac

  TOOLCHAIN_BIN="${NDK_PATH}/toolchains/llvm/prebuilt/linux-x86_64/bin"
  CC="${TOOLCHAIN_BIN}/${CLANG_TRIPLE}"
  AR="${TOOLCHAIN_BIN}/llvm-ar"
  RANLIB="${TOOLCHAIN_BIN}/llvm-ranlib"

  echo "  - Using clang: ${CC}"

  # Build liblz4 static
  echo "  - Building liblz4..."
  pushd "${LZ4_SRC}" > /dev/null
  make clean || true
  # Attempt to use minimal build with single-file compilation (not robust for all versions)
  ${CC} -O3 -fPIC -c lib/lz4.c -o "${ABI_DIR}/lz4.o"
  ${AR} rcs "${ABI_DIR}/liblz4.a" "${ABI_DIR}/lz4.o"
  ${RANLIB} "${ABI_DIR}/liblz4.a"
  rm -f "${ABI_DIR}/lz4.o"
  popd > /dev/null

  # Build liblzma (xz)
  echo "  - Building liblzma (xz)..."
  pushd "${LZMA_SRC}" > /dev/null
  make clean || true
  # liblzma is more complex; try a simple ./configure + make with CC set (may require autotools)
  # If autotools not present, users should build liblzma manually per ABI and copy .a into ABI_DIR
  export CC="${CC}"
  if [ -f "./autogen.sh" ]; then
    ./autogen.sh || true
  fi
  ./configure --host=${TARGET_HOST} --enable-static --disable-shared CC=${CC} AR=${AR} RANLIB=${RANLIB} --prefix="${ABI_DIR}/install" || true
  make -j$(nproc) || true
  # try to find liblzma.a in build
  if [ -f "src/liblzma/.libs/liblzma.a" ]; then
    cp src/liblzma/.libs/liblzma.a "${ABI_DIR}/liblzma.a"
  elif [ -f "lib/.libs/liblzma.a" ]; then
    cp lib/.libs/liblzma.a "${ABI_DIR}/liblzma.a"
  else
    echo "  Warning: liblzma static not found automatically. Please build liblzma for ${ABI} and place liblzma.a in ${ABI_DIR}"
  fi
  popd > /dev/null

  echo "  -> Prebuilts for ${ABI} should be in ${ABI_DIR}"
done

echo "[*] Done. Verify ${PREBUILT_DIR} contains liblz4.a and liblzma.a for each ABI required."
