#!/usr/bin/env python3
import argparse
import copy
import glob
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
default_archs = {"armeabi-v7a", "x86", "arm64-v8a", "x86_64"}
default_targets = {"magisk", "magiskinit", "magiskboot", "magiskpolicy"}
support_targets = default_targets | {"resetprop"}
rust_targets = {"magisk", "magiskinit", "magiskboot", "magiskpolicy"}
ondk_version = "r28.5"

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


###############
# Build Native
###############


def clean_elf():
    cargo_toml = Path("tools", "elf-cleaner", "Cargo.toml")
    cmds = ["run", "--release", "--manifest-path", cargo_toml]
    if args.verbose == 0:
        cmds.append("-q")
    elif args.verbose > 1:
        cmds.append("--verbose")
    cmds.append("--")
    cmds.extend(glob.glob("native/out/*/magisk"))
    cmds.extend(glob.glob("native/out/*/magiskpolicy"))
    run_cargo(cmds)


def collect_ndk_build():
    for arch in build_abis.keys():
        arch_dir = Path("native", "libs", arch)
        out_dir = Path("native", "out", arch)
        for source in arch_dir.iterdir():
            target = out_dir / source.name
            mv(source, target)


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


def build_cpp_src(targets: set):
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
        collect_ndk_build()

    cmds.clear()

    if "magiskinit" in targets:
        cmds.append("B_INIT=1")

    if "magiskboot" in targets:
        cmds.append("B_BOOT=1")

    if cmds:
        cmds.append("B_CRT0=1")
        run_ndk_build(cmds)
        collect_ndk_build()

    if clean:
        clean_elf()


def run_cargo(cmds):
    ensure_paths()
    env = os.environ.copy()
    env["RUSTUP_TOOLCHAIN"] = str(rust_sysroot)
    env["CARGO_BUILD_RUSTFLAGS"] = f"-Z threads={min(8, cpu_count)}"
    return execv(["cargo", *cmds], env)


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
    flag_txt = "#pragma once\n"
    flag_txt += f'#define MAGISK_VERSION      "{config["version"]}"\n'
    flag_txt += f'#define MAGISK_VER_CODE     {config["versionCode"]}\n'
    flag_txt += f"#define MAGISK_DEBUG        {0 if args.release else 1}\n"

    native_gen_path = Path("native", "out", "generated")
    native_gen_path.mkdir(mode=0o755, parents=True, exist_ok=True)
    write_if_diff(native_gen_path / "flags.h", flag_txt)

    rust_flag_txt = f'pub const MAGISK_VERSION: &str = "{config["version"]}";\n'
    rust_flag_txt += f'pub const MAGISK_VER_CODE: i32 = {config["versionCode"]};\n'
    write_if_diff(native_gen_path / "flags.rs", rust_flag_txt)


def ensure_toolchain():
    ensure_paths()

    # Verify NDK install
    try:
        with open(Path(ndk_path, "ONDK_VERSION"), "r") as ondk_ver:
            assert ondk_ver.read().strip(" \t\r\n") == ondk_version
    except:
        error('Unmatched NDK. Please install/upgrade NDK with "build.py ndk"')

    if sccache := shutil.which("sccache"):
        os.environ["RUSTC_WRAPPER"] = sccache
        os.environ["NDK_CCACHE"] = sccache
        os.environ["CARGO_INCREMENTAL"] = "0"
    if ccache := shutil.which("ccache"):
        os.environ["NDK_CCACHE"] = ccache


def build_native():
    ensure_toolchain()

    if "targets" not in vars(args) or not args.targets:
        targets = default_targets
    else:
        targets = set(args.targets) & support_targets
        if not targets:
            return

    header("* Building: " + " ".join(targets))

    dump_flag_header()
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
            + "or install JDK 21 and make sure 'javac' is available in PATH"
        )

    return env


def build_apk(module: str):
    ensure_paths()
    env = find_jdk()
    props = args.config.resolve()

    os.chdir("app")
    build_type = "Release" if args.release else "Debug"
    proc = execv(
        [
            gradlew,
            f"{module}:assemble{build_type}",
            f"-PconfigPath={props}",
        ],
        env=env,
    )
    os.chdir("..")
    if proc.returncode != 0:
        error(f"Build {module} failed!")

    build_type = build_type.lower()

    paths = module.split(":")

    apk = f"{paths[-1]}-{build_type}.apk"
    source = Path("app", *paths, "build", "outputs", "apk", build_type, apk)
    target = config["outdir"] / apk
    mv(source, target)
    return target


def build_app():
    header("* Building the Magisk app")
    apk = build_apk(":apk")

    build_type = "release" if args.release else "debug"

    # Rename apk-variant.apk to app-variant.apk
    source = apk
    target = apk.parent / apk.name.replace("apk-", "app-")
    mv(source, target)
    header(f"Output: {target}")

    # Stub building is directly integrated into the main app
    # build process. Copy the stub APK into output directory.
    source = Path("app", "core", "src", build_type, "assets", "stub.apk")
    target = config["outdir"] / f"stub-{build_type}.apk"
    cp(source, target)


def build_stub():
    header("* Building the stub app")
    apk = build_apk(":stub")
    header(f"Output: {apk}")


def build_test():
    global args
    args_bak = copy.copy(args)
    # Test APK has to be built as release to prevent classname clash
    args.release = True
    try:
        header("* Building the test app")
        source = build_apk(":test")
        target = source.parent / "test.apk"
        mv(source, target)
        header(f"Output: {target}")
    finally:
        args = args_bak


################
# Build General
################


def cleanup():
    ensure_paths()
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
        header("* Cleaning native")
        rm_rf(Path("native", "out"))
        rm_rf(Path("tools", "elf-cleaner", "target"))

    if "app" in targets:
        header("* Cleaning app")
        os.chdir("app")
        execv([gradlew, ":clean"], env=find_jdk())
        os.chdir("..")


def build_all():
    build_native()
    build_app()
    build_test()


############
# Utilities
############


def gen_ide():
    ensure_paths()
    set_archs({args.abi})

    # Dump flags for both C++ and Rust code
    dump_flag_header()

    # Run build.rs to generate Rust/C++ FFI bindings
    os.chdir(Path("native", "src"))
    run_cargo(["check"])
    os.chdir(Path("..", ".."))

    # Generate compilation database
    rm_rf(Path("native", "compile_commands.json"))
    run_ndk_build(
        [
            "B_MAGISK=1",
            "B_INIT=1",
            "B_BOOT=1",
            "B_POLICY=1",
            "B_PRELOAD=1",
            "B_PROP=1",
            "B_CRT0=1",
            "compile_commands.json",
        ]
    )


def clippy_cli():
    ensure_toolchain()
    args.force_out = True
    set_archs(default_archs)

    os.chdir(Path("native", "src"))
    cmds = ["clippy", "--no-deps", "--target"]
    for triple in build_abis.values():
        run_cargo(cmds + [triple])
        run_cargo(cmds + [triple, "--release"])
    os.chdir(Path("..", ".."))


def cargo_cli():
    args.force_out = True
    if len(args.commands) >= 1 and args.commands[0] == "--":
        args.commands = args.commands[1:]
    os.chdir(Path("native", "src"))
    run_cargo(args.commands)
    os.chdir(Path("..", ".."))


def setup_ndk():
    ensure_paths()
    url = f"https://github.com/topjohnwu/ondk/releases/download/{ondk_version}/ondk-{ondk_version}-{os_name}.tar.xz"
    ndk_archive = url.split("/")[-1]
    ondk_path = Path(ndk_root, f"ondk-{ondk_version}")

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
        tgt.symlink_to(f"rustup{EXE_EXT}")

    # Build rustup-wrapper
    wrapper_src = Path("tools", "rustup-wrapper")
    cargo_toml = wrapper_src / "Cargo.toml"
    cmds = ["build", "--release", f"--manifest-path={cargo_toml}"]
    if args.verbose > 1:
        cmds.append("--verbose")
    run_cargo(cmds)

    # Replace rustup with wrapper
    wrapper = wrapper_dir / (f"rustup{EXE_EXT}")
    wrapper.unlink(missing_ok=True)
    cp(wrapper_src / "target" / "release" / (f"rustup-wrapper{EXE_EXT}"), wrapper)
    wrapper.chmod(0o755)


##################
# AVD and testing
##################


def push_files(script):
    if args.build:
        build_all()
    ensure_adb()

    abi = cmd_out([adb_path, "shell", "getprop", "ro.product.cpu.abi"])
    if not abi:
        error("Cannot detect emulator ABI")

    if args.apk:
        apk = Path(args.apk)
    else:
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
    header("* Setting up emulator")

    push_files(Path("scripts", "live_setup.sh"))

    proc = execv([adb_path, "shell", "sh", "/data/local/tmp/live_setup.sh"])
    if proc.returncode != 0:
        error("live_setup.sh failed!")


def patch_avd_file():
    input = Path(args.image)
    output = Path(args.output)

    header(f"* Patching {input.name}")

    push_files(Path("scripts", "host_patch.sh"))

    proc = execv([adb_path, "push", input, "/data/local/tmp"])
    if proc.returncode != 0:
        error("adb push failed!")

    src_file = f"/data/local/tmp/{input.name}"
    out_file = f"{src_file}.magisk"

    proc = execv([adb_path, "shell", "sh", "/data/local/tmp/host_patch.sh", src_file])
    if proc.returncode != 0:
        error("host_patch.sh failed!")

    proc = execv([adb_path, "pull", out_file, output])
    if proc.returncode != 0:
        error("adb pull failed!")

    header(f"Output: {output}")


##########################
# Config, paths, argparse
##########################


def ensure_paths():
    global sdk_path, ndk_root, ndk_path, ndk_build, rust_sysroot
    global llvm_bin, gradlew, adb_path, native_gen_path

    # Skip if already initialized
    if "sdk_path" in globals():
        return

    try:
        sdk_path = Path(os.environ["ANDROID_HOME"])
    except KeyError:
        try:
            sdk_path = Path(os.environ["ANDROID_SDK_ROOT"])
        except KeyError:
            error("Please set Android SDK path to environment variable ANDROID_HOME")

    ndk_root = sdk_path / "ndk"
    ndk_path = ndk_root / "magisk"
    ndk_build = ndk_path / "ndk-build"
    rust_sysroot = ndk_path / "toolchains" / "rust"
    llvm_bin = (
        ndk_path / "toolchains" / "llvm" / "prebuilt" / f"{os_name}-x86_64" / "bin"
    )
    adb_path = sdk_path / "platform-tools" / "adb"
    gradlew = Path.cwd() / "app" / "gradlew"


# We allow using several functionality with only ADB
def ensure_adb():
    global adb_path
    if "adb_path" not in globals():
        adb_path = shutil.which("adb")
        if not adb_path:
            error("Command 'adb' cannot be found in PATH")
        else:
            adb_path = Path(adb_path)


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


def set_archs(archs: set):
    triples = map(support_abis.get, archs)
    global build_abis
    build_abis = dict(zip(archs, triples))


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

    gradle_props = Path("app", "gradle.properties")
    if gradle_props.exists():
        for key, value in parse_props(gradle_props).items():
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
        archs = default_archs

    set_archs(archs)


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

    test_parser = subparsers.add_parser("test", help="build the test app")

    clean_parser = subparsers.add_parser("clean", help="cleanup")
    clean_parser.add_argument(
        "targets", nargs="*", help="native, cpp, rust, java, or empty to clean all"
    )

    ndk_parser = subparsers.add_parser("ndk", help="setup Magisk NDK")

    emu_parser = subparsers.add_parser("emulator", help="setup AVD for development")
    emu_parser.add_argument("apk", help="a Magisk APK to use", nargs="?")
    emu_parser.add_argument(
        "-b", "--build", action="store_true", help="build before patching"
    )

    avd_patch_parser = subparsers.add_parser(
        "avd_patch", help="patch AVD ramdisk.img or init_boot.img"
    )
    avd_patch_parser.add_argument("image", help="path to ramdisk.img or init_boot.img")
    avd_patch_parser.add_argument("output", help="output file name")
    avd_patch_parser.add_argument("--apk", help="a Magisk APK to use")
    avd_patch_parser.add_argument(
        "-b", "--build", action="store_true", help="build before patching"
    )

    cargo_parser = subparsers.add_parser(
        "cargo", help="call 'cargo' commands against the project"
    )
    cargo_parser.add_argument("commands", nargs=argparse.REMAINDER)

    clippy_parser = subparsers.add_parser("clippy", help="run clippy on Rust sources")

    rustup_parser = subparsers.add_parser("rustup", help="setup rustup wrapper")
    rustup_parser.add_argument(
        "wrapper_dir", help="path to setup rustup wrapper binaries"
    )

    gen_parser = subparsers.add_parser("gen", help="generate files for IDE")
    gen_parser.add_argument("--abi", default="arm64-v8a", help="target ABI to generate")

    # Set callbacks
    all_parser.set_defaults(func=build_all)
    native_parser.set_defaults(func=build_native)
    cargo_parser.set_defaults(func=cargo_cli)
    clippy_parser.set_defaults(func=clippy_cli)
    rustup_parser.set_defaults(func=setup_rustup)
    gen_parser.set_defaults(func=gen_ide)
    app_parser.set_defaults(func=build_app)
    stub_parser.set_defaults(func=build_stub)
    test_parser.set_defaults(func=build_test)
    emu_parser.set_defaults(func=setup_avd)
    avd_patch_parser.set_defaults(func=patch_avd_file)
    clean_parser.set_defaults(func=cleanup)
    ndk_parser.set_defaults(func=setup_ndk)

    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)

    return parser.parse_args()


def main():
    global args
    args = parse_args()
    load_config()
    vars(args)["force_out"] = False
    args.func()


if __name__ == "__main__":
    main()
