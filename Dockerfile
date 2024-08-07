FROM gradle:8.9.0-jdk17-jammy

USER root

# Install Build Essentials
RUN set -eux; \
    apt-get update \
    && apt-get install -y \
        ca-certificates \
        curl \
        gnupg \
        wget \
        tzdata \
        git \
        procps \
        autoconf \
        automake \
        bzip2 \
        g++ \
        gcc \
        make \
        patch \
        unzip \
        xz-utils \
        build-essential \
        file \
        apt-utils \
    ; \
    rm -rf /var/lib/apt/lists/*

########################################################################
## Install Python
########################################################################
RUN set -eux; \
    apt-get update \
    && apt-get install -y --no-install-recommends \
        python3 \
    ; \
    rm -rf /var/lib/apt/lists/*

USER gradle

########################################################################
## Install Rust
########################################################################
RUN set -eux; \
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs -o /tmp/rustup.sh \
    && sh /tmp/rustup.sh -y \
    ;

########################################################################
## Install Android SDK
########################################################################
ENV ANDROID_SDK_ROOT="/home/gradle/android-sdk"
ENV PATH=$PATH:$ANDROID_SDK_ROOT/cmdline-tools/latest:$ANDROID_SDK_ROOT/cmdline-tools/latest/bin:$ANDROID_SDK_ROOT/platform-tools

ENV CMDLINE_TOOLS_URL="https://dl.google.com/android/repository/commandlinetools-linux-6858069_latest.zip"

# Download Android SDK
RUN mkdir "$ANDROID_SDK_ROOT" .android \
    && mkdir -p "$ANDROID_SDK_ROOT/cmdline-tools" \
    && curl -o commandlinetools.zip $CMDLINE_TOOLS_URL \
    && unzip commandlinetools.zip -d "$ANDROID_SDK_ROOT/cmdline-tools" \
    && mv "$ANDROID_SDK_ROOT/cmdline-tools/cmdline-tools" "$ANDROID_SDK_ROOT/cmdline-tools/latest" \
    && rm commandlinetools.zip

# Accept all licenses
RUN yes | sdkmanager --licenses

# Install Android build tools and platform tools
ENV ANDROID_BUILD_TOOLS_VERSION=34.0.0

RUN touch ~/.android/repositories.cfg
RUN sdkmanager --install "build-tools;${ANDROID_BUILD_TOOLS_VERSION}" \
    "platform-tools"
