#!/usr/bin/env bash
#===- llvm/utils/docker/scripts/checkout.sh ---------------------===//
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
Usage: checkout.sh [options]

Checkout svn sources into /tmp/clang-build/src. Used inside a docker container.

Available options:
  -h|--help           show this help message
  -b|--branch         svn branch to checkout, i.e. 'trunk',
                      'branches/release_40'
                      (default: 'trunk')
  -p|--llvm-project   name of an svn project to checkout.
                      For clang, please use 'clang', not 'cfe'.
                      Project 'llvm' is always included and ignored, if
                      specified.
                      Can be specified multiple times.
EOF
}

LLVM_BRANCH=""
# We always checkout llvm
LLVM_PROJECTS="llvm"
SOURCE_DIR=""

function contains_project() {
  local TARGET_PROJ="$1"
  local PROJ
  for PROJ in $LLVM_PROJECTS; do
    if [ "$PROJ" == "$TARGET_PROJ" ]; then
      return 0
    fi
  done
  return 1
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --to)
      shift
      SOURCE_DIR="$1"
      shift
      ;;
    -b|--branch)
      shift
      LLVM_BRANCH="$1"
      shift
      ;;
    -p|--llvm-project)
      shift
      PROJ="$1"
      shift

      if [ "$PROJ" == "cfe" ]; then
        PROJ="clang"
      fi

      if ! contains_project "$PROJ" ; then
        if [ "$PROJ" == "clang-tools-extra" ] && [ ! contains_project "clang" ]; then
          echo "Project 'clang-tools-extra' specified before 'clang'. Adding 'clang' to a list of projects first."
          LLVM_PROJECTS="$LLVM_PROJECTS clang"
        fi
        LLVM_PROJECTS="$LLVM_PROJECTS $PROJ"
      else
        echo "Project '$PROJ' is already enabled, ignoring extra occurrences."
      fi
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

if [ "$SOURCE_DIR" == "" ]; then
  echo "Must specify checkout directory using --to"
  exit 1
fi

if [ "$LLVM_BRANCH" == "" ]; then
  GIT_BRANCH_ARG=""
else
  GIT_BRANCH_ARG="--branch $LLVM_BRANCH"
fi

if [ "$LLVM_SVN_REV" != "" ]; then
  SVN_REV_ARG="-r$LLVM_SVN_REV"
  echo "Checking out svn revision r$LLVM_SVN_REV."
else
  SVN_REV_ARG=""
  echo "Checking out latest svn revision."
fi

# Get the sources from svn.
echo "Checking out sources from git"

for LLVM_PROJECT in $LLVM_PROJECTS; do
  if [ "$LLVM_PROJECT" == "llvm" ]; then
    CHECKOUT_DIR="$SOURCE_DIR"
  elif [ "$LLVM_PROJECT" == "libcxx" ] || [ "$LLVM_PROJECT" == "libcxxabi" ] || [ "$LLVM_PROJECT" == "compiler-rt" ]; then
    CHECKOUT_DIR="$SOURCE_DIR/projects/$LLVM_PROJECT"
  elif [ "$LLVM_PROJECT" == "clang" ]; then
    CHECKOUT_DIR="$SOURCE_DIR/tools/clang"
  elif [ "$LLVM_PROJECT" == "clang-tools-extra" ]; then
    CHECKOUT_DIR="$SOURCE_DIR/tools/clang/tools/extra"
  else
    CHECKOUT_DIR="$SOURCE_DIR/$LLVM_PROJECT"
  fi

  echo "Checking out https://git.llvm.org/git/$LLVM_PROJECT to $CHECKOUT_DIR"
  git clone --depth=1 $GIT_BRANCH_ARG \
    "https://git.llvm.org/git/$LLVM_PROJECT.git" \
    "$CHECKOUT_DIR"
done

echo "Done"
