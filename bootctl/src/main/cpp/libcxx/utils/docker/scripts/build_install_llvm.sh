#!/usr/bin/env bash
#===- llvm/utils/docker/scripts/build_install_llvm.sh ---------------------===//
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
Usage: build_install_llvm.sh [options] -- [cmake-args]

Run cmake with the specified arguments. Used inside docker container.
Passes additional -DCMAKE_INSTALL_PREFIX and puts the build results into
the directory specified by --to option.

Available options:
  -h|--help           show this help message
  -i|--install-target name of a cmake install target to build and include in
                      the resulting archive. Can be specified multiple times.
  --install           destination directory where to install the targets.
  --source            location of the source tree.
  --build             location to use as the build directory.
Required options: --to, --source, --build, and at least one --install-target.

All options after '--' are passed to CMake invocation.
EOF
}

CMAKE_ARGS=""
CMAKE_INSTALL_TARGETS=""
CLANG_INSTALL_DIR=""
CLANG_SOURCE_DIR=""
CLANG_BUILD_DIR=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    -i|--install-target)
      shift
      CMAKE_INSTALL_TARGETS="$CMAKE_INSTALL_TARGETS $1"
      shift
      ;;
    --source)
      shift
      CLANG_SOURCE_DIR="$1"
      shift
      ;;
    --build)
      shift
      CLANG_BUILD_DIR="$1"
      shift
      ;;
    --install)
      shift
      CLANG_INSTALL_DIR="$1"
      shift
      ;;
    --)
      shift
      CMAKE_ARGS="$*"
      shift $#
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

if [ "$CLANG_SOURCE_DIR" == "" ]; then
  echo "No source directory. Please pass --source."
  exit 1
fi

if [ "$CLANG_BUILD_DIR" == "" ]; then
  echo "No build directory. Please pass --build"
  exit 1
fi

if [ "$CMAKE_INSTALL_TARGETS" == "" ]; then
  echo "No install targets. Please pass one or more --install-target."
  exit 1
fi

if [ "$CLANG_INSTALL_DIR" == "" ]; then
  echo "No install directory. Please specify the --to argument."
  exit 1
fi

echo "Building in $CLANG_BUILD_DIR"
mkdir -p "$CLANG_BUILD_DIR"
pushd "$CLANG_BUILD_DIR"

# Run the build as specified in the build arguments.
echo "Running build"
cmake -GNinja \
  -DCMAKE_INSTALL_PREFIX="$CLANG_INSTALL_DIR" \
  $CMAKE_ARGS \
  "$CLANG_SOURCE_DIR"
ninja $CMAKE_INSTALL_TARGETS

popd

# Cleanup.
rm -rf "$CLANG_BUILD_DIR"

echo "Done"
