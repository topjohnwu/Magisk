use std::env;
use std::process::{Command, Stdio};

use home::cargo_home;

/********************************
 * Why do we need this wrapper?
 ********************************
 *
 * The command `rustup component list` does not work with custom toolchains:
 * > error: toolchain 'magisk' does not support components
 *
 * However, this command is used by several IDEs to determine available
 * components that can be used, such as clippy, rustfmt etc.
 * In this script, we simply redirect the output when using the nightly
 * channel if any `component` command failed.
*/

fn main() -> std::io::Result<()> {
    let rustup = cargo_home()?.join("bin").join("rustup");
    let argv: Vec<String> = env::args().skip(1).collect();

    if argv.iter().any(|s| s == "component") {
        let status = Command::new(&rustup)
            .args(&argv)
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status()?;
        if !status.success() {
            let mut cmd = Command::new(&rustup);
            cmd.arg("+nightly");
            if argv[0].starts_with('+') {
                cmd.args(argv.iter().skip(1));
            } else {
                cmd.args(&argv);
            }
            return cmd.status().map(|_| ());
        }
    }

    // Simply pass through
    Command::new(&rustup).args(argv.iter()).status().map(|_| ())
}
