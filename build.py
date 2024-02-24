#!/usr/bin/env python3
import argparse
import glob
import lzma
import multiprocessing
import os
import os.path as op
import platform
import shutil
import stat
import subprocess
import sys
import tarfile
import textwrap
import urllib.request
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
    if args.verbose:
        print(str)


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

# Environment checks
if not sys.version_info >= (3, 8):
    error("Requires Python 3.8+")

if "ANDROID_SDK_ROOT" not in os.environ:
    error("Please set Android SDK path to environment variable ANDROID_SDK_ROOT!")

if shutil.which("sccache") is not None:
    os.environ["RUSTC_WRAPPER"] = "sccache"
    os.environ["NDK_CCACHE"] = "sccache"
    os.environ["CARGO_INCREMENTAL"] = "0"
if shutil.which("ccache") is not None:
    os.environ["NDK_CCACHE"] = "ccache"

cpu_count = multiprocessing.cpu_count()
os_name = platform.system().lower()

archs = ["armeabi-v7a", "x86", "arm64-v8a", "x86_64"]
triples = [
    "armv7a-linux-androideabi",
    "i686-linux-android",
    "aarch64-linux-android",
    "x86_64-linux-android",
]
default_targets = ["magisk", "magiskinit", "magiskboot", "magiskpolicy", "busybox"]
support_targets = default_targets + ["resetprop"]
rust_targets = ["magisk", "magiskinit", "magiskboot", "magiskpolicy"]

sdk_path = os.environ["ANDROID_SDK_ROOT"]
ndk_root = op.join(sdk_path, "ndk")
ndk_path = op.join(ndk_root, "magisk")
ndk_build = op.join(ndk_path, "ndk-build")
rust_bin = op.join(ndk_path, "toolchains", "rust", "bin")
llvm_bin = op.join(
    ndk_path, "toolchains", "llvm", "prebuilt", f"{os_name}-x86_64", "bin"
)
cargo = op.join(rust_bin, "cargo" + EXE_EXT)
gradlew = op.join(".", "gradlew" + (".bat" if is_windows else ""))
adb_path = op.join(sdk_path, "platform-tools", "adb" + EXE_EXT)
native_gen_path = op.realpath(op.join("native", "out", "generated"))

# Global vars
config = {}
STDOUT = None
build_tools = None


def mv(source, target):
    try:
        shutil.move(source, target)
        vprint(f"mv {source} -> {target}")
    except:
        pass


def cp(source, target):
    try:
        shutil.copyfile(source, target)
        vprint(f"cp {source} -> {target}")
    except:
        pass


def rm(file):
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


def rm_rf(path):
    vprint(f"rm -rf {path}")
    shutil.rmtree(path, ignore_errors=False, onerror=rm_on_error)


def mkdir(path, mode=0o755):
    try:
        os.mkdir(path, mode)
    except:
        pass


def mkdir_p(path, mode=0o755):
    os.makedirs(path, mode, exist_ok=True)


def execv(cmd, env=None):
    return subprocess.run(cmd, stdout=STDOUT, env=env)


def system(cmd):
    return subprocess.run(cmd, shell=True, stdout=STDOUT)


def cmd_out(cmd, env=None):
    return (
        subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, env=env)
        .stdout.strip()
        .decode("utf-8")
    )


def xz(data):
    return lzma.compress(data, preset=9, check=lzma.CHECK_NONE)


def parse_props(file):
    props = {}
    with open(file, "r") as f:
        for line in [l.strip(" \t\r\n") for l in f]:
            if line.startswith("#") or len(line) == 0:
                continue
            prop = line.split("=")
            if len(prop) != 2:
                continue
            value = prop[1].strip(" \t\r\n")
            if len(value) == 0:
                continue
            props[prop[0].strip(" \t\r\n")] = value
    return props


def load_config(args):
    commit_hash = cmd_out(["git", "rev-parse", "--short=8", "HEAD"])

    # Default values
    config["version"] = commit_hash
    config["versionCode"] = 1000000
    config["outdir"] = "out"

    # Load prop files
    if op.exists(args.config):
        config.update(parse_props(args.config))

    if op.exists("gradle.properties"):
        for key, value in parse_props("gradle.properties").items():
            if key.startswith("magisk."):
                config[key[7:]] = value

    try:
        config["versionCode"] = int(config["versionCode"])
    except ValueError:
        error('Config error: "versionCode" is required to be an integer')

    mkdir_p(config["outdir"])
    global STDOUT
    STDOUT = None if args.verbose else subprocess.DEVNULL


def clean_elf():
    if is_windows:
        elf_cleaner = op.join("tools", "elf-cleaner.exe")
    else:
        elf_cleaner = op.join("native", "out", "elf-cleaner")
        if not op.exists(elf_cleaner):
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
    args = [elf_cleaner, "--api-level", "23"]
    args.extend(
        op.join("native", "out", arch, bin)
        for arch in archs
        for bin in ["magisk", "magiskpolicy"]
    )
    execv(args)


def run_ndk_build(flags):
    os.chdir("native")
    flags = "NDK_PROJECT_PATH=. NDK_APPLICATION_MK=src/Application.mk " + flags
    proc = system(f"{ndk_build} {flags} -j{cpu_count}")
    if proc.returncode != 0:
        error("Build binary failed!")
    os.chdir("..")
    for arch in archs:
        for tgt in support_targets + ["libinit-ld.so"]:
            source = op.join("native", "libs", arch, tgt)
            target = op.join("native", "out", arch, tgt)
            mv(source, target)


def run_cargo(cmds, triple="aarch64-linux-android"):
    env = os.environ.copy()
    env["PATH"] = f'{rust_bin}{os.pathsep}{env["PATH"]}'
    env["CARGO_BUILD_RUSTC"] = op.join(rust_bin, "rustc" + EXE_EXT)
    env["RUSTFLAGS"] = f"-Clinker-plugin-lto -Zthreads={min(8, cpu_count)}"
    env["TARGET_CC"] = op.join(llvm_bin, "clang" + EXE_EXT)
    env["TARGET_CFLAGS"] = f"--target={triple}23"
    return execv([cargo, *cmds], env)


def run_cargo_build(args):
    native_out = op.join("..", "out")
    mkdir(native_out)

    targets = set(args.target) & set(rust_targets)
    if "resetprop" in args.target:
        targets.add("magisk")

    if len(targets) == 0:
        return

    # Start building the actual build commands
    cmds = ["build", "-p", ""]
    rust_out = "debug"
    if args.release:
        cmds.append("-r")
        rust_out = "release"
    if not args.verbose:
        cmds.append("-q")

    cmds.append("--target")
    cmds.append("")

    for arch, triple in zip(archs, triples):
        rust_triple = (
            "thumbv7neon-linux-androideabi" if triple.startswith("armv7") else triple
        )
        cmds[-1] = rust_triple

        for target in targets:
            cmds[2] = target
            proc = run_cargo(cmds, triple)
            if proc.returncode != 0:
                error("Build binary failed!")

        arch_out = op.join(native_out, arch)
        mkdir(arch_out)
        for tgt in targets:
            source = op.join("target", rust_triple, rust_out, f"lib{tgt}.a")
            target = op.join(arch_out, f"lib{tgt}-rs.a")
            mv(source, target)


def run_cargo_cmd(args):
    global STDOUT
    STDOUT = None
    if len(args.commands) >= 1 and args.commands[0] == "--":
        args.commands = args.commands[1:]
    os.chdir(op.join("native", "src"))
    run_cargo(args.commands)
    os.chdir(op.join("..", ".."))


def write_if_diff(file_name, text):
    do_write = True
    if op.exists(file_name):
        with open(file_name, "r") as f:
            orig = f.read()
        do_write = orig != text
    if do_write:
        with open(file_name, "w") as f:
            f.write(text)


def binary_dump(src, var_name, compressor=xz):
    out_str = f"constexpr unsigned char {var_name}[] = {{"
    for i, c in enumerate(compressor(src.read())):
        if i % 16 == 0:
            out_str += "\n"
        out_str += f"0x{c:02X},"
    out_str += "\n};\n"
    return out_str


def dump_bin_header(args):
    mkdir_p(native_gen_path)
    for arch in archs:
        preload = op.join("native", "out", arch, "libinit-ld.so")
        with open(preload, "rb") as src:
            text = binary_dump(src, "init_ld_xz")
        write_if_diff(op.join(native_gen_path, f"{arch}_binaries.h"), text)


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

    mkdir_p(native_gen_path)
    write_if_diff(op.join(native_gen_path, "flags.h"), flag_txt)


def build_binary(args):
    # Verify NDK install
    try:
        with open(op.join(ndk_path, "ONDK_VERSION"), "r") as ondk_ver:
            assert ondk_ver.read().strip(" \t\r\n") == config["ondkVersion"]
    except:
        error('Unmatched NDK. Please install/upgrade NDK with "build.py ndk"')

    if "target" not in vars(args):
        vars(args)["target"] = []

    if args.target:
        args.target = set(args.target) & set(support_targets)
        if not args.target:
            return
    else:
        args.target = default_targets

    header("* Building binaries: " + " ".join(args.target))

    os.chdir(op.join("native", "src"))
    run_cargo_build(args)
    os.chdir(op.join("..", ".."))

    dump_flag_header()

    flag = ""
    clean = False

    if "magisk" in args.target:
        flag += " B_MAGISK=1"
        clean = True

    if "magiskpolicy" in args.target:
        flag += " B_POLICY=1"
        clean = True

    if "test" in args.target:
        flag += " B_TEST=1"

    if "magiskinit" in args.target:
        flag += " B_PRELOAD=1"

    if "resetprop" in args.target:
        flag += " B_PROP=1"

    if "magiskboot" in args.target:
        flag += " B_BOOT=1"

    if flag:
        run_ndk_build(flag)

    # magiskinit embeds preload.so

    flag = ""

    if "magiskinit" in args.target:
        flag += " B_INIT=1"

    if flag:
        dump_bin_header(args)
        run_ndk_build(flag)

    if clean:
        clean_elf()

    # BusyBox is built with different libc

    if "busybox" in args.target:
        run_ndk_build("B_BB=1")


def find_jdk():
    env = os.environ.copy()
    if "ANDROID_STUDIO" in env:
        studio = env["ANDROID_STUDIO"]
        jbr = op.join(studio, "jbr", "bin")
        if not op.exists(jbr):
            jbr = op.join(studio, "Contents", "jbr", "Contents", "Home", "bin")
        if op.exists(jbr):
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


def build_apk(args, module):
    env = find_jdk()

    build_type = "Release" if args.release else "Debug"
    proc = execv(
        [
            gradlew,
            f"{module}:assemble{build_type}",
            "-PconfigPath=" + op.abspath(args.config),
        ],
        env=env,
    )
    if proc.returncode != 0:
        error(f"Build {module} failed!")

    build_type = build_type.lower()

    apk = f"{module}-{build_type}.apk"
    source = op.join(module, "build", "outputs", "apk", build_type, apk)
    target = op.join(config["outdir"], apk)
    mv(source, target)
    header("Output: " + target)


def build_app(args):
    header("* Building the Magisk app")
    build_apk(args, "app")

    # Stub building is directly integrated into the main app
    # build process. Copy the stub APK into output directory.
    build_type = "release" if args.release else "debug"
    apk = f"stub-{build_type}.apk"
    source = op.join("app", "src", build_type, "assets", "stub.apk")
    target = op.join(config["outdir"], apk)
    cp(source, target)


def build_stub(args):
    header("* Building the stub app")
    build_apk(args, "stub")


def cleanup(args):
    support_targets = {"native", "cpp", "rust", "java"}
    if args.target:
        args.target = set(args.target) & support_targets
        if "native" in args.target:
            args.target.add("cpp")
            args.target.add("rust")
    else:
        args.target = support_targets

    if "cpp" in args.target:
        header("* Cleaning C++")
        rm_rf(op.join("native", "libs"))
        rm_rf(op.join("native", "obj"))
        rm_rf(op.join("native", "out"))

    if "rust" in args.target:
        header("* Cleaning Rust")
        rm_rf(op.join("native", "src", "target"))
        rm(op.join("native", "src", "boot", "proto", "mod.rs"))
        rm(op.join("native", "src", "boot", "proto", "update_metadata.rs"))
        for rs_gen in glob.glob("native/**/*-rs.*pp", recursive=True):
            rm(rs_gen)

    if "java" in args.target:
        header("* Cleaning java")
        execv([gradlew, "app:clean", "app:shared:clean", "stub:clean"], env=find_jdk())
        rm_rf(op.join("app", "src", "debug"))
        rm_rf(op.join("app", "src", "release"))


def setup_ndk(args):
    ndk_ver = config["ondkVersion"]
    url = f"https://github.com/topjohnwu/ondk/releases/download/{ndk_ver}/ondk-{ndk_ver}-{os_name}.tar.xz"
    ndk_archive = url.split("/")[-1]
    ondk_path = op.join(ndk_root, f"ondk-{ndk_ver}")

    header(f"* Downloading and extracting {ndk_archive}")
    rm_rf(ondk_path)
    with urllib.request.urlopen(url) as response:
        with tarfile.open(mode="r|xz", fileobj=response) as tar:
            tar.extractall(ndk_root)

    rm_rf(ndk_path)
    mv(ondk_path, ndk_path)

    header("* Patching static libs")
    for target in ["arm-linux-androideabi", "i686-linux-android"]:
        arch = target.split("-")[0]
        lib_dir = op.join(
            ndk_path,
            "toolchains",
            "llvm",
            "prebuilt",
            f"{os_name}-x86_64",
            "sysroot",
            "usr",
            "lib",
            f"{target}",
            "23",
        )
        if not op.exists(lib_dir):
            continue
        src_dir = op.join("tools", "ndk-bins", arch)
        rm(op.join(src_dir, ".DS_Store"))
        shutil.copytree(src_dir, lib_dir, copy_function=cp, dirs_exist_ok=True)


def push_files(args, script):
    abi = cmd_out([adb_path, "shell", "getprop", "ro.product.cpu.abi"])
    apk = config["outdir"] + ("/app-release.apk" if args.release else "/app-debug.apk")

    # Extract busybox from APK
    busybox = f'{config["outdir"]}/busybox'
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


def setup_avd(args):
    if not args.skip:
        build_all(args)

    header("* Setting up emulator")

    push_files(args, "scripts/avd_magisk.sh")

    proc = execv([adb_path, "shell", "sh", "/data/local/tmp/avd_magisk.sh"])
    if proc.returncode != 0:
        error("avd_magisk.sh failed!")


def patch_avd_ramdisk(args):
    if not args.skip:
        args.release = False
        build_all(args)

    header("* Patching emulator ramdisk.img")

    # Create a backup to prevent accidental overwrites
    backup = args.ramdisk + ".bak"
    if not op.exists(backup):
        cp(args.ramdisk, backup)

    ini = op.join(op.dirname(args.ramdisk), "advancedFeatures.ini")
    with open(ini, "r") as f:
        adv_ft = f.read()

    # Need to turn off system as root
    if "SystemAsRoot = on" in adv_ft:
        # Create a backup
        cp(ini, ini + ".bak")
        adv_ft = adv_ft.replace("SystemAsRoot = on", "SystemAsRoot = off")
        with open(ini, "w") as f:
            f.write(adv_ft)

    push_files(args, "scripts/avd_patch.sh")

    proc = execv([adb_path, "push", backup, "/data/local/tmp/ramdisk.cpio.tmp"])
    if proc.returncode != 0:
        error("adb push failed!")

    proc = execv([adb_path, "shell", "sh", "/data/local/tmp/avd_patch.sh"])
    if proc.returncode != 0:
        error("avd_patch.sh failed!")

    proc = execv([adb_path, "pull", "/data/local/tmp/ramdisk.cpio.gz", args.ramdisk])
    if proc.returncode != 0:
        error("adb pull failed!")


def build_all(args):
    build_binary(args)
    build_app(args)


parser = argparse.ArgumentParser(description="Magisk build script")
parser.set_defaults(func=lambda x: None)
parser.add_argument(
    "-r", "--release", action="store_true", help="compile in release mode"
)
parser.add_argument("-v", "--verbose", action="store_true", help="verbose output")
parser.add_argument(
    "-c",
    "--config",
    default="config.prop",
    help="custom config file (default: config.prop)",
)
subparsers = parser.add_subparsers(title="actions")

all_parser = subparsers.add_parser("all", help="build everything")
all_parser.set_defaults(func=build_all)

binary_parser = subparsers.add_parser("binary", help="build binaries")
binary_parser.add_argument(
    "target",
    nargs="*",
    help=f"{', '.join(support_targets)}, \
    or empty for defaults ({', '.join(default_targets)})",
)
binary_parser.set_defaults(func=build_binary)

cargo_parser = subparsers.add_parser("cargo", help="run cargo with proper environment")
cargo_parser.add_argument("commands", nargs=argparse.REMAINDER)
cargo_parser.set_defaults(func=run_cargo_cmd)

app_parser = subparsers.add_parser("app", help="build the Magisk app")
app_parser.set_defaults(func=build_app)

stub_parser = subparsers.add_parser("stub", help="build the stub app")
stub_parser.set_defaults(func=build_stub)

avd_parser = subparsers.add_parser("emulator", help="setup AVD for development")
avd_parser.add_argument(
    "-s", "--skip", action="store_true", help="skip building binaries and the app"
)
avd_parser.set_defaults(func=setup_avd)

avd_patch_parser = subparsers.add_parser("avd_patch", help="patch AVD ramdisk.img")
avd_patch_parser.add_argument("ramdisk", help="path to ramdisk.img")
avd_patch_parser.add_argument(
    "-s", "--skip", action="store_true", help="skip building binaries and the app"
)
avd_patch_parser.set_defaults(func=patch_avd_ramdisk)

clean_parser = subparsers.add_parser("clean", help="cleanup")
clean_parser.add_argument(
    "target", nargs="*", help="native, cpp, rust, java, or empty to clean all"
)
clean_parser.set_defaults(func=cleanup)

ndk_parser = subparsers.add_parser("ndk", help="setup Magisk NDK")
ndk_parser.set_defaults(func=setup_ndk)

if len(sys.argv) == 1:
    parser.print_help()
    sys.exit(1)

args = parser.parse_args()
load_config(args)

# Call corresponding functions
args.func(args)
