#!/usr/bin/env bash
#===- libcxx/utils/docker/scripts/build-gcc.sh ----------------------------===//
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===-----------------------------------------------------------------------===//

set -e


function show_usage() {
  cat << EOF
Usage: build-gcc.sh [options]

Run autoconf with the specified arguments. Used inside docker container.

Available options:
  -h|--help           show this help message
  --source            the source path from which to run the configuration.
  --to                destination directory where to install the targets.
Required options: --to, at least one --install-target.

All options after '--' are passed to CMake invocation.
EOF
}

GCC_INSTALL_DIR=""
GCC_SOURCE_DIR=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --to)
      shift
      GCC_INSTALL_DIR="$1"
      shift
      ;;
    --source)
      shift
      GCC_SOURCE_DIR="$1"
      shift
      ;;
    -h|--help)
      show_usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
  esac
done

if [ "$GCC_INSTALL_DIR" == "" ]; then
  echo "No install directory. Please specify the --to argument."
  exit 1
fi

if [ "$GCC_SOURCE_DIR" == "" ]; then
  echo "No source directory. Please specify the --source argument."
  exit 1
fi

GCC_NAME=`basename $GCC_SOURCE_DIR`
GCC_BUILD_DIR="/tmp/gcc-build-root/build-$GCC_NAME"

mkdir -p "$GCC_INSTALL_DIR"
mkdir -p "$GCC_BUILD_DIR"
pushd "$GCC_BUILD_DIR"

# Run the build as specified in the build arguments.
echo "Running configuration"
$GCC_SOURCE_DIR/configure --prefix=$GCC_INSTALL_DIR \
  --disable-bootstrap --disable-libgomp --disable-libitm \
  --disable-libvtv --disable-libcilkrts --disable-libmpx \
  --disable-liboffloadmic --disable-libcc1 --enable-languages=c,c++

NPROC=`nproc`
echo "Running build with $NPROC threads"
make -j$NPROC

echo "Installing to $GCC_INSTALL_DIR"
make install -j$NPROC

popd

# Cleanup.
rm -rf "$GCC_BUILD_DIR"

echo "Done"