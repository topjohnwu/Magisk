#!/usr/bin/env python3
import argparse
import glob
import lzma
import multiprocessing
import os
import platform
import re
import shutil
import stat
import subprocess
import sys
import tarfile
import textwrap
import urllib.request
from pathlib import Path
from zipfile import ZipFile


def color_print(code, str):
    if no_color:
        print(str)
    else:
        str = str.replace("\n", f"\033[0m\n{code}")
        print(f"{code}{str}\033[0m")


def error(str):
    color_print("\033[41;39m", f"\n! {str}\n")
    sys.exit(1)


def header(str):
    color_print("\033[44;39m", f"\n{str}\n")


def vprint(str):
    if args.verbose > 0:
        print(str)


# Environment checks and detection
is_windows = os.name == "nt"
EXE_EXT = ".exe" if is_windows else ""

no_color = False
if is_windows:
    try:
        import colorama

        colorama.init()
    except ImportError:
        # We can't do ANSI color codes in terminal on Windows without colorama
        no_color = True

if not sys.version_info >= (3, 8):
    error("Requires Python 3.8+")

try:
    sdk_path = Path(os.environ["ANDROID_HOME"])
except KeyError:
    try:
        sdk_path = Path(os.environ["ANDROID_SDK_ROOT"])
    except KeyError:
        error("Please set Android SDK path to environment variable ANDROID_HOME")

cpu_count = multiprocessing.cpu_count()
os_name = platform.system().lower()

# Common constants
support_abis = {
    "armeabi-v7a": "thumbv7neon-linux-androideabi",
    "x86": "i686-linux-android",
    "arm64-v8a": "aarch64-linux-android",
    "x86_64": "x86_64-linux-android",
    "riscv64": "riscv64-linux-android",
}
default_targets = {"magisk", "magiskinit", "magiskboot", "magiskpolicy"}
support_targets = default_targets | {"resetprop"}
rust_targets = {"magisk", "magiskinit", "magiskboot", "magiskpolicy"}

# Common paths
ndk_root = sdk_path / "ndk"
ndk_path = ndk_root / "magisk"
ndk_build = ndk_path / "ndk-build"
rust_bin = ndk_path / "toolchains" / "rust" / "bin"
llvm_bin = ndk_path / "toolchains" / "llvm" / "prebuilt" / f"{os_name}-x86_64" / "bin"
cargo = rust_bin / "cargo"
gradlew = Path.cwd() / "gradlew"
adb_path = sdk_path / "platform-tools" / "adb"
native_gen_path = Path("native", "out", "generated").resolve()

# Global vars
config = {}
args = {}
build_abis = {}

###################
# Helper functions
###################


def mv(source: Path, target: Path):
    try:
        shutil.move(source, target)
        vprint(f"mv {source} -> {target}")
    except:
        pass


def cp(source: Path, target: Path):
    try:
        shutil.copyfile(source, target)
        vprint(f"cp {source} -> {target}")
    except:
        pass


def rm(file: Path):
    try:
        os.remove(file)
        vprint(f"rm {file}")
    except FileNotFoundError as e:
        pass


def rm_on_error(func, path, _):
    # Removing a read-only file on Windows will get "WindowsError: [Error 5] Access is denied"
    # Clear the "read-only" bit and retry
    try:
        os.chmod(path, stat.S_IWRITE)
        os.unlink(path)
    except FileNotFoundError as e:
        pass


def rm_rf(path: Path):
    vprint(f"rm -rf {path}")
    if sys.version_info >= (3, 12):
        shutil.rmtree(path, ignore_errors=False, onexc=rm_on_error)
    else:
        shutil.rmtree(path, ignore_errors=False, onerror=rm_on_error)


def execv(cmds: list, env=None):
    out = None if args.force_out or args.verbose > 0 else subprocess.DEVNULL
    # Use shell on Windows to support PATHEXT
    return subprocess.run(cmds, stdout=out, env=env, shell=is_windows)


def cmd_out(cmds: list):
    return (
        subprocess.run(
            cmds,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            shell=is_windows,
        )
        .stdout.strip()
        .decode("utf-8")
    )


def xz(data):
    return lzma.compress(data, preset=9, check=lzma.CHECK_NONE)


###############
# Build Native
###############


def clean_elf():
    if is_windows:
        elf_cleaner = Path("tools", "elf-cleaner.exe")
    else:
        elf_cleaner = Path("native", "out", "elf-cleaner")
        if not elf_cleaner.exists():
            execv(
                [
                    "gcc",
                    '-DPACKAGE_NAME="termux-elf-cleaner"',
                    '-DPACKAGE_VERSION="2.1.1"',
                    '-DCOPYRIGHT="Copyright (C) 2022 Termux."',
                    "tools/termux-elf-cleaner/elf-cleaner.cpp",
                    "tools/termux-elf-cleaner/arghandling.c",
                    "-o",
                    elf_cleaner,
                ]
            )
    cmds = [elf_cleaner, "--api-level", "23"]
    cmds.extend(glob.glob("native/out/*/magisk"))
    cmds.extend(glob.glob("native/out/*/magiskpolicy"))
    execv(cmds)


def run_ndk_build(cmds: list):
    os.chdir("native")
    cmds.append("NDK_PROJECT_PATH=.")
    cmds.append("NDK_APPLICATION_MK=src/Application.mk")
    cmds.append(f"APP_ABI={' '.join(build_abis.keys())}")
    cmds.append(f"-j{cpu_count}")
    if args.verbose > 1:
        cmds.append("V=1")
    if not args.release:
        cmds.append("MAGISK_DEBUG=1")
    proc = execv([ndk_build, *cmds])
    if proc.returncode != 0:
        error("Build binary failed!")
    os.chdir("..")

    for arch in build_abis.keys():
        arch_dir = Path("native", "libs", arch)
        out_dir = Path("native", "out", arch)
        for source in arch_dir.iterdir():
            target = out_dir / source.name
            mv(source, target)


def build_cpp_src(targets: set):
    dump_flag_header()

    cmds = []
    clean = False

    if "magisk" in targets:
        cmds.append("B_MAGISK=1")
        clean = True

    if "magiskpolicy" in targets:
        cmds.append("B_POLICY=1")
        clean = True

    if "magiskinit" in targets:
        cmds.append("B_PRELOAD=1")

    if "resetprop" in targets:
        cmds.append("B_PROP=1")

    if cmds:
        run_ndk_build(cmds)

    cmds.clear()

    if "magiskinit" in targets:
        cmds.append("B_INIT=1")

    if "magiskboot" in targets:
        cmds.append("B_BOOT=1")

    if cmds:
        cmds.append("B_CRT0=1")
        run_ndk_build(cmds)

    if clean:
        clean_elf()


def run_cargo(cmds):
    env = os.environ.copy()
    env["PATH"] = f'{rust_bin}{os.pathsep}{env["PATH"]}'
    env["CARGO_BUILD_RUSTC"] = str(rust_bin / f"rustc{EXE_EXT}")
    env["CARGO_BUILD_RUSTFLAGS"] = f"-Z threads={min(8, cpu_count)}"
    return execv([cargo, *cmds], env)


def build_rust_src(targets: set):
    targets = targets.copy()
    if "resetprop" in targets:
        targets.add("magisk")
    targets = targets & rust_targets
    if not targets:
        return

    os.chdir(Path("native", "src"))

    # Start building the build commands
    cmds = ["build", "-p", ""]
    if args.release:
        cmds.append("-r")
        profile = "release"
    else:
        profile = "debug"
    if args.verbose == 0:
        cmds.append("-q")
    elif args.verbose > 1:
        cmds.append("--verbose")

    for triple in build_abis.values():
        cmds.append("--target")
        cmds.append(triple)

    for tgt in targets:
        cmds[2] = tgt
        proc = run_cargo(cmds)
        if proc.returncode != 0:
            error("Build binary failed!")

    os.chdir(Path("..", ".."))

    native_out = Path("native", "out")
    rust_out = native_out / "rust"
    for arch, triple in build_abis.items():
        arch_out = native_out / arch
        arch_out.mkdir(mode=0o755, exist_ok=True)
        for tgt in targets:
            source = rust_out / triple / profile / f"lib{tgt}.a"
            target = arch_out / f"lib{tgt}-rs.a"
            mv(source, target)


def write_if_diff(file_name: Path, text: str):
    do_write = True
    if file_name.exists():
        with open(file_name, "r") as f:
            orig = f.read()
        do_write = orig != text
    if do_write:
        with open(file_name, "w") as f:
            f.write(text)


def dump_flag_header():
    flag_txt = textwrap.dedent(
        """\
        #pragma once
        #define quote(s)            #s
        #define str(s)              quote(s)
        #define MAGISK_FULL_VER     MAGISK_VERSION "(" str(MAGISK_VER_CODE) ")"
        #define NAME_WITH_VER(name) str(name) " " MAGISK_FULL_VER
        """
    )
    flag_txt += f'#define MAGISK_VERSION      "{config["version"]}"\n'
    flag_txt += f'#define MAGISK_VER_CODE     {config["versionCode"]}\n'
    flag_txt += f"#define MAGISK_DEBUG        {0 if args.release else 1}\n"

    native_gen_path.mkdir(mode=0o755, parents=True, exist_ok=True)
    write_if_diff(Path(native_gen_path, "flags.h"), flag_txt)


def build_native():
    # Verify NDK install
    try:
        with open(Path(ndk_path, "ONDK_VERSION"), "r") as ondk_ver:
            assert ondk_ver.read().strip(" \t\r\n") == config["ondkVersion"]
    except:
        error('Unmatched NDK. Please install/upgrade NDK with "build.py ndk"')

    if "targets" not in vars(args) or not args.targets:
        targets = default_targets
    else:
        targets = set(args.targets) & support_targets
        if not targets:
            return

    header("* Building: " + " ".join(targets))

    if sccache := shutil.which("sccache"):
        os.environ["RUSTC_WRAPPER"] = sccache
        os.environ["NDK_CCACHE"] = sccache
        os.environ["CARGO_INCREMENTAL"] = "0"
    if ccache := shutil.which("ccache"):
        os.environ["NDK_CCACHE"] = ccache

    build_rust_src(targets)
    build_cpp_src(targets)


############
# Build App
############


def find_jdk():
    env = os.environ.copy()
    if "ANDROID_STUDIO" in env:
        studio = env["ANDROID_STUDIO"]
        jbr = Path(studio, "jbr", "bin")
        if not jbr.exists():
            jbr = Path(studio, "Contents", "jbr", "Contents", "Home", "bin")
        if jbr.exists():
            env["PATH"] = f'{jbr}{os.pathsep}{env["PATH"]}'

    no_jdk = False
    try:
        proc = subprocess.run(
            "javac -version",
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            env=env,
            shell=True,
        )
        no_jdk = proc.returncode != 0
    except FileNotFoundError:
        no_jdk = True

    if no_jdk:
        error(
            "Please set Android Studio's path to environment variable ANDROID_STUDIO,\n"
            + "or install JDK 17 and make sure 'javac' is available in PATH"
        )

    return env


def build_apk(module: str):
    env = find_jdk()

    build_type = "Release" if args.release else "Debug"
    proc = execv(
        [
            gradlew,
            f"{module}:assemble{build_type}",
            f"-PconfigPath={args.config.resolve()}",
        ],
        env=env,
    )
    if proc.returncode != 0:
        error(f"Build {module} failed!")

    build_type = build_type.lower()

    paths = module.split(":")

    apk = f"{paths[-1]}-{build_type}.apk"
    source = Path(*paths, "build", "outputs", "apk", build_type, apk)
    target = config["outdir"] / apk
    mv(source, target)
    header(f"Output: {target}")


def build_app():
    header("* Building the Magisk app")
    build_apk(":app:apk")

    build_type = "release" if args.release else "debug"

    # Rename apk-variant.apk to app-variant.apk
    source = config["outdir"] / f"apk-{build_type}.apk"
    target = config["outdir"] / f"app-{build_type}.apk"
    mv(source, target)

    # Stub building is directly integrated into the main app
    # build process. Copy the stub APK into output directory.
    source = Path("app", "core", "src", build_type, "assets", "stub.apk")
    target = config["outdir"] / f"stub-{build_type}.apk"
    cp(source, target)


def build_stub():
    header("* Building the stub app")
    build_apk(":app:stub")


################
# Build General
################


def cleanup():
    support_targets = {"native", "cpp", "rust", "app"}
    if args.targets:
        targets = set(args.targets) & support_targets
        if "native" in targets:
            targets.add("cpp")
            targets.add("rust")
    else:
        targets = support_targets

    if "cpp" in targets:
        header("* Cleaning C++")
        rm_rf(Path("native", "libs"))
        rm_rf(Path("native", "obj"))

    if "rust" in targets:
        header("* Cleaning Rust")
        rm_rf(Path("native", "src", "target"))
        rm(Path("native", "src", "boot", "proto", "mod.rs"))
        rm(Path("native", "src", "boot", "proto", "update_metadata.rs"))
        for rs_gen in glob.glob("native/**/*-rs.*pp", recursive=True):
            rm(rs_gen)

    if "native" in targets:
        rm_rf(Path("native", "out"))

    if "app" in targets:
        header("* Cleaning app")
        execv([gradlew, ":app:clean"], env=find_jdk())


def build_all():
    build_native()
    build_app()


############
# Utilities
############


def cargo_cli():
    args.force_out = True
    if len(args.commands) >= 1 and args.commands[0] == "--":
        args.commands = args.commands[1:]
    os.chdir(Path("native", "src"))
    run_cargo(args.commands)
    os.chdir(Path("..", ".."))


def setup_ndk():
    ndk_ver = config["ondkVersion"]
    url = f"https://github.com/topjohnwu/ondk/releases/download/{ndk_ver}/ondk-{ndk_ver}-{os_name}.tar.xz"
    ndk_archive = url.split("/")[-1]
    ondk_path = Path(ndk_root, f"ondk-{ndk_ver}")

    header(f"* Downloading and extracting {ndk_archive}")
    rm_rf(ondk_path)
    with urllib.request.urlopen(url) as response:
        with tarfile.open(mode="r|xz", fileobj=response) as tar:
            if hasattr(tarfile, "data_filter"):
                tar.extractall(ndk_root, filter="tar")
            else:
                tar.extractall(ndk_root)

    rm_rf(ndk_path)
    mv(ondk_path, ndk_path)


def push_files(script):
    abi = cmd_out([adb_path, "shell", "getprop", "ro.product.cpu.abi"])
    if not abi:
        error("Cannot detect emulator ABI")

    apk = Path(
        config["outdir"], ("app-release.apk" if args.release else "app-debug.apk")
    )

    # Extract busybox from APK
    busybox = Path(config["outdir"], "busybox")
    with ZipFile(apk) as zf:
        with zf.open(f"lib/{abi}/libbusybox.so") as libbb:
            with open(busybox, "wb") as bb:
                bb.write(libbb.read())

    try:
        proc = execv([adb_path, "push", busybox, script, "/data/local/tmp"])
        if proc.returncode != 0:
            error("adb push failed!")
    finally:
        rm_rf(busybox)

    proc = execv([adb_path, "push", apk, "/data/local/tmp/magisk.apk"])
    if proc.returncode != 0:
        error("adb push failed!")


def setup_avd():
    if not args.skip:
        build_all()

    header("* Setting up emulator")

    push_files(Path("scripts", "avd_magisk.sh"))

    proc = execv([adb_path, "shell", "sh", "/data/local/tmp/avd_magisk.sh"])
    if proc.returncode != 0:
        error("avd_magisk.sh failed!")


def patch_avd_file():
    if not args.skip:
        build_all()

    input = Path(args.image)
    if args.output:
        output = Path(args.output)
    else:
        output = input.parent / f"{input.name}.magisk"

    src_file = f"/data/local/tmp/{input.name}"
    out_file = f"{src_file}.magisk"

    header(f"* Patching {input.name}")

    push_files(Path("scripts", "avd_patch.sh"))

    proc = execv([adb_path, "push", input, "/data/local/tmp"])
    if proc.returncode != 0:
        error("adb push failed!")

    proc = execv([adb_path, "shell", "sh", "/data/local/tmp/avd_patch.sh", src_file])
    if proc.returncode != 0:
        error("avd_patch.sh failed!")

    proc = execv([adb_path, "pull", out_file, output])
    if proc.returncode != 0:
        error("adb pull failed!")

    header(f"Output: {output}")


def setup_rustup():
    wrapper_dir = Path(args.wrapper_dir)
    rm_rf(wrapper_dir)
    wrapper_dir.mkdir(mode=0o755, parents=True, exist_ok=True)
    if "CARGO_HOME" in os.environ:
        cargo_home = Path(os.environ["CARGO_HOME"])
    else:
        cargo_home = Path.home() / ".cargo"
    cargo_bin = cargo_home / "bin"
    for src in cargo_bin.iterdir():
        tgt = wrapper_dir / src.name
        tgt.symlink_to(src)

    # Build rustup_wrapper
    wrapper_src = Path("tools", "rustup_wrapper")
    cargo_toml = wrapper_src / "Cargo.toml"
    cmds = ["build", "--release", f"--manifest-path={cargo_toml}"]
    if args.verbose > 1:
        cmds.append("--verbose")
    run_cargo(cmds)

    # Replace rustup with wrapper
    wrapper = wrapper_dir / (f"rustup{EXE_EXT}")
    wrapper.unlink(missing_ok=True)
    cp(wrapper_src / "target" / "release" / (f"rustup_wrapper{EXE_EXT}"), wrapper)
    wrapper.chmod(0o755)


##################
# Config and args
##################


def parse_props(file):
    props = {}
    with open(file, "r") as f:
        for line in [l.strip(" \t\r\n") for l in f]:
            if line.startswith("#") or len(line) == 0:
                continue
            prop = line.split("=")
            if len(prop) != 2:
                continue
            key = prop[0].strip(" \t\r\n")
            value = prop[1].strip(" \t\r\n")
            if not key or not value:
                continue
            props[key] = value
    return props


def load_config():
    commit_hash = cmd_out(["git", "rev-parse", "--short=8", "HEAD"])

    # Default values
    config["version"] = commit_hash
    config["versionCode"] = 1000000
    config["outdir"] = "out"

    args.config = Path(args.config)

    # Load prop files
    if args.config.exists():
        config.update(parse_props(args.config))

    if Path("gradle.properties").exists():
        for key, value in parse_props("gradle.properties").items():
            if key.startswith("magisk."):
                config[key[7:]] = value

    try:
        config["versionCode"] = int(config["versionCode"])
    except ValueError:
        error('Config error: "versionCode" is required to be an integer')

    config["outdir"] = Path(config["outdir"])
    config["outdir"].mkdir(mode=0o755, parents=True, exist_ok=True)

    if "abiList" in config:
        abiList = re.split("\\s*,\\s*", config["abiList"])
        archs = set(abiList) & support_abis.keys()
    else:
        archs = {"armeabi-v7a", "x86", "arm64-v8a", "x86_64"}

    triples = map(support_abis.get, archs)

    global build_abis
    build_abis = dict(zip(archs, triples))


def parse_args():
    parser = argparse.ArgumentParser(description="Magisk build script")
    parser.set_defaults(func=lambda x: None)
    parser.add_argument(
        "-r", "--release", action="store_true", help="compile in release mode"
    )
    parser.add_argument(
        "-v", "--verbose", action="count", default=0, help="verbose output"
    )
    parser.add_argument(
        "-c",
        "--config",
        default="config.prop",
        help="custom config file (default: config.prop)",
    )
    subparsers = parser.add_subparsers(title="actions")

    all_parser = subparsers.add_parser("all", help="build everything")

    native_parser = subparsers.add_parser("native", help="build native binaries")
    native_parser.add_argument(
        "targets",
        nargs="*",
        help=f"{', '.join(support_targets)}, \
        or empty for defaults ({', '.join(default_targets)})",
    )

    app_parser = subparsers.add_parser("app", help="build the Magisk app")

    stub_parser = subparsers.add_parser("stub", help="build the stub app")

    clean_parser = subparsers.add_parser("clean", help="cleanup")
    clean_parser.add_argument(
        "targets", nargs="*", help="native, cpp, rust, java, or empty to clean all"
    )

    ndk_parser = subparsers.add_parser("ndk", help="setup Magisk NDK")

    emu_parser = subparsers.add_parser("emulator", help="setup AVD for development")
    emu_parser.add_argument(
        "-s", "--skip", action="store_true", help="skip building binaries and the app"
    )

    avd_patch_parser = subparsers.add_parser(
        "avd_patch", help="patch AVD ramdisk.img or init_boot.img"
    )
    avd_patch_parser.add_argument("image", help="path to ramdisk.img or init_boot.img")
    avd_patch_parser.add_argument("output", help="optional output file name", nargs="?")
    avd_patch_parser.add_argument(
        "-s", "--skip", action="store_true", help="skip building binaries and the app"
    )

    cargo_parser = subparsers.add_parser(
        "cargo", help="call 'cargo' commands against the project"
    )
    cargo_parser.add_argument("commands", nargs=argparse.REMAINDER)

    rustup_parser = subparsers.add_parser("rustup", help="setup rustup wrapper")
    rustup_parser.add_argument(
        "wrapper_dir", help="path to setup rustup wrapper binaries"
    )

    # Set callbacks
    all_parser.set_defaults(func=build_all)
    native_parser.set_defaults(func=build_native)
    cargo_parser.set_defaults(func=cargo_cli)
    rustup_parser.set_defaults(func=setup_rustup)
    app_parser.set_defaults(func=build_app)
    stub_parser.set_defaults(func=build_stub)
    emu_parser.set_defaults(func=setup_avd)
    avd_patch_parser.set_defaults(func=patch_avd_file)
    clean_parser.set_defaults(func=cleanup)
    ndk_parser.set_defaults(func=setup_ndk)

    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)

    return parser.parse_args()


args = parse_args()
load_config()
vars(args)["force_out"] = False
args.func()
