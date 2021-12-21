#!/bin/bash
#===- libcxx/utils/docker/build_docker_image.sh ----------------------------===//
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===//
set -e

IMAGE_SOURCE=""
DOCKER_REPOSITORY=""
DOCKER_TAG=""

function show_usage() {
  cat << EOF
Usage: build_docker_image.sh [options] [-- [cmake_args]...]

Available options:
  General:
    -h|--help               show this help message
  Docker-specific:
    -s|--source             image source dir (i.e. debian8, nvidia-cuda, etc)
    -d|--docker-repository  docker repository for the image
    -t|--docker-tag         docker tag for the image

Required options: --source and --docker-repository.

For example, running:
$ build_docker_image.sh -s debian9 -d mydocker/debian9-clang -t latest
will produce two docker images:
    mydocker/debian9-clang-build:latest - an intermediate image used to compile
      clang.
    mydocker/clang-debian9:latest       - a small image with preinstalled clang.
Please note that this example produces a not very useful installation, since it
doesn't override CMake defaults, which produces a Debug and non-boostrapped
version of clang.
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help)
      show_usage
      exit 0
      ;;
    -s|--source)
      shift
      IMAGE_SOURCE="$1"
      shift
      ;;
    -d|--docker-repository)
      shift
      DOCKER_REPOSITORY="$1"
      shift
      ;;
    -t|--docker-tag)
      shift
      DOCKER_TAG="$1"
      shift
      ;;
    *)
      echo "Unknown argument $1"
      exit 1
      ;;
  esac
done


command -v docker >/dev/null ||
  {
    echo "Docker binary cannot be found. Please install Docker to use this script."
    exit 1
  }

if [ "$IMAGE_SOURCE" == "" ]; then
  echo "Required argument missing: --source"
  exit 1
fi

if [ "$DOCKER_REPOSITORY" == "" ]; then
  echo "Required argument missing: --docker-repository"
  exit 1
fi

SOURCE_DIR=$(dirname $0)
if [ ! -d "$SOURCE_DIR/$IMAGE_SOURCE" ]; then
  echo "No sources for '$IMAGE_SOURCE' were found in $SOURCE_DIR"
  exit 1
fi

BUILD_DIR=$(mktemp -d)
trap "rm -rf $BUILD_DIR" EXIT
echo "Using a temporary directory for the build: $BUILD_DIR"

cp -r "$SOURCE_DIR/$IMAGE_SOURCE" "$BUILD_DIR/$IMAGE_SOURCE"
cp -r "$SOURCE_DIR/scripts" "$BUILD_DIR/scripts"


if [ "$DOCKER_TAG" != "" ]; then
  DOCKER_TAG=":$DOCKER_TAG"
fi

echo "Building ${DOCKER_REPOSITORY}${DOCKER_TAG} from $IMAGE_SOURCE"
docker build -t "${DOCKER_REPOSITORY}${DOCKER_TAG}" \
  -f "$BUILD_DIR/$IMAGE_SOURCE/Dockerfile" \
  "$BUILD_DIR"
echo "Done"
