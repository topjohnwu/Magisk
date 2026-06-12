#!/usr/bin/env python3
import functools
import multiprocessing
import os
import platform
import sys
import shutil
import subprocess
from pathlib import Path
from typing import NoReturn


ondk_version = "r30.0"


def color_print(code, str):
    if no_color:
        print(str)
    else:
        str = str.replace("\n", f"\033[0m\n{code}")
        print(f"{code}{str}\033[0m")


def error(str) -> NoReturn:
    color_print("\033[41;39m", f"\n! {str}\n")
    sys.exit(1)


def header(str):
    color_print("\033[44;39m", f"\n{str}\n")


class Paths:
    def __init__(self):
        try:
            self.sdk = Path(os.environ["ANDROID_HOME"])
        except KeyError:
            try:
                self.sdk = Path(os.environ["ANDROID_SDK_ROOT"])
            except KeyError:
                error(
                    "Please set Android SDK path to environment variable ANDROID_HOME"
                )

        self.ndk = self.sdk / "ndk" / "magisk"
        self.ndk_build = self.ndk / "ndk-build"
        self.rust_sysroot = self.ndk / "toolchains" / "rust"
        self.adb = self.sdk / "platform-tools" / "adb"
        project_root = Path(__file__).resolve().parent.parent
        self.gradlew = project_root / "app" / "gradlew"


@functools.cache
def paths() -> Paths:
    return Paths()


def run_once(func):
    def wrapper(*args, **kwargs):
        if not wrapper.has_run:
            wrapper.has_run = True
            return func(*args, **kwargs)

    wrapper.has_run = False
    return wrapper


@run_once
def ensure_toolchain():
    # Verify NDK install
    try:
        with open(paths().ndk / "ONDK_VERSION", "r") as ondk_ver:
            assert ondk_ver.read().strip(" \t\r\n") == ondk_version
    except:
        error('Unmatched NDK. Please install/upgrade NDK with "build.py ndk"')

    if sccache := shutil.which("sccache"):
        os.environ["RUSTC_WRAPPER"] = sccache
        os.environ["NDK_CCACHE"] = sccache
        os.environ["CARGO_INCREMENTAL"] = "0"
    if ccache := shutil.which("ccache"):
        os.environ["NDK_CCACHE"] = ccache


@run_once
def ensure_cargo():
    ensure_toolchain()

    os.environ["CARGO_BUILD_RUSTFLAGS"] = f"-Z threads={min(8, cpu_count)}"
    if shutil.which("rustup"):
        # Go through rustup proxies by default if available
        os.environ["RUSTUP_TOOLCHAIN"] = str(paths().rust_sysroot)
    else:
        os.environ["PATH"] = (
            f"{paths().rust_sysroot / "bin"}{os.pathsep}{os.environ["PATH"]}"
        )
        # Cargo calls executables in $RUSTROOT/lib/rustlib/$TRIPLE/bin, we need
        # to make sure the runtime linker also search $RUSTROOT/lib for libraries.
        # This is only required on Unix, as Windows search dlls from PATH.
        if os_name == "darwin":
            os.environ["DYLD_FALLBACK_LIBRARY_PATH"] = str(paths().rust_sysroot / "lib")
        elif os_name == "linux":
            os.environ["LD_LIBRARY_PATH"] = str(paths().rust_sysroot / "lib")


@run_once
def ensure_jdk():
    if "ANDROID_STUDIO" in os.environ:
        studio = os.environ["ANDROID_STUDIO"]
        jbr = Path(studio, "jbr")
        if not jbr.exists():
            jbr = Path(studio, "Contents", "jbr", "Contents", "Home")
        if jbr.exists():
            os.environ["PATH"] = f'{jbr / "bin"}{os.pathsep}{os.environ["PATH"]}'
            os.environ["JAVA_HOME"] = str(jbr)
            os.environ["JDK_HOME"] = str(jbr)

    no_jdk = False
    try:
        proc = subprocess.run(
            "javac -version",
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            shell=True,
        )
        output = proc.stdout.strip().decode("utf-8")
        no_jdk = proc.returncode != 0 and output.startswith("javac 21")
    except FileNotFoundError:
        no_jdk = True

    if no_jdk:
        error(
            "Please set Android Studio's path to environment variable ANDROID_STUDIO,\n"
            + "or install JDK 21 and make sure 'javac' is available in PATH"
        )


# OS detection
os_name = platform.system().lower()
is_windows = False
if os_name != "linux" and os_name != "darwin":
    # It's possible we're using MSYS/Cygwin/MinGW, treat them all as Windows
    is_windows = True
    os_name = "windows"
EXE_EXT = ".exe" if is_windows else ""

no_color = False
if is_windows:
    try:
        import colorama

        colorama.init()
    except ImportError:
        # We can't do ANSI color codes in terminal on Windows without colorama
        no_color = True

# Environment detection
if not sys.version_info >= (3, 8):
    error("Requires Python 3.8+")

cpu_count = multiprocessing.cpu_count()

# When directly invoked, make it a command wrapper
if __name__ == "__main__":
    ensure_cargo()
    ensure_jdk()
    subprocess.run(sys.argv[1:])
