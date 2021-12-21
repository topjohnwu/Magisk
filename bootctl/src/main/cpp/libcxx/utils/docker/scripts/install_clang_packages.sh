#!/usr/bin/env bash
#===- libcxx/utils/docker/scripts/install_clang_package.sh -----------------===//
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
Usage: install_clang_package.sh [options]

Install
Available options:
  -h|--help           show this help message
  --version           the numeric version of the package to use.
EOF
}

VERSION=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --version)
      shift
      VERSION="$1"
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



curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
add-apt-repository -s "deb http://apt.llvm.org/$(lsb_release -cs)/ llvm-toolchain-$(lsb_release -cs) main"
apt-get update
apt-get install -y --no-install-recommends clang

echo "Testing clang version..."
clang --version

echo "Testing clang++ version..."
clang++ --version

# Figure out the libc++ and libc++abi package versions that we want.
if [ "$VERSION" == "" ]; then
  VERSION="$(apt-cache search 'libc\+\+-[0-9]-dev' | awk '{print $1}' | awk -F- '{print $2}')"
  echo "Installing version '$VERSION'"
fi

apt-get install -y --no-install-recommends "libc++-$VERSION-dev" "libc++abi-$VERSION-dev"

echo "Done"
