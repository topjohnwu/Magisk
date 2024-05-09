#!/usr/bin/env python3
import subprocess
import sys
import os
from pathlib import Path

################################
# Why do we need this wrapper?
################################
#
# The command `rustup component list` does not work with custom toolchains:
# > error: toolchain 'magisk' does not support components
#
# However, this command is used by several IDEs to determine available
# components that can be used, such as clippy, rustfmt etc.
# In this script, we simply redirect the output when using the nightly
# channel if any `component` command failed.

if "CARGO_HOME" in os.environ:
    cargo_home = Path(os.environ["CARGO_HOME"])
else:
    cargo_home = Path.home() / ".cargo"

rustup = cargo_home / "bin" / "rustup"

if "component" in sys.argv:
    proc = subprocess.run(
        [rustup, *sys.argv[1:]],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    if proc.returncode != 0:
        # Remove any channel overrides
        if sys.argv[1].startswith("+"):
            sys.argv.pop(1)
        # Return the nightly channel results
        sys.exit(subprocess.run([rustup, "+nightly", *sys.argv[1:]]).returncode)

# Passthrough
sys.exit(subprocess.run([rustup, *sys.argv[1:]]).returncode)
