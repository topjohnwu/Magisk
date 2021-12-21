#===- libcxx/utils/docker/debian9/Dockerfile -------------------------===//
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===//

# Setup the base builder image with the packages we'll need to build GCC and Clang from source.
FROM launcher.gcr.io/google/debian9:latest as builder-base
LABEL maintainer "libc++ Developers"

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      ca-certificates \
      gnupg \
      build-essential \
      wget \
      subversion \
      unzip \
      automake \
      python \
      cmake \
      ninja-build \
      curl \
      git \
      gcc-multilib \
      g++-multilib \
      libc6-dev \
      bison \
      flex \
      libtool \
      autoconf \
      binutils-dev \
      binutils-gold \
      software-properties-common && \
  update-alternatives --install "/usr/bin/ld" "ld" "/usr/bin/ld.gold" 20 && \
  update-alternatives --install "/usr/bin/ld" "ld" "/usr/bin/ld.bfd" 10

# Build GCC 4.9 for testing our C++11 against
FROM builder-base as gcc-49-builder
LABEL maintainer "libc++ Developers"

ADD scripts/build_gcc.sh /tmp/build_gcc.sh

RUN git clone --depth=1 --branch gcc-4_9_4-release git://gcc.gnu.org/git/gcc.git /tmp/gcc-4.9.4
RUN cd /tmp/gcc-4.9.4/ && ./contrib/download_prerequisites
RUN /tmp/build_gcc.sh --source /tmp/gcc-4.9.4 --to /opt/gcc-4.9.4

# Build GCC ToT for testing in all dialects.
FROM builder-base as gcc-tot-builder
LABEL maintainer "libc++ Developers"

ADD scripts/build_gcc.sh /tmp/build_gcc.sh

RUN git clone --depth=1 git://gcc.gnu.org/git/gcc.git /tmp/gcc-tot
RUN cd /tmp/gcc-tot && ./contrib/download_prerequisites
RUN /tmp/build_gcc.sh --source /tmp/gcc-tot --to /opt/gcc-tot

# Build LLVM 4.0 which is used to test against a "legacy" compiler.
FROM builder-base as llvm-4-builder
LABEL maintainer "libc++ Developers"

ADD scripts/checkout_git.sh /tmp/checkout_git.sh
ADD scripts/build_install_llvm.sh /tmp/build_install_llvm.sh

RUN /tmp/checkout_git.sh --to /tmp/llvm-4.0 -p clang -p compiler-rt --branch release_40
RUN /tmp/build_install_llvm.sh \
    --install /opt/llvm-4.0 \
    --source /tmp/llvm-4.0 \
    --build /tmp/build-llvm-4.0 \
    -i install-clang -i install-clang-headers \
    -i install-compiler-rt \
    -- \
    -DCMAKE_BUILD_TYPE=RELEASE \
    -DLLVM_ENABLE_ASSERTIONS=ON

# Stage 2. Produce a minimal release image with build results.
FROM launcher.gcr.io/google/debian9:latest
LABEL maintainer "libc++ Developers"

# Copy over the GCC and Clang installations
COPY --from=gcc-49-builder /opt/gcc-4.9.4 /opt/gcc-4.9.4
COPY --from=gcc-tot-builder /opt/gcc-tot /opt/gcc-tot
COPY --from=llvm-4-builder /opt/llvm-4.0 /opt/llvm-4.0

RUN ln -s /opt/gcc-4.9.4/bin/gcc /usr/local/bin/gcc-4.9 && \
    ln -s /opt/gcc-4.9.4/bin/g++ /usr/local/bin/g++-4.9

RUN apt-get update && \
    apt-get install -y \
      ca-certificates \
      gnupg \
      build-essential \
      apt-transport-https \
      curl \
      software-properties-common

RUN apt-get install -y --no-install-recommends \
    systemd \
    sysvinit-utils \
    cmake \
    subversion \
    git \
    ninja-build \
    gcc-multilib \
    g++-multilib \
    python \
    buildbot-slave

ADD scripts/install_clang_packages.sh /tmp/install_clang_packages.sh
RUN /tmp/install_clang_packages.sh && rm /tmp/install_clang_packages.sh

RUN git clone https://git.llvm.org/git/libcxx.git /libcxx
